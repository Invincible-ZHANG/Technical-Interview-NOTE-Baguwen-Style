---
title: LaTex安装和使用教程
date: 2025-10-09
excerpt: "LaTex我不太会用，于是自己琢磨，加上开源文献，希望能给帮助"
layout: note
---

## 安装教程

Reference： https://zhuanlan.zhihu.com/p/166523064

## 论文书写教程

以RWTH工大的LaTex为例，给出的参考文件中包括这几部分：

### 参考文件Tree
```
E:\PPT\MMITHESISTEMPLATE
│ CHANGELOG.md —— 模板变更记录（说明文档）。
│ FOLDERTREE —— 目录树快照（给读者看的清单，非编译必需）。
│ LICENSE.md / LPPL-1.3c —— 许可协议（LaTeX Project Public License）。
│ main.tex —— 入口文件（整篇论文从这里编译/组织章节）。✅常改
│ metadata.tex —— 论文题目、作者、学院、提交日期等元数据。✅常改
│ mmiThesisClass.cls —— 模板类文件（版式/封面/页眉页脚规则）。❗一般别改
│ README.md —— 模板使用说明。
│ main.pdf —— 编译产物（最终论文 PDF）。
│ main.aux / .bbl / .blg / .log / .out / .toc / .lol / .synctex.gz / .auxlock —— 中间文件：
│ • .aux 交叉引用缓存；.bbl 由 biber/bibtex 生成的参考文献内容；
│ • .blg 文献工具日志；.toc/.lol 目录&代码清单；.out 书签/超链接信息；
│ • .log 编译日志；.synctex.gz 源码↔PDF 同步；.auxlock 构建锁。🗑️Clean 可删
│
├─appendix/ —— 附录区
│ acknowledgement.tex —— 致谢。✅
│ app_foldertree.tex —— 目录树附录（展示项目结构）。✅
│ app_history.tex —— 版本/研究历程附录。✅
│ declaration.tex —— 独立完成声明/诚信声明。✅
│ 以上各自的 .aux 是编译中间文件。🗑️
│
├─bibliography/ —— 参考文献库
│ bibliography.bib —— Bib 数据库（文献条目都放这里）。✅
│ └─resources/ —— 手册/论文 PDF 等原始资料（非必须，便于查阅）。
│
├─chapters/ —— 正文各章
│ chap_introduction.tex —— 引言/绪论。✅
│ chap_files.tex —— 文件/资源组织说明这一章（模板示例）。✅
│ chap_options.tex —— 模板可选项示例。✅
│ chap_packages.tex —— 常用宏包示例。✅
│ 对应的 .aux 为中间文件。🗑️
│
├─completion/ —— 编辑器补全支持
│ mmiThesisClass.cwl / mmiThesisTemplate.cwl —— TeX Studio/VS Code 的补全词库。
│ COMPLETION.md —— 如何启用补全说明。
│ placeCompletionFiles.sh —— 把 .cwl 放到编辑器路径的脚本（给 *nix 用）。
│
├─graphics/ —— 图片资源（\includegraphics{...} 从这里取）
│ rwth_mmi_*.png、StereoVision.png —— 模板自带图。
│ └─Infrared/ SingleMarker.png 等 —— 子图目录。
│
├─header/ —— 统一导言设置
│ packages.tex —— 集中 \usepackage{...}。✅常改
│ commands.tex —— 自定义命令/环境（\newcommand 等）。✅
│ hyphenation.tex —— 英文断词规则。可选改
│
├─prelims/ —— 前置部分（front matter）
│ abstract.tex —— 摘要（可做中英双语）。✅
│ acronyms.tex —— 缩略词表（配合 glossaries 或自定义环境）。✅
│ lists.tex —— 图/表清单设置（LoF/LoT 等）。
│ nomenclature.tex —— 符号表/术语表。✅
│ 同名 .aux 为中间文件。🗑️
│
└─tikz/ —— TikZ 绘图源码（需要就放 .tex/.tikz 图形）。
  README.txt —— 绘图说明。
        README.txt
```


## 论文整体蓝图

1.1 摘要（prelims/abstract.tex）













































