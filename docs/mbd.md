---
layout: default
title: 多体动力学 系列
permalink: /mbd/
---


<div class="post-list">
  {% assign posts = site.mbd %}
  {% for post in posts %}
    <div class="post-item">
      <h3><a href="{{ post.url | relative_url }}">{{ post.title }}</a></h3>
      {% if post.excerpt %}
        <p>{{ post.excerpt | strip_html | truncate: 100 }}</p>
      {% endif %}
      <small>🕒 {{ post.date | date: "%Y-%m-%d" }}</small>
      <hr>
    </div>
  {% endfor %}
</div>

<style>
.post-list {
  margin-top: 2em;
}
.post-item {
  margin-bottom: 1.5em;
}
.post-item h3 {
  margin: 0;
  font-size: 1.2em;
}
.post-item p {
  color: #555;
  margin: 0.3em 0;
}
.post-item small {
  color: #999;
}
</style>
