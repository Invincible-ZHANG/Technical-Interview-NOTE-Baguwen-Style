---
title: "Transformer"
layout: note
date: 2025-08-20 13:00:00 +0200
last_modified_at: 2025-08-20 13:00:00 +0200
author: "张子健"

categories: [Machine Learning, NLP]
tags: [Transformer, Attention, Deep Learning, NLP, PyTorch]

excerpt: "从注意力到编码器-解码器的全景梳理与实现要点"
description: "系统梳理 Transformer：Self-Attention、Multi-Head、位置编码、残差与规范化；含公式推导与 PyTorch 实现要点。"

permalink: /notes/transformer/
toc: true           # 主题支持目录时生效
math: true          # 主题/插件支持时启用数学公式
published: true     # false 可作为草稿不发布

---


## Reference

* [Transformer模型详解（图解最完整版）](https://zhuanlan.zhihu.com/p/338817680)
* [Attention is All You Need](./reference/transformer.pdf)

## Github项目

* [tensor2tensor/models/transformer.py](https://github.com/tensorflow/tensor2tensor/blob/master/tensor2tensor/models/transformer.py)
* [attention-is-all-you-need-pytorch](https://github.com/jadore801120/attention-is-all-you-need-pytorch?tab=readme-ov-file)