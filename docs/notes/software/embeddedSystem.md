以下所有的知识全部针对AVR讲解，8 位单片机架构/产品家族 ，由  **Atmel** （现归  **Microchip** ）。它以“简洁高效、上手容易、5V 友好”闻名，Arduino Uno 用的 **ATmega328P** 就是典型的 AVR 芯片。

核心特点：

* **指令集/架构** ：RISC、哈佛结构（程序存储器与数据存储器分离）；多数 ALU 指令单周期执行。
* **寄存器** ：32 个通用寄存器（R0–R31），状态寄存器  **SREG** 。
* **存储** ：Flash（程序）、SRAM（数据）、EEPROM（掉电不丢）。
* **外设** ：GPIO、定时器/计数器、PWM、ADC、USART、SPI、I²C(TWI)、看门狗、外部中断、睡眠省电等。
* **电压与时钟** ：常见 1.8–5V 供电，主频常见至 20 MHz（经典系列）。
* **系列** ： **ATmega** 、 **ATtiny** 、以及新一代  **AVR Dx/EA** 。
* **开发工具** ：`avr-gcc` + `avrdude`、Arduino IDE、Microchip Studio；下载/调试接口有 ISP、JTAG、debugWIRE、UPDI（新器件）。

## DDR/PORT/PIN：

### DDRx（Data Direction Register）：

* **作用** ：配置每一位引脚是输入还是输出。

  * 写 `1` → 该管脚为 **输出** 。
  * 写 `0` → 该管脚为**输入（高阻）。**
* **读写** ：可读可写，读到的是当前方向配置。
* **例子** （把 PB3 配成输出）：

  ```cpp
  DDRB |= _BV(PB3);          // PB3 -> output
  ```

`_BV(x)` 是宏：`#define _BV(x) (1U << (x))`，生成指定位的掩码。

### PORTx（Port Data Register）

* **对输出引脚** （DDRx 位=1）：

  * 写 `1` → 引脚 **输出高电平** 。
  * 写 `0` → 引脚 **输出低电平** 。
* **对输入引脚** （DDRx 位=0）：

  * 写 `1` →  **开启上拉电阻** （pull-up）。
  * 写 `0` →  **关闭上拉** （保持高阻，悬空会不稳定）。   Pull-up resistors allow the voltage to collapse骤变
* **读写** ：可读可写； **读到的是输出锁存值** （并不保证等于脚上真实电平）。

  ```cpp
  // 点亮接在 PB3 的 LED（假设低有效就反着写）
  DDRB  |= _BV(PB3);         // 配成输出
  PORTB |= _BV(PB3);         // 输出高
  PORTB &= ~_BV(PB3);        // 输出低
  // 把 PB2 做输入并开上拉
  DDRB  &= ~_BV(PB2);        // 输入
  PORTB |= _BV(PB2);         // 使能上拉
  ```


### PINx（Port Input Pins Register）:

* **作用** ： **读取引脚当前的实际电平** （无论该脚被配置为输入还是输出）。
  * 读到 `1` 表示脚上是高电平，`0` 表示低电平。
* **读写** ：按规范 **只读** ；但  **AVR 有一个“特性/技巧”** ：
  * **向 PINx 写入 `1` 会翻转（toggle）对应的 PORTx 位** （常用于快速翻转输出）。
  * 这行为是 AVR 特有的，不是所有 MCU 都支持；所以跨平台代码慎用。

```cpp
// 读 PB2 的实际电平
uint8_t level = (PINB & _BV(PB2)) ? 1 : 0;

// 翻转 PB3（已是输出）
PINB = _BV(PB3);           // AVR: 写 1 到 PINB 的 PB3 位 -> 翻转 PORTB.PB3
// 或者更通用：
PORTB ^= _BV(PB3);
```


其实这部分没搞太懂，存疑？





