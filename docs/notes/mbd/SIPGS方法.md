---
title: 求解器中SIPGS方法
date: 2025-08-25
excerpt: "SIPGS/逐次冲量。"
layout: note
---





***基于 Moreau/Anitescu 时间步进的 NSC/CCP 建模**，用 **PGS（Gauss–Seidel）式就地更新**来解接触/摩擦与关节约束。*


>*(Moreau 指让·雅克·莫罗（Jean-Jacques Moreau），非光滑力学/凸分析的大佬；Anitescu 指 Mihai Anitescu，他和合作者（Potra 等）把刚体接触+摩擦的时间步进离散做成了现在工程里常用的一整套“非光滑时间步进（event-capturing time stepping）”方案。两条线都不做“事件检测+解析冲量”，而是在固定步长里直接求解一次互补/锥互补问题，把接触、摩擦、碰撞冲量统一到一步里解决。)*


# 一、整体流程（doTimeStep）

`doTimeStep(newTime, delta_t)` 里干这几件事：

1. **收集簇内刚体与约束**

   * 给每个刚体一个连续索引 `clusterIndex`；
   * 遍历每个刚体携带的 `RBDConstraintResource`，筛掉禁用或不在本簇的，加入 `myConstraintResources`；
   * 把接触 `RBDContact` 另外汇入 `allHandledContactsInCluster`（并且只收一次）。

2. **计算“初始速度” allRigidBodyTwist**（外力推进一步）

   * 对每个**非固定**刚体：

     $$
     v \leftarrow v + \Delta t\,(f/m),\qquad \omega \leftarrow \omega + \Delta t\,I^{-1}\tau
     $$

     存到 `allRigidBodyTwist[i]`；固定体直接置零。
   * `bodyToIndex` 记录 `RBDRigidBody* → 连续下标` 的映射，后面查表快。

3. **装配雅可比与右端项**（约束准备 + 热启动）

   * 建一个稀疏雅可比 `J` 和右端 `constraintsRightSide`；
   * **关节**：`prepareJointConstraint<T, m>()`（此处未贴出实现），把各类关节的 $J$、bias 等加入系统，同时做**warm-start**（把旧拉格朗日乘子/冲量作用到 `allRigidBodyTwist`）。
   * **接触**：对每个 `RBDContact` 调 `prepareContactConstraint()`（下面详解），构出该接触的局部数据，并做 warm-start。

4. **PGS/Sequential Impulse 迭代**

   * 外层最多 `solverIterations=10000`；
   * 每次 sweep：先解**关节**（等式约束），再解**接触**（互补/摩擦约束）；
   * 用 `totalImpulseNorm` 统计本轮所有约束的冲量增量范数，若 `< epsilon(=1e-1)` 则提前停止。

5. **把速度写回刚体**、**更新关节约束力**

   * `body->setTwist(allRigidBodyTwist[i])`；
   * `updateJointConstraintForce(...)` 根据最后的冲量给出显示/调试用的约束力。

6. **若本簇没有任何约束**

   * 直接调用每个刚体的 `evolveStateUnconstrained(delta_t)`（只按外力推进），然后 `resetForceAndTorque()`。

---

# 二、关键数据结构：ContactConstraintData

每个接触一个小包，核心是“**12 维块 + 有效质量**”：

* `Jn/Jt1/Jt2`：该接触的**法向/两切向**雅可比**行向量**，长度 12（前 6 作用 body0，后 6 作用 body1）。
* `MinvJn/MinvJt*`：把上面 12×1 向量左乘局部 $M^{-1}$（12×12 块对角），得到**将“单位冲量”回散到两刚体 6D 速度**的向量（求解时高频使用）。
* `normalMass/tangentMass*`：**有效质量**标量：

  $$
  m_{\text{eff}}=\frac{1}{J\,M^{-1}J^\top}
  $$

  用来把“相对速度 + 偏置”转换为冲量增量。
* `bias*`：右端/稳定化项（例如 Baumgarte、restitution）。
* `lambdaLow/High*`：该分量的投影上下界（法向 $[0,+\infty)$，切向通常 $[-\mu\lambda_n,+\mu\lambda_n]$），实现的是**盒近似摩擦**。
* `constraintOffset`：该接触在“全局约束向量”里的首行偏移（用于和其它模块对齐）。

---

# 三、接触准备：prepareContactConstraint()

关键步骤逐条看（你代码里写得很清楚）：

1. **把接触约束加入系统**

   * `contact->addComplementaryConstraintsToSystem(...)`：在 `J` 中追加 1 或 3 行（无摩擦=1，只法向；带摩擦=3，多两条切向），同时写入 `lambdaLow/High`、`constraintsRightSide` 和 `frictionNormalIndices`；`currentRow` 向前推进。

2. **构造局部 $M^{-1}$（12×12 块对角）**

   * 前 3 维是平动质量逆 `m_inv`，中 3 维是 `I0_inv`，后 6 同理给 body1；这只是**接触-局部的小矩阵**，不用全局 $M^{-1}$。

3. **抽取 12 维雅可比行向量**

   * `extractJacobianRowToVector(...)` 用 `body0Offset/body1Offset` 把稀疏行拷到 12 维数组里：

     ```
     [ 6维(body0) | 6维(body1) ]
     ```

4. **计算 Minv \* J^T 与有效质量**

   * `MinvJn[i] = Σ_j Minv(i,j) * Jn[j]`（切向类似）；
   * 有效质量：

     $$
     \text{normalMass}=\frac{1}{\sum_i J_n[i]\cdot (M^{-1}J_n^\top)[i]}
     $$

     切向同理；如果分母为 0 就设 0 以避免 NaN。

5. **记录 bias 与边界**

   * `biasNormal = constraintsRightSide[row]` 等；
   * `lambdaLow/High*` 从全局下/上界读取，切向的上下界一般是由 `μ * λ_n` 生成（这一步在 `addComplementaryConstraintsToSystem()` 已经处理）。

6. **Warm-start（热启动）**

   * 确保 `contact->accumulatedImpulse` 的长度与分量数一致，必要时扩容；
   * 用旧冲量沿 `MinvJ*` 把**速度增量**加回 `allRigidBodyTwist`：

     $$
     \Delta v_{A} \mathrel{+}= (M^{-1}J^\top)\,\lambda_{\text{old}},\quad
     \Delta v_{B} \mathrel{+}= (M^{-1}J^\top)\,\lambda_{\text{old}}
     $$
   * 这让迭代从“较好的初值”开始。

---

# 四、接触求解：solveContactConstraint()

这就是 **Sequential Impulse/PGS** 的“一步”：

1. **法向分量**

   * 计算相对速度投影：

     $$
     \dot C_n = J_n\,v_A + J_n^{(B)}\,v_B
     $$
   * 冲量增量（沿有效质量）：

     $$
     \Delta\lambda_n = -\,m_{\text{eff},n}\,(\dot C_n + \text{bias}_n)
     $$
   * **投影/夹逼**到 $[\lambda_{\min},\lambda_{\max}]$（非穿透=下界 0）；
   * 把 $\Delta\lambda_n$ 通过 `MinvJn` 回散到 `twistA/twistB`（就地改速度）；
   * 记到 `deltaImpulse[0]`，用于统计范数。

2. **切向分量 t1/t2**（若存在且有效质量>0）

   * 同样算 $\Delta\lambda_{t}$，再**逐分量 clamp** 到 $[\lambda_{\min}^{t},\lambda_{\max}^{t}]$；
   * 通过 `MinvJt*` 回散速度，累加到 `deltaImpulse[1/2]`。

3. **累计范数 & 写回接触的累计冲量**

   * `totalImpulseNorm += deltaImpulse.length()`；
   * `contact->addAccumulatedImpulse(deltaImpulse)` 更新热启动缓存。

> 由于是 **Gauss–Seidel** 风格，“立刻回散速度”，后续约束能感知本次更新 —— 这就是“Sequential”的含义，也导致它**天然顺序**、并行要做多色/分岛才行。

---

# 五、关节求解（等式约束）

你用模板 `solveJointConstraint<T, m>()` 做法与接触类似：

* 每个关节 m 条约束，持有自己的 $J$、`MinvJ^T`、`bias`、有效质量或近似；
* 计算 $\dot C$、沿有效质量更新拉格朗日冲量/速度，再**无投影**（等式约束通常上下界是 $(-\infty,+\infty)$ 或通过稳定化使残差→0）。

---

# 六、停止准则与参数

* `solverIterations = 10000` 的硬上限；
* **早停**阈值 `epsilon = 1e-1`：一轮 sweep 的 `totalImpulseNorm` 小于此值就 break。
  这比很多“严格残差”宽松得多，属于工程上“够用就行”的设定（对实时仿真友好）。

---

# 七、写回与“无约束”分支

* 有约束时，把 `allRigidBodyTwist` 写回刚体；并调用 `updateJointConstraintForce(...)` 供调试/渲染使用；
* 没约束时，直接调用 `evolveStateUnconstrained(delta_t)`（仅外力推进）并清力矩。

---

# 八、你代码里值得注意/可改进的点

* **摩擦模型**：当前切向是逐分量 clamp ⇒ **盒/金字塔近似**。如需真圆锥，要把两切向组成 2D 向量再做“半径 $\mu\lambda_n$ 的圆盘投影”。
* **12×12 `Minv` 构造**：现在为每个接触都显式建了一个 12×12。可以直接用**体级别 `m_inv` 与 `I_inv` 的 6×6 作用**避免建矩阵（按分块乘，减少 cache 压力）。
* **停止准则**：`totalImpulseNorm<1e-1` 偏松。若要与其它解法（如 APGD）**公平对比**，建议加一个**自然残差**或 KKT 残差的统一阈值。
* **并行化**：这一版是顺序 GS。要并行可做**多色接触图**或**分岛**；不过在 SIPGS 上“并行效率不如 APGD”的经验比较常见。
* **Warm-start 边界**：切向分量长度变化时你做了兼容（旧 1 维扩到 3 维时保留法向），这个细节很好，别丢。
* **稀疏 J 的抽取**：`extractJacobianRowToVector(...)` 假设每行最多两个 6×6 块（两个刚体），与你的建模吻合；如果后续有三体/铰链跨多个体的特殊行，要格外小心索引。

---

如果你接下来想把 **APGD** 也接进来做公平对比，这套数据路径几乎都能复用：

* 用三步乘 $u=J^\top x,\ v=M^{-1}u,\ g=Jv-b$ 做**矩阵-自由**的梯度；
* 投影沿用你现在的**盒投影**；
* 对角预条件可直接用你这里的**有效质量**；
* 停止准则统一到同一残差即可。
  这样就能在“同一建模、同一容差”下看谁更快、更稳。
