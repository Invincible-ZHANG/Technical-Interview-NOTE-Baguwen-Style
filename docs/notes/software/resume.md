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


### 以UART为例，给我整个开发的详情例子



一、目标与工况

* **目标**：在 KaihongOS/OpenHarmony 标准系统上启用 **UART9**，通过 **HDF 独立服务模式**发布到用户态；应用用 **HDI/API** 打开串口、设置波特率、收发数据。
* **芯片/板卡**：以 RK3566/RK3568 系列为例（路径与语法也适用于同家族）。
* **HDF 模式**：UART 采用**独立服务模式**——每个 UART 设备作为独立服务发布，/dev 下会有对应的服务节点；此模式内存占用稍高，但用起来简单，便于多实例管理。


二、底层设备连通：DTS 使能 & 引脚复用

1. **使能 UART9 & 选择引脚 & DMA**（示例）：

```dts
&uart9 {
    status = "okay";
    dma-names = "tx", "rx";      // 开启 DMA
    pinctrl-names = "default";
    pinctrl-0 = <&uart9m2_xfer>; // 选择 UART9 的那组引脚
};
```

> 这一步是把 SoC 的 UART 控制器和具体引脚连起来，并且启用控制器；路径依项目而异（rk3566/3568 的 dtsi/dts），思路一致。

**常见坑位**

* `pinctrl-0` 要选对**正确复用组**；
* 若 /dev 下没有生成对应 **tty** 节点，多半是 DTS 没生效或引脚复用被别的外设占用了。



三、HDF 装载：HCS 的两处配置

> HDF 世界里，设备“出现”靠 **device\_info.hcs**，设备“参数与匹配”靠 **平台专属 hcs**（这里用 rk3568\_uart\_config.hcs 作为例子）。两者配对才能被 DevMgr 正确装载。

 1) `device_info.hcs`：声明“我要发布一个 UART9 服务”

* 路径（示例）：`vendor/kaihong/RK3566-xxx/hdf_config/khdf/device_info/device_info.hcs`
* 关键要点：

  * **Policy**：服务发布策略（2=对内核&用户态发布，/dev 下可见；1=仅内核态，不在 /dev 显示）。
  * **serviceName**：约定俗成形如 `HDF_PLATFORM_UART_9`；**后缀“9”会成为应用 open 的 port 参数**。
  * **moduleName**：要与驱动入口里的 `moduleName` 一致。
  * **deviceMatchAttr**：要与平台 hcs 里的 `match_attr` 对上。
    这些在官方开发指导书里都有明确说明。

 1) `rk3568_uart_config.hcs`：告诉系统“第几个 UART，用哪个内核驱动名”

* 路径（示例）：`vendor/kaihong/RK3566-xxx/hdf_config/khdf/platform/rk3568_uart_config.hcs`
* 典型片段（示意）：

```hcs
root {
  device_uart_0x0002 :: uart_device {
    num = 9;                                // 串口号：用于组装内核 tty 名
    driver_name = "ttyS";                   // 形成 /dev/ttyS9
    match_attr = "rockchip_rk3568_uart_9";  // 和 device_info.hcs 里的一致
  }
}
```

* **特殊板卡**：如果不是 `ttyS` 开头（比如 `ttyXRUSB`），就要改 `driver_name`，否则默认用 template 里的值。指导书里有明确提醒。


四、驱动骨架：HDF DriverEntry & 独立服务模式

UART 在 KaihongOS 中走 **独立服务模式**：每个设备对象会发布一个服务节点，由 HDF 设备管理器分发调用。应用通过 **HDI/API** 用 `port` 参数定位到具体实例（比如 9）。

**DriverEntry 极简骨架（示意 C）**：

```c
static int32_t UartBind(struct HdfDeviceObject *dev)    { /* 导出 service 指针 */ return HDF_SUCCESS; }
static int32_t UartInit(struct HdfDeviceObject *dev)    { /* 控制器初始化、ISR/DMA、队列等 */ return HDF_SUCCESS; }
static void    UartRelease(struct HdfDeviceObject *dev) { /* 资源回收 */ }

struct HdfDriverEntry g_uartEntry = {
    .moduleVersion = 1,
    .moduleName = "rockchip_rk3568_uart",   // 和 device_info.hcs 对齐
    .Bind = UartBind,
    .Init = UartInit,
    .Release = UartRelease,
};
HDF_INIT(g_uartEntry);
```

**注意**：如果你同时提供 HDI（IDL→Stub/Proxy），就需要在 `Init()` 里注册你的服务对象，HDF 的 DevMgr 才能把调用路由过来。








实现 usbip 自启动 == watchdog 

