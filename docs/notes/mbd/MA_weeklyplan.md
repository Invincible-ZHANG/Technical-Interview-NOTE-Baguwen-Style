---
title: MA周工作记录
date: 2025-07-09
layout: default
excerpt: 关于毕业设计每周任务同步以及在实现过程中的想法与设计思路，方便之后的追溯和毕业论文的编写。:)
---



*Tips：持续完善文档，记录每一次优化与踩坑体会，有助于后续论文撰写与项目迭代！*

# 毕业设计每周工作总结

## 本周任务（07.06.2025 – 07.11.2025）
- [ ] VEROSIM 实现 CCP / QP 建模  
- [x] 实现并集成 APGD 求解器

---

### 本周思考
1. **优先级**：先把 APGD 插到现有的 LCP 建模流程里，实现对接；  
2. **可配置性**：把 APGD 加入求解器枚举与下拉选项，方便在运行时自由切换；  
3. **后续准备**：完成 QP 建模（基于 Chrono 风格），为并行化、性能优化打基础。

---

 ### Data : 2025-07-10

---

### APGD 求解器集成步骤

#### 1. 源码接入
- 将 `RBDSolverAPGD.h/cpp` 添加到项目中，确保它们参与编译；  
- 在 `RBDClusterLagrangeMultipliers.cpp` 中加入：
  ```cpp
  #include "RBDSolverAPGD.h"
 - 执行全量 Rebuild，确保无编译或链接错误。

#### 2. 扩展求解器枚举

在 **RBDScene.h** 中：

```cpp
enum CONSTRAINTSOLVERTYPE {
  CST_LAGRANGEDANTZIG,
  CST_LAGRANGEGS,
  CST_LAGRANGEAUTO,
  CST_LAGRANGEINTERACTIVEDANTZIG,
  CST_LAGRANGEINTERACTIVEGS,
  CST_LAGRANGEINTERACTIVEAUTO,
  CST_LAGRANGEDANTZIGEXP,
  CST_IMPULSEBASED,
  CST_PENALTY,
  CST_SEQUENTIALIMPULSE,
  CST_LAGRANGEAPGD   // ← 新增 APGD
};
```

#### 3. 集群工厂中开启 APGD

在 **RBDScene.cpp** 的 `createNewCluster()` 方法里：

```cpp
else if (t == CST_LAGRANGEAPGD) {
  // 为非交互式 Lagrange 乘子集群
  return new RBDClusterLagrangeMultipliers(this);
  // 如需调试/单步，可改为：
  // return new RBDClusterLagrangeMultipliersInteractive(this);
}
```

#### 4. 在 doTimeStep() 中植入 APGD 支持

在 **RBDClusterLagrangeMultipliers::doTimeStep(...)** 的 solver 选择 switch 中添加：

```cpp
case RBDScene::CST_LAGRANGEAPGD:
  myLcp = new RBDLcpAPGD(
    matA.rows(),                   // LCP 维度
    numberEqualityConstraints,     // nub
    myScene->numberIterations(),   // maxIters
    myScene->getTol(),             // tol（需在 RBDScene 中暴露）
    myScene->getAccel()            // accel（需在 RBDScene 中暴露）
  );
  break;
```

其余分支保持不变，并做好失败时增加 CFM 或退化到 GS 的回退策略。

#### 5. 界面 & 配置同步

##### 5.1 Qt 下拉菜单

在 `VSPluginRBDynamXOptions.cpp`：

```cpp
constraintSolverValues.append({
  "apgd",
  tr("Lagrange Multipliers, APGD")
});
```

##### 5.2 读取并应用设置

在 `MainSimStateExtension::slotSyncSettings()` 中：

```cpp
else if (solverName.toLower() == "apgd")
  myRBDScene->setConstraintSolverType(
    VSLibRBDynamX::RBDScene::CST_LAGRANGEAPGD
  );
```

---

## 下一步 & 建议

* **测试对比**：用单摆、双摆等场景对比 APGD 与 Dantzig/GS 的收敛性和效率；
* **日志输出**：在 `RBDLcpAPGD::solve()` 内打印残差、迭代次数，便于定位收敛瓶颈；
* **界面调优**：将 `tol`、`maxIters`、`accel` 参数也暴露到 GUI；
* **性能分析**：跑大规模约束场景，统计内存占用与迭代耗时，为并行化与 GPU 加速打基础。


### 2025-07-11

### 测试对比

将新增的对默认的lagrangmultipliers的求解器（加Dantzig）效果做对比：

 - 默认的求解器是正常的单摆，符合物理特性。
 - APGD基于LAGRANGEMUITIPLIERS的LCP建模的方式去写的求解器，得到的单摆结果：就是直接下落。

### 问题猜测：

同一个单摆模型，用默认的 Lagrange‐multiplier+Dantzig/GS 求解器能正常振动，
但用你接进去的 APGD 解算器就直接自由落体”，
**基本上说明你的 APGD 分支在求解约束力的时候返回了全零（或者极小）的 λ**，
导致根本**没把摆杆的杆约束力施加上去**，物体直接受重力加速度下落。


 - 单摆的绳长保持不变属于等式约束（holonomic equality constraint）。
 - 约束方程写作：

单摆的 holonomic equality constraint（等式约束）写作：

$$
g(\mathbf{q})
=
\bigl\lVert\,\mathbf{x}_{\text{质点}} - \mathbf{x}_{\text{支点}}\bigr\rVert
- L
= 0.
$$

对应的速度约束是

$$
J(\mathbf{q})\,\dot{\mathbf{q}} = 0,
$$

其中

$$
J(\mathbf{q}) = \frac{\partial g}{\partial \mathbf{q}}.
$$










