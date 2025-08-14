---
title: "工厂模式"
layout: note
date: 2025-08-13
excerpt: "Factory"
categories: embedded
tags:
  - C++
  - 嵌入式
  - 基础知识
  - Factory
---

# 工厂模式（Factory Pattern）

**一句话**：把“创建对象”这件事封装起来，让使用者只关心“要什么”，不关心“怎么造”。
**动机**：解耦模块与具体实现，降低“新增一种产品/驱动”时的改动面；在嵌入式里还能集中管控内存与初始化流程。

## 1. 何时使用

* 需要根据**配置/运行时参数**选择不同实现（如不同传感器、不同通信协议）。
* 期望**对扩展开放、对修改关闭**（新增产品时尽量不改原调用代码）。
* 需要集中管理对象**生命周期/内存**（嵌入式常见：禁用 `new`、使用对象池）。

## 2. 三种常见形态

* **简单工厂（Static/Concrete Factory）**：一个函数或类里用 `switch/if` 返回不同产品对象。简单直接，但新增分支需改工厂源码。
* **工厂方法（Factory Method）**：把“如何生产”下放到子类，新增产品→新增对应工厂子类。
* **抽象工厂（Abstract Factory）**：按“**产品族**”成套创建（如同一供应商的 UART/GPIO/SPI 一套驱动）。

---

## 3. 简单工厂（可运行、最直接）

**场景**：根据配置选择不同的 Logger。

```cpp
#include <memory>
#include <string>
#include <iostream>

struct ILogger {
    virtual ~ILogger() = default;
    virtual void log(const std::string& msg) = 0;
};

struct UartLogger : ILogger {
    void log(const std::string& msg) override { std::cout << "[UART] " << msg << "\n"; }
};

struct CanLogger : ILogger {
    void log(const std::string& msg) override { std::cout << "[CAN ] " << msg << "\n"; }
};

enum class LoggerType { UART, CAN };

struct LoggerFactory {
    static std::unique_ptr<ILogger> create(LoggerType t) {
        switch (t) {
        case LoggerType::UART: return std::make_unique<UartLogger>();
        case LoggerType::CAN:  return std::make_unique<CanLogger>();
        }
        return nullptr;
    }
};

int main() {
    auto log = LoggerFactory::create(LoggerType::UART);
    log->log("hello factory");
}
```

**优点**：容易上手。
**缺点**：每加一个实现就得改 `switch`，违背开闭原则。

---

## 4. 工厂方法（扩展友好）

**场景**：将“生产逻辑”交给不同的工厂子类，新增产品只新增类，不动老代码。

```cpp
#include <memory>
#include <string>
#include <iostream>

struct ISensor { virtual ~ISensor()=default; virtual int read() = 0; };

struct I2CSensor : ISensor { int read() override { return 100; } };
struct SPISensor : ISensor { int read() override { return 200; } };

struct ISensorFactory {
    virtual ~ISensorFactory() = default;
    virtual std::unique_ptr<ISensor> create() = 0;
};

struct I2CFactory : ISensorFactory {
    std::unique_ptr<ISensor> create() override { return std::make_unique<I2CSensor>(); }
};

struct SPIFactory : ISensorFactory {
    std::unique_ptr<ISensor> create() override { return std::make_unique<SPISensor>(); }
};

int main() {
    std::unique_ptr<ISensorFactory> f = std::make_unique<I2CFactory>();
    auto s = f->create();
    std::cout << s->read() << "\n";
}
```

**优点**：新增产品→新增工厂子类即可。
**缺点**：类数量增多；如果产品很多，层级也会多。

---

## 5. 抽象工厂（创建“产品族”）

**场景**：同一芯片厂商的一组外设驱动（UART/GPIO/SPI）需要配套创建。

```cpp
#include <memory>
#include <iostream>

// 抽象产品
struct UART { virtual ~UART()=default; virtual void send(const char*)=0; };
struct GPIO { virtual ~GPIO()=default; virtual void write(int)=0; };

// 具体产品族 A
struct VendorA_UART : UART { void send(const char* s) override { std::cout << "A.UART " << s << "\n"; } };
struct VendorA_GPIO : GPIO { void write(int v) override { std::cout << "A.GPIO " << v << "\n"; } };

// 具体产品族 B
struct VendorB_UART : UART { void send(const char* s) override { std::cout << "B.UART " << s << "\n"; } };
struct VendorB_GPIO : GPIO { void write(int v) override { std::cout << "B.GPIO " << v << "\n"; } };

// 抽象工厂：成套创建
struct DriverFactory {
    virtual ~DriverFactory()=default;
    virtual std::unique_ptr<UART> createUART() = 0;
    virtual std::unique_ptr<GPIO> createGPIO() = 0;
};

struct VendorA_Factory : DriverFactory {
    std::unique_ptr<UART> createUART() override { return std::make_unique<VendorA_UART>(); }
    std::unique_ptr<GPIO> createGPIO() override { return std::make_unique<VendorA_GPIO>(); }
};

struct VendorB_Factory : DriverFactory {
    std::unique_ptr<UART> createUART() override { return std::make_unique<VendorB_UART>(); }
    std::unique_ptr<GPIO> createGPIO() override { return std::make_unique<VendorB_GPIO>(); }
};

int main() {
    std::unique_ptr<DriverFactory> f = std::make_unique<VendorA_Factory>();
    auto uart = f->createUART();
    auto gpio = f->createGPIO();
    uart->send("ping");
    gpio->write(1);
}
```

**优点**：保证同一“产品族”一致性；切换厂商只换工厂。
**缺点**：对“跨族混搭”支持不友好。

---

## 6. 注册式工厂（运行时按字符串/ID 创建）

**场景**：从配置文件/命令行/网络报文里读取产品名，然后创建对象。对扩展更友好：新增类只需注册。

```cpp
#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

template <typename Base>
class RegistryFactory {
public:
    using Creator = std::function<std::unique_ptr<Base>()>;

    static RegistryFactory& instance() {
        static RegistryFactory inst; // 避免静态初始化次序问题
        return inst;
    }

    void reg(const std::string& key, Creator c) { creators_[key] = std::move(c); }

    std::unique_ptr<Base> create(const std::string& key) const {
        if (auto it = creators_.find(key); it != creators_.end()) return (it->second)();
        return nullptr;
    }
private:
    std::unordered_map<std::string, Creator> creators_;
};

struct IAlgo { virtual ~IAlgo()=default; virtual int run(int)=0; };
struct AlgoA : IAlgo { int run(int x) override { return x+1; } };
struct AlgoB : IAlgo { int run(int x) override { return x*2; } };

struct AutoRegister {
    AutoRegister() {
        RegistryFactory<IAlgo>::instance().reg("A", []{ return std::make_unique<AlgoA>(); });
        RegistryFactory<IAlgo>::instance().reg("B", []{ return std::make_unique<AlgoB>(); });
    }
} _auto_register_once;

int main() {
    auto a = RegistryFactory<IAlgo>::instance().create("B");
    return a ? a->run(21) : -1; // 42
}
```

**要点**

* 通过函数内局部 `static` 避免**静态初始化次序**坑。
* 多线程下可用 `std::call_once` 或在初始化阶段单线程注册。
* 嵌入式禁用 RTTI/异常时，`std::function` 可换成**函数指针**。

---

## 7. 嵌入式落地指南（内存/性能/可维护）

1. **禁用动态分配**：用**对象池**+ placement-new。

```cpp
#include <new>
#include <array>

template <typename T, size_t N>
class ObjectPool {
public:
    template <typename...Args>
    T* create(Args&&...args) {
        for (auto& slot : used_) if (!slot) {
            size_t idx = &slot - used_.data();
            slot = true;
            return new (&buf_[idx]) T(std::forward<Args>(args)...);
        }
        return nullptr; // 满了
    }
    void destroy(T* p) {
        if (!p) return;
        p->~T();
        size_t idx = (reinterpret_cast<char*>(p) - reinterpret_cast<char*>(buf_.data())) / sizeof(T);
        used_[idx] = false;
    }
private:
    std::array<std::aligned_storage_t<sizeof(T), alignof(T)>, N> buf_{};
    std::array<bool, N> used_{};
};
```

2. **去虚函数**：在极端性能/尺寸场景用 `std::variant` + `std::visit` 或 CRTP，避免 vtable 开销。
3. **初始化顺序**：将注册表/单例都做成**函数内静态**。
4. **错误处理**：禁用异常时返回 `nullptr/错误码`，集中在工厂处理。
5. **可测试性**：工厂接收**依赖注入**（硬件句柄、配置结构体），单测时注入假设备。

---

## 8. 现代 C++ 替代与增强

* **模板工厂（零开销转发）**

```cpp
template <class T, class...Args>
std::unique_ptr<T> make(Args&&...args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

* **`std::variant` 作为“静态工厂”**（无分配、无虚函数）

```cpp
#include <variant>
struct UartLogger { void log(const std::string& s) { /*...*/ } };
struct CanLogger  { void log(const std::string& s) { /*...*/ } };
using Logger = std::variant<UartLogger, CanLogger>;

void log(Logger& L, const std::string& s) {
    std::visit([&](auto& impl){ impl.log(s); }, L);
}
```

* **策略模式（Strategy）+ 工厂**：工厂只负责“挑选策略”，运行时热插。
* **构建者（Builder） vs 工厂**：Builder 负责**复杂参数拼装**，工厂负责**挑选实现**；两者常搭配。

---

## 9. 反例与坑

* **巨型 `switch` 工厂**：新增必改、易冲突；优先用**注册式工厂**或工厂方法。
* **全局单例乱飞**：难以单测与复用；用**受控生命周期**（对象池 / `unique_ptr`）+ 明确拥有者。
* **静态注册顺序问题**：跨编译单元可能先用后定义；用**函数内静态**或显式 `init()` 步骤。
* **在中断里创建对象**：避免；在中断上下文只使用已就绪对象。

---

## 10. 小结与选型建议

* **配置型选择/插件化**：注册式工厂。
* **大量可扩展的产品层级**：工厂方法。
* **成套配套件（产品族）**：抽象工厂。
* **低资源/高确定性场景**：对象池 + `variant`/CRTP，尽量无动态分配与虚表。

---

## 11. 常见面试角度（速记）

* **为何需要工厂**：解耦创建与使用，便于扩展与测试。
* **开闭原则如何体现**：新增产品避免改调用方；注册式工厂实现“对修改关闭”。
* **与策略/建造者/原型区别**：策略关注行为切换，建造者关注复杂构造流程，原型用拷贝创建，工厂关注**选择与生产**。
* **嵌入式特别点**：内存受限、禁异常/RTTI、启动顺序严格 → 对象池、函数内静态、最小依赖。

---

## 12. 延伸阅读与实践任务

* 给你现有的驱动层加一层**注册式工厂**，用 **JSON/命令行** 选择设备实现；再写一个**对象池版**以便对比内存足迹。
* 做一版 **`std::variant`** 驱动，用 `std::visit` 走完全链路，测量与虚函数版本的指令数/时延差异。

> 写代码是搭积木，工厂就是“零件中心”。当零件越来越多，先把库房管好，世界才不会一开机就乱成一锅粥。
