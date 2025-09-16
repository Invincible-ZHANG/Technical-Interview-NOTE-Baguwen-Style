---
layout: note
title: "AI INFRA LEARNING MAP"
date: 2025-08-05
excerpt: "AI INFRA LEARNING MAP :)"
categories: AI
tags:
  - AI
---

可以，而且别一下子猛堆课。给你一条**从零到能做项目**的清爽路线（先上手、再系统、最后专项到大模型/工程）。每个阶段都给出最强口碑课程（中英混排课程名保留原文），你照顺序学就行。

---

# 0. 预备（1 周）

* **Python 科学计算**：`NumPy → Pandas → Matplotlib`（B站/廖雪峰/CS231n作业的 NumPy 足够）
* **PyTorch 入门**：官方 60-min Blitz 或 Karpathy 的 *Neural Networks: Zero to Hero*（看 1–3 集会跑 MLP/CNN/RNN）
* **线性代数&微积分最少集**：矩阵乘法、特征向量/奇异值、链式法则、梯度下降；概率里把期望、方差、条件概率吃透即可

  > 只补用得到的；遇到不懂再回补，别在数学里“迷路”。

---

# 1. 快速上手（2–3 周）—先能跑起来

1. **《机器学习基础（吴恩达·Machine Learning Specialization）》**

   * 作用：把监督/无监督学习“地图”装脑子；理解偏差-方差、正则化、评估方法。
   * 作业：线性回归/LogReg/树模型/聚类都跑一遍，知道什么时候换模型。

2. **fast.ai · Practical Deep Learning for Coders**（第1–3课）

   * 作用：**从“能做”出发**，图像/NLP 任务拿预训练模型就能复现 SOTA 的 70–80%。
   * 作业：用你的数据做一个**小分类/分割**或文本情感分析；学会数据增广、冻结/解冻层、学习率搜寻。

---

# 2. 系统基础（3–4 周）—把“为什么”补上

3. **Stanford CS231n（Convolutional Neural Networks for Visual Recognition）**

   * 作用：从计算图、反向传播、自定义层，到CNN设计与正则化；作业质量极高。
   * 目标：独立实现一个**小型CNN**并解释每个超参的作用。

4. **NLP 基础（两选一）**

   * **李宏毅《生成式 AI》**（中文、直觉清晰）
   * **Stanford CS224n（NLP with Deep Learning）**（词向量→注意力→Transformer→训练技巧）
   * 目标：能读懂 Transformer 的层结构、注意力、位置编码；会写一个简化版的文本分类/序列标注。

---

# 3. 大语言模型专项（2–3 周）—把 LLM 玩明白

5. **Hands-On Large Language Models（O’Reilly）**（或 HuggingFace NLP Course 的 LLM 部分）

   * 作用：端到端搞定 **提示工程 → RAG → 微调/LoRA → 评测 → 部署**。
   * 作业：做一个你的**领域问答系统（RAG）**：用 Colab T4 + `transformers` + `faiss`/`chromadb`，评测用准确率/覆盖率/幻觉率。

6. **Build a Large Language Model (From Scratch)**（Sebastian Raschka）

   * 作用：把“黑盒”拆开：数据管线、分词、训练循环、采样策略、对齐（指令微调/RLHF）
   * 目标：能训练一个**迷你 GPT**，看懂 loss 曲线，知道梯度爆炸/衰减与优化器设置。

---

# 4. 工程化与加速（1–2 周）—做成可交付的东西

7. **DeepLearning.AI · MLOps/LLMOps 短课** 或 **Made With ML**

   * 作用：数据版本、实验跟踪、可复现、评测基线、上线监控。
8. **加速与部署**

   * PyTorch AMP 混合精度、KV-cache、批量化；推理框架（vLLM / TensorRT-LLM）；轻量化（量化、蒸馏、LoRA/QLoRA）。

---

## 12 周极简学习表（含可交付里程碑）

* **第 1–2 周**：吴恩达专精 + Karpathy 前几节 → 交付：线性回归/LogReg + 一个简单图像/文本分类 Notebook
* **第 3–4 周**：fast.ai 前半 → 交付：复现一个 Kaggle 小竞赛 Top 25%
* **第 5–7 周**：CS231n + CS224n → 交付：从零实现简化 CNN/注意力模块（PyTorch），写一页技术说明
* **第 8–10 周**：Hands-On LLMs → 交付：**RAG 小系统**（检索/重排/生成/评测），跑在 Colab T4
* **第 11–12 周**：Raschka from-scratch + 部署 → 交付：迷你 GPT 训练报告 + 在线 Demo（Gradio/Streamlit）

---

## 工具栈清单（直接抄）

* **环境**：Python 3.10+，Jupyter/VSCode；`conda` 或 `uv` 管理环境
* **核心框架**：`pytorch`，`transformers`，`datasets`，`accelerate`
* **检索/RAG**：`faiss` / `chromadb`，`sentence-transformers`
* **可视化/实验管理**：`matplotlib`/`wandb`
* **部署**：`gradio`/`streamlit`，可选 `vllm`/`tensorrt-llm`
* **GPU**：Colab 选 **T4** 足够入门；显存不够先用 **LoRA/QLoRA** 与 4bit 量化

---

## 学习原则（防走弯路）

* **先成事，后完美**：每个阶段都要有“能跑的作品”；遇到数学卡点再回补。
* **小数据先行**：用玩具数据把训练-评测-部署串起来，再换真数据。
* **记录实验**：固定随机种子 + 保存配置 + 一页结果总结，形成“科研/工程闭环思维”。

如果你告诉我更偏向**理论/应用/系统工程**哪条，我直接把上面的清单缩成“**3 门课 + 1 个项目**”的极速路径，并给你一份 Colab 可直接跑的起步模板。
