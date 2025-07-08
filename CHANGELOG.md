
# Changelog

## [1.1.0] – 2025-07-08

### 新增
- 🖼️ 添加多分辨率 `favicon.ico`，支持浏览器标签页和书签图标  
- 🌄 首页新增背景图支持，并修复 Liquid 模板在 SCSS 中渲染背景路径的问题  (依旧存在BUG)
- 🔧 引入 `custom.scss`（原 `custom.css` 重命名并添加 Front-Matter），开启 Jekyll 对背景 URL 的解析  
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
