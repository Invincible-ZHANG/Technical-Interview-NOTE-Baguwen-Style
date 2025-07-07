# 八股文笔记 📚

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE) [![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/) [![Languages](https://img.shields.io/badge/Langs-C%2B%2B%20%7C%20Python%20%7C%20CUDA-lightgrey)]()

> 嵌入式 · 软件开发 · GPU/CUDA · 算法

---

## 目录

* [简介](#简介)
* [快速开始](#快速开始)
* [核心特性](#核心特性)
* [项目结构](#项目结构)
* [示例](#示例)
* [定制与部署](#定制与部署)
* [贡献指南](#贡献指南)
* [许可证](#许可证)

---

## 简介

「八股文笔记」是一个面向嵌入式系统、多体动力学、求解器/优化算法以及 GPU/CUDA 并行计算的技术知识库，采用 **Markdown** 撰写，**Jekyll + GitHub Pages** 自动部署，支持本地预览与 CI/CD。

---

## 快速开始

```bash
# 克隆仓库
git clone https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style.git
cd Technical-Interview-NOTE-Baguwen-Style

# 本地启动（Jekyll）
cd docs
bundle install        # 如果第一次运行需安装依赖
bundle exec jekyll serve --port 4000
# 浏览器访问 http://localhost:4000
```

---

## 核心特性

* 🚀 **领域覆盖**：嵌入式开发 · 软件工程 · GPU/CUDA 加速 · 算法与数值求解
* ✍️ **Markdown 笔记**：统一格式、YAML Front Matter、自动生成导航和索引
* 🔄 **Jekyll 构建**：`docs/` 目录 + Minima 主题，自动渲染成静态网站
* 🔧 **GitHub Actions**：CI/CD 流水线，支持自动构建与部署到 GitHub Pages
* 📂 **Jekyll Collections**：按模块（embedded、software、gpu\_cuda、algorithms）组织内容

---

## 项目结构

```text
├── .github/               # Actions 流水线与模板
├── docs/                  # Jekyll 源码 & GitHub Pages 根
│   ├── _config.yml        # Jekyll 配置
│   ├── index.md           # 站点首页
│   ├── assets/            # 静态资源（CSS、图片）
│   ├── _embedded/         # 嵌入式笔记集合
│   ├── _software/         # 软件开发笔记集合
│   ├── _gpu_cuda/         # GPU/CUDA 笔记集合
│   ├── _algorithms/       # 算法笔记集合
│   ├── embedded/          # 嵌入式索引页
│   ├── software/          # 软件索引页
│   ├── gpu-cuda/          # GPU/CUDA 索引页
│   └── algorithms/        # 算法索引页
├── examples/              # 配套示例代码
├── notes/                 # 原始笔记（可选同步）
├── scripts/               # 构建、校验脚本
├── .gitignore             # 忽略项
├── LICENSE                # MIT 许可证
└── README.md              # 项目说明
```

---

## 示例 

* **在线浏览**： [GitHub Pages 站点](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/)
* **示例笔记**：嵌入式 [RTOS 基础](/embedded/rtos-basics.html)、GPU/CUDA [并行模型](/gpu-cuda/cuda-intro.html)

---

## 定制与部署

1. 修改 `docs/_config.yml` 进行主题或插件配置。
2. 更新或新增 Markdown 文件到对应集合目录 (`docs/_embedded`, `docs/_software` 等)。
3. 在 `.github/workflows/deploy.yml` 配置自动化构建（参考模板已有示例）。
4. 推送到 `main` 分支，GitHub Actions 会自动部署。

---

## 贡献指南

欢迎提交 **Issues** 讨论或 **Pull Requests** 改进：

1. Fork 本仓库
2. 创建新分支 `feature/xxx`
3. 提交改动并推送
4. 提交 Pull Request，描述你的改动内容

---

## 许可证

本项目遵循 [MIT License](LICENSE)。 欢迎自由使用、修改和分发，请保留版权声明。
