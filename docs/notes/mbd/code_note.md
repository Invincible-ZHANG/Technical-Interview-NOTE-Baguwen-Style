---
title: C++语法
date: 2025-07-23
layout: note
excerpt: 关于在进行代码实现时候涉及到的一些C++语法。:)
---


# C++语法

> *Tip: 都是些基础语法，有些我知道，有些我不知道，知道的也就不做强调了，不知道的会加粗。*


## 1.头文件保护（Include Guard）

~~~cpp
#ifndef CLASS_ODE_LCP_APGD
#define CLASS_ODE_LCP_APGD
…
#endif // CLASS_ODE_LCP_APGD
~~~

目的：防止头文件被多重包含导致重定义错误。

#ifndef 判断宏 CLASS_ODE_LCP_APGD 是否未定义；如果未定义，就定义它，并包含后续内容；最后用 #endif 结束。




