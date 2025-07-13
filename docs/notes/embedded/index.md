---
layout: default
title: 嵌入式 系列
---

<!-- 标题区域：半透明磨砂背景 -->
<header class="section-header">
  <h1>嵌入式 系列</h1>
</header>

<div class="post-grid">
  {% assign mbd_notes = site.pages | where_exp: "page", "page.path contains 'embedded/'" %}
  {% assign sorted_mbd_notes = mbd_notes | sort: "date" | reverse %}
  {% for note in sorted_mbd_notes %}
    {% assign filename = note.path | split:'/' | last %}
    {% unless filename == "index.html" or filename == "index.md" %}
      <article class="post-card">
        <h3 class="post-title">
          <a href="{{ note.url }}">{{ note.title }}</a>
        </h3>
        <p class="post-excerpt">
          {{ note.excerpt | default: note.content | truncatewords: 30 }}
        </p>
        <time class="post-date">🕒 {{ note.date | date: "%Y-%m-%d" }}</time>
      </article>
    {% endunless %}
  {% endfor %}
</div>