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

### 1) `device_info.hcs`：声明“我要发布一个 UART9 服务”

* 路径（示例）：`vendor/kaihong/RK3566-xxx/hdf_config/khdf/device_info/device_info.hcs`
* 关键要点：

  * **Policy**：服务发布策略（2=对内核&用户态发布，/dev 下可见；1=仅内核态，不在 /dev 显示）。
  * **serviceName**：约定俗成形如 `HDF_PLATFORM_UART_9`；**后缀“9”会成为应用 open 的 port 参数**。
  * **moduleName**：要与驱动入口里的 `moduleName` 一致。
  * **deviceMatchAttr**：要与平台 hcs 里的 `match_attr` 对上。
    这些在官方开发指导书里都有明确说明。

### 2) `rk3568_uart_config.hcs`：告诉系统“第几个 UART，用哪个内核驱动名”

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

---

# 四、驱动骨架：HDF DriverEntry & 独立服务模式

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

---

# 五、应用 API 的使用（端到端验证）

KaihongOS 提供了上层 UART API；常用的方法包括：

* `uartOpen(port)`：port 就是前面 `HDF_PLATFORM_UART_9` 的后缀 **9**；
* `uartSetBaud()`：设置波特率；
* `uartSetAttribute()`：设置 8N1、硬件流控等。
  这几项在官方文档的 “应用使用流程 / 常用 UART API” 部分明确写了说明。

**示例（伪代码）**：

```cpp
int fd = uartOpen(9);                      // 对应 HDF_PLATFORM_UART_9
uartSetBaud(fd, 115200);
UartAttr attr{ .dataBits=8, .parity='N', .stopBits=1, .rtscts=true };
uartSetAttribute(fd, &attr);

// Write / Read
const char *msg = "hello\n";
write(fd, msg, strlen(msg));
char buf[256];
int n = read(fd, buf, sizeof(buf));
```

---

# 六、自检与联调：Loopback & 双终端

**最小自测法**：短接 **RX 与 TX**，用两个终端对着**同一个 tty 节点**做回环测试：

* 终端 A：`cat /dev/ttyS9`
* 终端 B：`echo "abc" > /dev/ttyS9`
* 若 A 收不到：

  1. 看 DTS 的 `pinctrl-0` 是否选错；
  2. 硬件连接是否 OK；
  3. 是否有其他进程占用串口；
     这些“常见问题处理”的步骤，官方文档给出了逐条排查建议。

**/dev 节点检查**

* 是否有 `/dev/ttyS9`（或你配置的 `driver_name + num`）
* 是否有 `HDF_PLATFORM_UART_9`（Policy=2 时可见）
  没有的话，倒查 3.1 的 DTS 与 3.2 的 HCS 两步。

---

# 七、性能/稳定性参数建议

* **DMA**：尽量打开；小包多发可考虑聚合发送或适当放大环形缓冲。
* **流控**：有线长或高波特率就上 `RTS/CTS`，能显著降低丢包。
* **P99 延迟**：打点统计写/读完成耗时；必要时在驱动侧增加水位线与超时告警。
* **电源管理**：补齐 `suspend/resume`，恢复时重配串口寄存器，避免恢复后“偶发黑洞”。

---

# 八、提交流程清单（把变更一次性讲清楚）

1. DTS：`&uart9 { status="okay"; pinctrl-0=<...>; dma-names="tx","rx"; }`（含引脚组）。
2. HCS：

   * `device_info.hcs`：`serviceName=HDF_PLATFORM_UART_9`、`Policy=2`、`moduleName`、`deviceMatchAttr`。
   * `rk3568_uart_config.hcs`：`num=9`、`driver_name="ttyS"`（或特殊板卡改成 `"ttyXRUSB"`）、`match_attr` 对齐。
3. 驱动入口：`HdfDriverEntry` 与 `moduleName` 对齐，`Bind/Init/Release` 完整；若提供 HDI，确保已注册服务对象。
4. 应用/测试：

   * API：`uartOpen/uartSetBaud/uartSetAttribute`；回环测试；
   * /dev 节点检查：`/dev/ttyS9` 与 `HDF_PLATFORM_UART_9`。

---

# 九、你需要的“一句话证据链”（放到评审或简历里）

* UART 采用**独立服务模式**，每个设备单独发布服务（节省上层路由复杂度），但需要为每个设备配置节点，内存占用略增。
* `serviceName` 的数字后缀和应用层 `uartOpen(port)` 的 `port` 一致（例如 9）。
* DTS 负责控制器与引脚、DMA 使能；HCS 负责 HDF 装载、匹配与设备命名（`driver_name+num`→`/dev/ttyS9`）。
* 常见问题排查：/dev 节点存在性、引脚复用正确性、硬件回路、并发占用。

---

需要我把上面这套变更整理成 **git patch 清单**（按文件路径列出增删改）或做一份 **A4 速查图（DTS→HCS→DriverEntry→API）**，我可以直接给你成品模板，复制到 MR 描述里就能过审。


