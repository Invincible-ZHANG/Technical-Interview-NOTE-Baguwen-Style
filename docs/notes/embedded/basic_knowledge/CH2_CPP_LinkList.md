---
layout: note
title: "链表C++"
date: 2025-07-28
excerpt: "Link List。"
categories: embedded
tags:
  - C++
  - 嵌入式
  - 基础知识
---


# c++链表

## 怎么理解链表
你现在有一个小纸条，上面写着一个抽屉的地址，那个抽屉里有一些你需要的东西，和一个新的写着地址的小纸条，这个小纸条又指向了一个新的抽屉，大体可以这么理解。

**程序所包含的头文件**

~~~
#include <iostream>
#include <cstdlib>

using namespace std;
~~~

当然如果要做随机顺序的链表的话

最好也包含ctime这个库

## 第一部分—构建抽屉

既然把装有东西和写有地址的小纸条比作抽屉那么我们不妨先写出抽屉的结构体。

~~~
typedef struct listpoint
{
    int data;
    listpoint *next;
}listpoint;
~~~

这就是一个最简单的结构体

int data,是一个数字，是我们存在抽屉里的东西

而listpoint *next是一个指向和这个抽屉结构一样的新的抽屉的指针；

我们可以在抽屉里放指向下一个抽屉的指针，自然也就可以在抽屉里放指向上一个抽屉的指针

~~~
typedef struct listpoint
{
    int data;
    listpoint *next;
    listpoint *last;
}listpoint;
~~~


我们在抽屉里不仅仅可以放一个数，我们可以往里面放一个收纳盒，例如，在下面的结构体中包含了另一个结构体

~~~
typedef struct data
{
    int number;
    string name;
    string sex;
}data;

typedef struct listpoint 
{
    data *information;
    listpoint *next;
    listpoint *list;
}listpoint;
~~~
那个叫做information的小收纳盒里，装着一个人的学号，姓名，性别等信息

---

### 第二部分—创建一个链表

**创建一个基础链表**

https://blog.csdn.net/slandarer/article/details/91863177


https://zhuanlan.zhihu.com/p/105749135

