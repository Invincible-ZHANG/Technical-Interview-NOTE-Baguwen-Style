---
title: 一文告诉你区分DVI,NCP,CCP,QP问题
date: 2025-09-29
layout: note
excerpt: 我也被这个困惑很久，索性写一下自己的理解，告诉你怎么区分DVI,NCP,CCP,QP问题:)
---


## 一、QP（Quadratic Programming，二次规划）

### 定义

给定对称矩阵 $Q\in\mathbb{R}^{n\times n}$、向量 $c\in\mathbb{R}^n$，线性约束 $Ax\le b,\ Ex=d$。
二次规划：

$$
\min_{x\in\mathbb{R}^n}\ \tfrac12 x^\top Qx + c^\top x
\quad
\text{s.t.}\quad
Ax\le b,\ Ex=d.
$$

若 $Q\succeq 0$（半正定），这是**凸QP**，解与算法都很稳定。

#### 极简例（单变量、带区间约束）

$$
\min_{x}\ \tfrac12 (x-3)^2
\quad \text{s.t.}\quad 0\le x \le 2.
$$

**解法**：先看无约束极小值 $x^\star=3$，但违反约束；把它投影到区间 $[0,2]$ 上，得

$$
x^\star=\operatorname{proj}_{[0,2]}(3)=2.
$$

**要点**：很多小型接触子步都等价于“把某个向量投影到一个简单集合（区间/球/锥）上”，而**凸QP的KKT条件**正是这类“投影=互补”关系的另一种表达。

---

## 二、NCP（Nonlinear Complementarity Problem，非线性互补问题）

#### 定义

给定连续映射 $f:\mathbb{R}^n\to\mathbb{R}^n$，求 $x\in\mathbb{R}^n$ 使

$$
x\ge 0,\quad f(x)\ge 0,\quad x^\top f(x)=0.
$$

读作：“$x$ 与 $f(x)$ **互补**”，即每个分量 $x_i f_i(x)=0$。

#### 极简例（1D 单边接触的“玩具模型”）

设地面在 $y=0$，小球“受一个线性弹力”逼近地面。用一个标量互补近似地表述接触：

$$
\text{求 } \lambda\ge 0 \text{ 使 } g(\lambda)\ge 0,\ \lambda\, g(\lambda)=0,
\qquad g(\lambda)=a - b\lambda,
$$

其中 $g$ 是“间隙”（受 $\lambda$ 影响压缩），$a>0,\ b>0$ 为常数。

**求解**：两种情况

* 若 $\lambda=0$，则需 $g(0)=a\ge 0$ 成立；
* 若 $\lambda>0$，则需 $g(\lambda)=0\Rightarrow \lambda=a/b$。
  综合：$\lambda^\star=\max\{0,\ a/b\}$，且对应的 $g^\star=\max\{a-b\lambda,\,0\}=0$。

**要点**：间隙和支持力互补。

---

## 三、CCP（Cone Complementarity Problem，锥互补问题）

#### 定义

把 NCP 的“非负正交”推广到**一般闭凸锥 $K$**。
求 $x$ 使

$$
x\in K,\quad F(x)\in K^\*,\quad \langle x,\,F(x)\rangle=0,
$$

其中 $K^\*$ 为 $K$ 的对偶锥（所有与 $K$ 非负内积的向量集合）。
当 $K=\mathbb{R}^n_+$ 时就退化为 NCP。
在接触-摩擦里，常见的锥是**二阶锥（摩擦圆锥）**。

#### 物理例（库仑摩擦的切向圆盘）

在某接触点，法向冲量（或力）给定为 $\gamma_n\ge 0$，摩擦系数 $\mu$。
切向变量 $\gamma_T\in\mathbb{R}^2$ 必须满足圆盘约束

$$
\|\gamma_T\|\le \mu\,\gamma_n \quad (\text{二维切向；三维情形是圆锥/二阶锥}).
$$

**最大耗散原理**给出优化/互补的一阶条件：

$$
\gamma_T\in \mathcal{K}_T,\quad
v_T + y = 0,\ y\in N_{\mathcal{K}_T}(\gamma_T),
$$

等价于“**把 $-v_T$** 投影到圆盘 $\mathcal{K}_T=\{\|\gamma_T\|\le \mu\gamma_n\}$ 上”。
显式解（当 $v_T\ne 0$）：

$$
\gamma_T^\star = -\,\mu\,\gamma_n\ \frac{v_T}{\|v_T\|}.
$$

###### 数字例

给 $v_T=(3,-4)$，$\|v_T\|=5$；取 $\mu=0.5,\ \gamma_n=10$，则 $\mu\gamma_n=5$。
有

$$
\gamma_T^\star = -5\cdot \frac{(3,-4)}{5}=(-3,\,4),
\quad
\|\gamma_T^\star\|=5=\mu\gamma_n.
$$

解释：摩擦切向量**反向对齐**于切向速度，且恰在半径 $5$ 的圆盘边界上。

**要点**：这就是 **SOCCP（Second-Order Cone Complementarity Problem）** 的基本一格，工程上常用**投影算子**（一次算一次）或**加速投影迭代**（APGD/FISTA）来求解大规模问题。

---

## 四、DVI（Differential Variational Inequality，微分变分不等式）

#### 定义（动力学 + 互补/锥约束）

设状态 $x(t)$、输入/乘子 $u(t)$。DVI 以“微分规律 + 变分不等式”耦合表示系统：

$$
\dot x = f(x,t) + G^\top u,\qquad
u\in K,\quad F(x,t,u)\in K^\*,\quad \langle u,\ F(x,t,u)\rangle=0.
$$

它把**非穿透、摩擦、饱和/限幅**等“非光滑律”直接融到动态模型里，允许出现速度跳变（冲击）。

#### 教科书例（单刚体+地面接触）

* 状态：位置/速度 $(q,v)$；动力学：$\dot q=v,\ M\dot v = f(q,v,t) + J_n^\top \lambda_n$.
* 几何：间隙 $\phi(q)\ge 0$（地面为 0），接触力 $\lambda_n\ge 0$。
* 互补：$0\le \lambda_n \perp \phi(q)\ge 0$。
* 若含摩擦，再加切向锥条件（见上节 CCP）。

###### “从 DVI 到离散子问题”的最小例

用固定步长 $\Delta t$ 做时间步进，把力积分成**冲量** $\gamma$：

$$
M v^{k+1} = M v^{k} + \Delta t\,f^k + J_n^\top \gamma_n + J_T^\top \gamma_T.
$$

把 $v^{k+1}$ 代回到“法向相对速度/间隙预测”里，就得到**一步的互补/锥互补方程**（NCP/CCP/SOCCP）；这一步就是你在引擎里每帧要解的**代数子问题**。
**因此：DVI（连续） → 时间离散 → NCP/CCP（离散代数） → 数值求解（PGS/ADMM/APGD/牛顿/路径法）**。

---

## 五、四者关系 & 选型心法

* **DVI**：是**建模框架**（动态 + 互补/锥），最贴合接触/冲击/摩擦的物理。
* **NCP/CCP**：是**离散后**每步落地的**代数互补子问题**（CCP 更一般，能覆盖摩擦锥）。
* **QP**：很多小子步/近似可化为**凸QP**（或“投影”），也可从 KKT 把 QP ↔ 互补问题互相改写。
* **实现建议**：

  * 有“简单集合投影”（区间、球、二阶锥）时，优先用**投影迭代**（PGS/Projective Splitting/APGD），一步就是“算梯度 + 投影”。
  * 有强耦合或高精度需要时，考虑基于牛顿/半平滑牛顿/路径法的 **NCP/CCP 专用求解器**。
  * 子问题若自然就是“二次 + 线性约束”，可直接上 **QP**（OSQP、quadprog 等），或把它嵌作预条件/局部解算器。

---

## 六、再给一组“可手算”的迷你练习（巩固手感）

1. **QP 练习**：

$$
\min_x \ \tfrac12 x^\top\!\begin{bmatrix}2&0\\0&8\end{bmatrix}x
+\begin{bmatrix}-4\\-8\end{bmatrix}^\top x
\quad \text{s.t.}\quad x_1\ge 0,\ x_2\ge 0.
$$

先解无约束最优 $x=(2,1)$，已满足约束 → 即为最优。若把右边线性项改为 $(-1,-1)^\top$，再解一次并比较。

2. **NCP 练习**（两个接触点耦合的玩具）：
   找 $(x_1,x_2)\ge 0$ 使 $f(x)=Ax+b\ge 0$、$x^\top f(x)=0$，其中

$$
A=\begin{bmatrix}2 & -1\\ -1 & 2\end{bmatrix},\quad b=\begin{bmatrix}-1\\2\end{bmatrix}.
$$

枚举“活跃集”（哪个分量为 0、哪个解等式）即可解出闭式解。

3. **CCP/二阶锥投影**：
   给 $\mu=0.6,\ \gamma_n=5,\ v_T=(4,3)$。求 $\gamma_T^\star$。
   解：$\|v_T\|=5,\ \mu\gamma_n=3$ ⇒ $\gamma_T^\star=-3\cdot \frac{(4,3)}{5}=(-12/5,\ -9/5)$。

4. **DVI→一步离散**（单法向）：

$$
m(v^{k+1}-v^k)= -mg\Delta t + \gamma_n,\qquad
0\le \gamma_n \perp \phi^k + \Delta t\,v^{k+1}\ge 0.
$$

代入 $v^{k+1}$ 得单变量互补方程，设 $m=1,g=10,\Delta t=0.01,\ \phi^k=0$ 手算 $\gamma_n$ 与 $v^{k+1}$。




