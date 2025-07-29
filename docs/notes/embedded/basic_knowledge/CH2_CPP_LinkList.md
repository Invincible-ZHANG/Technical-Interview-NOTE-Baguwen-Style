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
typedef struct listnode
{
    int data;
    listpoint *next;
}listnode;
~~~

这就是一个最简单的结构体

int data,是一个数字，是我们存在抽屉里的东西

而listpoint *next是一个指向和这个抽屉结构一样的新的抽屉的指针；

我们可以在抽屉里放指向下一个抽屉的指针，自然也就可以在抽屉里放指向上一个抽屉的指针

~~~
typedef struct listnode
{
    int data;
    listpoint *next;
    listpoint *last;
}listnode;
~~~


我们在抽屉里不仅仅可以放一个数，我们可以往里面放一个收纳盒，例如，在下面的结构体中包含了另一个结构体

~~~
typedef struct data
{
    int number;
    string name;
    string sex;
}data;

typedef struct listnode 
{
    data *information;
    listpoint *next;
    listpoint *list;
}listnode;
~~~
那个叫做information的小收纳盒里，装着一个人的学号，姓名，性别等信息

---

### 第二部分—创建一个链表

**创建一个基础链表**
~~~
listnode* create_normal_list(int n) /*链表每一个节点都是指向  listpoint结构的指针，所以返回值是listpoint *,n是指创建的链表的节点数目*/
{
    listnode *head, *normal, *end;/*创建头节点，其他节点，和尾节点*/
    /*分配内存*/
    head = (listnode*)malloc(sizeof(listnode*));
    head->information = (data*)malloc(sizeof(data));
    end = head;/*最开始最后一个节点就是头节点，注意因为通过指针可以直接对地址上的东西进行操作，此时end和head指向同一个地址，对end所指向地址进行操作，等同于对head地址所做的操作*/
    for (int i=0;i < n;i++)
    {
        /*给新节点分配内存*/
        normal = (listnode*)malloc(sizeof(listnode*));
        normal->information = (data*)malloc(sizeof(data));
        /* 往新节点存入数据，注意我们只给后面的节点存入数据，head不存数据*/
        cout<<"input the number : ";
        cin>>normal->information->number;
        cout<<"input the name : ";
        cin>>normal->information->name;
        cout<<"input the sex : ";
        cin>>normal->information->sex;
        cou<<"-----------------------------"<<endl;
        end->next = normal;/*往end后增添新节点*/
        normal->last = end;/*新节点的上一个节点就是end*/
        end = normal; /*最后一个节点变成新节点*/

    }
    end->next = NULL;/*链表的最后指向一个新地址*/
    head->last = NULL;/*链表最开始的节点没有上一个节点*/
    return head;
}
~~~

**创建环状链表**

操作和之前一样，只不过最后一个节点的下一个指向头节点

~~~
listnode* create_normal_list(int n) /*链表每一个节点都是指向  listpoint结构的指针，所以返回值是listpoint *,n是指创建的链表的节点数目*/
{
    listnode *head, *normal, *end;/*创建头节点，其他节点，和尾节点*/
    /*分配内存*/
    head = (listnode*)malloc(sizeof(listnode*));
    head->information = (data*)malloc(sizeof(data));
    end = head;/*最开始最后一个节点就是头节点，注意因为通过指针可以直接对地址上的东西进行操作，此时end和head指向同一个地址，对end所指向地址进行操作，等同于对head地址所做的操作*/
    for (int i=0;i < n;i++)
    {
        /*给新节点分配内存*/
        normal = (listnode*)malloc(sizeof(listnode*));
        normal->information = (data*)malloc(sizeof(data));
        /* 往新节点存入数据，注意我们只给后面的节点存入数据，head不存数据*/
        cout<<"input the number : ";
        cin>>normal->information->number;
        cout<<"input the name : ";
        cin>>normal->information->name;
        cout<<"input the sex : ";
        cin>>normal->information->sex;
        cou<<"-----------------------------"<<endl;
        end->next = normal;/*往end后增添新节点*/
        normal->last = end;/*新节点的上一个节点就是end*/
        end = normal; /*最后一个节点变成新节点*/

    }
    end->next = head;/*链表的最后指向一个新地址*/
    head->last = end;/*链表最开始的节点没有上一个节点*/
    return head;
}
~~~

**创建随机枝杈链表**

**生成随机排序链表**

同可以最后再看

先生成正常顺序链表，再从最后n个节点中随机选择一个，将其剔除并插入到第一个节点的位置，然后再从最后n-1个节点中随机选择一个，剔除后插入第二个节点位置，以此类推



---
### 第三部分—修改链表

修改数据，因为我们通过指针可以直接修改地址储存的信息，所以函数并不需要返回值

~~~
void change_point(listnode* list,int n,data* newInfo)
{
    //为了不破坏表头，需要新建一个节点
    listnode* p;
    p = list;
    
    for(int i=0;i < n; i++)
    {
        p = p->next;
    }
    
    p->information = newInfo;
}
~~~


删除节点
~~~

~~~
https://blog.csdn.net/slandarer/article/details/91863177


https://zhuanlan.zhihu.com/p/105749135