---
title: Verosim并行化设计
date: 2025-11-18
excerpt: "先在 Verosim 里搭一个“Chrono 风格的解算接口 + SchurProduct 抽象”，但仍然用你现在的密集矩阵 A 做参考；
然后再一步步换成矩阵-free，再加并行。"
layout: note
---



先把结论说在前面：
Chrono 的 Multicore 框架确实是 **矩阵-free 的 Schur 补操作 + 并行 data manager + 迭代投影解算器**，你要移植到 Verosim/RBD 里，最理性的做法不是“一股脑照搬”，而是：

> **先在 Verosim 里搭一个“Chrono 风格的解算接口 + SchurProduct 抽象”，但仍然用你现在的密集矩阵 A 做参考；
> 然后再一步步换成矩阵-free，再加并行。**

下面我给你一个比较“工程可落地”的里程碑方案，每一步都能单独编译、单独跑测试。

---

## 0. 先统一脑内架构图：Chrono ↔ Verosim 对应关系

### Chrono Multicore 这边

关键几块你已经有代码：

* **ChMulticoreDataManager**：所有数据的 SOA 容器（刚体状态、约束、稀疏矩阵 `D`, `D_T`, `M_inv`, `M_invD`, `Nschur` 等）
* **ChIterativeSolverMulticoreNSC**：

  * 组装 `D, E, R, N` 等矩阵/向量（`ComputeD/ComputeE/ComputeR/ComputeN`）
  * 设置 `SchurProductFull`, `SchurProductBilateral`, `ProjectFull` functor
  * 调用 `solver->Solve(SchurProductFull, ProjectFull, ...)` 来做迭代解算
* **ChSchurProduct / ChSchurProductBilateral**：

  * 实现 **矩阵-free 的 Schur 补算子**：

    * 当 `compute_N == false` 时：
      [
      y = D^T ,(M^{-1}D,x) + E,x
      ]
      `$y = D^T (M^{-1} D x) + E x$`
    * 只在设置上要求时才用预先显式构造好的 `Nschur * x`
* **ChSolverMulticore + 各种子类 (APGD, CG, GS, Jacobi, MINRES...)**

  * 统一接口：

    ```cpp
    uint Solve(ChSchurProduct& SchurProduct,
               ChProjectConstraints& Project,
               uint max_iter, uint size,
               const DynamicVector<real>& b,
               DynamicVector<real>& x);
    ```

所以很明确：**Multicore 确实是 matrix-free 的**，核心就是 `ChSchurProduct::operator()` 这玩意，所有迭代解算器只需要 “给我一个算子 A(x) + 约束投影就行”。

---

### Verosim / RBD 这边现状

* 约束装配：`RBDClusterLagrangeMultipliers::doTimeStep`

  * 构造 `RBMJacobeanMatrix J`，密集化成 `VSM::MatrixNxM jMatrix`（只是做奇异性诊断用途）
  * 通过 `multiplyMinvImplJT` 得到 `M_inv_JT`，再 `J.multiplySymmetricFast(matA, MInv_JT)` 得到
    [
    A = J M^{-1} J^T
    ]
    `$A = J M^{-1} J^T$`
  * 再加 CFM / 辅助摩擦约束，形成最终的 `matA`
* LCP 求解：

  * 通过 `myScene->constraintSolverType()` 选择 Dantzig / GS / 你的 `RBDLcpAPGD` 等解算器
  * 解出 `lambda` 后，用 `MInv_JT * lambda + MInv_dt_fext` 得到 `v_new`，最后写回刚体状态

你已经实现了一个 **密集矩阵版 APGD**：`RBDLcpAPGD`，内部持有 pad 对齐的 `A` 数组 + `b` + 上下界 + 摩擦块索引等等。

---

## 总体迁移策略（大纲）

我们目标是：**保留 Verosim 的 Cluster & 约束装配风格，只在“求解这一步”换成 Chrono 风格：SchurProduct + Project + matrix-free + 并行。**

一个合理的分阶段路线：

1. **接口对齐（仍然用密集 A）**

   * 在 RBD 里仿造 `ChSolverMulticore` 定义一个统一求解接口 + `RBDSchurProduct`/`RBDProjectConstraints` 抽象，但内部还是直接调你现有的 `RBDLcpAPGD(A,b)`。
   * 好处：先把“上层调用风格”改成 Chrono 这种 functor 结构，不动数学和数值行为。

2. **在 RBD 内部实现一个 “伪 SchurProduct”**（用 dense A 实现）

   * 写一个 `RBDSchurProductDense`：内部存一个 `VSM::MatrixNxM` 或 double* A，operator()(x,y) 做 `y = A*x`。
   * 你的 “multicore 风格 APGD” 直接用这个 SchurProduct；数值上对齐当前 RBDLcpAPGD。

3. **把 APGD 算法改成真正的 functor 版本**

   * 参考 `ChSolverMulticoreAPGDREF`，写一个 `RBDSolverAPGD_MF`：

     * 不再持有 A 矩阵，而是只持有：

       * `RBDSchurProduct& Schur`
       * `RBDProjectConstraints& Proj`
       * `b`、`gamma`、临时向量等
     * 梯度用 `Schur(gamma, tmp)` 计算，投影用 `Proj(gamma)`。
   * 这一步完成后，你就有了：**“接口 = Chrono，多核的数学结构 = 你的 APGD”，但还在密集矩阵世界里。**

4. **引入真正的矩阵-free SchurProduct（仍然单线程）**

   * 利用 `RBDClusterLagrangeMultipliers` 里已经有的 `multiplyMinvImplJT` / `multiplyMinvImplVector`：

     * 你有稀疏的 `RBMJacobeanMatrix J` + 当前 cluster bodies + 质量信息
   * 新建一个类 `RBDSchurProductMatrixFree`，构造时传入：

     * 对应 cluster 的 `RBMJacobeanMatrix& J`
     * bodies 列表引用
     * 预存 `M_inv_JT` 或直接在 `operator()` 里：

       1. 临时 `tmp = M_invD * x`，用 `multiplyMinvImplJT` 或改个版本 `JTMinv`
       2. 再 `J.multiplySymmetricVec(tmp, y)` 或直接 `J^T ( tmp )` 组合出 SchurProduct：
          [
          y = J M^{-1} J^T x + E x
          ]
          `$y = J M^{-1} J^T x + E x$`
   * 先不考虑稀疏格式转换（比如压成 blaze 的 `CompressedMatrix`），完全用现有 RBMJacobean 的存储结构做 matrix-free 运算。

5. **把 RBDClusterLagrangeMultipliers 的求解路径切换到“新接口 + matrix-free APGD”**

   * 在 `doTimeStep()` 里，原来是：

     1. 构造 `J`，密集化成 `matA`
     2. new 一个 `RBDLcpAPGD(size, nub, maxIters)`
     3. `setValuesInMatrix(matA)`/`setLowVector`/.../`solve()`
   * 改成：

     1. 构造 `J`（这一段不变）
     2. 构造 `RBDSchurProductMatrixFree Schur(J, bodies, ...)`
     3. 构造 `RBDProjectConstraintsFromBounds`，从 `lambdaLow / lambdaHigh / frictionIndex / addFriction` 生成投影逻辑（其实你 `RBDLcpAPGD::projectBounds/projectFriction` 的代码可以直接搬）
     4. 构造 `RBDSolverAPGD_MF solver; solver.Solve(Schur, Project, maxIter, size, b, lambda);`
   * **关键点**：在这一步你可以同时保留旧路径，用一个 `if (use_multicore_style)` 开关做 A/B 对比（lambda、v_new、残差对比）。

6. **并行化热点：SchurProduct & 投影**

等到 matrix-free 路线在单线程下跑得对、数值上跟密集版对齐之后，再加 OpenMP：

* 并行区域 1：`RBDSchurProductMatrixFree::operator()`：

  * J 的每一行/每个 contact block 都可以独立计算 contribution；
  * 参考 Chrono 在 `ChSchurProduct::operator()` 里用 blaze 的并行 sparse 乘法策略。
* 并行区域 2：投影 `Project( gamma )`：

  * 你的 `projectBounds` 本身就是纯逐元素 clamp，非常容易 `#pragma omp parallel for`。
  * `projectFriction` 中每个 friction 块之间也独立，可以一个外层 for 并行，注意块内要避免 data race（每个块只在一个线程内处理）。

因为你说“只考虑我自己求解器的加速”，完全可以只在 APGD 路径上做这些，Dantzig/GS 继续用旧代码，当作 fallback。

---

## 按“可执行任务”拆成里程碑

我给你一个更“todo 风”的分解，你可以几天 / 一周完成一个小块：

### 里程碑 1：建立 RBD 版 Multicore 解算接口（仍然用密集 A）

**目标**：

* 在 RBD 代码里新增三类东西：

  1. `class RBDISchurProduct { virtual void operator()(const VSM::VectorN& x, VSM::VectorN& y) = 0; };`
  2. `class RBDIProjectConstraints { virtual void operator()(VSM::VectorN& x) = 0; };`
  3. `class RBDIterativeSolverBase { virtual bool Solve(RBDISchurProduct&, RBDIProjectConstraints&, ... ) = 0; };`

**实现建议**：

* 写一个 `RBDSchurProductDense`：持有 `VSM::MatrixNxM A` / double* A，operator 做 `y = A * x`。
* 写一个 `RBDProjectFromBounds`：内部持有 `myLo/myHi/frictionIndices/addFriction`，operator 调用你当前 `RBDLcpAPGD::projectBounds/projectFriction` 的逻辑。
* 写一个 `RBDSolverAPGD_DenseAdapter`：Solve() 内部其实只是构造一个临时 `RBDLcpAPGD`，然后调用它，并把结果写回 `x`。

**改动点**：

* `RBDClusterLagrangeMultipliers::doTimeStep` 里，在 `myScene->constraintSolverType() == CST_LAGRANGEAPGD` 分支中，改成调用这个新接口，同时保留原有 `RBDLcpAPGD` 路线做 debug 对比。

**测试**：

* 选一个你现在已经能稳定跑的场景（例如箱子落在地面+几个关节），比较：

  * 旧 APGD vs 新 Adapter 的 lambda、v_new、位置演化曲线。
  * 数值几乎一致（浮点误差级别差异）。

---

### 里程碑 2：实现 functor 风格的 APGD（但还是靠 dense SchurProduct）

**目标**：

* 写一个新的类：`RBDSolverAPGD_Functor`，大体仿照 `ChSolverMulticoreAPGDREF` 的接口/实现：

  * Solve 中不再访问 A/b/lo/hi，全部通过：

    * `Schur(x, Ax)` 得到 `A x`
    * `Project(x)` 实现约束投影
    * `b` 从调用者传进来

**实现建议（和你现有 RBDLcpAPGD 映射）**：

* 你现在的 `RBDLcpAPGD::solve()` 里大部分逻辑可以直接搬，只需把“求 A*yk - b”替换成调用 Schur：
  [
  g = A y_k - b
  ]
  `$g = A y_k - b$`
  改成：

  ```cpp
  Schur(yk, tmp);
  g[i] = tmp[i] - b[i];
  ```
* 投影步 `xnew = Proj(yk - t*g)` 也同理，直接调用 `Project(xnew)`。
* 迭代控制（theta, r4, history best）可以照抄现在的。

**测试**：

* 用 `RBDSchurProductDense + RBDProjectFromBounds + RBDSolverAPGD_Functor` 和原先 `RBDLcpAPGD` 做残差/解对比。
* 这一步通过之后，你的求解器就已经是“Chrono 形状+你自己实现”的状态了。

---

### 里程碑 3：在 RBD 中做真正的 Matrix-Free SchurProduct（单线程）

**目标**：

* 实现 `RBDSchurProductMatrixFree`，使用现有的 `RBMJacobeanMatrix J` + `multiplyMinvImplJT / multiplyMinvImplVector`，不再构造 dense `A`。

**实现思路**（伪代码）：

```cpp
class RBDSchurProductMatrixFree : public RBDISchurProduct {
public:
    RBDSchurProductMatrixFree(const RBMJacobeanMatrix& J,
                              const RBDRigidBodyPtrSet& bodies,
                              const VSM::VectorNDynamic& E_diag_or_matrix, ...);

    void operator()(const VSM::VectorN& x, VSM::VectorN& y) override {
        // 1) tmp = J^T * x   （用 RBMJacobeanMatrix 的数据结构）
        // 2) z   = M^{-1} * tmp  （用 multiplyMinvImplVector）
        // 3) y   = J * z         （再走一遍 J 的 rows）
        // 4) y  += E * x         （如果有 CFM/compliance）
    }
};
```

Chrono 那边是 `output = D_T * (M_invD * x) + E * x`，我们只是用 RBD 自己的 Jacobian & Minv 代替 blaze 稀疏矩阵而已。

**测试策略**：

* 同一个 cluster，同一个 J 和 bodies：

  1. 构造 dense `A = J M^{-1} J^T + E`（用当前路径），
  2. 用 `RBDSchurProductDense` 和 `RBDSchurProductMatrixFree` 各算一次 `y = A x`，对比误差。
* 误差 OK 后，再让 APGD_Functor 切换到 MatrixFree 的 SchurProduct，看解的差别。

---

### 里程碑 4：把 Cluster 的 APGD 路线切到 Matrix-Free

**目标**：

* 在 `RBDClusterLagrangeMultipliers::doTimeStep` 中，APGD 分支从：

```cpp
myLcp = new RBDLcpAPGD(matA.rows(), numberEqualityConstraints, ...);
// ...
myLcp->setValuesInMatrix(matA);
myLcp->setLowVector(lambdaLow);
...
myLcp->solve();
```

变成：

```cpp
RBDSchurProductMatrixFree schur(J, getRigidBodies(), ..., /* E / CFM 信息 */);
RBDProjectFromBounds proj(lambdaLow, lambdaHigh, frictionIndices, addFriction);

RBDSolverAPGD_Functor solver(numberEqualityConstraints, maxIters, tol);
solver.Solve(schur, proj, b, lambda);
```

**保留 debug 开关**：

* 例如 `myScene->getUseMatrixFreeSolver()`：

  * `false`：走原来 `RBDLcpAPGD(A,b)`
  * `true`：走新接口
* 在一段时间内维持双路径方便你对比、查 bug。

---

### 里程碑 5：SchurProduct & Project 的并行化

你只关心“自己求解器”的加速，所以最值得并行的就是：

* SchurProduct: `y = J M^{-1} J^T x + E x`
* 投影: `Project(gamma)`

这俩都满足“很规整的并行循环 + 几乎没有复杂依赖”。

**示例方向**：

* SchurProduct：

  * 最直接方式：按 row 分块，`#pragma omp parallel for` 遍历 J 的行，每个线程局部 accumulate 对应 y[i]。
  * 也可以像 Chrono 一样：预先构造 M_invD（多线程遍历 J，每个 entry 计算一次 M_inv * D），然后 `D_T * (M_invD * x)` 用稀疏矩阵乘法的多线程实现。这一步可以留到以后慢慢做。
* Project：

  * `projectBounds`：

    ```cpp
    #pragma omp parallel for
    for (int i = 0; i < n; ++i)
        v[i] = std::min(std::max(v[i], myLo[i]), myHi[i]);
    ```
  * `projectFriction`：外层 while over blocks 可以改为“先扫描生成 block 列表，再并行遍历 block 列表”。

---

## 顺便回答你之前的问题：Multicore 用到 matrix-free 吗？

非常明确：**用到，而且是默认路径**。

* 在 `ChSchurProduct::operator()` 里，只有当 `settings.solver.compute_N == true` 才会直接用 `Nschur * x`，否则走：

  [
  y = D_T ,(M^{-1}D,x) + E x
  ]
  `$y = D_T (M^{-1} D x) + E x$`

* 而 `ChIterativeSolverMulticoreNSC` 里调用 APGD/CG/GS 之类解算器时，只传进去 `ChSchurProduct` functor，solver 根本不知道有 `Nschur` 这种东西。

你现在在 RBD 里已经有 **J + M_inv 的稀疏结构 + 高效的 multiplyMinvImpl**，所以可以不必构造完整矩阵，直接学这套模式。

---

## 总结一下这套方案的味道

* 第 1–2 步：**只是接口层重构**，不改变数学结果，让 RBD 变成“Chrono 形状”。
* 第 3–4 步：**引入 matrix-free SchurProduct**，但算法还是你自己的 APGD。
* 第 5 步：**在最关键的两个算子上加并行**，真正开始吃多核红利。

每一步你都可以：

* 用老 APGD 作为 “ground truth”，
* 对比 lambda / v_new / 残差曲线，
* 再测性能，逐渐放大场景规模。

这样你不会陷入“一口吃掉整个 Chrono multicore”的深坑，而是稳步把 Verosim 的求解器长成一只多核怪兽。



## Step 1

不加 .cpp，不改 APGD 算法逻辑、不碰 SOA。

把 Chrono 风格的接口骨架 搭出来，但一切实现先还是用你现有的 dense matA / lambdaLow / lambdaHigh。

在 RBD 里增加一套统一接口：SchurProduct + Project + Solver，全部 header-only，方便后面逐步接 APGD 和 matrix-free。
* 只新建 一个 文件：RBDSolverMulticoreInterface.h
* 不动 RBDSolverAPGD.cpp、RBDClusterLagrangeMultipliers.cpp 的核心逻辑
* 先不写 adapter、不改 cluster，只保证工程能编译通过
* 后面 step 2 再把 APGD 改成 functor 风格 / 挂到 cluster 上


1. 新增一个头文件（唯一新文件）：RBDSolverMulticoreInterface.h

