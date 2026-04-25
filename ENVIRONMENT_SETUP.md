# 环境配置说明

本项目已经从旧版 Jekyll（Ruby）方案迁移到现代前端构建方案：

- `VitePress` 1.x
- `Vue 3` + `Vite`
- `TypeScript`
- `Node.js`（推荐 20 LTS，最低 18）

本地开发不再需要 `Ruby`、`Bundler`、`Gemfile`。

## 一、准备工作

### 1. 安装 Node.js

前往 [nodejs.org](https://nodejs.org) 下载 LTS 版本（推荐 `20.x`）。安装完成后在 PowerShell 里确认：

```powershell
node -v
npm -v
```

输出类似：

```text
v20.x.x
10.x.x
```

### 2.（可选）安装 pnpm

本项目既支持 `npm` 也支持 `pnpm`，如果你更习惯 `pnpm`：

```powershell
npm install -g pnpm
```

## 二、首次启动

在仓库根目录 `C:\Users\Zhang Zijian\Desktop\code_project\web` 下执行：

```powershell
npm install
npm run dev
```

或者用 `pnpm`：

```powershell
pnpm install
pnpm dev
```

终端会输出开发服务器地址，默认是：

- <http://localhost:5173>

在浏览器里打开即可看到新版站点，支持热更新：改 Markdown、保存、刷新即可看到效果。

## 三、常用脚本

在 `package.json` 里定义了三个脚本：

| 命令 | 作用 |
| ---- | ---- |
| `npm run dev` | 启动本地开发服务器，带热更新 |
| `npm run build` | 构建生产静态站点，输出到 `docs/.vitepress/dist/` |
| `npm run preview` | 在本地预览构建产物 |

## 四、项目结构

```text
.
├── package.json                    # Node 依赖与脚本入口
├── docs/                           # VitePress srcDir
│   ├── index.md                    # 首页（Hero + Features）
│   ├── public/                     # 直接被拷贝到根路径的静态资源
│   │   ├── favicon.svg
│   │   ├── logo.svg
│   │   └── hero.svg
│   ├── notes/                      # 所有笔记内容（保留原目录结构）
│   │   ├── mbd/
│   │   ├── software/
│   │   ├── algorithms/
│   │   ├── gpu_cuda/
│   │   ├── AI_Infra/
│   │   ├── LLM/
│   │   ├── Linux/
│   │   ├── Fall_Reviews/
│   │   ├── English/
│   │   ├── travel/
│   │   └── web/
│   └── .vitepress/
│       ├── config.mts              # 站点主配置
│       ├── utils/
│       │   └── sidebar.mjs         # 自动生成侧边栏 / 导航
│       └── theme/
│           ├── index.ts            # 自定义主题入口（扩展默认主题）
│           ├── styles/
│           │   ├── vars.css        # 品牌色、渐变等变量
│           │   └── custom.css      # 导航栏玻璃效果、卡片 hover 等
│           └── components/
│               ├── Giscus.vue              # giscus 评论组件
│               └── HomeFeatureExtras.vue   # 首页补充板块
├── .github/workflows/deploy.yml    # GitHub Pages 自动部署
└── ENVIRONMENT_SETUP.md            # 本文件
```

## 五、关键特性（已开箱）

- 首页 Hero + Features 现代布局，渐变主色
- 自动深浅色主题切换，夜间模式下色板单独调优
- 基于目录自动生成的侧边栏和顶部导航（无需手动维护 `navigation.yml`）
- 本地全文搜索（右上角搜索框，按 `Ctrl+K` 快捷唤起）
- 数学公式（`$...$` 和 `$$...$$`）原生支持
- 代码块行号、图片懒加载
- giscus 评论（每篇正文末尾）
- GA4 统计（`G-WNZR555V1D`，与旧配置保持一致）
- GitHub 「在 GitHub 上编辑此页」跳转
- 最后更新时间自动注入
- 上一篇 / 下一篇自动导航

## 六、如何写新笔记

在 `docs/notes/<分类>/` 下新建一个 `.md` 文件即可。最简 frontmatter：

```markdown
---
title: 我的新笔记
---

# 正文从这里开始
```

说明：

- 旧笔记里的 `layout: note` / `layout: default` frontmatter 已经通过 `config.mts` 里的 `transformPageData` 自动忽略，不用改动历史文件。
- 侧边栏会在下次 `npm run dev` 启动时重新扫描目录并自动刷新。
- 如果某篇文章不想显示评论区，加一行 `comments: false` 到 frontmatter 即可。

## 七、新增一个分类

编辑 `docs/.vitepress/config.mts` 的 `categories` 数组：

```ts
const categories = [
  { dir: 'mbd', text: 'MBD', emoji: '🧮' },
  // ...
  { dir: '你的新分类目录名', text: '显示名', emoji: '🆕' },
]
```

然后把 Markdown 文件放进 `docs/notes/你的新分类目录名/` 即可。

## 八、数学公式

VitePress 已经启用 MathJax 3，直接用 `LaTeX` 语法：

```markdown
行内：$E = mc^2$

块级：
$$
\int_0^\infty e^{-x^2}\, dx = \frac{\sqrt{\pi}}{2}
$$
```

## 九、giscus 评论配置

已经在 `docs/.vitepress/theme/components/Giscus.vue` 里写好仓库和分类，但 `data-repo-id` 和 `data-category-id` 是空的，你需要：

1. 打开 <https://giscus.app/zh-CN>
2. 填入仓库 `Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style`
3. 页面下方会生成 `<script>` 标签，把里面的 `data-repo-id` 和 `data-category-id` 复制出来
4. 粘贴到 `Giscus.vue` 顶部的 `GISCUS_REPO_ID` 和 `GISCUS_CATEGORY_ID` 常量里
5. 保存，热更新后评论区就能正常加载

## 十、部署

### 部署到 GitHub Pages

已配置好工作流 `.github/workflows/deploy.yml`：

- 每次推送到 `main` 分支自动构建 + 部署
- 使用 `actions/setup-node@v4` 安装 Node 20
- 运行 `npm ci`（若失败回退到 `npm install`），然后 `npm run build`
- 把 `docs/.vitepress/dist` 发布到 GitHub Pages

**首次启用**：在 GitHub 仓库 → Settings → Pages 里把 Source 切换成 `GitHub Actions`。

### 部署到自定义域名

如果以后要用自定义域名（比如 `blog.k1n.asia`）：

1. 在 `docs/public/` 下新建 `CNAME` 文件，内容写你的域名
2. 在 `docs/.vitepress/config.mts` 里加一行 `sitemap: { hostname: 'https://blog.k1n.asia' }`
3. 在 DNS 里把域名 `CNAME` 到 `<你的用户名>.github.io`

### 本地预览部署产物

```powershell
npm run build
npm run preview
```

## 十一、常见问题

### 1. `npm install` 很慢或超时

切换国内镜像：

```powershell
npm config set registry https://registry.npmmirror.com
```

恢复官方源：

```powershell
npm config set registry https://registry.npmjs.org
```

### 2. 端口 5173 被占用

VitePress 会自动递增端口，或者你自己指定：

```powershell
npm run dev -- --port 5174
```

### 3. 启动后侧边栏没有新加的文章

- 检查 Markdown 文件是否放在 `docs/notes/<某个已注册分类>/` 下
- 关掉 dev server 再 `npm run dev` 重启（侧边栏在配置加载时才扫描磁盘）

### 4. 构建失败提示 `Cannot find module 'gray-matter'`

确认你已经执行过 `npm install`，如果仍然失败可以手动重装：

```powershell
npm install gray-matter markdown-it-mathjax3 vitepress vue --save-dev
```

### 5. 想回退到旧版 Jekyll

旧方案已经被完整移除（包括 `Gemfile`、`_config.yml`、`_layouts/`、`_includes/`、`assets/` 里的 Jekyll 资源）。如果需要恢复，请通过 `git` 历史回滚对应 commit。

## 十二、最小命令清单

```powershell
npm install
npm run dev
```

浏览器打开 <http://localhost:5173>。
