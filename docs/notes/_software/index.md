---
layout: default
title: 软件开发 系列
permalink: /software/
---

<ul>
{% for post in site.software %}
  <li><a href="{{ post.url | relative_url }}">{{ post.title }}</a> - {{ post.date | date: "%Y-%m-%d" }}</li>
{% endfor %}
</ul>
