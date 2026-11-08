中文 | [English](./README_EN.md)
# K1n · 摄影与成像笔记 📷

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE) [![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://www.zhangzijian.cn/) [![Type](https://img.shields.io/badge/Site-Photography-lightgrey)]()

> 自己的随手写

---

## 目录

* [简介](#简介)
* [快速开始](#快速开始)
* [核心特性](#核心特性)
* [项目结构](#项目结构)
* [示例](#示例)
* [定制与部署](#定制与部署)
* [小K 站内助手](#小k-站内助手)
* [贡献指南](#贡献指南)
* [许可证](#许可证)

---

## 简介

个人站点，主题是摄影。`/gallery/` 是照片墙（瀑布流 + 灯箱 + 按系列筛选），`/log/` 是影像笔记，分成两路：「Sensor 与成像」记工作里攒下的成像技术，「摄影美学」记构图、光线和审美的练习；`/life/` 放旅行记录，`/notes/` 把过去几年写的技术笔记整理成归档。采用 **Markdown** 撰写，**Jekyll + GitHub Pages** 自动部署，支持本地一键构建与预览。

照片走一套轻量的静态流程：原图登记在 `docs/_tools/make_photos.py`，跑一次脚本生成 480 / 960 / 1800 三档宽度，元数据写在 `docs/_data/photos.yml`，页面自动渲染，不用手拼 HTML。

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
📷 照片墙：瀑布流布局、全屏灯箱（键盘 / 滑动翻页）、按系列筛选

✍️ 三种内容形态：影像笔记 `/log/`（Sensor 与成像 / 摄影美学）、旅记 `/life/`、旧笔记归档 `/notes/`
🎮 游戏栏目 `/game/`：攻略与打法，带 Leaflet 互动地图（可拖拽缩放，机密 / 绝密两套数据）

🔍 纯前端搜索：`search.json` 静态索引，无后端依赖

🤖 小K 智能助手：右下角悬浮球，接自己的 Key（DeepSeek / Kimi / 智谱 / 通义 / 硅基流动 / OpenAI），流式回答并检索站内文章

🌙 暗色暖金主题：手写 CSS，无 UI 框架，支持自适应与动效降级

🖼️ 图片流程脚本化：`_tools/make_photos.py` 一键生成多档尺寸

🔧 GitHub Actions：自动化构建与部署到 GitHub Pages

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
│   ├── gallery/                # 摄影页（照片墙 / 灯箱）
│   ├── log/                    # 影像笔记（总览）
│   │   ├── sensor/             #   └ Sensor 与成像：成像链路、ISP、驱动侧积累
│   │   └── aesthetic/          #   └ 摄影美学：构图、光线、审美练习
│   ├── life/                   # 旅记与生活记录
│   ├── assets/photos/          # 由脚本生成的网页尺寸照片
│   ├── _tools/                 # 图片处理脚本（make_photos.py）
│   ├── _data/                  # Jekyll 数据文件（profile / photos / navigation）
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

* **在线浏览**： [GitHub Pages 站点](https://www.zhangzijian.cn/)
* **示例笔记**：嵌入式 [RTOS 基础](/embedded/rtos-basics.html)、GPU/CUDA [并行模型](/gpu-cuda/cuda-intro.html)

---

## 定制与部署

1. 修改 `docs/_config.yml` 进行主题或插件配置。
2. 更新或新增 Markdown 文件到对应集合目录 (`docs/_embedded`, `docs/_software` 等)。
3. 在 `.github/workflows/deploy.yml` 配置自动化构建（参考模板已有示例）。
4. 推送到 `main` 分支，GitHub Actions 会自动部署。

---

## 小K 站内助手

右下角那颗金球就是它。任意页面按 `Alt + K` 唤起，也可以直接开独立页 `/assistant/`。设计上跟站里一套令牌（暖金 + 玻璃），没有引入任何 UI 框架或第三方脚本。

**接入方式**：静态站没有后端，所以走「浏览器直连 + 访客自带 Key（BYOK）」。统一用各家 OpenAI 兼容的 `/chat/completions`，已内置：

| 服务商 | 默认模型 | 申请 Key |
| --- | --- | --- |
| DeepSeek | `deepseek-chat` | platform.deepseek.com |
| Kimi（月之暗面） | `kimi-latest` | platform.moonshot.cn |
| 智谱 GLM | `glm-4.5` | open.bigmodel.cn |
| 通义千问（百炼） | `qwen-plus` | bailian.console.aliyun.com |
| 硅基流动 | `deepseek-ai/DeepSeek-V3` | cloud.siliconflow.cn |
| OpenAI | `gpt-4o-mini` | platform.openai.com |
| 自定义 | 手填 | 中转 / Ollama（`http://localhost:11434/v1`） |

模型名可手填，服务商出新模型不用改代码。填完点「测试连接」，会打一次真实请求报时延。

**改文案与默认值**：`docs/_data/assistant.yml` —— 名字、开场白、推荐问题、默认服务商/模型、历史条数、温度都在这里，不用碰 JS。

**关于 Key 的安全**：Key 只写进访客自己浏览器的 `localStorage`，直接从浏览器发给你选的服务商，不经过本站服务器。所以请不要把 Key 填进 `assistant.yml`（本站是公开仓库，填了等于公开发布）；需要「开箱即用」就自建一个转发（Cloudflare Worker 之类），把地址写进 `proxy_url`，浏览器就不会接触 Key 了。

**CORS 提醒**：部分服务商不允许浏览器直连（表现为控制台 `Failed to fetch`）。遇到就换 DeepSeek / 硅基流动，或走上面的代理。

**站内检索**：每次提问前会读一遍 `search.json`，用关键词打分取前 3 篇摘要塞进 system prompt，回答里引用的文章会在气泡下方列成「站内」小标签。只在设置里关掉「检索站内文章」就不再读索引。

相关文件：`_data/assistant.yml`、`_includes/assistant.html`、`assets/css/assistant.css`、`assets/js/assistant.js`、`assistant/index.md`。

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

📁 **主站入口地址**：[https://www.zhangzijian.cn/](https://www.zhangzijian.cn/)

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
