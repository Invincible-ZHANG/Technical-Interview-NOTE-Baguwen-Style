# -*- coding: utf-8 -*-
"""
把源照片压成网页友好尺寸。

用法：python _tools/make_photos.py
读取本文件里的 SOURCES（slug → 原图路径），输出到 assets/photos/：
  <slug>-w480.jpg   小图（列表 / 移动端）
  <slug>-w960.jpg   中图（网格 / 平板）
  <slug>-w1800.jpg  大图（灯箱）

重新生成某个 slug 只是覆盖同名文件，原图不动，可以反复跑。
新照片 = 往 SOURCES 里加一行 + 同步在 _data/photos.yml 里加一条。
"""
import os
import sys
from PIL import Image, ImageOps

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "assets", "photos")

# slug  ->  源文件（相对 docs/）
SOURCES = {
    "cologne-dome-night":   "assets/images/Cologne.jpg",
    "eiffel-night":         "assets/images/background12.jpg",
    "beach-dusk":           "assets/images/Beach.png",
    "brussels-grand-place": "life/image/GrandPlace.png",
    "antwerp-cathedral":    "life/image/Cathedral.png",
    "antwerp-central":      "life/image/CentralStation.png",
}

WIDTHS = [(480, 74), (960, 78), (1800, 82)]


def convert(slug, rel_src):
    src = os.path.join(ROOT, rel_src.replace("/", os.sep))
    if not os.path.exists(src):
        print(f"  ! 缺少源文件 {rel_src}")
        return
    img = Image.open(src)
    img = ImageOps.exif_transpose(img)          # 按 EXIF 摆正
    if img.mode in ("RGBA", "LA", "P"):
        img = img.convert("RGB")

    w, h = img.size
    made = []
    for target, q in WIDTHS:
        if w <= target:
            im = img
        else:
            ratio = target / w
            im = img.resize((target, max(1, round(h * ratio))), Image.LANCZOS)
        out = os.path.join(OUT, f"{slug}-w{target}.jpg")
        im.save(out, "JPEG", quality=q, optimize=True, progressive=True)
        made.append(f"{target}:{os.path.getsize(out)//1024}KB")
    print(f"  ✓ {slug:22s} {w}x{h}  →  {'  '.join(made)}")


def main():
    os.makedirs(OUT, exist_ok=True)
    print(f"输出目录：{OUT}")
    for slug, rel in SOURCES.items():
        convert(slug, rel)
    total = sum(
        os.path.getsize(os.path.join(OUT, f)) for f in os.listdir(OUT)
    )
    print(f"\n共 {len(SOURCES)} 组，总体积 {total/1024/1024:.1f} MB")


if __name__ == "__main__":
    main()
