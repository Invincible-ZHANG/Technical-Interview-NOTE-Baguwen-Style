import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import matter from 'gray-matter'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const DOCS_ROOT = path.resolve(__dirname, '../../')

const IGNORED_FILES = new Set(['index.md'])
const IGNORED_DIRS = new Set(['.vitepress', 'public', 'assets'])

function readTitle(absFile, fallbackName) {
  try {
    const raw = fs.readFileSync(absFile, 'utf-8')
    const { data } = matter(raw)
    const title = (data && data.title) || (data && data.Title)
    if (typeof title === 'string' && title.trim().length > 0) {
      return title.trim()
    }
  } catch {
    // ignore parse errors and use filename fallback
  }
  return fallbackName
}

function toLink(absFile) {
  const rel = path.relative(DOCS_ROOT, absFile).split(path.sep).join('/')
  return '/' + rel.replace(/\.md$/i, '')
}

function walkDir(absDir) {
  if (!fs.existsSync(absDir)) return []
  const entries = fs
    .readdirSync(absDir, { withFileTypes: true })
    .filter((e) => !e.name.startsWith('.'))
    .filter((e) => !IGNORED_DIRS.has(e.name))

  const dirs = entries.filter((e) => e.isDirectory())
  const files = entries.filter(
    (e) =>
      e.isFile() &&
      e.name.toLowerCase().endsWith('.md') &&
      !IGNORED_FILES.has(e.name),
  )

  dirs.sort((a, b) => a.name.localeCompare(b.name, 'zh-Hans-CN'))
  files.sort((a, b) => a.name.localeCompare(b.name, 'zh-Hans-CN'))

  const items = []

  for (const file of files) {
    const abs = path.join(absDir, file.name)
    const base = file.name.replace(/\.md$/i, '')
    items.push({
      text: readTitle(abs, base),
      link: toLink(abs),
    })
  }

  for (const dir of dirs) {
    const abs = path.join(absDir, dir.name)
    const children = walkDir(abs)
    if (children.length === 0) continue
    items.push({
      text: dir.name,
      collapsed: true,
      items: children,
    })
  }

  return items
}

export function buildSidebar(notesRoot, categories) {
  const sidebar = {}
  // "\u6982\u89C8" = "����"
  const OVERVIEW_LABEL = '\u6982\u89C8'
  for (const cat of categories) {
    const absCatDir = path.join(DOCS_ROOT, notesRoot, cat.dir)
    if (!fs.existsSync(absCatDir)) continue

    const intro = path.join(absCatDir, 'index.md')
    const catItems = []

    if (fs.existsSync(intro)) {
      catItems.push({
        text: OVERVIEW_LABEL,
        link: `/${notesRoot}/${cat.dir}/`,
      })
    }

    const walked = walkDir(absCatDir)
    if (walked.length > 0) {
      catItems.push({
        text: `${cat.emoji || ''} ${cat.text}`.trim(),
        collapsed: false,
        items: walked,
      })
    }

    if (catItems.length > 0) {
      sidebar[`/${notesRoot}/${cat.dir}/`] = catItems
    }
  }
  return sidebar
}

export function buildNav(notesRoot, categories) {
  // "\u9996\u9875" = "��ҳ", "\u66F4\u591A" = "����"
  const HOME_LABEL = '\u9996\u9875'
  const MORE_LABEL = '\u66F4\u591A'

  const nav = [{ text: HOME_LABEL, link: '/' }]

  const topCount = 4
  const top = categories.slice(0, topCount)
  const rest = categories.slice(topCount)

  for (const cat of top) {
    const absCatDir = path.join(DOCS_ROOT, notesRoot, cat.dir)
    if (!fs.existsSync(absCatDir)) continue
    nav.push({
      text: `${cat.emoji || ''} ${cat.text}`.trim(),
      link: `/${notesRoot}/${cat.dir}/`,
      activeMatch: `^/${notesRoot}/${cat.dir}/`,
    })
  }

  if (rest.length > 0) {
    nav.push({
      text: MORE_LABEL,
      items: rest
        .filter((cat) =>
          fs.existsSync(path.join(DOCS_ROOT, notesRoot, cat.dir)),
        )
        .map((cat) => ({
          text: `${cat.emoji || ''} ${cat.text}`.trim(),
          link: `/${notesRoot}/${cat.dir}/`,
        })),
    })
  }

  return nav
}
