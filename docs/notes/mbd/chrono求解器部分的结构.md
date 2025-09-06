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




## ChIterativeSolverVI

ChIterativeSolverVI 是 面向 VI（Variational Inequality，变分不等式）/互补问题 的迭代求解器基类。它站在 Chrono 的求解器栈中间位置：

* 顶层接口：ChSolver

* VI 专用接口：ChSolverVI（声明了问题形式与类别）

* 你看到的类：ChIterativeSolverVI（提供迭代法通用参数、迭代历史记录、归档等）

* 具体算法派生类（例如基于 P(S)OR、投影梯度、Nesterov/FISTA 等）从它继承并实现 Solve() 内部的迭代循环。


类注释写了 VI 的标准块结构（把约束统一装进大矩阵/向量）：

$$
\begin{bmatrix} M & -C_q^{\top}\\ C_q & -E\end{bmatrix}
\begin{bmatrix} q\\ \lambda \end{bmatrix}
-
\begin{bmatrix} f\\ b \end{bmatrix}
=
\begin{bmatrix} 0\\ c \end{bmatrix},\quad 
\lambda \in Y,\; C\in N_Y
$$

* `M` 是质量（或广义质量）矩阵；
* `C_q` 是约束 Jacobian；
* `E` 用于 CFM/合规项等；
* `q` 是状态量的增量（速度/加速度/冲量依具体 formulation）；
* `\lambda` 是拉格朗日乘子（接触法向力、摩擦力等）；
* `f, b` 是右端项（外力、稳定化项/恢复速度等）；
* `c` 是约束残差。

此外，为了让整体矩阵 **Z 对称**，注释里说明了一个“把 $\lambda$ 翻符号”的等价写法（把第二块行写成 `| Cq  E | |-l|`），这样许多对称正定/半正定的技巧可以用得更顺手。这些公式与分类（Linear/LCP/CCP）都在 `ChSolverVI.h` 的文档块中明示。

**三种特例的集合 $Y$：**

* 线性问题：全部 $Y_i=\mathbb{R}$（纯等式/双边约束，无互补条件）。
* LCP：全部 $Y_i=\mathbb{R}_+$，并满足 $\lambda\ge 0,\; c\ge 0,\; \lambda^\top c = 0$。
* CCP：$Y_i$ 是**摩擦圆锥**（切向分量被投影到半径 $\mu \lambda_n$ 的圆盘/圆锥上）。

> 这些分类与注释里的“`- case LCP/CCP`”对得上。



 成员与可调参数（直说怎么用）

* `m_max_iterations`（父类里）：最大迭代次数（默认 50）。你可以改它。改完会顺带**重设历史记录数组大小**（见下）。
* `m_tolerance`（父类里）：停止准则所用阈值；具体含义由派生类解释（最大约束违背、投影梯度范数……）。
* `m_omega`：**过松弛因子** $\omega$（PSOR/Jacobi 一类会用到）。注释建议：Jacobi 取 \~0.2，其他迭代法可到 1.0。设置时强制 $>0$。
* `m_shlambda`：**锐化因子**（Mangasarian 风格 LCP 投影里会出现；0.8–1.0，越低越稳/慢，越高越快/可能震荡）。设置时强制 $>0$。
* `m_iterations`：**上次 Solve 实际跑了多少步**（派生类里需要更新它）。
* `record_violation_history`：是否记录每步的“最大约束违背量”和“$\Delta\lambda$ 的最大变化量”。用
  `SetRecordViolation(true)` 开启后，`SetMaxIterations()` 会自动把两条历史向量 `violation_history` / `dlambda_history` 调整到合适长度。

---

# 关键方法（派生类一定会用到的）

**1) `SetMaxIterations(int)`**
除了设置上限，还会 `resize` 两个历史数组，使其能按迭代步保存轨迹。

**2) `SetRecordViolation(bool)`**
开关历史记录，并调用一次 `SetMaxIterations(m_max_iterations)` 来保证容量匹配。

**3) `AtIterationEnd(double mmaxviolation, double mdeltalambda, unsigned iternum)`**
这是**最重要的“迭代尾钩子”**：**所有迭代法的派生类都应在每一步末尾调用它**来把当前步的“最大约束违背量”和“$\Delta\lambda$ 最大变化”记录下来（如果 `record_violation_history=true`）。超界访问会断言。

**4) `SolveRequiresMatrix() const`**
这里返回 `true`，表示 **Solve 阶段需要可用的系统矩阵**（至少要能做 SpMV）。Chrono 的基类 `ChIterativeSolver` 也提供了调试辅助：

* `WriteMatrices(sysd)` 会装配稀疏 Z 并用 \*\*系统提供的 `SystemProduct`（矩阵-向量乘）\*\*再重建一份，输出到 `Z1.dat/Z2.dat` 对比；
* `CheckSolution(sysd, x)` 用“全矩阵”和“SPMV”两种方式计算残差范数，方便核对。

**5) 归档**
`ArchiveOut/ArchiveIn` 把 `m_max_iterations / m_warm_start / m_tolerance / m_omega / m_shlambda` 等成员序列化，便于保存/恢复配置。

---

# 派生类应该长啥样（最小骨架）

派生类（比如一个投影高斯–赛德尔/投影梯度的实现）通常会：

1. 在 `Solve()` 前，确保 `ChSystemDescriptor` 已经把大系统（$Z, d$）装配好；
2. 进行迭代，每一轮：

   * 做一次矩阵-向量乘、投影/更新 $\lambda$、评估违背量；
   * 如果启用了历史记录，**调用** `AtIterationEnd(max_violation, max_dlambda, it)`；
   * 如达到 `m_tolerance` 就停；并把 `m_iterations = it+1`。
3. 返回 `true/false` 由你定义的“收敛判据”决定（Chrono 中不同算法 GetError 的语义略有差异，类注释也提醒了这点）。

> 样例中的历史记录与矩阵检查函数在 `ChIterativeSolver.cpp` / `ChIterativeSolverVI.cpp` 里都能对上。

---

# 与系统数据结构如何配合

* 变量与力向量由 `ChVariables` 抽象（每个变量块会告诉你如何做 $M^{-1}x$、装配到全局等），**并不强制显式存 `M`**，以便矩阵-自由算法或稀疏装配。`ChVariables` 提供了 `State()`（局部 $q_b$）和 `Force()`（局部 $f_b$）的引用。
* `ChSystemDescriptor` 负责把所有 `Variables` 和 `Constraints` 组合出系统 $Zx=d$，然后迭代器在 `Solve()` 里不断做 SpMV 与投影（或显式矩阵操作，视实现而定）。
* 由于 `SolveRequiresMatrix()==true`，这个基类假定你的迭代过程需要至少能做“矩阵-向量运算”。如果你的算法是**严格矩阵-自由**的（像你自己实现的 APGD-MF），则会在**另一个基类**或自定义路径下实现，不必继承这里这一条约束（参考你项目中的 `RBDSolverAPGD_MF` 和 `RBDChronoNSC` 的矩阵自由施密特乘实现）。

---

# 参数调优一览（经验值）

* **Omega（过松弛）**：Jacobi/PSOR 一类从 0.2 起试，逐步加大到 1.0 左右；过大易震荡。
* **SharpnessLambda（锐化）**：0.8–1.0；越小保守，越大更快但可能抖。
* **MaxIterations**：场景稀疏、良态时几十步内应收敛；难问题（强摩擦、多体啮合）可适当提高，但更建议改模型/预条件。
* **Tolerance**：依据你派生类定义的误差度量（例如最大违背或投影梯度范数）去设，不同算法的“1e-4”含义不同，这点类注释也特别提醒了。

---

# 常见踩坑

* **忘了调用 `AtIterationEnd`**：你会发现 `GetViolationHistory()` 和 `GetDeltalambdaHistory()` 一直是零或者脏值。
* **把 `SetRecordViolation(true)` 和 `SetMaxIterations()` 顺序弄乱**：虽然类内部做了自洽处理（开关时会再 `resize`），但你最好在设置迭代上限后立刻开启记录，这样一目了然。
* **误解 `SolveRequiresMatrix()`**：这里返回 true，不代表必须“显式装配稀疏矩阵”，而是说**需要系统提供 SpMV**（SystemProduct），调试函数里也提供了这两条路径的对比输出来帮你排查装配/乘法是否一致。

---

# 小结与延伸

* `ChIterativeSolverVI` 为 **VI/互补问题** 的迭代法提供了**统一的外壳**：参数（$\omega$、锐化、阈值、迭代上限）、历史记录钩子、序列化、以及“需要矩阵/SpMV”的约定。派生类只管专心把**每一步**写扎实，记得在回合末用 `AtIterationEnd` 把指标喂给基类。
* 调试时，配合 `ChIterativeSolver::WriteMatrices/CheckSolution` 很好用，能快速定位是装配问题还是算法本体问题。

如果你接下来在做 **APGD/PGS/PSOR** 的派生实现，我可以给你一个“**Solve() 骨架模板**”（含误差度量、历史记录、早停策略），或者把它接到你当前的 NSC/CCP 管线里和 `RBDChronoNSC` 的矩阵自由版本对齐做 A/B 测试（同一停止准则、同一容差），这样能更科学地比较收敛行为与性能。
