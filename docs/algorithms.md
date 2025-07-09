---
layout: default
title: 算法 系列
permalink: /algorithms/
---

<ul>
{% for post in site.algorithms %}
  <li><a href="{{ post.url | relative_url }}">{{ post.title }}</a> - {{ post.date | date: "%Y-%m-%d" }}</li>
{% endfor %}
</ul>
