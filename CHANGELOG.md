
# Changelog

## [1.3.0] – 2026-10-25

**从「技术笔记站」改成「个人网站」**

### 重构
- 🏠 **首页改为个人主页**：左侧吸附栏（头像 / 姓名 / 一句话简介 / 锚点导航 / GitHub、RSS），右侧 `关于 → 最近 → 精选 → 生活 → 笔记` 单列长文式布局，参考常见开源个人站
- 🗂 **新增 `/notes/` 归档页**：102 篇技术笔记按分类分组的**纯列表**（标题 + 日期），不再铺卡片。毕业季过后笔记退居存档，占据首页的权重交还给生活
- 🌱 **新增 `/life/` 生活记录区**：按年份分组的竖向时间线，`notes/travel` 迁出为 `life/belgium-road-trip.md`，图片一同搬到 `docs/life/image/`
- 🧭 **导航精简**：`首页 / 生活 / 笔记 / 搜索` 四项，去掉原来的分类堆叠
- 📄 **新增 `_data/profile.yml`**：姓名、简介、社交链接、Now 清单集中在数据文件里，改文案不用碰模板
- 🎨 **新增 `site.css`（约 1200 行）**：设计令牌、hero 字阶、锚点导航、时间线、归档行、卡片悬停、`prefers-reduced-motion`，以及 1024 / 860 / 560 三档响应式

### 修复
- ✅ **16 篇笔记会把原始 `.md` 源文件一并发布到线上**：这些文件此前没有 front matter，Jekyll 既按 page 生成 HTML，又当静态文件原样复制。已补全 front matter（标题 + 取自 git log 的日期）
- ✅ **搜索结果里的 `<mark>` 被 flex 拉伸成整条色块**：标题改用 `span` 包裹，`mark` 恢复行内渲染
- ✅ **搜索摘要混入 Markdown/LaTeX 符号**：`search.json` 生成时去掉 `# * $ \` | >`
- ✅ **功能性页面出现评论区**：`搜索 / 404 / 全部笔记 / 生活` 加 `comments: false`，单篇笔记保留
- ✅ 搜索页补标题、简介与「常搜」关键词胶囊，空状态不再是一片荒芜

---

## [1.2.0] – 2026-10-25

### 修复
- ✅ **16 篇笔记变「裸页面」**：`_config.yml` 增加 `defaults`，`notes/**` 统一兜底 `layout: note`，此前无 front matter 的文章不套布局、无导航/CSS/公式/代码高亮
- ✅ **代码高亮完全失效**：删除 `code_style.css` 中 `pre code span.token { all: unset }`（它清掉了所有着色）；新增暖金暗色 Prism 主题，语言组件从 2 个扩展到 17 个（C/C++/Python/Bash/JSON/YAML/Java/Go/Rust/Makefile/Diff/SQL/LaTeX/Markdown）
- ✅ **域名三处不一致**：`_config.yml` 的 `url` 与 `CNAME`、README 徽章统一为 `https://www.zhangzijian.cn`
- ✅ **41 篇笔记是「内容孤岛」**：分类页只列同级目录，`MA_WP_CH`(19)、`basic_knowledge`(8)、`Leetcode`(5) 等子目录内容无入口 → 改为同级 + 子目录分组递归展示
- ✅ 移动端 `alert()` 硬阻断改为可关闭的底部横幅（8 秒自动收起，会话内不重复）
- ✅ 62 个文本文件清除 UTF-8 BOM；2 个 GBK 编码文件转为 UTF-8
- ✅ 硬编码 `/assets/...` 路径改为 `relative_url`，兼容子路径部署
- ✅ 两个 layout 中 giscus 配置分裂 → 收敛到 `_config.yml` 统一变量；孤儿页 `notes/web/web.md` 加入导航
- ✅ **`where_exp` 组合条件导致构建失败**：Jekyll 3.10 的 `where_exp` 只解析单个比较，不支持 `and` / `or`（`parse_condition` 解析完一个比较后立刻要求字符串结束）。`default.html` / `home.html` / `search.json` 三处的 `"a and b"` 写法会让 `jekyll build` 直接抛 `Liquid syntax error: Expected end_of_string but found id`，改为 `for` + `if` 手工筛选。**此问题静态检查查不出，只有真机构建才能暴露**
- ✅ **分类页中文目录名显示为 `%E8%AE%BA%E6%96%87%E7%A8%BF`**：`page.dir` 是 URL 编码后的路径，改用 `page.path` 去掉文件名的方式推算目录，恢复「论文草稿」「算法」等原始中文
- ✅ **`feed.xml` 是空壳**：`jekyll-feed` 只收 `_posts`，而本站文章都是 `notes/` 下的 pages → 移除该插件，手写 Atom 模板（`docs/feed.xml`），按日期收录最近 20 篇

### 新增
- 🔍 全站搜索页 `/search.html`：纯前端索引（Liquid 生成 `search.json`），标题命中优先、关键词高亮、支持 `?q=` 直达
- 🧭 阅读进度条、导航当前页高亮、平滑回到顶部、正文外链新窗口、`prefers-reduced-motion` 适配
- 📄 404 页面（`/404.html`）、`sitemap.xml`、`feed.xml`
- 🏷️ `<head>` 补齐 description / Open Graph / Twitter Card / canonical
- 🏠 首页新增「最近更新」区块（按日期取最近 6 篇）
- 📦 **前端依赖全部本地化**：Prism（core + 14 语言，51KB）、MathJax（2.0MB）、tocbot 下载进 `assets/`，布局层零外部 CDN 依赖（原先单篇文章就要发 17 个 jsdelivr 请求，断网即无高亮、公式退化为源码）
- 🛠️ **本地构建环境**：`serve.cmd`（热重载预览 http://127.0.0.1:4000）与 `build.cmd`（一次性构建校验），自带便携 Ruby 3.3.13 + MSYS2 工具链路径探测，双击即用
- 🧩 `Gemfile` 补 `tzinfo-data`（仅 Windows 平台），修复 Windows 无 zoneinfo 导致的 `TZInfo::DataSources::ZoneinfoDirectoryNotFound`

### 优化
- 🎨 美化：引入设计令牌（暖金 `--gold-*` + 玻璃层），重做标题层级（渐变主标题 / 金色竖线 h2）、卡片（修复深底浅字对比度、金色描边、悬停抬升）、表格（圆角 + 斑马纹 + 悬停高亮）、引用块、左侧目录（冷白蓝 → 暖金玻璃）
- 📐 响应式：≤1279px 收起侧边目录避免与正文压字；≤600px 卡片单列、导航紧凑
- ⚡ 移除重复的 Google Fonts 请求与重复 `<meta charset>`

---

## [1.1.0] – 2025-07-08

### 新增
- 🖼️ 添加多分辨率 `favicon.ico`，支持浏览器标签页和书签图标  
- 🌄 首页新增背景图支持，并修复 Liquid 模板在 SCSS 中渲染背景路径的问题  (依旧存在BUG)  
- ⏰ “实时时钟”模块加入秒级刷新显示  
- 💬 “Hitokoto 一言”模块集成 v1.hitokoto.cn API，异步加载名人名言并附带出处链接  
- 📱 新增响应式媒体查询：手机、平板、PC 三档布局自动适配  
- 🐳 提供 Dockerfile，本地无需安装 Ruby 环境即可在 VS/Docker 里预览 Jekyll  

### 优化
- 🔄 重构首页布局——模块化卡片设计（关于作者、创意共创、快速入口）  
- 🛠️ 统一所有页面 `<head>` 引入 `head.html`，新增 `<link rel="icon">` 声明  
- 📦 修正 GitHub Pages Actions 工作流：使用官方 `actions/jekyll-build-pages` + `actions/deploy-pages`  
- 🧹 清理不必要的 Liquid 语法残留，CSS 选择器优先级调整  

### 修复
- ✅ 修复了一言模块引用错误的 Liquid 语法  
- ✅ 解决 `git push` 分支上游未设置的问题  
- ✅ 去除默认 VS/.vs 等编译产物，完善 `.gitignore`  

---

> **下个版本**：计划新增多语言支持 (English/中文)、主题切换、更多互动小组件  
