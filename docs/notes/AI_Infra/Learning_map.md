---
layout: note
title: "AI INFRA LEARNING MAP"
date: 2025-08-05
excerpt: "AI INFRA LEARNING MAP :)"
categories: AI
tags:
  - AI
---


> 下面是 GPT生成的关于AI INFRA的学习路径，我会在实际实施的过程中逐渐修改。



当然可以，张子健！以下是为你量身打造的：

---

# 🧭 AI Infra 学习路线图（2025版 · 嵌入式/系统背景定制）

> 🎯 **目标**：从 C++/嵌入式工程师出发，3-6 个月系统掌握 AI Infra 核心能力，具备秋招/内推/进组竞争力，方向聚焦**推理加速 + 算子优化 + 系统调度**。

---

## 📍 整体图谱（推荐收藏）

```
基础知识
├── C++ / Python 工程能力
├── 深度学习基础（PyTorch）
├── 算子原理（GEMM、Conv、Attention）
├── AI 模型格式（ONNX、IR）

推理加速
├── 模型压缩 & 量化（INT8、剪枝）
├── 推理框架：TensorRT / ONNXRuntime
├── 性能分析：Nsight、nvprof、perf

算子优化
├── CUDA kernel 编写（GEMM、Conv）
├── TVM schedule & AutoTune
├── 编译器体系（MLIR、XLA 简介）

系统层 Infra
├── Triton 推理服务框架
├── 分布式训练原理（FSDP、ZeRO）
├── GPU 资源调度（K8s + CUDA runtime）

实战项目
├── yolov5 INT8 加速部署
├── TVM 算子性能对比实验
├── 推理服务部署 & benchmark

补充：面试准备 & 开源参与
```

---

## 📘 阶段一：打基础（0-2 周）

### 🎯 目标：掌握 AI Infra 所需的基本技能 & 背景知识

| 内容                       | 推荐资源                                                   | 说明                       |
| ------------------------ | ------------------------------------------------------ | ------------------------ |
| C++ STL / 多线程 / Makefile | 牛客 / leetcode / 编译小项目                                  | 强化系统基础                   |
| Python + PyTorch 基础      | [深度之眼 PyTorch 入门](https://space.bilibili.com/85331466) | 用 PyTorch 跑 mnist、resnet |
| AI 模型格式                  | ONNX / TorchScript                                     | 学会模型导出与加载                |

**第一阶段笔记：**

 - [C++ STL](../embedded/basic_knowledge/CH3_STL.md)


---

## ⚡ 阶段二：推理加速精修（2-4 周）

### 🎯 目标：能完成 PyTorch → ONNX → TensorRT 的部署全流程

| 内容              | 推荐工具                   | 任务                      |
| --------------- | ---------------------- | ----------------------- |
| 模型导出流程          | `torch.onnx.export()`  | 将 resnet/yolov5 导出 ONNX |
| TensorRT 基础     | `trtexec` / Python API | 加速推理，调精度（FP16/INT8）     |
| ONNX Runtime 加速 | ONNXRuntime 官方文档       | 用 CPU / GPU 跑通 demo     |

✅ **重点实战项目**：

* yolov5 on TensorRT（INT8 加速）
* trtexec benchmark 参数调优
* 比较 PyTorch vs ONNX vs TensorRT 推理延迟

---

## ⚙️ 阶段三：算子优化进阶（4-6 周）

### 🎯 目标：能写算子 / 调度 / 优化调试

| 模块                     | 推荐学习资源                                            | 实战建议                        |
| ---------------------- | ------------------------------------------------- | --------------------------- |
| CUDA kernel 编写         | CUDA by Example / Nsight 教程                       | 实现 GEMM / Conv 算子           |
| TVM 编译 & schedule      | [TVM 官方教程](https://tvm.apache.org/docs/tutorial/) | 手写 tile/unroll/vectorize 调度 |
| AutoTVM / MetaSchedule | [官方调优示例](https://tvm.apache.org/docs/how_to/)     | 跑 AutoTuner，对比性能            |

✅ **重点实战项目**：

* GEMM C++ → CUDA → TVM schedule 三种实现对比
* 使用 AutoTVM 自动调优
* Nsight Compute 分析 kernel bottleneck

---

## 🔧 阶段四：系统层架构实践（6-8 周）

### 🎯 目标：理解 AI Infra 背后的服务/资源/平台逻辑

| 模块                      | 工具 / 框架                  | 实战建议              |
| ----------------------- | ------------------------ | ----------------- |
| Triton Inference Server | NVIDIA Triton + HTTP API | 实现多模型推理接口服务       |
| 分布式训练原理                 | FSDP / ZeRO / Megatron   | 理解参数/数据并行策略       |
| GPU 资源调度                | CUDA Stream / K8s 简介     | 学会在程序中调多个模型共享 GPU |

✅ **重点实战项目**：

* Triton 部署 yolov5 + bert 模型服务（REST 接口）
* 模拟 GPU 利用率对比（推理延迟 vs batch size）

---

## 🧪 阶段五：项目整合 + 开源参与（8-12 周）

### 🎯 目标：完成一到两个高质量项目，提升 GitHub、简历竞争力

| 项目方向 | 示例                                           |
| ---- | -------------------------------------------- |
| 算子优化 | GEMM 算子性能优化对比项目（支持 C++ / CUDA / TVM）         |
| 推理加速 | yolov5 INT8 TensorRT 部署 + Triton Server 接口服务 |
| 框架适配 | 阅读 vLLM 中调度模块，尝试改进缓存策略                       |
| 开源参与 | 给 SGLang、vLLM、TVM 提 PR（文档 / 实验 / 算子）         |

---

## 🧠 附加：AI Infra 岗位面试准备建议

| 模块   | 面试常问                                   | 建议准备             |
| ---- | -------------------------------------- | ---------------- |
| 模型知识 | Transformer, MoE, BERT                 | 看清华 LLM 教程       |
| 框架原理 | vLLM 架构、Paged Attention、Prefill/Decode | 看源码 & 画架构图       |
| 算子优化 | GEMM、Conv、INT8、共享内存优化                  | 会讲实现 + benchmark |
| 系统层  | Triton 架构 / K8s 调度 / 多模型服务             | 会画数据流、服务部署图      |
| 实战故事 | 为什么这样做？带来了多少提升？                        | 项目 STAR 框架答题法 ✅  |

---