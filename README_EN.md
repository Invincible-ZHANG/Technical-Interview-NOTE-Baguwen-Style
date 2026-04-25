[中文](./README.md) | English

# K1n's Blog 📚

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![GitHub Pages](https://img.shields.io/badge/Pages-📖%20Online-blue)](https://invincible-zhang.github.io/Technical-Interview-NOTE-Baguwen-Style/)
[![VitePress](https://img.shields.io/badge/VitePress-1.x-3eaf7c)](https://vitepress.dev/)
[![Vue](https://img.shields.io/badge/Vue-3-42b883)](https://vuejs.org/)
[![Node](https://img.shields.io/badge/Node-%E2%89%A518-339933)](https://nodejs.org/)

> Personal tech notebook · Embedded · MBD · GPU/CUDA · AI Infra · Algorithms

---

## Table of Contents

- [Introduction](#introduction)
- [Tech Stack](#tech-stack)
- [Quick Start](#quick-start)
- [Key Features](#key-features)
- [Project Structure](#project-structure)
- [Authoring Conventions](#authoring-conventions)
- [Deployment](#deployment)
- [Migration Notes](#migration-notes)
- [Contributing](#contributing)
- [Comments](#comments)
- [Acknowledgements](#acknowledgements)
- [License](#license)

---

## Introduction

**K1n's Blog** is a personal knowledge base covering **Embedded Systems, Multibody Dynamics (MBD), Solvers & Numerical Optimization, GPU/CUDA, AI Infra, and Algorithms**.

All content is written in Markdown. The site is built with **VitePress + Vue 3 + Vite** and deployed to **GitHub Pages**. Local development supports hot reload; the build output is fully static.

> The repo was migrated from the legacy Jekyll (Ruby) stack to a modern frontend stack in 2026 — see [Migration Notes](#migration-notes).

---

## Tech Stack

| Area | Choice |
| --- | --- |
| Static Site Generator | [VitePress](https://vitepress.dev/) 1.x |
| Framework | Vue 3 |
| Build Tool | Vite |
| Language | Markdown + TypeScript |
| Math | MathJax 3 (`markdown-it-mathjax3`) |
| Comments | giscus + GitHub Discussions |
| Analytics | Google Analytics 4 |
| Runtime | Node.js ≥ 18 (LTS 20 recommended) |
| Deployment | GitHub Actions → GitHub Pages |

---

## Quick Start

### 1. Install Node.js

Download the LTS version (20.x recommended) from [nodejs.org](https://nodejs.org/), then verify:

```bash
node -v
npm -v
```

> Windows users: if `npm -v` complains about disabled scripts, run once as administrator:
> `Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned`

### 2. Clone & Run

```bash
git clone https://github.com/Invincible-ZHANG/Technical-Interview-NOTE-Baguwen-Style.git
cd Technical-Interview-NOTE-Baguwen-Style

npm install
npm run dev
```

Open <http://localhost:5173>. Edits to Markdown files hot reload automatically.

### 3. Scripts

| Command | Purpose |
| ---- | ---- |
| `npm run dev` | Start the dev server with HMR |
| `npm run build` | Build a production site to `docs/.vitepress/dist/` |
| `npm run preview` | Preview the production build locally |

For detailed setup, troubleshooting, giscus and custom-domain configuration, see [ENVIRONMENT_SETUP.md](./ENVIRONMENT_SETUP.md).

---

## Key Features

- 🚀 **Domain Coverage**: Embedded · Software · MBD · GPU/CUDA · AI Infra · LLM · Algorithms · Career Notes
- ✍️ **Markdown First**: All content authored in Markdown with YAML front matter
- 🧭 **Auto Sidebar**: Generated from `docs/notes/**` directory structure and front-matter titles — no config edits when adding posts
- 🔍 **Built-in Local Search**: Top-right search box, `Ctrl+K` to focus
- 🌗 **Dark Mode**: Auto follows system; tuned dark palette
- ➗ **Math**: Native `$...$` (inline) and `$$...$$` (block) LaTeX support
- 💬 **giscus Comments**: At the bottom of every article, theme-aware
- 📈 **GA4**: Google Analytics 4 baked in
- ✏️ **Edit on GitHub**: Per-page edit link to the source file
- ⏱ **Last Updated**: Auto injected from git commit history
- 🎨 **Modern Brand**: Purple-blue-pink gradient, frosted-glass navbar, card hover micro-animations

---

## Project Structure

```text
.
├── package.json                    # Node deps & build scripts
├── docs/                           # VitePress source (srcDir)
│   ├── index.md                    # Home (Hero + Features)
│   ├── public/                     # Static assets served at root
│   │   ├── favicon.svg
│   │   ├── logo.svg
│   │   └── hero.svg
│   ├── notes/                      # All notes
│   │   ├── mbd/                    # Multibody dynamics, solvers, parallelism
│   │   ├── software/               # C/C++, embedded, device tree, RTOS
│   │   ├── algorithms/             # LeetCode, DP, BFS, trees
│   │   ├── gpu_cuda/               # CUDA, parallel computing
│   │   ├── AI_Infra/               # AI infra, scientific computing
│   │   ├── LLM/                    # Transformer, LLM tour
│   │   ├── Linux/                  # Kernel, system programming
│   │   ├── Fall_Reviews/           # Interview experiences
│   │   ├── English/                # Language learning
│   │   ├── travel/                 # Travel logs
│   │   └── web/                    # Web miscellany
│   └── .vitepress/
│       ├── config.mts              # Main site config
│       ├── utils/
│       │   └── sidebar.mjs         # Auto-scans dirs, builds sidebar/nav
│       └── theme/
│           ├── index.ts            # Theme entry (extends default theme)
│           ├── styles/
│           │   ├── vars.css        # Brand colors & dark-mode vars
│           │   └── custom.css      # Glass navbar, card hover, callouts
│           └── components/
│               ├── Giscus.vue              # Comments
│               └── HomeFeatureExtras.vue   # Home extra section
├── .github/
│   └── workflows/deploy.yml        # GitHub Pages auto-deploy
├── examples/                       # Sample code (C++/Python/CUDA …)
├── resume/                         # Resume
├── scripts/                        # Helper scripts
├── ENVIRONMENT_SETUP.md            # Detailed environment guide
├── CHANGELOG.md                    # Changelog
└── README.md / README_EN.md        # Project overview
```

---

## Authoring Conventions

To add a new note, drop a file under `docs/notes/<category>/`:

```markdown
---
title: My New Note
---

# Body starts here
```

Notes:

- `title` is recommended in front matter; the sidebar uses it (falls back to filename otherwise)
- Legacy `layout: note` / `layout: default` from the Jekyll era is auto-stripped via `transformPageData` in `config.mts` — historical files don't need editing
- Subdirectories render as collapsible groups automatically
- Add `comments: false` to front matter to hide giscus on a specific page
- Math works out of the box with `$E=mc^2$` or `$$ ... $$`

To add a new category (e.g. `embedded`), edit the `categories` array in `docs/.vitepress/config.mts`:

```ts
{ dir: 'embedded', text: 'Embedded', emoji: '\u{1F50C}' },
```

Then place content under `docs/notes/embedded/`.

---

## Deployment

### GitHub Pages (default)

`.github/workflows/deploy.yml` is preconfigured:

- Triggered on every push to `main`
- Installs Node 20 via `actions/setup-node@v4`
- Runs `npm ci` + `npm run build`
- Uploads `docs/.vitepress/dist/` and publishes to GitHub Pages

**First-time setup**: Repo → Settings → Pages → Source = `GitHub Actions`.

### Custom Domain

1. Add a `CNAME` file under `docs/public/` containing your domain
2. Add `sitemap: { hostname: 'https://your-domain' }` to `docs/.vitepress/config.mts`
3. Point your domain's DNS `CNAME` to `<username>.github.io`

### Preview the Build Locally

```bash
npm run build
npm run preview
```

---

## Migration Notes

This project went through one architectural migration:

| | Legacy (≤ 2025) | Current (2026+) |
| --- | --- | --- |
| Static Site Generator | Jekyll 3.10 | **VitePress 1.x** |
| Runtime | Ruby + Bundler | **Node.js** |
| Theming | Hand-written `_layouts` + Minima | **Vue components on top of default theme** |
| Config | `_config.yml` | **`.vitepress/config.mts`** |
| Sidebar | Manual `_data/navigation.yml` | **Auto-generated from filesystem** |
| Search | None | **Built-in local search** |
| Dark Mode | None | **Out of the box** |
| Math | jsDelivr CDN MathJax | **VitePress built-in** |

Migration removed all Jekyll artifacts (`Gemfile`, `_config.yml`, `_layouts/`, `_includes/`, `_data/`, `assets/`) while preserving every Markdown file under `docs/notes/**`. To roll back, use git history.

---

## Contributing

Issues and pull requests are welcome:

1. Fork the repository
2. Create a feature branch: `feature/xxx`
3. Commit and push your changes
4. Open a PR describing the change

---

## Comments

This project uses [GitHub Discussions](https://docs.github.com/en/discussions) + [giscus](https://giscus.app/):

- **Read/post comments**: scroll to the bottom of any article
- **First-time setup**: visit <https://giscus.app/>, generate the config, and copy `data-repo-id` and `data-category-id` into the constants in `docs/.vitepress/theme/components/Giscus.vue`

---

## Acknowledgements

- [VitePress](https://vitepress.dev/) — excellent Vue-based docs/blog framework
- The [Vue.js](https://vuejs.org/) and [Vite](https://vitejs.dev/) teams
- [SimonAKing/HomePage](https://github.com/SimonAKing/HomePage) — inspiration for the legacy animated home page
- All issue / PR contributors 🙏

---

## License

Released under the [MIT License](LICENSE). Free to use, modify, and redistribute — please keep the copyright notice.
