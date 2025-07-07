---
layout: default
title: 嵌入式 系列
permalink: /embedded/
---

# 嵌入式 系列

下面是所有嵌入式相关笔记：

<ul>
{% for post in site.embedded %}
  <li><a href="{{ post.url | relative_url }}">{{ post.title }}</a> - {{ post.date | date: "%Y-%m-%d" }}</li>
{% endfor %}
</ul>