---
layout: default
title: 多体动力学 系列
permalink: /mbd/
---

# 多体动力学 系列

<div class="post-grid">
  <!-- 第一篇笔记 -->
  <article class="post-card">
    <h3 class="post-title">
      <a href="/Technical-Interview-NOTE-Baguwen-Style/notes/_mbd/MA_weeklyplan.html">
        MA周工作记录
      </a>
    </h3>
    <p class="post-excerpt">
      关于毕业设计每周任务同步以及在实现过程中的想法与设计思路，方便之后的追溯和毕业论文的编写。:)
    </p>
    <time class="post-date">🕒 2025-07-09</time>
  </article>

  <!-- 第二篇笔记 -->
  <article class="post-card">
    <h3 class="post-title">
      <a href="/Technical-Interview-NOTE-Baguwen-Style/notes/_mbd/MBD.html">
        多体动力学基础
      </a>
    </h3>
    <p class="post-excerpt">
      本文介绍多体动力学系统的基本概念、方程推导与常用求解器……
    </p>
    <time class="post-date">🕒 2025-07-05</time>
  </article>

  <!-- 以后再新增笔记就复制上面一个 article 块 -->
</div>

<style>
.post-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(260px, 1fr));
  gap: 1.5rem;
  margin-top: 2rem;
}
.post-card {
  background: #fff;
  padding: 1rem;
  border-radius: 6px;
  box-shadow: 0 1px 4px rgba(0,0,0,0.1);
  transition: transform .2s;
}
.post-card:hover {
  transform: translateY(-3px);
}
.post-title {
  margin: 0 0 .5rem;
  font-size: 1.15rem;
}
.post-title a {
  text-decoration: none;
  color: #333;
}
.post-excerpt {
  color: #555;
  font-size: 0.9rem;
  margin: 0 0 .7rem;
}
.post-date {
  font-size: 0.8rem;
  color: #888;
  display: block;
  text-align: right;
}
</style>
