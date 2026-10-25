# 怎么写一篇新文章

这个站点没有后台、没有编辑器。**新建一个 `.md` 文件就是发一篇文章**，剩下的交给 Jekyll。

---

## 一、先选地方：路径决定它出现在哪

| 你写的内容 | 放在 | 出现在 |
|---|---|---|
| 成像链路、ISP、sensor、驱动侧的积累 | `docs/log/sensor/` | `/log/sensor/` Sensor 与成像 |
| 构图、光、色彩、审美上的想法 | `docs/log/aesthetic/` | `/log/aesthetic/` 摄影美学 |
| 旅行、读书、日常，跟技术无关 | `docs/life/` | `/life/` 生活记录 |
| 游戏攻略、路线打法 | `docs/game/` | `/game/` 游戏 |
| 以前的技术笔记，只是留个档 | `docs/notes/<分类>/` | `/notes/` 归档列表 |

> ⚠️ **影像笔记必须放进 `sensor/` 或 `aesthetic/` 子目录。**
> 直接丢在 `docs/log/` 根目录下的 .md 不会出现在任何列表里——总览页按子目录名归集，
> 认不出归属的就静默消失了。

旧技术笔记不用再投入精力，扔进 `notes/` 对应的分类文件夹里就会自动进归档列表。

---

## 二、最小可用模板

新建 `docs/log/sensor/2026-10-25-some-title.md`（写美学就换成 `aesthetic/`），开头必须是 `---` 包起来的这一段：

```yaml
---
title: 夜里在桥上拍的二十分钟
date: 2026-10-25
categories: [摄影, 夜景]
tags: [手持, 长曝光, 噪点]
excerpt: 风很大，栏杆是凉的，最后只有两张能用。记一下为什么。
---

正文从这里开始。
```

**`layout` 不用写。** `_config.yml` 已经给 `notes/`、`log/`、`life/` 三个目录统一指定了 `note` 布局，写了也是多余的。

---

## 三、每个字段管什么

| 字段 | 要不要写 | 作用 |
|---|---|---|
| `title` | **必写** | 页面标题、列表标题、搜索结果标题 |
| `date` | **必写** | 手记列表靠它排序和分年份，写错就排错位置 |
| `excerpt` | 强烈建议 | 列表页摘要 + 搜索结果里那一行小字；不写就没有 |
| `categories` | 建议 | 文章页头部的分类标签 |
| `tags` | 可选 | 标签，用于检索 |
| `cover` | 生活记录用 | 列表卡片的封面图，例如 `/life/image/GrandPlace.png` |
| `place` | 生活记录用 | 地点，会显示在卡片上 |

---

## 四、两条必须遵守的规则

**1. 文件名就是网址。** 用英文小写 + 连字符：

- ✅ `raw-headroom.md` → `/log/raw-headroom.html`
- ❌ `RAW 能救回什么.md` → 网址变成一串 `%E6%8B%AF...`，分享出去很难看

**2. 正文不要写 `# 一级标题`。** 页面布局会自动把 `title` 渲染成 H1，你正文里再写一个就成了两个大标题。**正文一律从 `##` 开始。**

---

## 五、图片

图片放 `docs/assets/images/`（通用）或 `docs/life/image/`（生活记录），然后这样引用：

```markdown
![科隆大教堂夜景](/assets/images/Cologne.jpg)
```

- 用**以 `/` 开头的绝对路径**，不要用相对路径，否则子目录里的文章会找不到图
- 上传前压一下：长边 1600px、单张 200KB 以内，手机原图直接传会让页面很慢

---

## 六、代码块

四个反引号里写语言名，高亮是本地 Prism，不依赖外网：

````markdown
```python
def exposure(iso, shutter):
    return iso * shutter
```
````

常见语言：`python` `c` `cpp` `bash` `yaml` `json` `javascript` `makefile`

数学公式用 `$...$` 和 `$$...$$`，MathJax 已本地化。

---

## 七、往摄影页加一张照片
这一步比写文章多一个环节，因为要先出三种尺寸的图：

1. 把原图放进 `docs/assets/images/`（或任意位置）
2. 打开 `docs/_tools/make_photos.py`，在 `SOURCES` 里加一行：
   ```python
   "westlake-morning": "assets/images/IMG_1234.jpg",
   ```
3. 跑一次脚本，自动生成 `-w480` / `-w960` / `-w1800` 三个文件：
   ```
   C:\Users\kin\.workbuddy\binaries\python\envs\default\Scripts\python.exe docs\_tools\make_photos.py
   ```
4. 在 `docs/_data/photos.yml` 末尾追加一条：

```yaml
- slug: westlake-morning        # 必须和 SOURCES 里的键一致
  title: 西湖，六点
  date: 2026-10-25
  place: 杭州 · 中国
  series: 光                  # 用来做筛选：夜行 / 光 / 路上
  camera: 手机 · 主摄
  settings: 24mm · f/1.8 · 1/60s · ISO 100
  ratio: "3/2"                # 4/3 横、3/2 横、2/3 竖
  featured: false             # true 才会进首页那组照片
  tags: [清晨, 湖]
  note: >
    写一两句当时为什么按快门。灯箱里会显示这段。
```

`camera` 和 `settings` 写不准就留空，页面上会自动省略那一行，不会留空壳。

---

## 八、本地看效果

项目根目录双击 `serve.cmd`，浏览器打开 <http://127.0.0.1:4000>。

它带自动重建，存一次 `.md` 刷新页面就能看到，不用重启。只检查能不能构建成功就直接跑 `build.cmd`。

---

## 九、发布

```bash
git add -A
git commit -m "新增：夜里在桥上拍的二十分钟"
git push
```

推上去之后 GitHub Actions 自动构建部署，一两分钟后 <https://www.zhangzijian.cn> 就能看到。

---

## 十、最容易踩的坑

| 现象 | 原因 |
|---|---|
| 文章点开是.markdown 源码 | 文件开头忘了写 `---` 那段 front matter，Jekyll 当静态文件原样复制了 |
| 文章写完了但列表里找不到 | 直接放在 `docs/log/` 根目录下了，必须进 `sensor/` 或 `aesthetic/` 子目录 |
| 手记列表里位置不对 | `date` 没写或格式不是 `2026-10-25` |
| 页面出现两个大标题 | 正文里又写了一次 `# 标题` |
| 图片显示不出来 | 用了相对路径，或图片没放进 `assets/` |
| 中文网址 | 文件名用了中文，改成英文小写连字符 |

---

## 十一、改游戏地图

地图用 Leaflet 1.9.4 做的（本地部署在 `docs/assets/vendor/leaflet/`），
配 `CRS.Simple` 把一张图当坐标平面，所以能拖拽、缩放，点位是可以点的标记。

三个文件各管一件事：

| 文件 | 管什么 |
|---|---|
| `docs/assets/maps/space-base.svg` | 底图。区域色块、水渠、桥，纯图片 |
| `docs/assets/js/space-map.js` | 开头的 `P` 对象：每个点位的坐标和文案 |
| `docs/_includes/space-map.html` | 工具栏、地图容器、脚本引用 |

**加点位只改 `P` 对象**，底图和 include 都不用动：

```js
newpoint: {
    name: '点位全名',        // 面板标题
    short: '短名',           // 地图上的标签；留空就只显示圆点
    type: 'loot',           // loot 资源 / exit 撤离 / gate 通道，决定筛选归类和颜色
    dir: 'r',               // 标签往哪边写：r 右 / l 左 / t 上 / b 下，用来避开邻居
    x: 500, y: 400,         // 底图像素坐标，画布 1200 x 900，原点在左上
    sub: '副标题，写在名字下面',
    jm: '机密模式下的说明，可以带 <strong>',
    js: '绝密模式下的说明'
}
```

坐标怎么量：直接用浏览器打开 `space-base.svg`，按画布尺寸估就行。
代码里会自动把 y 翻过来（`yx()` 那个函数），不用自己算。

**换底图**：想要更真实的效果，把游戏内地图截图存成
`docs/assets/maps/space-base.png`（或 .svg），改 include 里 `data-base` 的路径，
再对着新图把 `P` 里的 x / y 重新量一遍。其余代码不用改。
