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


