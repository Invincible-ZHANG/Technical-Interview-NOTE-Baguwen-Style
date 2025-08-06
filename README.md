中文 | [English](./README_EN.md)
# 个人博客 📚

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE) [![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/) [![Languages](https://img.shields.io/badge/Langs-C%2B%2B%20%7C%20Python%20%7C%20CUDA-lightgrey)]()

> 自己的随手写

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

「个人Blog」是一个面向嵌入式系统、多体动力学、求解器/优化算法以及 GPU/CUDA 并行计算的技术知识库，采用 **Markdown** 撰写，**Jekyll + GitHub Pages** 自动部署，支持本地预览与 CI/CD。

---

## 快速开始

```bash
# 克隆仓库
git clone https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style.git
cd Technical-Interview-NOTE-Baguwen-Style

# 本地启动（Jekyll）
'Start Command Prompt with Ruby'

cd docs
bundle install        # 如果第一次运行需安装依赖
bundle exec jekyll serve --port 4000
# 浏览器访问 http://localhost:4000

# 代码速记
git add .
git commit -m ""
git push

# 安装依赖（仅第一次或有更新时）
bundle install
# 本地部署
bundle exec jekyll build

bundle exec jekyll serve  #尽量用这个，可以实时看自己改的结果

```

---
## 更新日志

详见 [CHANGELOG](CHANGELOG.md)。

---

## 核心特性
🚀 领域覆盖：嵌入式开发 · 软件工程 · GPU/CUDA 加速 · 算法与数值求解

✍️ Markdown 笔记：统一格式、YAML Front Matter、自动生成导航和索引

🔄 Jekyll 构建：docs/ 目录 + Minima 主题 + 自定义 home.html 布局

🔧 GitHub Actions：自动化构建与部署到 GitHub Pages

📦 Docker 预览：无需本地 Ruby 环境，容器中一键启动

🔗 名言模块：Hitokoto API 动态加载并附带出处链接

📱 响应式布局：手机/平板/PC 三档自适应

---

## 项目结构

```text
## 项目结构

```text
.
├── .github
│   └── workflows/             # GitHub Actions CI/CD 配置
├── .vs
│   └── baguwen-notes/v16/     # Visual Studio 配置文件
├── docs/                      # 静态网站源码（GitHub Pages）
│   ├── assets/                # 公共资源
│   │   ├── css/               # 样式表
│   │   ├── images/            # 图片
│   │   └── js/                # 脚本
│   ├── notes/                 # 各类笔记
│   │   ├── AI_Infra/AI_Learning_Map/   # AI 基础设施学习地图
│   │   ├── algorithms/Leetcode/        # 算法与 LeetCode 刷题
│   │   ├── embedded/basic_knowledge/images/  # 嵌入式基础与配套图片
│   │   ├── English/                     # 英语相关笔记
│   │   ├── Fall_Reviews/Hide/           # 秋招复习与隐藏内容
│   │   ├── gpu_cuda/                    # GPU 与 CUDA 学习
│   │   ├── LLM/llm_content/             # 大语言模型相关
│   │   ├── mbd/                         # MBD 系列笔记
│   │   │   ├── MA_weeklyplan_image/  
│   │   │   ├── MA_WP_CH/            
│   │   │   └── 论文草稿/             
│   │   ├── travel/image/                # 旅行笔记及图片
│   │   └── web/                         # Web 开发相关
│   ├── _data/                  # Jekyll 数据文件
│   ├── _includes/              # Jekyll 可重用片段
│   └── _layouts/               # Jekyll 布局模板
├── examples/                  # 示例代码项目
│   ├── ai-infra-pytorch-demo/ # PyTorch AI 基础设施示例
│   ├── algorithms/            # 算法示例
│   ├── embedded/              # 嵌入式示例
│   ├── gpu-cuda/              # GPU/CUDA 示例
│   └── software/              # 其他软件示例
├── resume/                    # 简历文件
└── scripts/                   # 辅助脚本

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

## 动画主页设计说明（WebGL-Fluid 首页）

本项目集成了一个现代动态主页，作为笔记导航的视觉入口，主页具有如下特点：

🎨 **WebGL 动态背景**：使用 [WebGL-Fluid-Simulation](https://github.com/PavelDoGreat/WebGL-Fluid-Simulation/) 实现流体互动动画，具有强烈科技感。

🧩 **响应式布局 + 动效切换**：适配手机与 PC 端，导航区采用模块化配置，可自定义图标与跳转链接。

🔧 **纯静态部署**：主页构建后产出纯 HTML/CSS/JS，可部署在 GitHub Pages 或任意静态服务器。

📁 **主站入口地址**：[https://invincible-zhang.github.io/blog](https://invincible-zhang.github.io/blog)

---
## 未来展望

1. **多语言支持**：计划增加英文版，覆盖更广泛的读者群体。
2. 微信小程序：考虑开发微信小程序版，方便在移动端访问。
3. **更多主题**：增加更多技术领域的笔记集合，如机器学习、数据科学等。
4. 优化设计：持续改进主页设计与用户体验，增加更多互动元素。


---

## 致谢 SimonAKing 🙏

主页设计参考并改编自 [SimonAKing/HomePage](https://github.com/SimonAKing/HomePage) 项目，该项目原始仓库结构清晰、界面优雅，是国内极具美感的开源主页设计之一。

原始作者仓库中文文档：[README.zh_CN.md](https://github.com/SimonAKing/HomePage/blob/master/README.zh_CN.md)

在此向原作者 SimonAKing 致以诚挚感谢 🙇！


---


## 💬 评论区

本项目已启用 [GitHub Discussions](https://docs.github.com/en/discussions) + [giscus](https://giscus.app/) 评论系统：  
- **查看/发表评论**：打开任意文章，滚动到页面最底部即可。  
- **快速安装指南**：https://giscus.app/（Configure → 选择本仓库 → 选择 “Comments” 分类 → 复制 `<script>` → 粘到文章模板 `{{ content }}` 之后）  

感谢你的支持与反馈！  

---


## 许可证

本项目遵循 [MIT License](LICENSE)。 欢迎自由使用、修改和分发，请保留版权声明。

---
