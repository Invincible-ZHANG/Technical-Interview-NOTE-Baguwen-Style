---
title: APGD阶段性总结（算法方向）
date: 2025-09-26
excerpt: "阶段性总结汇总，PPT草稿，演讲搞"
layout: note
---

## 2. NCP -> QP

在三维欧氏空间中描述刚体 $j$ 的位置与姿态的**广义坐标**分别为 $\mathbf{r}_j \in \mathbb{R}^3$ 与 $\boldsymbol{\varepsilon}_j \in \mathbb{R}^4$。前者给出刚体 $j$ 质心的绝对位置，后者为一组**欧拉参数（四元数）**，用于在全局参考系下表征刚体的取向。由 $n_b$ 个刚体组成的系统，其广义坐标集合写作

$$
\mathbf{q}=\begin{bmatrix}
\mathbf{r}_1^{\mathsf T},\ \boldsymbol{\varepsilon}_1^{\mathsf T},\ \ldots,\ \mathbf{r}_{n_b}^{\mathsf T},\ \boldsymbol{\varepsilon}_{n_b}^{\mathsf T}
\end{bmatrix}^{\mathsf T}\in\mathbb{R}^{7n_b},
$$

其时间导数为

$$
\dot{\mathbf{q}}=\begin{bmatrix}
\dot{\mathbf{r}}_1^{\mathsf T},\ \dot{\boldsymbol{\varepsilon}}_1^{\mathsf T},\ \ldots,\ \dot{\mathbf{r}}_{n_b}^{\mathsf T},\ \dot{\boldsymbol{\varepsilon}}_{n_b}^{\mathsf T}
\end{bmatrix}^{\mathsf T}\in\mathbb{R}^{7n_b}.
$$

为了建立牛顿–欧拉方程，本文不直接使用 $\dot{\mathbf{q}}$，而改用

$$
\mathbf{v}=\begin{bmatrix}
\dot{\mathbf{r}}_1^{\mathsf T},\ \boldsymbol{\omega}_1^{\mathsf T},\ \ldots,\ \dot{\mathbf{r}}_{n_b}^{\mathsf T},\ \boldsymbol{\omega}_{n_b}^{\mathsf T}
\end{bmatrix}^{\mathsf T}\in\mathbb{R}^{6n_b},
$$

其优点在于：（i）未知量更少；（ii）可得到**常数、对称且正定**的质量矩阵。

对每一个刚体 $B$，存在一个简单的**线性变换**，将其以**体固系**表达的角速度 $\bar{\boldsymbol{\omega}}_B$ 与欧拉参数的时间导数 $\dot{\boldsymbol{\varepsilon}}_B$ 相联系。具体为

$$
\bar{\boldsymbol{\omega}}_B = 2\,\mathbf{G}\!\left(\boldsymbol{\varepsilon}_B\right)\,\dot{\boldsymbol{\varepsilon}}_B,
$$

其中矩阵 $\mathbf{G}\in\mathbb{R}^{3\times4}$ 的各元素**线性依赖**于欧拉参数 $\boldsymbol{\varepsilon}_B$。

$$
\mathbf G(\boldsymbol\varepsilon)=
\begin{bmatrix}
-e_1 & e_0 & e_3 & -e_2\\
-e_2 & -e_3 & e_0 & e_1\\
-e_3 & e_2 & -e_1 & e_0
\end{bmatrix}.
$$   
（不同版本可能只差符号或行列顺序，本质等价。）

定义分块对角矩阵

$$
\mathbf{L}(\mathbf{q}) \equiv \mathrm{diag}\!\Big[
\mathbf{I}_{3\times3},\ \tfrac{1}{2}\mathbf{G}^{\mathsf T}(\boldsymbol{\varepsilon}_1),\ \ldots,\ 
\mathbf{I}_{3\times3},\ \tfrac{1}{2}\mathbf{G}^{\mathsf T}(\boldsymbol{\varepsilon}_{n_b})
\Big]\in\mathbb{R}^{7n_b\times6n_b},
$$

其中 $\mathbf{I}_{3\times3}$ 为单位阵，则有

$$
\dot{\mathbf{q}}=\mathbf{L}(\mathbf{q})\,\mathbf{v}.
$$

把“物理速度”转换成“参数速度”

**因为建模/求解用 $\mathbf v$ 更舒服，但积分与几何都离不开 $\dot{\mathbf q}$。**

所以做法是“混合坐标”：方程里用 $\mathbf v=[\dot{\mathbf r};\boldsymbol\omega]$ 去写牛顿–欧拉方程，解完后再用

$$
\dot{\mathbf q}=\mathbf L(\mathbf q)\,\mathbf v
$$

把它变回“参数速度”，以更新位姿、计算雅可比与约束速度。

---

### 2.1 建模要点（Modeling Aspects）

考虑图 1 中所示的两刚体 $A$ 与 $B$ 的接触。对物体 $A$，在接触点 $i$ 处取法向量 $\mathbf{n}_{i,A}$，其与该点的切平面正交，并指向**物体 $A$ 的外部**。再选取两条互相正交的单位向量 $\mathbf{u}_{i,A}$ 与 $\mathbf{w}_{i,A}$，与 $\mathbf{n}_{i,A}$ 一起在 $A$ 上构成右手坐标系，作为接触点 $i$ 的局部参考系。对物体 $B$ 亦按同样步骤定义，以 $\mathbf{n}_{i,B},\,\mathbf{u}_{i,B},\,\mathbf{w}_{i,B}\in\mathbb{R}^3$ 构成其局部系。

与接触 $i$ 相关联的**拉格朗日乘子** $\hat{\gamma}_i$ 被用来就**间隙（距离）**函数 $\Phi$ 在两物体 $A$ 与 $B$ 之间建立**互补性条件**：

$$
0 \le \hat{\gamma}_{i,n}\ \perp\ \Phi_i(\mathbf{q}) \ge 0 .
$$

当两物体**恰好接触**时，$\Phi_i(\mathbf{q})=0$；否则 $\Phi_i(\mathbf{q})>0$。若接触几何在接触处**光滑且凸**，则构造 $\Phi_i(\mathbf{q})$ 相对直接。对于**复杂且/或非凸**的几何，精确定义 $\Phi_i(\mathbf{q})$ 可能较为困难，这超出了本文讨论范围。
![图 1](image-1.png)

约定，$\alpha_i \equiv \alpha_i,A$，其中 $\alpha \in \{\mathbf n,\mathbf u,\mathbf w\}$。与接触点 $i$ 相关的接触力可分解为**法向分量**与**切向分量**。
法向分量：

$$
\mathbf F_{i,N}=\hat\gamma_{i,n}\,\mathbf n_i,
$$

切向分量：

$$
\mathbf F_{i,T}=\hat\gamma_{i,u}\,\mathbf u_i+\hat\gamma_{i,w}\,\mathbf w_i,
$$

其中乘子 $\hat\gamma_{i,n}\ge 0,\ \hat\gamma_{i,u},\ \hat\gamma_{i,w}$ 表示各方向上的力幅值。摩擦力假设满足**库仑干摩擦模型**可写为

> 圆锥约束（不等式）
> 
> 互补条件（粘—滑切换）
> 
> 方向对齐（反向做功）
$$
\begin{equation}
\sqrt{\hat\gamma_{i,u}^{\,2}+\hat\gamma_{i,w}^{\,2}}\le \mu_i\,\hat\gamma_{i,n}\qquad

\|\mathbf v_{i,T}\|\!
\left(\sqrt{\hat\gamma_{i,u}^{\,2}+\hat\gamma_{i,w}^{\,2}}-\mu_i\,\hat\gamma_{i,n}\right)=0,\qquad
\left\langle \mathbf F_{i,T},\,\mathbf v_{i,T}\right\rangle=-\|\mathbf F_{i,T}\|\,\|\mathbf v_{i,T}\|,
\end{equation}
$$

其中 $\mathbf v_{i,T}$ 为在接触点处刚体 $A$ 与 $B$ 的**相对切向速度**。以上方程是如下**优化问题**的一阶 Karush–Kuhn–Tucker（KKT）**最优性条件**，该优化问题在两个虚变量 $y,z\in\mathbb R$ 上定义：

$$
\begin{equation}
\big(\hat\gamma_{i,u},\hat\gamma_{i,w}\big)
=\operatorname*{arg\,min}_{\sqrt{y^{2}+z^{2}}\le \mu_i\,\hat\gamma_{i,n}}
\ \mathbf v_{i,T}^{\top}\big(y\,\mathbf u_i+z\,\mathbf w_i\big)
\end{equation}
$$

第 $i$ 个接触点的合力记为 $\mathbf F_i=\mathbf F_{i,N}+\mathbf F_{i,T}$，并满足
$\mathbf F_{i,T}=\hat\gamma_{i,n}\,\mathbf n_i+\hat\gamma_{i,u}\,\mathbf u_i+\hat\gamma_{i,w}\,\mathbf w_i$，
即 $\mathbf F_i\in\mathcal C_i$。其中 $\mathcal C_i$ 是一枚**圆锥**：其半角斜率为 $\tan^{-1}\mu_i$，顶点在接触点 $i$，轴线沿 $\mathbf n_i$ 指向外侧。形式化地，

$$
\mathcal C_i=\left\{\,\mathbf x=[y,z,t]^{\top}\in\mathbb R^3\ \big|\ y^2+z^2\le \mu_i^2\,t^2,\ t\ge 0\,\right\},
$$

并以接触点为锥尖、$\mathbf n_i$ 为锥轴方向。

下面展示：从公式（1）推导到公式（2）的过程








































ppt draft:


下面这三页内容，按“PPT一页=一句话主旨+关键公式”来写；末尾给一段中文演讲稿（2–3 分钟）。你可以把“公式区”直接粘进 PPT 的公式框（已用 Markdown+LaTeX）。

---

# Page 1：DVI 接触-摩擦的最小耗散与KKT条件（SOCCP）

**要点（放标题/要点区）**

* 非光滑动力学（DVI）把接触-摩擦写成二阶锥互补问题（SOCCP）。
* 法向互补 + 切向“最大耗散”投影 ⇒ 统一成一个变分不等式/锥约束。

**公式区（直接粘贴）**

法向互补（不穿透）
[
0 \le \hat\gamma_{i,n}\ \perp\ \Phi_i(q)\ge 0
]

库仑圆锥（切向乘子受法向支撑并被锥约束）
[
\sqrt{\hat\gamma_{i,u}^2+\hat\gamma_{i,w}^2};\le;\mu_i,\hat\gamma_{i,n}
]

最大耗散（切向沿相对切向速度的反方向，等价于在锥上做一次最小化）
[
(\hat\gamma_{i,u},\hat\gamma_{i,w})
=\arg\min_{\sqrt{y^2+z^2}\le \mu_i\hat\gamma_{i,n}}
; v_{i,T}^{!\top},(y,\mathbf u_i+z,\mathbf w_i)
\tag{1}
]

接触点合力（位于以 (\mathbf n_i) 为轴、坡度 (\mu_i) 的摩擦圆锥 (\mathcal T_i) 内）
[
\mathbf F_i=\hat\gamma_{i,n},\mathbf n_i+\hat\gamma_{i,u},\mathbf u_i+\hat\gamma_{i,w},\mathbf w_i
\in \mathcal T_i=
\Bigl{,[x,y,z]^{!\top}:\ \sqrt{y^2+z^2}\le \mu_i x,\Bigr}
]

> 讲法提示：把((1))理解成“把切向分量往速度的反方向投影到摩擦圆锥边界上”，就是“最大耗散原理”的离散版本。

---

# Page 2：系统方程（牛顿-欧拉 + 约束冲量）

**要点（放标题/要点区）**

* 广义速度为每刚体的 twist（线速度+角速度）。
* 外力 + 约束冲量的“生成子”叠加进入动力学方程。

**公式区（直接粘贴）**

状态-速度映射
[
\dot{\mathbf q}= \mathbf L(\mathbf q),\mathbf v
]

动力学（Stewart–Trinkle 形式）
[
\mathbf M,\dot{\mathbf v}
=\mathbf f(t,\mathbf q,\mathbf v)
+\sum_{i\in\mathcal A(\mathbf q,\delta)}
\Bigl(\hat\gamma_{i,n}\mathbf D_{i,n}
+\hat\gamma_{i,u}\mathbf D_{i,u}
+\hat\gamma_{i,w}\mathbf D_{i,w}\Bigr)
\tag{2}
]

其中，(\mathbf D_i=[\mathbf D_{i,n},\mathbf D_{i,u},\mathbf D_{i,w}]\in\mathbb R^{6n_b\times 3}) 是接触 i 的**切空间生成子**（把接触方向映到各刚体的广义力上）。对一对接触体 (A,B) 与接触点相对位置 (\mathbf s_{i,A},\mathbf s_{i,B})，在世界系单位向量 (\mathbf d\in{\mathbf n_i,\mathbf u_i,\mathbf w_i}) 下，块结构可写成（更易做PPT图示）：
[
\mathbf D_i^{!\top}(\mathbf d)=
\begin{bmatrix}
\vdots[2pt]
\underbrace{\begin{matrix}
\mathbf d\
\mathbf s_{i,A}\times \mathbf d
\end{matrix}}*{\text{作用在 }A}
[8pt]
\underbrace{\begin{matrix}
-\mathbf d\
-\mathbf s*{i,B}\times \mathbf d
\end{matrix}}_{\text{作用在 }B}
[2pt]
\vdots
\end{bmatrix}
\tag{3}
]

配合 Page 1 的锥约束/最小化条件，就得到一个标准的 **DVI→SOCCP** 问题在每个时间步上的离散形式。

---

# Page 3：参数化与变量选择（四元数 + twist）

**要点（放标题/要点区）**

* 广义坐标：每体位置 (\mathbf r_B) + 姿态四元数 (\boldsymbol\epsilon_B)。
* 选择 twist (\mathbf v=[\dot{\mathbf r}^\top,\ \boldsymbol\omega^\top]^\top) 作为速度变量，可得到常对称正定的质量矩阵 (\mathbf M) 与更小的状态维度。

**公式区（直接粘贴）**

广义坐标堆叠
[
\mathbf q=\bigl[\mathbf r_1^{!\top},\ \boldsymbol\epsilon_1^{!\top},\ \ldots,\
\mathbf r_{n_b}^{!\top},\ \boldsymbol\epsilon_{n_b}^{!\top}\bigr]^{!\top}\in\mathbb R^{7n_b}
]

广义速度（twist 堆叠）
[
\mathbf v=\bigl[\dot{\mathbf r}*1^{!\top},\ \boldsymbol\omega_1^{!\top},\ \ldots,\
\dot{\mathbf r}*{n_b}^{!\top},\ \boldsymbol\omega_{n_b}^{!\top}\bigr]^{!\top}\in\mathbb R^{6n_b}
]

四元数与角速度关系（每体 (B)）
[
\boldsymbol\omega_B = 2,\mathbf G(\boldsymbol\epsilon_B),\dot{\boldsymbol\epsilon}_B
\quad\Longrightarrow\quad
\dot{\boldsymbol\epsilon}_B = \tfrac12,\mathbf G(\boldsymbol\epsilon_B)^{!\top}\boldsymbol\omega_B
]

于是
[
\dot{\mathbf q}=\mathbf L(\mathbf q),\mathbf v,\qquad
\mathbf L(\mathbf q)=\mathrm{blkdiag}\bigl(\mathbf I_{3\times3},\ \tfrac12\mathbf G(\boldsymbol\epsilon_1)^{!\top},\ldots,
\mathbf I_{3\times3},\ \tfrac12\mathbf G(\boldsymbol\epsilon_{n_b})^{!\top}\bigr).
\tag{4}
]

---

## 演讲稿（中文，2–3 分钟）

各位好，这一页我们把 DVI（Differential Variational Inequality）框架下的接触与摩擦，压缩成一个“二阶锥互补问题”。直觉是：法向不穿透，对应“非负间隙-非负法向力-互补”；切向服从库仑摩擦，对应“切向合力落在以法向为轴的圆锥里”。**最大耗散原理**告诉我们——在所有满足摩擦锥的切向力里，真正发生的那个，会让功率 (v_T^\top F_T) 最小，也就是与相对切向速度尽可能“对着干”。把这件事写成式子，就是我们第一页的式(1)：在半径 (\mu \hat\gamma_n) 的圆盘上做一个很小的凸优化。

第二页我们回到全系统：广义速度我们选 twist（每个刚体的线速度和角速度），写成 (\dot q=L(q)v)。动力学里，除了常规外力，我们把每个接触的“冲量生成子” (\mathbf D_{i,\cdot}) 乘上对应的乘子 (\hat\gamma) 相加进来，就得到式(2)。(\mathbf D_i) 的结构很容易记：对 A 号刚体是 ([\mathbf d;\ s_A\times\mathbf d])，对 B 号刚体取相反号——这反映了动量守恒和力-偶矩的平衡。这样做的好处是，我们可以在同一套形式里，把双边约束、单边接触、以及摩擦都统一起来。

第三页我们解释为什么用四元数+twist。四元数避免奇异，twist 让质量矩阵 (\mathbf M) 是常对称正定的块结构，方便矩阵-向量乘和并行化。通过 (\omega=2G(\epsilon)\dot\epsilon) 的关系，我们得到 (\dot q=L(q)v) 的块对角映射式(4)，这让积分器和约束求解器都能在“速度层”工作，从而自然落在“DVI→SOCCP”的离散时间步求解流程上。

落地实现时，你可以把“每一帧的 (\hat\gamma)”看作在摩擦锥+盒约束上的一次**投影梯度**求解；矩阵部分只需要用 (\mathbf J\mathbf M^{-1}\mathbf J^\top) 的乘算子，不必显式装配整块矩阵，这就非常适合大规模并行（见你项目里的 APGD / matrix-free 设计思路）。接下来可以接入你的 APGD/FISTA 求解器与 Chrono/VEROSIM 的 Jacobian 生成器，实现端到端的 DVI 仿真管线。
