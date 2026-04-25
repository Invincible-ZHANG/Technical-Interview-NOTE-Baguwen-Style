中文 | [English](./README_EN.md)

# K1n's Blog 📚

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/)
[![VitePress](https://img.shields.io/badge/VitePress-1.x-3eaf7c)](https://vitepress.dev/)
[![Vue](https://img.shields.io/badge/Vue-3-42b883)](https://vuejs.org/)
[![Node](https://img.shields.io/badge/Node-%E2%89%A518-339933)](https://nodejs.org/)

> 个人技术笔记站 · 嵌入式 · MBD · GPU/CUDA · AI Infra · 算法

---

## 目录

- [简介](#简介)
- [技术栈](#技术栈)
- [快速开始](#快速开始)
- [核心特性](#核心特性)
- [项目结构](#项目结构)
- [写作约定](#写作约定)
- [部署](#部署)
- [迁移说明](#迁移说明)
- [贡献指南](#贡献指南)
- [评论区](#评论区)
- [致谢](#致谢)
- [许可证](#许可证)

---

## 简介

「K1n's Blog」是一个面向 **嵌入式系统、多体动力学（MBD）、求解器与数值优化、GPU/CUDA 并行计算、AI Infra、算法** 的个人技术笔记站。

内容采用 Markdown 撰写，站点通过 **VitePress + Vue 3 + Vite** 构建，部署到 **GitHub Pages**。本地开发支持热更新，构建产物为纯静态文件。

> 本仓库已于 2026 年从旧版 Jekyll（Ruby）方案重构为现代前端方案，详见 [迁移说明](#迁移说明)。

---

## 技术栈

| 方面 | 选型 |
| --- | --- |
| 静态站点生成 | [VitePress](https://vitepress.dev/) 1.x |
| 框架 | Vue 3 |
| 构建工具 | Vite |
| 语言 | Markdown + TypeScript |
| 数学公式 | MathJax 3（`markdown-it-mathjax3`） |
| 评论 | giscus + GitHub Discussions |
| 站点统计 | Google Analytics 4 |
| 运行时 | Node.js ≥ 18（推荐 20 LTS） |
| 部署 | GitHub Actions → GitHub Pages |

---

## 快速开始

### 1. 安装 Node.js

前往 [nodejs.org](https://nodejs.org/) 安装 LTS 版本（推荐 20.x），然后在 PowerShell / 终端验证：

```bash
node -v
npm -v
```

> Windows 用户如果 `npm -v` 报 "禁止运行脚本"，用管理员身份执行一次：
> `Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned`

### 2. 克隆并启动

```bash
git clone https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style.git
cd Technical-Interview-NOTE-Baguwen-Style

npm install
npm run dev
```

浏览器打开 <http://localhost:5173> 即可。改 Markdown 保存后会热更新。

### 3. 常用脚本

| 命令 | 作用 |
| ---- | ---- |
| `npm run dev` | 启动开发服务器（带热更新） |
| `npm run build` | 构建生产静态站点到 `docs/.vitepress/dist/` |
| `npm run preview` | 本地预览构建产物 |

更详细的环境配置、常见问题、giscus / 自定义域名设置见 [ENVIRONMENT_SETUP.md](./ENVIRONMENT_SETUP.md)。

---

## 核心特性

- 🚀 **领域覆盖**：嵌入式 · 软件工程 · MBD · GPU/CUDA · AI Infra · LLM · 算法 · 秋招
- ✍️ **Markdown 优先**：所有内容用 Markdown 写，YAML Front Matter 维护元信息
- 🧭 **自动侧边栏**：基于 `docs/notes/**` 目录结构和 frontmatter 标题自动生成，新增笔记无需改配置
- 🔍 **本地全文搜索**：右上角搜索框，`Ctrl+K` 快捷键唤起
- 🌗 **暗色模式**：自动跟随系统，单独调过夜间色板
- ➗ **数学公式**：原生支持 `$...$`（行内）和 `$$...$$`（块级）LaTeX 语法
- 💬 **giscus 评论**：每篇正文末尾，自动跟随主题色切换
- 📈 **GA4 统计**：内置 Google Analytics 4
- ✏️ **GitHub 编辑链接**：每页底部「在 GitHub 上编辑此页」直跳源文件
- ⏱ **最后更新时间**：基于 git 提交自动注入
- 🎨 **现代品牌色**：紫蓝粉渐变，玻璃磨砂导航栏，卡片悬浮微动效

---

## 项目结构

```text
.
├── package.json                    # Node 依赖与构建脚本
├── docs/                           # VitePress 站点源（srcDir）
│   ├── index.md                    # 首页（Hero + Features）
│   ├── public/                     # 静态资源（直接拷到根路径）
│   │   ├── favicon.svg
│   │   ├── logo.svg
│   │   └── hero.svg
│   ├── notes/                      # 全部笔记内容
│   │   ├── mbd/                    # 多体动力学、求解器、并行化
│   │   ├── software/               # C/C++、嵌入式、设备树、RTOS
│   │   ├── algorithms/             # LeetCode、DP、BFS、树
│   │   ├── gpu_cuda/               # CUDA、并行计算
│   │   ├── AI_Infra/               # AI 基础设施、科学计算
│   │   ├── LLM/                    # Transformer、LLM 导览
│   │   ├── Linux/                  # 内核、系统编程
│   │   ├── Fall_Reviews/           # 秋招面经、复盘
│   │   ├── English/                # 英语学习
│   │   ├── travel/                 # 旅行记录
│   │   └── web/                    # Web 杂记
│   └── .vitepress/
│       ├── config.mts              # 站点主配置
│       ├── utils/
│       │   └── sidebar.mjs         # 自动扫描目录、生成侧边栏 / 导航
│       └── theme/
│           ├── index.ts            # 主题入口（扩展默认主题）
│           ├── styles/
│           │   ├── vars.css        # 品牌色与暗色变量
│           │   └── custom.css      # 玻璃导航栏、卡片 hover、callout
│           └── components/
│               ├── Giscus.vue              # 评论组件
│               └── HomeFeatureExtras.vue   # 首页补充板块
├── .github/
│   └── workflows/deploy.yml        # GitHub Pages 自动部署
├── examples/                       # 示例代码（C++/Python/CUDA 等）
├── resume/                         # 简历
├── scripts/                        # 辅助脚本
├── ENVIRONMENT_SETUP.md            # 详细环境配置指南
├── CHANGELOG.md                    # 更新日志
└── README.md / README_EN.md        # 项目说明
```

---

## 写作约定

新笔记直接放进 `docs/notes/<分类>/` 下即可：

```markdown
---
title: 我的新笔记
---

# 正文从这里开始
```

要点：

- frontmatter 至少有 `title`，侧边栏会用它做显示名（缺省回退到文件名）
- 旧笔记里的 `layout: note` / `layout: default` 已经在 `config.mts` 里通过 `transformPageData` 自动忽略，不用改
- 子目录会自动渲染成可折叠分组
- 不想显示评论区的页面加一行 `comments: false` 到 frontmatter 即可
- 数学公式直接写 `$E=mc^2$` 或 `$$ ... $$`，开箱即用

新增一个分类（比如 `embedded`）时，编辑 `docs/.vitepress/config.mts` 的 `categories` 数组，加一行：

```ts
{ dir: 'embedded', text: 'Embedded', emoji: '\u{1F50C}' },
```

然后把内容放进 `docs/notes/embedded/` 即可。

---

## 部署

### GitHub Pages（默认）

`.github/workflows/deploy.yml` 已经配置好：

- 每次推送到 `main` 自动触发
- 用 `actions/setup-node@v4` 装 Node 20
- 跑 `npm ci` + `npm run build`
- 把 `docs/.vitepress/dist/` 上传并发布到 GitHub Pages

**首次启用**：仓库 → Settings → Pages → Source 切换为 `GitHub Actions`。

### 自定义域名

1. 在 `docs/public/` 下新建 `CNAME` 文件，写你的域名
2. 在 `docs/.vitepress/config.mts` 加 `sitemap: { hostname: 'https://your-domain' }`
3. DNS 把域名 `CNAME` 到 `<username>.github.io`

### 本地预览部署产物

```bash
npm run build
npm run preview
```

---

## 迁移说明

本项目历史上经历过一次架构级迁移：

| | 旧（≤ 2025） | 新（2026 起） |
| --- | --- | --- |
| 静态站点生成 | Jekyll 3.10 | **VitePress 1.x** |
| 运行时 | Ruby + Bundler | **Node.js** |
| 主题方案 | 手写 `_layouts` + Minima | **Vue 组件 + 默认主题扩展** |
| 配置文件 | `_config.yml` | **`.vitepress/config.mts`** |
| 侧边栏维护 | `_data/navigation.yml` 手动 | **基于目录自动生成** |
| 搜索 | 无 | **本地全文搜索** |
| 暗色模式 | 无 | **开箱即用** |
| 数学公式 | jsDelivr 拉 MathJax | **VitePress 内置** |

迁移过程：删除全部 Jekyll 产物（`Gemfile`、`_config.yml`、`_layouts/`、`_includes/`、`_data/`、`assets/`），保留所有 `docs/notes/**` 下的 Markdown 内容。如需回滚旧版本，请通过 git 历史。

---

## 贡献指南

欢迎提交 **Issues** 讨论或 **Pull Requests** 改进：

1. Fork 本仓库
2. 创建新分支 `feature/xxx`
3. 提交改动并推送
4. 提交 Pull Request，描述改动内容

---

## 评论区

本项目使用 [GitHub Discussions](https://docs.github.com/en/discussions) + [giscus](https://giscus.app/) 评论系统：

- **查看 / 发表评论**：打开任意文章，滚动到正文末尾即可
- **首次启用**：去 <https://giscus.app/zh-CN> 生成配置，把 `data-repo-id` 和 `data-category-id` 填到 `docs/.vitepress/theme/components/Giscus.vue` 的常量里

---

## 致谢

- [VitePress](https://vitepress.dev/) — 优秀的 Vue 文档与博客框架
- [Vue.js](https://vuejs.org/) 与 [Vite](https://vitejs.dev/) 团队
- [SimonAKing/HomePage](https://github.com/SimonAKing/HomePage) — 旧版动态主页设计参考与启发
- 所有 Issue / PR 贡献者 🙏

---

## 许可证

本项目遵循 [MIT License](LICENSE)，欢迎自由使用、修改和分发，请保留版权声明。
