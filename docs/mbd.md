---
layout: default
title: 多体动力学 系列
permalink: /mbd/
---

# 多体动力学 系列

<div class="post-list">
  {% assign mbd_pages = site.pages | where_exp: "p", "p.path contains 'notes/_mbd/'" %}
  {% for p in mbd_pages %}
    <div class="post-item">
      <h3><a href="{{ p.url | relative_url }}">{{ p.title }}</a></h3>
      {% if p.excerpt %}
        <p>{{ p.excerpt | strip_html | truncate: 100 }}</p>
      {% endif %}
      <small>🕒 {{ p.date | date: "%Y-%m-%d" }}</small>
      <hr>
    </div>
  {% endfor %}
</div>

<style>
.post-list { margin-top: 2em; }
.post-item { margin-bottom: 1.5em; }
.post-item h3 { margin: 0; font-size: 1.2em; }
.post-item p { color: #555; margin: 0.3em 0; }
.post-item small { color: #999; }
</style>
