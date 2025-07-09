---
layout: default
title: 多体动力学 系列
permalink: /mbd/
---

<!-- 标题区域：半透明磨砂背景 -->
<header class="mbd-header">
  <h1>多体动力学 系列</h1>
</header>

<div class="post-grid">
  <!-- 第一篇笔记 -->
  <article class="post-card">
    <h3 class="post-title">
      <a href="./MA_weeklyplan.md">MA周工作记录</a>
    </h3>
    <p class="post-excerpt">
      关于毕业设计每周任务同步以及在实现过程中的想法与设计思路，方便之后的追溯和毕业论文的编写。:)
    </p>
    <time class="post-date">🕒 2025-07-09</time>
  </article>

  <!-- 第二篇笔记 -->
  <article class="post-card">
    <h3 class="post-title">
      <a href="./MBD.md">多体动力学基础</a>
    </h3>
    <p class="post-excerpt">
      本文介绍多体动力学系统的基本概念、方程推导与常用求解器……
    </p>
    <time class="post-date">🕒 2025-07-05</time>
  </article>

  <!-- 以后新增只要复制上面 block 并改链接+标题+摘要+日期 -->
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
  background: rgba(255,255,255,0.8);
  border-radius: 8px;
  padding: 1.2rem;
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
  overflow: hidden;
  animation: fadeInUp 0.5s ease forwards;
  /* 默认先隐藏，等动画触发 */
  opacity: 0;
  transform: translateY(20px);
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}
/* 设置渐入延迟，可根据序号调整 */
.post-card:nth-child(1) { animation-delay: 0.1s; }
.post-card:nth-child(2) { animation-delay: 0.2s; }
.post-card:nth-child(3) { animation-delay: 0.3s; }
/* …如果更多卡片，可继续 nth-child(4) … */

/* 悬浮放大 */
.post-card:hover {
  transform: translateY(-5px) scale(1.03);
  box-shadow: 0 8px 20px rgba(0,0,0,0.15);
}

/* 标题链接 */
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

/* 摘要 */
.post-excerpt {
  margin: 0 0 1rem;
  color: #555;
  font-size: 0.95rem;
  line-height: 1.4;
}

/* 日期 */
.post-date {
  display: block;
  text-align: right;
  color: #888;
  font-size: 0.85rem;
}
</style>
