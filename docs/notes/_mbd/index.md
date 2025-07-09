---
layout: default
title: 多体动力学 系列
permalink: /mbd/
---

<ul>
{% for post in site.mbd %}
  <li><a href="{{ post.url | relative_url }}">{{ post.title }}</a> - {{ post.date | date: "%Y-%m-%d" }}</li>
{% endfor %}
</ul>
