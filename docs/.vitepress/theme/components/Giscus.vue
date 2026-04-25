<script setup lang="ts">
import { onMounted, onUnmounted, ref, watch } from 'vue'
import { useData, useRoute } from 'vitepress'

const route = useRoute()
const { isDark, frontmatter } = useData()
const container = ref<HTMLDivElement | null>(null)

const GISCUS_REPO = 'Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style'
const GISCUS_REPO_ID = ''
const GISCUS_CATEGORY = 'Comments'
const GISCUS_CATEGORY_ID = ''

// Non-ASCII text uses \u escapes on purpose: this Vue file is saved as plain
// text and some Windows toolchains default to a non-UTF-8 code page.
// "\u8BC4\u8BBA\u533A" = "评论区"
const HEADING = '\u{1F4AC} \u8BC4\u8BBA\u533A'
// "\u57FA\u4E8E GitHub Discussions\uFF0C\u9700\u767B\u5F55 GitHub \u8D26\u53F7\u53D1\u8868\u8BC4\u8BBA\u3002"
const TIP =
  '\u57FA\u4E8E GitHub Discussions\uFF0C\u9700\u767B\u5F55 GitHub \u8D26\u53F7\u53D1\u8868\u8BC4\u8BBA\u3002'

function mountGiscus() {
  if (!container.value) return
  if (frontmatter.value?.comments === false) return
  container.value.innerHTML = ''

  const script = document.createElement('script')
  script.src = 'https://giscus.app/client.js'
  script.async = true
  script.crossOrigin = 'anonymous'
  script.setAttribute('data-repo', GISCUS_REPO)
  if (GISCUS_REPO_ID) script.setAttribute('data-repo-id', GISCUS_REPO_ID)
  script.setAttribute('data-category', GISCUS_CATEGORY)
  if (GISCUS_CATEGORY_ID)
    script.setAttribute('data-category-id', GISCUS_CATEGORY_ID)
  script.setAttribute('data-mapping', 'pathname')
  script.setAttribute('data-strict', '0')
  script.setAttribute('data-reactions-enabled', '1')
  script.setAttribute('data-emit-metadata', '0')
  script.setAttribute('data-input-position', 'bottom')
  script.setAttribute('data-theme', isDark.value ? 'dark_dimmed' : 'light')
  script.setAttribute('data-lang', 'zh-CN')
  script.setAttribute('data-loading', 'lazy')
  container.value.appendChild(script)
}

function updateTheme(dark: boolean) {
  const iframe = document.querySelector<HTMLIFrameElement>(
    'iframe.giscus-frame',
  )
  if (!iframe) return
  iframe.contentWindow?.postMessage(
    { giscus: { setConfig: { theme: dark ? 'dark_dimmed' : 'light' } } },
    'https://giscus.app',
  )
}

onMounted(() => {
  mountGiscus()
})

onUnmounted(() => {
  if (container.value) container.value.innerHTML = ''
})

watch(
  () => route.path,
  () => {
    mountGiscus()
  },
)

watch(isDark, (dark) => {
  updateTheme(dark)
})
</script>

<template>
  <div class="giscus-wrapper">
    <hr />
    <h2>{{ HEADING }}</h2>
    <p class="giscus-tip">{{ TIP }}</p>
    <div ref="container" class="giscus-slot"></div>
  </div>
</template>

<style scoped>
.giscus-wrapper {
  margin-top: 48px;
}

.giscus-wrapper hr {
  border: none;
  border-top: 1px dashed var(--vp-c-divider);
  margin-bottom: 24px;
}

.giscus-tip {
  color: var(--vp-c-text-2);
  font-size: 14px;
  margin-bottom: 20px;
}

.giscus-slot {
  min-height: 120px;
}
</style>
