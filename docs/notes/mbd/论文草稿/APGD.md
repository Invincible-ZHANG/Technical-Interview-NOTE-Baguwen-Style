---
title: APGD部分草稿
date: 2025-07-28
layout: note
excerpt: 毕业论文APGD部分草稿草稿。:)
---




## Accelerated Projected Gradient Descent (APGD) 算法

### 1 算法背景

在非光滑动力学（Non-Smooth Dynamics）仿真中，刚体系统往往包含大量的不等式约束，如摩擦接触、单侧约束及驱动电机约束等。这些约束问题通常可转化为线性互补问题（Linear Complementarity Problem, LCP）或变分不等式问题（Variational Inequality, VI）。传统直接求解方法（如Dantzig算法）虽然精确，但在大规模问题中计算代价高且扩展性差。

投影梯度法（Projected Gradient Method, PGM）是一种广受关注的替代方案，但其收敛速度较慢，难以满足实时性要求。因此，加速投影梯度下降算法（Accelerated Projected Gradient Descent, APGD）被提出，广泛应用于非光滑动力学求解。

### 2 数学模型

APGD算法求解如下凸优化问题：

$$
\min_{\lambda \in \mathcal{K}} \quad f(\lambda) = \frac{1}{2} \lambda^T Z \lambda + \lambda^T d
$$

其中，\$\lambda\$ 是约束力（拉格朗日乘子），\$Z \in \mathbb{R}^{n \times n}\$ 是对称正定矩阵（Schur complement矩阵），\$d \in \mathbb{R}^{n}\$ 为右端向量，集合\$\mathcal{K}\$ 为约束投影集合，定义了约束的上下界。

刚体动力学中矩阵 \$Z\$ 典型的构造方式为：

$$
Z = C_q M^{-1} C_q^T + E
$$

其中，\$C\_q\$ 为约束雅可比矩阵，\$M\$ 为系统质量矩阵，\$E\$ 为约束力混合项（Constraint Force Mixing, CFM）。

### 3 算法流程

APGD采用Nesterov加速技巧提高收敛速度，具体迭代流程如下：

* 初始化：

  * \$\lambda^{(0)} = 0\$, \$y^{(0)} = \lambda^{(0)}\$, \$t^{(0)} = 1\$
  * 确定初始步长 \$L\$

* 第 \$k\$ 步迭代：

  1. 梯度计算：

     $$
     \nabla f(y^{(k)}) = Z y^{(k)} + d
     $$

  2. 梯度步长迭代及约束投影：

     $$
     \lambda^{(k+1)} = \Pi_{\mathcal{K}} \left(y^{(k)} - \frac{1}{L}\nabla f(y^{(k)})\right)
     $$

  3. 更新加速参数：

     $$
     t^{(k+1)} = \frac{1 + \sqrt{1 + 4(t^{(k)})^2}}{2}
     $$

  4. 估计下一迭代点：

     $$
     y^{(k+1)} = \lambda^{(k+1)} + \frac{t^{(k)} - 1}{t^{(k+1)}}(\lambda^{(k+1)} - \lambda^{(k)})
     $$

重复上述过程直至满足收敛准则。

### 4 约束投影操作

投影操作 \$\Pi\_{\mathcal{K}}(\cdot)\$ 针对不同类型约束定义为：

* **单边约束（Unilateral）**：

  $$
  \Pi_{[0, \infty)}(x) = \max(0, x)
  $$

* **有界约束（Motor约束）**：

  $$
  \Pi_{[l, h]}(x) = \min(\max(x, l), h)
  $$

* **摩擦锥约束（Coulomb friction cone）**：确保摩擦力不超过最大静摩擦力。

### 5 收敛性分析

APGD算法具有\$O(1/k^2)\$的加速收敛性，其中\$k\$为迭代次数。相比传统的PGM方法（\$O(1/k)\$），APGD显著提高了收敛速度。实践中，初始步长\$L\$及约束边界的选择对收敛影响显著，需合理设置。

### 6 参数调控策略

为确保算法稳定性和收敛性，需进行如下参数调控：

* **动态步长（Step Size）**：使用回溯线搜索动态调整。
* **Warm-Start**：利用前一步求解的\$\lambda\$作为迭代初始点。
* **迭代停止准则**：设定误差阈值\$\varepsilon\$：

  $$
  \| \lambda^{(k+1)} - \lambda^{(k)} \| \le \varepsilon
  $$

### 7 实际应用

APGD算法成功应用于大规模接触摩擦系统、机器人关节、电机驱动器约束等非光滑动力学问题。特别地，Motor约束通常设置为较大有限上下界，如$\[-10^6, 10^6]\$，以保证算法收敛。

### 8 关键优化策略

* **Warm Start**：显著减少迭代次数。
* **摩擦约束处理**：确保摩擦力投影满足物理约束。
* **自适应步长**：结合Armijo条件回溯调整步长。
* **非单调策略**：允许短暂的目标函数上升以提升鲁棒性。

### 9 数值表现与参数影响

实际仿真显示，APGD算法迭代次数和步长对收敛性能影响明显。过高迭代次数可能提高精度但增加计算成本甚至引发数值不稳定现象。应根据具体动力学场景选取合适参数，平衡计算性能与精度。

### 10 小结与展望

APGD算法通过加速策略显著提升求解效率，尤其适用于大规模并行计算。未来研究可关注参数自动调整及不同投影方法对算法性能的进一步影响。

### 参考文献

1. Anitescu, M., et al. (1997). Time-stepping for rigid body dynamics. *Computer Methods in Applied Mechanics and Engineering*.
2. Tasora, A., et al. (2015). Chrono: An Open Source Multi-physics Dynamics Engine. *International Conference on High Performance Computing in Science and Engineering*.
3. Project Chrono. Chrono Engine Documentation. [http://projectchrono.org](http://projectchrono.org).
4. Serban, R. et al. (2014). ChIterativeSolverVI.cpp, Project Chrono. [https://github.com/projectchrono/chrono](https://github.com/projectchrono/chrono).
