// assets/js/image-zoom.js
(() => {
    const CONTENT_SELECTOR = '.post-content, .subpage-container, .main-content';
    const imgs = Array.from(document.querySelectorAll(`${CONTENT_SELECTOR} img:not(.no-zoom)`));
    if (!imgs.length) return;

    // 创建遮罩层
    const overlay = document.createElement('div');
    overlay.className = 'iv-overlay';
    overlay.innerHTML = `
    <img alt="">
    <div class="iv-toolbar" role="toolbar" aria-label="Image viewer toolbar">
      <button class="iv-btn iv-close" aria-label="关闭">✕</button>
      <button class="iv-btn iv-fs" aria-label="切换全屏">⛶</button>
      <a class="iv-btn iv-link iv-dl" aria-label="下载原图" download>⬇</a>
    </div>
  `;
    document.body.appendChild(overlay);

    const bigImg = overlay.querySelector('img');
    const btnClose = overlay.querySelector('.iv-close');
    const btnFS = overlay.querySelector('.iv-fs');
    const linkDL = overlay.querySelector('.iv-dl');

    let list = imgs;
    let idx = -1;

    function getSrc(img) {
        return img.currentSrc || img.src || img.getAttribute('data-src');
    }

    function openAt(i) {
        idx = i;
        const src = getSrc(list[idx]);
        if (!src) return;
        bigImg.src = src;
        bigImg.alt = list[idx].alt || '';
        linkDL.href = src;

        overlay.classList.add('open');
        // 禁止背景滚动
        document.documentElement.style.overflow = 'hidden';
        document.body.style.overflow = 'hidden';
    }

    function close() {
        overlay.classList.remove('open');
        bigImg.removeAttribute('src');
        document.exitFullscreen?.();
        document.documentElement.style.overflow = '';
        document.body.style.overflow = '';
    }

    // 点击正文图片打开
    list.forEach((img, i) => {
        img.addEventListener('click', (e) => {
            // 如果外面套了 <a>，避免直接跳转
            if (img.closest('a')) { e.preventDefault(); e.stopPropagation(); }
            openAt(i);
        });
    });

    // 点击空白处或按钮关闭
    overlay.addEventListener('click', (e) => {
        if (e.target === overlay) close();
    });
    btnClose.addEventListener('click', close);

    // 键盘控制
    document.addEventListener('keydown', (e) => {
        if (!overlay.classList.contains('open')) return;
        if (e.key === 'Escape') close();
        if (e.key === 'ArrowRight') openAt((idx + 1) % list.length);
        if (e.key === 'ArrowLeft') openAt((idx - 1 + list.length) % list.length);
    });

    // 切换浏览器“真正的全屏”
    btnFS.addEventListener('click', () => {
        if (!document.fullscreenElement) {
            overlay.requestFullscreen?.();
        } else {
            document.exitFullscreen?.();
        }
    });
})();
