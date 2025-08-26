---
title: 求解器中SIPGS方法
date: 2025-08-25
excerpt: "SIPGS/逐次冲量。"
layout: note
---





***基于 Moreau/Anitescu 时间步进的 NSC/CCP 建模**，用 **PGS（Gauss–Seidel）式就地更新**来解接触/摩擦与关节约束。*


>*(Moreau 指让·雅克·莫罗（Jean-Jacques Moreau），非光滑力学/凸分析的大佬；Anitescu 指 Mihai Anitescu，他和合作者（Potra 等）把刚体接触+摩擦的时间步进离散做成了现在工程里常用的一整套“非光滑时间步进（event-capturing time stepping）”方案。两条线都不做“事件检测+解析冲量”，而是在固定步长里直接求解一次互补/锥互补问题，把接触、摩擦、碰撞冲量统一到一步里解决。)*


# SIPGS

