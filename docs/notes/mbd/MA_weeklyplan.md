---
title: MA周工作记录
date: 2025-07-26
layout: note
excerpt: 关于毕业设计每周任务同步以及在实现过程中的想法与设计思路，方便之后的追溯和毕业论文的编写。:)
---

*Tips：持续完善文档，记录每一次优化与踩坑体会，有助于后续论文撰写与项目迭代！*

*请点击每个链接，查看详细的过程*

**论文注册时间：01.08.2025**

# 毕业设计每周工作总结 

---

## 未来24周计划

- [x] 准备工作
  - [x] 学习C++
  - [x] 学习多体动力学仿真
- [ ] 英语演讲答辩能力
- [x] 理解文献和chrono参考代码(3)
- [x] 熟悉VEROSIM的求解器代码结构(1)
- [x] 实现QP的建模和Interior-Point简单求解(4)
- [ ] 应用APGD求解器(8)
  - [ ] 等式约束(hinge, prismatic, rigid, ball and socket)
  - [ ] contact and friction
  - [ ] Motor，limit及其它的约束类型
- [ ] 分析结果，优化算法(4)
  - [ ] 简单模型
  - [ ] 大规模模型
  - [ ] 实际模型(Harvester)
- [ ] 写论文(4)


## 目录



## [WEEK 03](./MA_WP_CH/WP03.md)   Time:26.05.2025 - 30.05.2025
- [x] APGD文献 （CCP → LCP →QP）
- [x] Chrono 源码 找到主函数以及运行流程

## [WEEK 04](./MA_WP_CH/WP04.md)   Time:02.06.2025 - 06.06.2025

- [x] 继续文献
- [x] 安装VEROSIM，学习建模，了解RBDynamX的求解器结构 （02.06）
- [x] APGD相关文献算法（初级和高级）
- [x] QP问题求解MBD(相关论文)怎么从LCP或CCP建模QP问题，QP问题如何被求解，等式约束和不等式约束
- [x] Chrono debug APGD 设置断点


## [WEEK 05](./MA_WP_CH/WP05.md)   Time:09.06.2025 - 13.06.2025

- [x] QP问题求解MBD(相关论文)怎么从LCP或CCP**建模QP问题**，QP问题如何被求解，等式约束和不等式约束
- [x] Chrono debug APGD 设置断点
  - [x] 详细看APGD算法，及相关函数


- [x] 学习VEROSIM建模，了解RBDynamX的求解器结构 （02.06）
- [x] SOCCP second order 正交圆锥


## [WEEK 06](./MA_WP_CH/WP06.md)   Time:16.06.2025 - 20.06.2025

- [x] 论文细节 关于APGD
- [x] 代码细节，以及代码结构
- [x] 了解RBDynamX的求解器结构 思考程序设计




## [WEEK 07](./MA_WP_CH/WP07.md)   Time:23.06.2025 - 27.06.2025

- [x] chrono代码实现细节，输入输出
- [x] VEROSIM代码实现细节
- [x] chrono中QP问题怎么被构建出来的 



## [WEEK 08](./MA_WP_CH/WP08.md)   Time:30.06.2025 - 04.07.2025

- [x] 实现QP建模
- [x] 实现APGD
- [x] CCP建模 chrono里给CCP建模，与VEROSIM中的LCP建模比较区别在哪



## [WEEK 09](./MA_WP_CH/WP09.md)   Time:06.07.2025 – 11.07.2025

- [ ] VEROSIM 实现 CCP / QP 建模  
- [x] 实现并集成 APGD 求解器



## [WEEK 10](./MA_WP_CH/WP10.md)   Time:14.07.2025 - 18.07.2025

- [x] Debug APGD求解器的自由落体问题
- [x] 测试其他的等式约束的关节



## [Week 11](./MA_WP_CH/WP11.md)   Time:21.06.2025 - 25.07.2025

- [x] 测试其他的等式约束的关节
- [x] 加入摩擦力，做这些测试，按道理来时没问题



## [Week 12](./MA_WP_CH/WP12.md)   Time:25.06.2025 - 01.08.2025

>**考证：我查了下 APGD在处理大规模碰撞问题时比PGS快一个数量级  ,有可能是因为CCP摩擦你还木有加进去  或者其他人用了并行运算**

> *情况说明：因为本周一和周二，个人出去玩请假两天，所以需要在周末补工作量，所以将周计划调整到周五到下周五*

- [x] 解决问题3个
  - [x] 1.为什么在迭代次数增加后反而会出现刚体弹飞的现象
  - [x] 2.为什么在加入摩擦力后，所有的碰撞算法是正常的，在没加入contact friction 的时候，第二个和第三个碰撞算法是不可行的。
  - [x] 3.求证是否可以微调参数，来增加实时性
- [x] motor velocity的模型测试
- [x] 关节内部的friction(另外三个)测试，不用作fix的
- [x] spring damping 测试（优先级低）
- [ ] APGD算法部分的论文
- [ ] 证明算法更好（100刚体），想办法，出图，数据罗列等，去证明这个问题。
- [ ] CCP转化QP建模(可以后续去做)