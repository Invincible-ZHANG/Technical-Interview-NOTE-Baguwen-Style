---
title: APGD部分草稿
date: 2025-07-28
layout: note
excerpt: 毕业论文APGD部分草稿草稿。:)
---


## Accelerated Projected Gradient Descent (APGD) 算法

### 1 算法背景

在非光滑动力学（Non-Smooth Dynamics）仿真中，刚体系统中往往包含大量的不等式约束，例如摩擦接触、单侧约束以及驱动电机约束等。这些约束问题通常可被转化为线性互补问题（Linear Complementarity Problem, LCP）或变分不等式问题（Variational Inequality, VI）。传统的直接求解方法（如Dantzig算法）虽然能够准确地一次性求解，但在大规模问题中存在计算代价高、扩展性差等缺陷。

为了解决上述问题，投影梯度法（Projected Gradient Method, PGM）得到了广泛的关注。但传统的投影梯度法收敛速度较慢，难以满足实时性要求。为此，Accelerated Projected Gradient Descent (APGD)，即加速投影梯度下降算法被提出并应用于非光滑动力学求解领域。


首先基于LCP建模去完成APGD求解器算法的实现，然后在去进行CCP的大规模建模，从而提高大规模问题的计算速度


### 2 数学模型

APGD算法用于求解以下凸优化问题：

$$
\min_{\lambda \in \mathcal{K}} \quad f(\lambda) = \frac{1}{2} \lambda^T Z \lambda + \lambda^T d
$$

其中，\$\lambda\$ 是约束力（拉格朗日乘子），\$Z \in \mathbb{R}^{n \times n}\$ 是对称正定矩阵（Schur complement矩阵），\$d \in \mathbb{R}^{n}\$ 为右端项向量，集合\$\mathcal{K}\$ 为约束投影集合，定义了约束的上下界或投影区域。

在刚体动力学中，矩阵 \$Z\$ 的典型构造方式为：

$$
Z = C_q M^{-1} C_q^T + E
$$

其中，\$C\_q\$ 为约束的雅可比矩阵，\$M\$ 是系统质量矩阵，\$E\$ 是约束力混合项（Constraint Force Mixing, CFM）。

### 3 APGD算法流程

APGD采用了Nesterov的加速技巧来提高收敛速度，具体的迭代流程如下：

* 初始化：

  * \$\lambda^{(0)} = 0\$, \$y^{(0)} = \lambda^{(0)}\$, \$t^{(0)} = 1\$
  * 确定合适的初始步长 \$L\$

* 第 \$k\$ 步迭代：

  1. 计算梯度：

     $$
     \nabla f(y^{(k)}) = Z y^{(k)} + d
     $$

  2. 进行梯度步长迭代，并投影到约束集合 \$\mathcal{K}\$ 上：

     $$
     \lambda^{(k+1)} = \Pi_{\mathcal{K}} \left(y^{(k)} - \frac{1}{L}\nabla f(y^{(k)})\right)
     $$

  3. 更新加速参数：

     $$
     t^{(k+1)} = \frac{1 + \sqrt{1 + 4(t^{(k)})^2}}{2}
     $$

  4. 计算下一次的估计点：

     $$
     y^{(k+1)} = \lambda^{(k+1)} + \frac{t^{(k)} - 1}{t^{(k+1)}}(\lambda^{(k+1)} - \lambda^{(k)})
     $$

上述过程重复进行，直至满足收敛准则。

### 4 约束投影操作

投影操作 \$\Pi\_{\mathcal{K}}(\cdot)\$ 针对不同类型约束定义如下：

* **单边约束（Unilateral）**：

  $$
  \Pi_{[0, \infty)}(x) = \max(0, x)
  $$

* **有界约束（如Motor约束）**：

  $$
  \Pi_{[l, h]}(x) = \min(\max(x, l), h)
  $$

* **摩擦锥约束（Coulomb friction cone）**：

  对二维或三维摩擦约束进行特殊投影处理，确保摩擦力不超过最大静摩擦力。

### 5 APGD算法的收敛性分析

APGD的核心优势在于其具有 \$O(1/k^2)\$ 的加速收敛性，其中 \$k\$ 为迭代次数。相比传统的Projected Gradient Method（PGM）为 \$O(1/k)\$ 的收敛速度，APGD显著提高了算法的效率，特别适用于实时动力学仿真中对精度和速度兼顾的需求。

然而，APGD的收敛依赖于初始步长 \$L\$ 和约束的边界设置。实践中，步长选取过大可能导致振荡，步长选取过小则可能导致收敛速度慢。

### 6 参数调控策略

为了保障APGD的稳定性和收敛性，通常需要进行以下参数调控：

* **步长（Step Size）**：采用回溯线搜索（backtracking）动态调整步长。
* **Warm-Start**：利用前一步求解的 \$\lambda\$ 值作为下一步迭代的初始点。
* **迭代停止准则**：设置误差阈值 \$\varepsilon\$，一旦满足：

  $$
  \| \lambda^{(k+1)} - \lambda^{(k)} \| \le \varepsilon
  $$

  则停止迭代。

### 7 APGD在非光滑动力学中的实际应用

APGD算法被成功应用于非光滑动力学求解器中，包括大规模接触和摩擦系统、机器人关节驱动、电机驱动器约束（Motor Constraints）等问题。通过约束矩阵构建与投影操作的精心设计，APGD算法能够高效地处理复杂动力学问题并实现良好的扩展性。

具体实现中，需注意Motor类约束的特殊处理。由于理论上的Motor约束为无界区间，APGD难以直接处理，因此实际应用中往往设置为较大的有限上下界（如$\[-10^6, 10^6]\$）以保证算法的收敛性。

### 8 小结

综上所述，APGD算法以其高效的加速收敛特性，适合于大规模非光滑动力学系统的求解。通过精细的参数调控和约束建模策略，该算法在实际仿真系统中展现出良好的精度和计算性能平衡，成为求解复杂非光滑动力学系统的重要算法之一。



## APGD算法的理论背景与实现细节

本节详细介绍用于求解非光滑动力学问题的APGD算法（Accelerated Projected Gradient Descent），并给出其具体的数学描述、实现细节与参数分析。

### APGD算法的数学描述

给定一个凸集约束优化问题：

$$
\min_{x \in C} f(x)
$$

其中\$C\$是凸集，函数\$f(x)\$是连续可微的凸函数。APGD算法通过对梯度下降法的迭代过程加入动量项，显著提高了收敛速度。其迭代过程如下：

初始化参数\$x^{(0)} \in C, t\_0=1, y^{(0)}=x^{(0)}\$，对于每次迭代\$k \geq 0\$，进行以下步骤：

$$
\begin{aligned}
x^{(k+1)} &= \text{proj}_C \left(y^{(k)} - \eta \nabla f(y^{(k)})\right) \\
t_{k+1} &= \frac{1 + \sqrt{1 + 4 t_k^2}}{2} \\
y^{(k+1)} &= x^{(k+1)} + \frac{t_k - 1}{t_{k+1}} \left(x^{(k+1)} - x^{(k)}\right)
\end{aligned}
$$

其中，\$\text{proj}\_C(\cdot)\$表示向量投影到凸集\$C\$上，\$\eta\$表示步长，通常可通过回溯线搜索（backtracking line search）确定。

### Chrono框架中的APGD实现

Project Chrono框架通过定义系统描述子（System Descriptor），将非光滑动力学问题形式化为以下线性系统：

$$
\begin{bmatrix}
M & -C_q^T \\
C_q & E
\end{bmatrix}
\begin{bmatrix}
q \\
\lambda
\end{bmatrix}
-
\begin{bmatrix}
f \\
b
\end{bmatrix}
=
\begin{bmatrix}
0 \\
c
\end{bmatrix}
$$

此处，\$q\$表示广义速度，\$\lambda\$表示约束反作用力（Lagrange乘子），\$M\$是质量矩阵，\$C\_q\$是约束的雅克比矩阵，\$f\$为外力项，\$b\$和\$c\$为约束相关已知向量，矩阵\$E\$通常表示用于接触约束的正则化项或阻尼项。

在Chrono的实际代码实现中，如文件`ChIterativeSolverVI.cpp`所示，APGD算法的核心迭代过程使用了以下参数：

* 最大迭代次数 `max_iterations`
* 收敛容忍度 `tolerance`
* 超松弛因子 \$\omega\$
* 摩擦项尖锐系数 `sharpness_lambda`

其中，\$\omega\$通常取值在0.2至1.0之间，通过调节松弛参数能够提高算法的稳定性和收敛速度；而尖锐系数则用于精细控制摩擦接触条件的处理。

### 收敛性与参数分析

实际仿真发现，APGD算法的收敛性能受到迭代次数及步长控制的显著影响。当迭代次数增加时，解的精度明显提高。然而，过大的迭代次数可能导致计算成本上升，甚至引发数值不稳定性，表现为仿真中刚体弹飞或解不稳定的现象。这表明，在实际应用中，应根据问题规模与具体动力学场景选取适当的参数组合，以实现精度与性能的平衡。

### 结论

综上所述，APGD算法通过引入动量项显著提高了求解效率，尤其适合大规模并行计算场景。未来的研究可关注如何有效自动调整算法参数，并探索不同投影方法对APGD性能的影响。

### 参考文献

1. Anitescu, M., Potra, F. A., & Stewart, D. E. (1997). Time-stepping for three-dimensional rigid body dynamics. *Computer Methods in Applied Mechanics and Engineering*, 177(3-4), 183-197.
2. Tasora, A., Anitescu, M., Mazhar, H., Heyn, T., & Negrut, D. (2015). Chrono: An Open Source Multi-physics Dynamics Engine. *International Conference on High Performance Computing in Science and Engineering*, Springer, Cham.
3. Project Chrono. Chrono Engine Documentation. [http://projectchrono.org](http://projectchrono.org).
4. Serban, R. et al. (2014). `ChIterativeSolverVI.cpp`, Project Chrono Source Code. [https://github.com/projectchrono/chrono](https://github.com/projectchrono/chrono).
