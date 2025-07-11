---
title: MA周工作记录
date: 2025-07-09
layout: note
excerpt: 关于毕业设计每周任务同步以及在实现过程中的想法与设计思路，方便之后的追溯和毕业论文的编写。:)
---

*Tips：持续完善文档，记录每一次优化与踩坑体会，有助于后续论文撰写与项目迭代！*


# 毕业设计每周工作总结

#### 未来24周计划

- [x] 准备工作
  - [x] 学习C++
  - [x] 学习多体动力学仿真
- [ ] 英语演讲答辩能力
- [ ] 理解文献和chrono参考代码(3)
- [ ] 熟悉VEROSIM的求解器代码结构(1)
- [ ] 实现QP的建模和Interior-Point简单求解(4)
- [ ] 应用APGD求解器(8)
  - [ ] 等式约束(hinge, prismatic, rigid, ball and socket)
  - [ ] contact and friction
  - [ ] Motor，limit及其它的约束类型
- [ ] 分析结果，优化算法(4)
  - [ ] 简单模型
  - [ ] 大规模模型
  - [ ] 实际模型(Harvester)
- [ ] 写论文(4)










## 26.05.2025-30.05.2025
- [x] APGD文献 （CCP → LCP →QP）
- [x] Chrono 源码 找到主函数以及运行流程

## 02.06.2025-06.06.2025

- [x] 继续文献
- [x] 安装VEROSIM，学习建模，了解RBDynamX的求解器结构 （02.06）
- [x] APGD相关文献算法（初级和高级）
- [x] QP问题求解MBD(相关论文)怎么从LCP或CCP建模QP问题，QP问题如何被求解，等式约束和不等式约束
- [x] Chrono debug APGD 设置断点


## 09.06.2025-13.06.2025

- [x] QP问题求解MBD(相关论文)怎么从LCP或CCP**建模QP问题**，QP问题如何被求解，等式约束和不等式约束
- [x] Chrono debug APGD 设置断点
  - [x] 详细看APGD算法，及相关函数


- [x] 学习VEROSIM建模，了解RBDynamX的求解器结构 （02.06）
- [x] SOCCP second order 正交圆锥


## 16.06.2025-20.06.2025

- [x] 论文细节 关于APGD
- [x] 代码细节，以及代码结构
- [x] 了解RBDynamX的求解器结构 思考程序设计


## 23.06.2025-27.06.2025

- [x] chrono代码实现细节，输入输出
- [x] VEROSIM代码实现细节
- [x] chrono中QP问题怎么被构建出来的 


## 30.06.2025-04.07.2025

- [x] 实现QP建模
- [x] 实现APGD
- [x] CCP建模 chrono里给CCP建模，与VEROSIM中的LCP建模比较区别在哪



## 周任务（07.06.2025 – 07.11.2025）
- [x] VEROSIM 实现 CCP / QP 建模  
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





## 周任务（14.06.2025-18.07.2025）

#### 

- [ ] Debug APGD求解器的自由落体问题
- [ ] 完成CCP建模，去名字为RBDClusterLagrangeMultipliersQP






