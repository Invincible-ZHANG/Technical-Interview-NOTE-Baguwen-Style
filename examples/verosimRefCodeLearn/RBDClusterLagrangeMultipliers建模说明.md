# RBDClusterLagrangeMultipliers 建模思路

## 1. 模块定位
`RBDClusterLagrangeMultipliers` 是 VSLib 动力学框架中负责 **单个刚体簇** 的约束组装与求解模块。它将当前时间步内集群中所有刚体、关节、接触等约束汇总成一个线性互补问题（LCP），并调用选定的 LCP 求解器（Dantzig、GS、APGD）求出约束冲量。

## 2. 状态与变量
- **状态向量**：集群内 `n` 个刚体的广义速度 `v`，长度为 `6n`（线速度 3 + 角速度 3）。
- **外力向量**：`f_ext`，按刚体顺序堆叠的受力/受矩。
- **约束变量**：拉格朗日乘子 `λ`，其大小等于雅可比矩阵 `J` 的行数（等式约束 + 互补约束）。
- **界限**：`lambdaLow` / `lambdaHigh`，分别描述每条约束允许的下界和上界，用于表达非穿透、摩擦等单边条件。

## 3. 约束建模
1. **收集资源**：遍历簇内所有刚体，筛选出属于该簇的约束资源（跳过禁用、跨簇资源）。
2. **等式约束**：关节、锁定等约束通过 `addEqualityConstraintsToSystem` 写入 `J` 与右端项 `constraintsRightSide`，并累计误差做均方根统计。
3. **互补约束**：限位、接触等单边约束通过 `addComplementaryConstraintsToSystem` 写入系统，同时维护摩擦索引 `frictionNormalIndices` 以便后续将摩擦变量与法向变量配对。
4. **接触摩擦**：对于接触资源，还会计算额外的摩擦行数，在 `numberFrictionConstraints` 中记录，便于求解器区分法向与摩擦 DOF。

## 4. 求解流程
1. **构建质量相关项**：使用 `multiplyMinvImplJT` 与 `multiplyMinvImplVector` 直接在稀疏结构上计算 `M⁻¹ Jᵀ`、`J M⁻¹ Jᵀ` 及 `M⁻¹ dt f_ext`，避免显式矩阵。
2. **添加 CFM**：先对每条约束调用 `addConstraintForceMixing`，再根据场景的全局 `GlobalCFM` 调整系统矩阵对角线，改善条件数。
3. **配置求解器**：根据场景配置选择 LCP 求解器，并设置
   - 系统矩阵 `matA = J M⁻¹ Jᵀ`
   - 右端项 `-b = -[ J (v_old + M⁻¹ dt f_ext) + constraintsRightSide ]`
   - 上下界、摩擦索引、摩擦附加项等。
4. **求解与自救**：若求解失败，会逐步增大 `GlobalCFM` 并重试；仍失败时退化为零解以保证仿真继续。
5. **回写结果**：将 λ 回代得到新速度 `v_new = M⁻¹ Jᵀ λ + M⁻¹ dt f_ext`，更新刚体 twist，并记录调试快照。

## 5. 调试与日志
- **性能计时**：大量 `PERFORMANCESUITE_TIC/TOC` 包裹关键阶段，便于分析瓶颈。
- **约束历史**：若启用 `RBDClusterLagrangeDebugObserver`，会对每帧存档 J、A、约束界限及求解结果。
- **接触日志**：`ensureLogsOpen` 负责将接触点坐标与数量输出到 `logs/contact_points.dat`、`logs/contact_count.dat`，用于后处理观测。

## 6. 适用场景
该建模方式特别适合：
- 刚体数量中等（几十到上百）且约束关系复杂的簇；
- 需要严格遵守关节/接触条件或对约束误差有量化需求的仿真；
- 需要快速切换不同 LCP 求解器策略以在“速度 vs. 稳定性”间权衡的研发环境。

