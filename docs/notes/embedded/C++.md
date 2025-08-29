---
layout: note
title: "C++"
date: 2025-07-26
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

