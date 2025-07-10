在 Jekyll（以及很多静态博客系统）里：

_layouts/default.html
通常是所有页面的通用基础模板。
你的绝大部分 Markdown 文档（比如 notes/_mbd/MA_weeklyplan.md）都会在 Front Matter 里写 layout: default，或者你在 _config.yml 里用 defaults 规则自动指定，最终都被这个模板包裹。它一般包含 <html>, <head>, <body>，页面主结构，导航、footer 也可能写在这里。

_layouts/home.html
通常是首页（Home Page）专用的模板。
你的 index.md 或 index.html 文件通常会在 Front Matter 里写 layout: home，那它就只被 home.html 这个模板渲染。
home.html 往往会有更特殊的布局（比如文章列表、封面大图、欢迎语等），内容和样式可以和普通文档页不同。