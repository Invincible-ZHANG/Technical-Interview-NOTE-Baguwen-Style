import { h } from 'vue'
import DefaultTheme from 'vitepress/theme'
import type { Theme } from 'vitepress'
import Giscus from './components/Giscus.vue'
import HomeFeatureExtras from './components/HomeFeatureExtras.vue'
import './styles/vars.css'
import './styles/custom.css'

export default {
  extends: DefaultTheme,
  Layout: () =>
    h(DefaultTheme.Layout, null, {
      'doc-after': () => h(Giscus),
      'home-features-after': () => h(HomeFeatureExtras),
    }),
} satisfies Theme
