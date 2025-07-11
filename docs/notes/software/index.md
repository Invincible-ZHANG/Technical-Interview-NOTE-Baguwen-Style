---
layout: default
title: 软件开发 系列
---

<!-- 标题区域：半透明磨砂背景 -->
<header class="mbd-header">
  <h1>软件开发 系列</h1>
</header>

<div class="post-grid">
  {% assign mbd_notes = site.pages | where_exp: "page", "page.path contains 'software/'" %}
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

<style>
/* ------ 标题磨砂风 ------ */
.mbd-header {
  margin: 1.5rem auto;
  padding: 0.8rem 1.2rem;
  max-width: 600px;
  background: rgba(255,255,255,0.2);
  backdrop-filter: blur(8px);
  border-radius: 8px;
  text-align: center;
}
.mbd-header h1 {
  margin: 0;
  font-size: 2.5rem;
  color: #fff;
  text-shadow: 0 2px 4px rgba(0,0,0,0.5);
}

/* ------ 卡片网格 ------ */
.post-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(280px, 1fr));
  gap: 1.5rem;
  margin: 2rem 0;
}

/* ------ 卡片样式 & 动画 ------ */
@keyframes fadeInUp {
  from {
    opacity: 0;
    transform: translateY(20px);
  }
  to {
    opacity: 1;
    transform: translateY(0);
  }
}

.post-card {
  position: relative;
  background: rgba(255,255,255,0.32); /* 更加透明一点 */
  border-radius: 8px;
  padding: 1.2rem;
  box-shadow: 0 4px 12px rgba(0,0,0,0.08);
  overflow: hidden;
  animation: fadeInUp 0.5s ease forwards;
  opacity: 0;
  transform: translateY(20px);
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}
.post-card:nth-child(1) { animation-delay: 0.1s; }
.post-card:nth-child(2) { animation-delay: 0.2s; }
.post-card:nth-child(3) { animation-delay: 0.3s; }
/* …可以继续 nth-child(4) … */

.post-card:hover {
  transform: translateY(-5px) scale(1.03);
  box-shadow: 0 8px 20px rgba(0,0,0,0.15);
}

.post-title {
  margin: 0 0 .6rem;
  font-size: 1.2rem;
}
.post-title a {
  color: #333;
  text-decoration: none;
}
.post-title a:hover {
  color: #007ACC;
  text-decoration: underline;
}

.post-excerpt {
  margin: 0 0 1rem;
  color: #555;
  font-size: 0.95rem;
  line-height: 1.4;
}

.post-date {
  display: block;
  text-align: right;
  color: #888;
  font-size: 0.85rem;
}
</style>
