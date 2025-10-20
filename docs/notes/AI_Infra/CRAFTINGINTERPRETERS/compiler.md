---
title: compiler(编译器)
date: 2025-10-18
excerpt: "compiler(编译器)学习。"
layout: note
---

## 入门（循序渐进造语言）

* **Crafting Interpreters**（先解释器、再字节码虚拟机，讲解极清楚，免费在线）([craftinginterpreters.com][1])
* **Stanford CS143（COOL 语言全栈编译器实验）**：词法/语法/语义/代码生成一步步做下来，有讲义与工具链。([Stanford University][2])
* **MIT 6.035（Computer Language Engineering）**：系统化讲义，覆盖 IR、数据流分析、寄存器分配等。([MIT OpenCourseWare][3])
* **CMU 15-411 Compiler Design**：课程主页含讲义/项目说明，偏工程与优化思维。([cs.cmu.edu][4])

## 工程化练手（跟着做一个 C 编译器）

* **chibicc 项目 & 作者文章**：从零实现能自举的 C11 编译器（可编 Git/SQLite），源码精炼、讲解实在。([GitHub][5])

## LLVM/MLIR 正统路线（工业级工具链）

* **Kaleidoscope 官方教程**：从词法/语法到 IR 生成、优化、JIT/目标文件，最适合第一次用 LLVM。([llvm.org][6])
* **写一个 LLVM Pass（新 PM）**：官方文档与社区 step-by-step 教程，学会插入你自己的优化/分析。([llvm.org][7])
* **MLIR 入门 & Toy Tutorial**：当你需要多层 IR、做 DSL/AI 编译（图算子等），官方“Getting Started + Tutorials”。([mlir.llvm.org][8])

## 备选后端（更轻、更易读）

* **Cranelift**（Rust 实现，适合做 JIT/AOT，API 友好；有 JIT demo 与设计文章）([cranelift.dev][9])
* **QBE**（极简后端，文档短小精悍，自带迷你 C 前端 minic/，非常适合学习代码生成）([c9x.me][10])

---

#### 12 周实战路线（可直接照抄执行）

**第 1–2 周**：读完 *Crafting Interpreters* 上半部并写出词法/语法/AST；并行看 CS143 的 PA2/PA3 作业结构（词法/语法）。([craftinginterpreters.com][1])
**第 3–4 周**：做语义分析（作用域、类型检查），实现简单 IR（或直接上 LLVM IR 生成，按 Kaleidoscope 第 1–3 篇）。([llvm.org][6])
**第 5–6 周**：把 IR 喂给后端（先 JIT 再目标文件），接上优化 Pass（Dead Code、Mem2Reg），跑 `opt/llc/lli`。([llvm.org][6])
**第 7–8 周**：跟官方文档写一个**自己的 Pass**（新 PM），做一个可见成效的小优化（如常量折叠统计/循环不变外提）。([llvm.org][7])
**第 9–10 周**：选一条“轻量后端”支线：

* 方案 A：用 **Cranelift** 做一个玩具语言 JIT（抄 demo 起步，改语义）。([GitHub][11])
* 方案 B：用 **QBE** 的 IR 做代码生成（读 docs 与 minic/，先把表达式/控制流编译出来）。([c9x.me][12])
  **第 11–12 周**：如果做 AI/数值 DSL，转向 **MLIR Toy 教程**，感受多层 IR + Dialect；否则补完前端错误恢复、测试与基准。([mlir.llvm.org][13])

---

#### 实操清单（马上能做）

1. 跑 Kaleidoscope 到“JIT + 优化”那一节，观察 IR 变化。([llvm.org][6])
2. 用“Writing an LLVM Pass（新 PM）”模版，新建一个 `-count-mul` Pass，统计乘法出现次数，学完整构建/加载/测试流程。([llvm.org][7])
3. 选 chibicc 或 QBE minic，照着把表达式/控制流/函数调用打通一遍。([GitHub][5])
4. 想做 JIT？抄 Cranelift JIT demo 跑起来，再替换成你自己的语法前端。([GitHub][11])

---


[1]: https://craftinginterpreters.com/?utm_source=chatgpt.com "Crafting Interpreters"
[2]: https://web.stanford.edu/class/cs143/?utm_source=chatgpt.com "CS143: Compilers"
[3]: https://ocw.mit.edu/courses/6-035-computer-language-engineering-spring-2010/pages/lecture-notes/?utm_source=chatgpt.com "Lecture Notes | Computer Language Engineering"
[4]: https://www.cs.cmu.edu/~janh/courses/411/17/?utm_source=chatgpt.com "15-411 Compiler Design"
[5]: https://github.com/rui314/chibicc?utm_source=chatgpt.com "rui314/chibicc: A small C compiler"
[6]: https://llvm.org/docs/tutorial/?utm_source=chatgpt.com "LLVM Tutorial: Table of Contents"
[7]: https://llvm.org/docs/WritingAnLLVMNewPMPass.html?utm_source=chatgpt.com "Writing an LLVM Pass — LLVM 21.0.0git documentation"
[8]: https://mlir.llvm.org/getting_started/?utm_source=chatgpt.com "Getting Started"
[9]: https://cranelift.dev/?utm_source=chatgpt.com "Cranelift"
[10]: https://c9x.me/compile/?utm_source=chatgpt.com "QBE - Compiler Backend"
[11]: https://github.com/bytecodealliance/cranelift-jit-demo?utm_source=chatgpt.com "JIT compiler and runtime for a toy language, using Cranelift"
[12]: https://c9x.me/compile/docs.html?utm_source=chatgpt.com "QBE - Compiler Backend"
[13]: https://mlir.llvm.org/docs/Tutorials/?utm_source=chatgpt.com "Tutorials"




