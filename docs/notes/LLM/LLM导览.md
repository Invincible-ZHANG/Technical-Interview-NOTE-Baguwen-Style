---
title: 大模型学习笔记导览
date: 2025-07-18
layout: note
excerpt: 本笔记主要来源于GITHUB的开源博主的笔记，在此基础上添加自己的见解和想法。:)
---











# 大模型基础导览

> 特别鸣谢： 感谢 GitHub 开源项目[so-large-lm](https://github.com/datawhalechina/so-large-lm.git)精彩分享，为本笔记提供了宝贵的参考和启发。


## 笔记来源
大部分基于别人已经开源的笔记进行学习，并且适当的增加了个人的想法，以及笔记的补充，如涉及侵权问题，请联系本人。

## 项目规划

**目录**
1. [引言](./llm_content/ch01.md)
    - 项目目标：目前对大规模预训练语言模型的相关知识的重点讲解
    - 项目背景：GPT-3等大型语言模型的出现，以及相关领域研究的发展
2. [大模型的能力](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch02.md)
    - 模型适应转换：大模型预训练往下游任务迁移
    - 模型性能评估：基于多个任务对GPT-3模型进行评估和分析
3. [模型架构](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch03.md)
    - 模型结构：研究和实现RNN, Transformer等网络结构
    - Transformer各层细节：从位置信息编码到注意力机制
4. [新的模型架构](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch04.md)
    - 混合专家模型（MoE）
    - 基于检索的模型
5. [大模型的数据](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch05.md)
    - 数据收集：从公开数据集中获取训练和评估所需数据，如The Pile数据集
    - 数据预处理：数据清洗、分词等
6. [模型训练](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch06.md)
    - 目标函数：大模型的训练方法
    - 优化算法：模型训练所使用的优化算法
7. [大模型之Adaptation](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch07.md)
    - 讨论为什么需要Adaptation
    - 当前主流的Adaptation方法（Probing/微调/高效微调） 
8. [分布式训练](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch08.md)
    - 为什么需要分布式训练
    - 常见的并行策略：数据并行、模型并行、流水线并行、混合并行
9. [大模型的有害性-上](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch09.md)
    - 模型性能差异：预训练或数据处理影响大模型性能
    - 社会偏见：模型表现出的显性的社会偏见
10. [大模型的有害性-下](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch10.md)
    - 模型有害信息：模型有毒信息的情况
    - 模型虚假信息：大模型的虚假信息情况
11. [大模型法律](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch11.md)
    - 新技术引发的司法挑战：司法随着新技术的出现而不断完善
    - 过去司法案例汇总：过去案例的汇总
12. [环境影响](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch12.md)
    - 了解大语言模型对环境的影响
    - 估算模型训练产生的排放量
13. [智能体（Agent）](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch13.md)
    - 了解Agent各组件细节
    - Agent的挑战与机遇
14. [Llama开源家族：从Llama-1到Llama-3](https://github.com/datawhalechina/so-large-lm/blob/main/docs/content/ch14.md)
    - Llama进化史（第1节）/ 模型架构（第2节）/训练数据（第3节）/训练方法（第4节）/效果对比（第5节）/社区生态（第6节）
