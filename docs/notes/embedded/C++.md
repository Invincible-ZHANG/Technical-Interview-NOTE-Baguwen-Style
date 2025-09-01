---
layout: note
title: "C++"
date: 2025-08-31
excerpt: "C++数据结构。"
categories: embedded
tags:
  - C语言
creat_date: 2025-08-29
---


> 题目主要参考：https://blog.csdn.net/songbijian/article/details/132507421 ，但是内容为自己写的。


## 1.封装、继承、多态

* 封装（Encapsulation）

    核心思想：
    
    把 **数据（成员变量）和方法（成员函数）** 绑在一起，外部不能随意动内部的数据，只能通过公开的“接口”来访问或修改。

    这就像你用手机，看到的是“打电话”“拍照”的按钮，但你不用知道里面具体的电路逻辑，它都封装好了。

    意义拓展：

    * 数据安全：别人不能随便乱改你类里的变量。
    * 降低复杂度：别人只需要知道接口（方法），不用管内部实现。
    * 更易维护：如果以后想修改实现，只要接口不变，外部调用的代码不用改。

    举例（生活版）：

    你去银行取钱 → 只能通过 **柜台（接口）** 来操作，而不能直接跑到保险柜里掏钱。

~~~
class BankAccount {
private:
    double balance;  // 封装在类内部，外界无法直接访问

public:
    BankAccount(double init) : balance(init) {}

    void deposit(double amount) {
        if (amount > 0) balance += amount;
    }

    double getBalance() const {
        return balance;
    }
};

int main() {
    BankAccount acc(1000);
    acc.deposit(500);
    // acc.balance = -999; // ❌ 不允许直接访问
    std::cout << acc.getBalance(); // ✅ 通过接口访问
}
~~~


* 继承（Inheritance）

子类自动“拿到”父类的成员（变量、方法），从而实现代码复用。

**但私有成员虽然继承了，但子类不能直接访问。**

意义拓展：

让通用逻辑写在父类里，子类只管扩展差异部分。

形成层级关系（例如：动物 → 哺乳动物 → 狗）。

注意耦合性：继承太多层会导致修改父类影响所有子类。


举例（生活版）：

“电动车”继承了“自行车”的特性（有轮子、能骑），但增加了“电池”和“充电功能”。


~~~
class Animal {
public:
    void eat() { std::cout << "Eating...\n"; }
};

class Dog : public Animal {
public:
    void bark() { std::cout << "Woof!\n"; }
};

int main() {
    Dog d;
    d.eat();  // 继承自 Animal
    d.bark(); // Dog 自己的功能
}
~~~


* 多态（Polymorphism）
  
相同的接口，针对不同的对象有不同的表现。

多态常见于虚函数（C++），即“父类指针/引用调用子类重写的方法”。

**C++ 在调用虚函数时的一种运行时机制**

意义拓展：

可替代性：用父类指针就能操作所有子类对象。

可扩展性：增加新子类时，旧代码无需修改。

简化逻辑：避免写大量的 if-else 来区分不同对象。

举例（生活版）：

“播放”这个动作，对不同对象表现不同：

播放 CD → 放音乐

播放 视频文件 → 放电影

播放 游戏 → 进入游戏场景


~~~
class Shape {
public:
    virtual void draw() { std::cout << "Drawing a shape\n"; }
};

class Circle : public Shape {
public:
    void draw() override { std::cout << "Drawing a circle\n"; }
};

class Square : public Shape {
public:
    void draw() override { std::cout << "Drawing a square\n"; }
};

int main() {
    Shape* s1 = new Circle();
    Shape* s2 = new Square();

    s1->draw(); // 多态：输出 "Drawing a circle"
    s2->draw(); // 多态：输出 "Drawing a square"
}
~~~

> Shape* s1 = new Circle(); 看上去 s1 是一个 Shape*，但实际上它存的是 Circle 对象的地址。编译器在编译时只知道它是 Shape*，但运行时 s1 真正指向的是一个 Circle。


> 如果没有 virtual，编译器会在编译阶段就确定调用 Shape::draw()（静态绑定）。

> 加了 virtual 后，编译器会在对象里存一张“虚函数表 vtable”，表里记录了当前对象该调用哪个版本的 draw()。

> 所以 s1->draw() 运行时会去查 vtable，发现 s1 指向的是 Circle，于是调用 Circle::draw()。 

---


## 2.多态的实现原理是什么？以及多态的优点（特点）？

* 多态的实现方式（原理）

1. 静态多态（编译期多态）

**定义**：编译器在**编译阶段**就能确定调用哪个函数，不需要运行时再查表。
**实现方式**：

* **函数重载**（Overloading）：函数名相同，但参数个数或类型不同。
* **运算符重载**（Operator overloading）：例如 `+` 可以让两个对象相加。
* **模板（Template，多见于泛型编程）**：在编译时根据类型生成对应代码。
* **函数隐藏（Redefinition/Name hiding）**：子类定义了同名函数，父类版本被屏蔽。

**示例（函数重载）**

```cpp
class Printer {
public:
    void print(int x) { std::cout << "Printing int: " << x << "\n"; }
    void print(double y) { std::cout << "Printing double: " << y << "\n"; }
};
int main() {
    Printer p;
    p.print(5);    // 调用 int 版本
    p.print(3.14); // 调用 double 版本
}
```

**示例（运算符重载（Operator overloading））**
```
#include <iostream>
using namespace std;

class Complex {
public:
    double real, imag;

    Complex(double r, double i) : real(r), imag(i) {}

    // 重载 + 运算符
    Complex operator+(const Complex& other) {
        return Complex(real + other.real, imag + other.imag);
    }

    void display() { cout << real << " + " << imag << "i" << endl; }
};

int main() {
    Complex c1(1.0, 2.0), c2(3.5, 4.5);
    Complex c3 = c1 + c2;  // 调用重载的 +
    c3.display();          // 输出 4.5 + 6.5i
}
```

**示例（Template，多见于泛型编程）**
```
#include <iostream>
using namespace std;

// 函数模板
template <typename T>
T add(T a, T b) {
    return a + b;
}

int main() {
    cout << add(3, 5) << endl;         // int 版本 → 输出 8
    cout << add(3.14, 2.71) << endl;   // double 版本 → 输出 5.85
    cout << add(string("Hi "), string("there")) << endl; // string 版本 → "Hi there"
}
```

**示例（函数隐藏（Redefinition / Name hiding））**
```
#include <iostream>
using namespace std;

class Base {
public:
    void show(int x) { cout << "Base show(int): " << x << endl; }
    void show(double y) { cout << "Base show(double): " << y << endl; }
};

class Derived : public Base {
public:
    void show(string s) { cout << "Derived show(string): " << s << endl; }
};

int main() {
    Derived d;
    // d.show(10);       // ❌ 编译错误：Base::show 被隐藏
    // d.show(3.14);     // ❌ 也不行
    d.show("Hello");     // ✅ 调用子类版本
}
```

编译器在编译时就能决定调用哪个函数，所以叫“静态多态”。


2. 动态多态（运行时多态）

**定义**：编译器在编译时只知道调用的是“基类接口”，但**真正调用哪个版本要运行时才能决定**。
**实现方式**：

* **虚函数（virtual function）**：基类声明虚函数，子类重写。
* **虚函数表（vtable）+ 虚函数指针（vptr）**：

  * 每个含虚函数的类，编译器都会为它建立一张虚函数表（vtable）。
  * 每个对象里会偷偷塞一个指针（vptr），指向自己类的虚函数表。
  * 调用虚函数时，运行时通过 vptr 查 vtable 找到真正要执行的函数。

**示例（动态多态）**

```cpp
class Shape {
public:
    virtual void draw() { std::cout << "Drawing shape\n"; }
};

class Circle : public Shape {
public:
    void draw() override { std::cout << "Drawing circle\n"; }
};

int main() {
    Shape* s = new Circle(); // 基类指针指向派生类对象
    s->draw(); // 运行时决定：调用 Circle::draw()
}
```

**背后原理（简化版）**
假设 `Shape` 的虚函数表是：

| vtable for Shape |
| ---------------- |
| \&Shape::draw    |

`Circle` 的虚函数表是：

| vtable for Circle |
| ----------------- |
| \&Circle::draw    |

当你写 `Shape* s = new Circle();` 时：

* s 对象里有个 vptr，指向 `Circle` 的 vtable。
* 调用 `s->draw()` 时，程序查表发现 vptr 指向的是 `Circle::draw`，于是执行它。



* 多态的优点（特点）

1. 可替代性（Substitutability）

父类指针/引用可以指向任意子类对象，代码更通用。

```cpp
void render(Shape* s) { s->draw(); } 
// 不管是 Circle 还是 Square，都能传进来
```

2. 可扩展性（Extensibility）

新增一个子类，不需要修改现有代码，只要继承并重写虚函数即可。

→ 典型的“对扩展开放，对修改关闭”（开闭原则）。

3. 灵活性（Flexibility）

同一接口调用，不同对象响应不同 → “一招多用”。

4. 简化代码（Maintainability）

避免大量 `if-else` 判断类型：

```cpp
// ❌ 没有多态时
if (type == "circle") drawCircle();
else if (type == "square") drawSquare();

// ✅ 有多态时
Shape* s = new Circle();
s->draw();
```

* 生活类比帮助理解

  * **静态多态**：像“重名的人”——张伟（医生）、张伟（律师）。当你说“张伟医生”，别人立刻知道是哪个（编译期确定）。
  * **动态多态**：像“遥控器接口”——你按下“播放”键（相同接口），插电视放电视，插 DVD 播电影，插游戏机进游戏（运行时确定）。




## 3.final标识符的作用是什么？

1. `final` 用在类上

**作用**：

* 表示这个类不能被继承。
* 一旦某个类被标记为 `final`，别人就不能再写子类去继承它。

**示例**：

```cpp
class Animal final {
public:
    void sound() { std::cout << "Some sound\n"; }
};

// ❌ 编译错误：final 类不能被继承
class Dog : public Animal {
};
```

2. `final` 用在虚函数上

**作用**：

* 表示这个虚函数在当前类已经是**最后版本**，不能再被子类重写。
* 阻止“无限重写”，锁定接口。

**示例**：

```cpp
class Base {
public:
    virtual void show() final {  // 不能再被重写
        std::cout << "Base show\n";
    }
};

class Derived : public Base {
public:
    // ❌ 编译错误：final 函数不能再被重写
    void show() override {
        std::cout << "Derived show\n";
    }
};
```

3. 为什么需要 `final`？

* **设计意图明确**：告诉别人“这个类不该被继承”或“这个接口不能再改”。
* **防止滥用继承**：有些类（比如工具类、单例类）不希望再派生。
* **保证接口稳定性**：有些虚函数如果继续被重写，可能破坏逻辑，就可以 `final`。
* **性能优化**：编译器知道函数不会被重写时，可以直接静态绑定，省掉虚函数表查找。




## 4.虚函数是怎么实现的？它存放在哪里在内存的哪个区？什么时候生成的？



1. 虚函数的实现核心：**vtable（虚函数表）+ vptr（虚表指针）**

* **虚函数表 (vtable)**

  * 编译器为每个有虚函数的类生成一张表。
  * 表里存放的是**该类虚函数的地址**（函数入口地址）。
  * 就像一张“菜单”，告诉对象：`draw` 在哪里，`show` 在哪里。

* **虚表指针 (vptr)**

  * 每个含虚函数的对象实例里，编译器会偷偷塞一个指针（vptr）。
  * vptr 指向该对象所属类的 vtable。
  * 当你调用虚函数时，程序先通过对象里的 vptr 找到 vtable，再跳到正确的函数地址。


2. 内存布局示意

假设有代码：

```cpp
class Base {
public:
    virtual void show() { std::cout << "Base show\n"; }
};

class Derived : public Base {
public:
    void show() override { std::cout << "Derived show\n"; }
};

int main() {
    Base* p = new Derived();
    p->show();
}
```

运行时，`p->show()` 调用流程：

1). `p` 是一个指向 `Derived` 对象的基类指针。

2). 该对象里有一个 `vptr`，指向 `Derived` 的虚函数表。

3). 在虚表里，`show` 的入口被填成 `Derived::show`。

4). 所以执行的是 **Derived 的版本**。





3. 内存在哪儿？

* **虚函数表 (vtable)**

  * **编译时生成**，是一个静态结构。
  * 存在**全局/只读数据区**（通常在**静态存储区**，跟全局变量、静态变量类似）。
  * 所有该类对象共享同一张 vtable，不会为每个对象单独分配。

* **虚表指针 (vptr)**

  * 存在于对象实例内部，跟随对象一起分配在 **栈区**（局部对象）或 **堆区**（动态分配的对象）。
  * 占用对象内存的一部分（通常是一个指针大小：32 位机器是 4 字节，64 位机器是 8 字节）。

* **虚函数本身代码**

  * 和普通函数一样，存在于 **代码段（text/code segment）**。
  * vtable 里存的就是这些函数入口地址。


4. 什么时候生成？

* **编译阶段**：编译器就会为类生成 vtable，并在对象构造时设置好 vptr。
* **运行阶段**：调用虚函数时，通过 vptr 查表，执行函数。


5. 直观例子（模拟打印 vtable 地址）

```cpp
#include <iostream>
using namespace std;

class Base {
public:
    virtual void show() { cout << "Base show\n"; }
};

class Derived : public Base {
public:
    void show() override { cout << "Derived show\n"; }
};

int main() {
    Base* p = new Derived();

    cout << "对象地址: " << p << endl;
    cout << "虚表指针(vptr)地址: " << *((void**)p) << endl;
    cout << "虚函数入口地址: " << *((void**)(*((void**)p))) << endl;

    p->show();  // 实际调用 Derived::show
}
```



## 5.智能指针的本质是什么，它们的实现原理是什么？


智能指针本质是一个封装了一个原始C++指针的类模板，为了确保动态内存的安全性而产生的。实现原理是通过一个对象存储需要被自动释放的资源，然后依靠对象的析构函数来释放资源。

1. 智能指针的本质

* **普通指针**：`int* p = new int(10);` → 需要手动 `delete p;`。
* **智能指针**：用一个类封装 `int*`，当类对象生命周期结束（析构）时，自动调用 `delete`。

**一句话**：智能指针就是**带有自动回收功能的指针包装类**。


2. 实现原理（核心思想）

(1) RAII

* **R**esource **A**cquisition **I**s **I**nitialization
* 当对象构造时获取资源（比如申请内存），当对象析构时释放资源。
* 智能指针就是 RAII 的典型应用。

(2) 内部封装

* 智能指针类里有一个**原始指针成员**。
* 在智能指针的 **析构函数** 里调用 `delete` 或 `delete[]` 来释放指针。

(3) 常见实现方式

* 重载 `*` 和 `->` 运算符，让智能指针用起来和普通指针一样。
* 可能需要**引用计数**，来支持多个智能指针共享同一块资源。


3. 常见智能指针种类（C++11）

1). **`unique_ptr`**

   * 独占所有权，一个对象只能有一个 `unique_ptr` 指向它。
   * 禁止拷贝，只能转移（move）。

2). **`shared_ptr`**

   * 共享所有权，多个 `shared_ptr` 可以指向同一块内存。
   * 内部用**引用计数**来决定什么时候释放资源。

3). **`weak_ptr`**

   * 弱引用，不增加引用计数。
   * 避免循环引用（`shared_ptr` 之间相互引用会导致内存无法释放）。



4. 简单模拟实现

(1) 模拟 `unique_ptr`

```cpp
template <typename T>
class UniquePtr {
private:
    T* ptr;
public:
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}
    ~UniquePtr() { delete ptr; }

    // 禁止拷贝
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // 允许移动
    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
};
```

### (2) 模拟 `shared_ptr`（引用计数）

```cpp
template <typename T>
class SharedPtr {
private:
    T* ptr;
    int* ref_count;

public:
    explicit SharedPtr(T* p = nullptr) : ptr(p), ref_count(new int(1)) {}

    ~SharedPtr() {
        if (--(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
    }

    SharedPtr(const SharedPtr& other) {
        ptr = other.ptr;
        ref_count = other.ref_count;
        ++(*ref_count);
    }

    SharedPtr& operator=(const SharedPtr& other) {
        if (this != &other) {
            if (--(*ref_count) == 0) {
                delete ptr;
                delete ref_count;
            }
            ptr = other.ptr;
            ref_count = other.ref_count;
            ++(*ref_count);
        }
        return *this;
    }

    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
};
```
