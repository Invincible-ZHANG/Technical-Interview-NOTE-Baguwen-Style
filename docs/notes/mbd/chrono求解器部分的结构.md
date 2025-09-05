---
title: chrono求解器部分的结构
date: 2025-09-01
excerpt: "为了方便后续的建模，去学习一下chrono中关于求解器部分的结构。"
layout: note
---

## Simulation system


### ChSystem 与两种系统

* **ChSystemNSC**（Non-Smooth Contacts, DVI/硬接触）：接触约束走互补问题（LCP/VI），可用较大步长、效率高。
* **ChSystemSMC**（SMooth Contacts, 软接触/罚函数）：接触可变形，常与弹簧-阻尼模型、FEA耦合。

### Time steppers（时间积分器）

* **EULER\_IMPLICIT\_LINEARIZED**（默认）

  * 一阶、**无内迭代**、快；**支持 DVI(硬接触)**；FEA 给一阶精度；约束靠稳定化。
* **HHT**

  * 隐式、**有内迭代**、二阶、可调数值阻尼；**目前不能用于 DVI 硬接触**；适合 FEA/SMC；约束因内迭代收敛“闭合得更严”。
* **NEWMARK**

  * 与 HHT 类似；除“梯形法则”特参外整体是一阶；多用于 FEA/SMC。

> 经验：**DVI/硬接触 ⇒ EULER\_IMPLICIT\_LINEARIZED**；**软接触/FEA/需要二阶与阻尼 ⇒ HHT/NEWMARK**。

### Solvers（求解器）★重点

时间积分器每步都会调用“求解器”来求未知的加速度与反力。它往往是**计算热点**。


**推荐的迭代求解器与特性：**

* **PSOR**

  * 最常见入门选择；实现简单、收敛精度偏低，**质量比奇怪时易卡**；
  * **支持 DVI/硬接触**；
  * 适合小规模、精度要求不高的场景。
* **APGD**（加速投影梯度）

  * **收敛好**，高精度仿真常用；**支持 DVI/硬接触**；
  * 对大问题更稳健，建议优先于 PSOR。
* **BARZILAIBORWEIN**（BB） （后期是否考虑？）

  * 收敛也不错；**支持 DVI/硬接触**；
  * 与 APGD 相近，有时在大质量比下更鲁棒。
* **MINRES**

  * 适合 **FEA**；**当前不支持 DVI/硬接触**；
  * 配合对称稀疏线性系统，常与预条件配合使用。
* **ADMM + PardisoMKL**

  * 同时支持 **FEA 与 DVI**；
  * 需要内部线性解算器，最佳是 **PardisoMKL**（需单独模块），否则退化到 **ChSolverSparseQR**。


## chrono 求解器结构

![A P G D](images/APGD.png)



### ChSolverAPGD.cpp

 A)`SchurBvectorCompute(ChSystemDescriptor& sysd)`：装配 Schur 右端 `r`

目标：构造 **Schur 方程** `N * λ = b_schur` 的右端，其中

* `N = D' * (M^-1) * D`（约束 Schur 矩阵，别名 A、JM^-1J'）；
* `b_schur = -c + D' * (M^-1) * k`（含约束偏置 c、外力项 k 映射到约束空间）。
  代码里为了统一号，后续把目标函数写成 `f(λ) = 0.5 λ' N λ + r' λ`，梯度是 `N λ + r`，所以这里构造的 `r` 等于 `-b_schur`。你在后面会看到更新步 `y - t*(g + r)`，对应的梯度就是 `N*y + r`。

逐行解释：

1. 对每个变量做“解质量矩阵”，把 `q = M^-1 * k` 写回变量的 `State()`：

   ```cpp
   var->ComputeMassInverseTimesVector(var->State(), var->Force());
   ```

   这一步把“广义力 k”通过 `M^-1` 映射成速度增量模板 `q`（等会儿进入 `D' q`）。

2. 清空 `r`，然后遍历所有**激活的约束**，累加

   ```cpp
   r[s_i] = constraint->ComputeJacobianTimesState();
   ```

   这就是 `- D' * q`（命名和号可能与你直觉相反，但看后面的目标写法可知，最终得到的是我们想要的 `r`）。

3. 再取系统装配的 `b_i = -c = phi/h`（接触/约束的“偏置项”，例如恢复量/归一化渗透），加到 `r` 上：

   ```cpp
   sysd.BuildBiVector(tmp);  // tmp = b_i
   r += tmp;                 // r = -D' q + b_i
   ```

得到的这个 `r`，被用于后续所有梯度和目标计算；配合下面的写法，最终的目标是

* 目标：`f(λ) = 0.5 λ' N λ + r' λ`
* 梯度：`grad f(λ) = N λ + r`

---

 B) `Res4(sysd)`：全局最优性（KKT/VI）残差

作用：计算 **投影梯度映射** 的范数
`‖ λ - Proj_K( λ - gdiff * (N*λ + r) ) ‖ / gdiff`，其中 `Proj_K` 是把乘子投回可行集 K 的投影（包含等式/不等式/摩擦锥），`gdiff` 取一个很小的步长（这里固定为 `1/(nc^2)`，nc=约束数）。

流程：

1. `tmp = N * gammaNew`（这里 `gammaNew` 就是当前 λ）；
2. `tmp = gammaNew - gdiff * (tmp + r)`（负梯度迈一步）；
3. `sysd.ConstraintsProject(tmp)`（把 `tmp` 投回可行域 K：

   * 等式约束：保持等式（实现细节在 `ConstraintsProject` 内部）；
   * 单边/盒约束：逐分量 clamp；
   * 摩擦锥：按接触块把切向缩回半径 = μ \* λ\_n 的圆盘/圆锥）；
4. `tmp = (gammaNew - tmp) / gdiff`（这就是“梯度映射”向量）；
5. 返回 `tmp.norm()` 作为**全局一阶最优性残差**。
   当这个残差 → 0，就意味着 `λ = Proj_K( λ - t * grad f(λ) )` 成立，也就是 KKT/VI 的不动点条件成立 ⇒ 达到全局最优（在这类凸问题里）。

---
 C) `Solve(sysd)`：APGD 主流程（逐步讲清）

 C.0 预备与内存

* 打印激活约束与变量数（可选 `verbose`）。

* `Update_auxiliary()`：每个约束预计算本地辅助量（如 `Eq = M^-1 Cq'`、`g_i = Cq_i M^-1 Cq_i'` 等，供后续 Schur 乘/回写使用）。

* 为各种向量按 `nc`（激活约束计数）分配尺寸：`gamma, y, gammaNew, yNew, g, r, tmp, gamma_hat` 等。

  * `gamma`：当前乘子；
  * `y`：Nesterov 的外推点（动量点）；
  * `gammaNew`：投影后新点；
  * `gamma_hat`：到目前为止“最优”（残差最小）的那一个解的快照；
  * `g`：用来装 `N * y`；
  * `r`：上一步函数算好的右端；
  * `tmp`：工作区。

* 初始化控制量：`residual` 取个极大数；`Beta`、`obj1/obj2` 清零。

 C.1 右端装配（全局）

* 调 `SchurBvectorCompute(sysd)` 得到 `r`。
* 若 `nc == 0`（没约束），直接返回（也避免除零）。

 C.2 备份 `Minvk`（回写原始变量要用）

* `FromVariablesToVector(Minvk, true)`：把之前 `ComputeMassInverseTimesVector` 产生的 `(M^-1) * k` 拷到一个向量 `Minvk`。
* 这会在收敛后用于速度回写：`v = (M^-1)k + (M^-1)D*λ`。

 C.3 初值与动量初始化

* 若 `m_warm_start`：把上一步各约束的乘子增量进系统（等价于以旧解作为初值）。否则把所有乘子清零。
  然后 `sysd.FromConstraintsToVector(gamma)` 把（可能的）初值读入 `gamma`。

* `gamma_hat = 全 1`（工程上常用的“保底可行/非零”起点，后面只是用来估 L）；

* `y = gamma`（第一步外推点等于初值）；

* `theta = 1`（Nesterov 初始动量参数）。

 C.4 估 `L` 并设步长 `t = 1/L`

* 估计 Lipschitz 常数 `L`：
  做一次向量 `tmp = gamma - gamma_hat`，然后算 `yNew = N * tmp`，取 `L = ‖yNew‖ / ‖tmp‖`。
  这是“单步拉普拉斯估计”：近似 `‖N‖_2`（N 的谱范数），在工程上够用。
* 置步长 `t = 1/L`。

> 解释：对光滑凸目标 `f(λ) = 0.5 λ' N λ + r' λ`，梯度 `∇f(λ) = N λ + r` 的 Lipschitz 常数是 `‖N‖_2`。APGD/FGM 的稳定步长就是 `1/L`。

 C.5 迭代主循环（k = 0...max\_iter）

每一轮做三件事：**梯度 + 投影**、**回溯线搜索**、**Nesterov 动量 & 收敛管理**。

梯度 + 投影（全局耦合更新）

* `g = N * y`（用 `sysd.SchurComplementProduct` 乘一次，全局耦合）；
* `gammaNew = y - t * (g + r)`（沿负梯度迈一步。注意梯度写法是 `N*y + r`）；
* `sysd.ConstraintsProject(gammaNew)`（把 `gammaNew` 投到可行域 K：
  等式/单边/摩擦锥按约束类型做投影，这一步是**分块可分离**的，但对的是**整条 λ 向量**）。

回溯线搜索（保证“足够下降”的上界条件）

* 先计算

  * `obj1 = gammaNew' * (0.5 * (N*gammaNew) + r)`，也就是 `f(gammaNew)`；
  * `obj2 = y' * (0.5 * (N*y) + r) + (gammaNew - y)' * (g + 0.5 * L * (gammaNew - y))`。
    第二项是“Lipschitz 上界的二次模型”，来自经典的下降引理：
    `f(x) ≤ f(y) + ∇f(y)'(x − y) + 0.5 L ‖x − y‖^2`。
* 若 `obj1 >= obj2`，说明步长太大（L 估小了），就

  * 把 `L` 翻倍，`t = 1/L`；
  * 用新 `t` 重新做 `gammaNew = Proj( y − t * g )`；
  * 重新计算 `obj1/obj2`；
  * 循环直到满足 `obj1 < obj2`。
    这就是经典回溯（Armijo/Descent-lemma 型）保证每步“合规”。

 Nesterov 动量、残差、重启与 L 的轻微衰减

* 动量参数更新（标准 FGM 公式）：

  ```
  thetaNew = (-theta^2 + theta*sqrt(theta^2 + 4)) / 2
  Beta     = theta * (1 - theta) / (theta^2 + thetaNew)
  yNew     = gammaNew + Beta * (gammaNew - gamma)
  ```

  其中 `theta` 是“估计序列”参数；`Beta` 是动量系数。

* 计算 **全局最优性残差**：`res = Res4(sysd)`。

  * 若 `res < residual`（有史以来最小），就把 `gamma_hat = gammaNew` 存起来。
  * 若 `residual < m_tolerance`（达到容差），直接 `break`。

* **动量重启**（实战稳健性技巧）：
  如果 `g · (gammaNew − gamma) > 0`，说明“动量方向”和“局部陡降方向”有冲突，容易振荡，就

  ```
  yNew   = gammaNew
  thetaNew = 1
  ```

  这等价于“重启”为标准投影梯度（无动量）一步。

* **轻微减小 L**：`L = 0.9 * L`，再设 `t = 1/L`。
  这是一个“乐观”策略：如果回溯没触发，渐进地放宽 L，有助于加大步长；一旦过头，回溯会把 L 翻倍拉回来。

* 记录历史（可选），然后把本轮的

  ```
  theta = thetaNew
  gamma = gammaNew
  y     = yNew
  ```

  作为下一轮初值。

循环直到迭代次数用完或残差达标。

 C.6 写回乘子与原始变量（速度）

* 把最好的 `gamma_hat` 写回各约束：`sysd.FromVectorToConstraints(gamma_hat)`。
* 恢复 `v = (M^-1) * k`：`sysd.FromVectorToVariables(Minvk)`；
* 再叠加 `(M^-1) * D * λ`：遍历每个活跃约束，

  ```cpp
  constraint->IncrementState(constraint->GetLagrangeMultiplier());
  ```

  这一步利用了前面 `Update_auxiliary` 准备好的 `Eq = M^-1 Cq'` 等中间量，把乘子贡献加到变量速度里。
* 返回最终的 `residual`（最小残差）。



## 你关心的几个关键点，钉死它们

1. **“全局 vs 逐约束”**：

   * `SchurComplementProduct` 是对**整个** λ 向量做一次 `N*λ`（全局耦合）；
   * `ConstraintsProject` 是对集合 `K = 盒 × 等式 × 摩擦锥(分块)` 做**一次性**投影，虽然实现上“逐分量/按摩擦块”处理，但数学上是对整个 `λ` 的投影；
   * 残差 `Res4` 是**全局**的 KKT/VI 不动点残差（不是单约束的误差）。
     所以它解的是**全局（在当前系统描述符内）的凸二次问题**，不是“每个约束的小最优”。

2. **目标与梯度到底是什么（由代码反推）**：

   * 你看到 `gammaNew = y - t * (g + r)`，以及 `obj = x' (0.5 N x + r)`，所以
     目标函数是 `f(λ) = 0.5 * λ' N λ + r' λ`，
     梯度是 `∇f(λ) = N λ + r`。
   * 这与 `SchurBvectorCompute` 的产物一致：`r = -D' q + b_i`，在标准 NSC 推导里，这正对应 `-b_schur`。

3. **为什么残差是“全局最优性”**：

   * 最优性等价式：解 λ\* 满足 `λ* = Proj_K(λ* − t * ∇f(λ*))`（任意 t>0），也等价于 VI/KKT 条件；
   * `Res4` 计算的就是 `‖λ − Proj_K(λ − gdiff * ∇f(λ))‖/gdiff` 的范数 → 0，表示满足最优性；
   * 在这类凸问题（N 半正定、K 凸）里，“局部最优 = 全局最优”，因此这个残差达标就意味着全局最优达标。

4. **回溯线搜索在保什么**：

   * 用 Lipschitz 上界 `f(x) ≤ f(y) + ∇f(y)'(x−y) + 0.5 L ‖x−y‖^2` 做判据；
   * 若不满足就把 L 翻倍（步长 t 变小），直到满足；
   * 这保证了每步更新后目标不“犯规”，数值更稳。

