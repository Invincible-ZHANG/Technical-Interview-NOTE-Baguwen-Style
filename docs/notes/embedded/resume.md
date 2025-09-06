---
title: "resume 介绍"
date: 2025-09-01
layout: note
categories: 
tags: 
excerpt: "项目介绍。"
toc: true
---

## 嵌入式驱动工程师 08/2022 – 03/2023

### OpenHarmony（KaihongOS）HDF 驱动这是什么东西
HDF（Hardware Driver Foundation）是 OpenHarmony / KaihongOS 的统一驱动框架。它把“写驱动、装驱动、让上层调用驱动”这一整套事做成了标准化流程，目标是同一套接口跑在不同内核/芯片上（Linux、LiteOS、小型 RTOS 等），上层通过稳定的 HDI（Hardware Driver Interface） 访问硬件。


框架：驱动的生命周期与代码骨架（Bind / Init / Release / Dispatch），驱动管理服务 DevMgr 负责按配置加载驱动。

配置：用 HCS（HDF Configuration Source） 取代 Linux 的 DTS，描述设备/总线/匹配信息。

接口：对上提供 HDI（IDL 生成的接口），应用或系统服务通过 IPC 调用；对下封装 总线子系统（I2C/SPI/UART/USB/GPIO/SDIO…）

### 它解决了什么问题？

跨平台统一：同一套驱动模型适配多内核、多 SoC。

解耦：上层调 HDI，不直接绑具体驱动；驱动对下用统一的 Bus API，少碰各家寄存器差异。

可维护：用 HCS 声明式配置，DevMgr 按需加载、启动、卸载；日志与事件链路统一。


### 典型调用链（从开机到应用用到设备）

开机：DevMgr 读取 HCS → 解析设备/驱动匹配信息。

装载：找到你的驱动模块 → 调 Bind() 绑定对象 → 调 Init() 完成硬件初始化/注册服务。

发布服务：驱动把 HDI 服务对象挂出去（内核态或用户态），注册到服务名空间。

上层调用：应用/系统服务通过 HDI（IPC）→ 驱动 Dispatch() 处理 → 经 I2C/SPI/USB 等访问硬件。

收尾：卸载时走 Release()，资源回收。


DTS 是“告诉内核：板子长什么样”。

HCS/HDF 是“告诉系统：这个设备由哪个驱动如何被装载、如何被上层调用”。

HDF 把“驱动 → 服务”这一层做了统一抽象，方便跨内核、跨平台复用与上层的一致调用。


