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



## 代码


ChVersion : 版本号头文件”（version header）

Chplatform: Chrono 的“跨平台胶水” ，ChApiEXPORT/IMPORT 解决不同平台/构建方式下动态库符号可见性问题；CH_DEPRECATED 用于标记弃用 API并在编译期发出友好提醒。

ChApiCE: ChApi 是一个“可见性贴纸”宏——你把它贴在要对外公开的类/函数前面，它会在编库时展开成“导出符号”，在用库时展开成“导入符号”；如果是静态库或非 Windows，它通常什么也不做。

ChClassFactory: