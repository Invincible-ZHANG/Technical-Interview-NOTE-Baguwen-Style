<script setup lang="ts">
// Non-ASCII text uses \u escapes on purpose: this Vue file is saved as plain
// text and some Windows toolchains default to a non-UTF-8 code page.

// "\u5173\u4E8E\u8FD9\u4E2A\u535A\u5BA2" = "关于这个博客"
const SECTION_TITLE = '\u5173\u4E8E\u8FD9\u4E2A\u535A\u5BA2'
// "这里主要记录我在 MBD、嵌入式、GPU/CUDA、AI Infra、算法等方向上的学习轨迹。"
const SECTION_SUB =
  '\u8FD9\u91CC\u4E3B\u8981\u8BB0\u5F55\u6211\u5728 MBD\u3001\u5D4C\u5165\u5F0F\u3001GPU/CUDA\u3001AI Infra\u3001\u7B97\u6CD5\u7B49\u65B9\u5411\u4E0A\u7684\u5B66\u4E60\u8F68\u8FF9\u3002'

const spotlights = [
  {
    // "持续更新"
    title: '\u{1F4DA} \u6301\u7EED\u66F4\u65B0',
    // "研究生课题、工程调试、面经复盘、旅行碎片……随想随记，颗粒度大小通吃。"
    desc: '\u7814\u7A76\u751F\u8BFE\u9898\u3001\u5DE5\u7A0B\u8C03\u8BD5\u3001\u9762\u7ECF\u590D\u76D8\u3001\u65C5\u884C\u788E\u7247\u2026\u2026\u968F\u60F3\u968F\u8BB0\uFF0C\u9897\u7C92\u5EA6\u5927\u5C0F\u901A\u5403\u3002',
  },
  {
    // "现代构建"
    title: '\u{1F6E0}\uFE0F \u73B0\u4EE3\u6784\u5EFA',
    // "VitePress + Vue 3 + Vite，开发热更新、暗色模式、本地搜索开箱即用。"
    desc: 'VitePress + Vue 3 + Vite\uFF0C\u5F00\u53D1\u70ED\u66F4\u65B0\u3001\u6697\u8272\u6A21\u5F0F\u3001\u672C\u5730\u641C\u7D22\u5F00\u7BB1\u5373\u7528\u3002',
  },
  {
    // "全部开源"
    title: '\u{1F517} \u5168\u90E8\u5F00\u6E90',
    // "内容与主题都托管在 GitHub，欢迎 Issue/PR 补充讨论或纠正错误。"
    desc: '\u5185\u5BB9\u4E0E\u4E3B\u9898\u90FD\u6258\u7BA1\u5728 GitHub\uFF0C\u6B22\u8FCE Issue/PR \u8865\u5145\u8BA8\u8BBA\u6216\u7EA0\u6B63\u9519\u8BEF\u3002',
  },
]
</script>

<template>
  <section class="extras">
    <div class="extras-inner">
      <h2 class="extras-title">{{ SECTION_TITLE }}</h2>
      <p class="extras-sub">{{ SECTION_SUB }}</p>
      <div class="extras-grid">
        <div v-for="item in spotlights" :key="item.title" class="card">
          <h3>{{ item.title }}</h3>
          <p>{{ item.desc }}</p>
        </div>
      </div>
    </div>
  </section>
</template>

<style scoped>
.extras {
  padding: 64px 24px 32px;
  border-top: 1px solid var(--vp-c-divider);
  margin-top: 48px;
}

.extras-inner {
  max-width: 1152px;
  margin: 0 auto;
}

.extras-title {
  font-size: 28px;
  font-weight: 700;
  letter-spacing: -0.01em;
  margin: 0 0 12px;
}

.extras-sub {
  color: var(--vp-c-text-2);
  margin: 0 0 32px;
  max-width: 720px;
}

.extras-grid {
  display: grid;
  gap: 20px;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
}

.card {
  background: var(--vp-c-bg-soft);
  border: 1px solid var(--vp-c-divider);
  border-radius: 14px;
  padding: 20px 22px;
  transition:
    transform 240ms ease,
    border-color 240ms ease,
    box-shadow 240ms ease;
}

.card:hover {
  transform: translateY(-2px);
  border-color: var(--vp-c-brand-2);
  box-shadow: 0 12px 28px -14px rgba(91, 124, 250, 0.35);
}

.card h3 {
  font-size: 17px;
  margin: 0 0 8px;
}

.card p {
  color: var(--vp-c-text-2);
  font-size: 14px;
  line-height: 1.6;
  margin: 0;
}
</style>
