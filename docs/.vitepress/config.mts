import { defineConfig, type DefaultTheme } from 'vitepress'
import { buildSidebar, buildNav } from './utils/sidebar.mjs'

const NOTES_ROOT = 'notes'

// Non-ASCII text (Chinese + emoji) is written via escape sequences on purpose:
// this source file is saved as plain text and some Windows toolchains default to
// a non-UTF-8 code page, which would otherwise mangle the characters.
const categories = [
  { dir: 'mbd', text: 'MBD', emoji: '\u{1F9EE}' },
  { dir: 'software', text: 'Software', emoji: '\u{1F4BB}' },
  { dir: 'algorithms', text: 'Algorithms', emoji: '\u{1F9E0}' },
  { dir: 'gpu_cuda', text: 'GPU / CUDA', emoji: '\u26A1' },
  { dir: 'AI_Infra', text: 'AI Infra', emoji: '\u{1F916}' },
  { dir: 'LLM', text: 'LLM', emoji: '\u{1FA84}' },
  { dir: 'Linux', text: 'Linux', emoji: '\u{1F427}' },
  { dir: 'Fall_Reviews', text: '\u79CB\u62DB', emoji: '\u{1F341}' },
  { dir: 'English', text: 'English', emoji: '\u{1F1EC}\u{1F1E7}' },
  { dir: 'travel', text: 'Travel', emoji: '\u2708\uFE0F' },
  { dir: 'web', text: 'Web', emoji: '\u{1F310}' },
]

const sidebar = buildSidebar(NOTES_ROOT, categories)
const nav = buildNav(NOTES_ROOT, categories)

export default defineConfig({
  lang: 'zh-CN',
  title: "K1n's Blog",
  description:
    '\u9762\u5411\u5D4C\u5165\u5F0F\u3001MBD\u3001GPU/CUDA\u3001AI Infra \u4E0E\u7B97\u6CD5\u7684\u4E2A\u4EBA\u6280\u672F\u7B14\u8BB0\u7AD9',
  cleanUrls: true,
  lastUpdated: true,
  ignoreDeadLinks: true,

  sitemap: {
    hostname: 'https://blog.k1n.asia',
  },

  head: [
    ['link', { rel: 'icon', type: 'image/svg+xml', href: '/favicon.svg' }],
    ['meta', { name: 'theme-color', content: '#646cff' }],
    ['meta', { property: 'og:type', content: 'website' }],
    ['meta', { property: 'og:locale', content: 'zh_CN' }],
    ['meta', { property: 'og:title', content: "K1n's Blog" }],
    [
      'script',
      {
        async: '',
        src: 'https://www.googletagmanager.com/gtag/js?id=G-WNZR555V1D',
      },
    ],
    [
      'script',
      {},
      `window.dataLayer = window.dataLayer || [];
function gtag(){dataLayer.push(arguments);}
gtag('js', new Date());
gtag('config', 'G-WNZR555V1D');`,
    ],
  ],

  markdown: {
    math: true,
    lineNumbers: true,
    image: {
      lazyLoading: true,
    },
  },

  transformPageData(pageData) {
    const fm = pageData.frontmatter as Record<string, unknown>
    if (fm && (fm.layout === 'note' || fm.layout === 'default')) {
      delete fm.layout
    }
  },

  themeConfig: {
    logo: { src: '/logo.svg', width: 24, height: 24 },
    siteTitle: "K1n's Blog",

    nav,
    sidebar,

    outline: {
      level: [2, 4],
      label: '\u672C\u9875\u76EE\u5F55',
    },

    search: {
      provider: 'local',
      options: {
        locales: {
          root: {
            translations: {
              button: {
                buttonText: '\u641C\u7D22\u6587\u6863',
                buttonAriaLabel: '\u641C\u7D22\u6587\u6863',
              },
              modal: {
                noResultsText: '\u65E0\u6CD5\u627E\u5230\u76F8\u5173\u7ED3\u679C',
                resetButtonTitle: '\u6E05\u9664\u67E5\u8BE2\u6761\u4EF6',
                footer: {
                  selectText: '\u9009\u62E9',
                  navigateText: '\u5207\u6362',
                  closeText: '\u5173\u95ED',
                },
              },
            },
          },
        },
      },
    } as DefaultTheme.LocalSearchOptions,

    socialLinks: [
      {
        icon: 'github',
        link: 'https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style',
      },
    ],

    footer: {
      message:
        '\u57FA\u4E8E MIT \u534F\u8BAE\u53D1\u5E03 \u00B7 \u4F7F\u7528 <a href="https://vitepress.dev" target="_blank">VitePress</a> \u6784\u5EFA',
      copyright: `Copyright \u00A9 2024-${new Date().getFullYear()} K1n`,
    },

    editLink: {
      pattern:
        'https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style/edit/main/docs/:path',
      text: '\u5728 GitHub \u4E0A\u7F16\u8F91\u6B64\u9875',
    },

    lastUpdated: {
      text: '\u6700\u540E\u66F4\u65B0\u4E8E',
      formatOptions: {
        dateStyle: 'medium',
        timeStyle: 'short',
      },
    },

    docFooter: {
      prev: '\u4E0A\u4E00\u7BC7',
      next: '\u4E0B\u4E00\u7BC7',
    },

    returnToTopLabel: '\u56DE\u5230\u9876\u90E8',
    sidebarMenuLabel: '\u83DC\u5355',
    darkModeSwitchLabel: '\u4E3B\u9898',
    lightModeSwitchTitle: '\u5207\u6362\u5230\u6D45\u8272\u6A21\u5F0F',
    darkModeSwitchTitle: '\u5207\u6362\u5230\u6DF1\u8272\u6A21\u5F0F',
  },
})
