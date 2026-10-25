/* 全站 UI 增强：阅读进度条 / 导航高亮 / 平滑回到顶部 / 外链新窗口 */
(function () {
    'use strict';

    /* ─── 1. 阅读进度条 ─── */
    var bar = document.querySelector('.reading-progress-bar');
    if (bar) {
        var ticking = false;
        function update() {
            var h = document.documentElement;
            var scrollable = h.scrollHeight - h.clientHeight;
            var pct = scrollable > 0 ? (h.scrollTop || document.body.scrollTop) / scrollable : 0;
            bar.style.width = Math.min(100, Math.max(0, pct * 100)).toFixed(2) + '%';
            ticking = false;
        }
        window.addEventListener('scroll', function () {
            if (!ticking) { ticking = true; window.requestAnimationFrame(update); }
        }, { passive: true });
        update();
    }

    /* ─── 2. 导航栏当前位置高亮 ─── */
    function markActive() {
        var here = location.pathname.replace(/index\.html$/, '');
        var links = document.querySelectorAll('.sticky-nav-link[href], .dropdown-link[href]');
        links.forEach(function (a) {
            var href = a.getAttribute('href') || '';
            if (!href || href.charAt(0) !== '/') return;
            var target = href.replace(/index\.html$/, '');
            if (target === '/' || target === '') {
                if (here === '/' || here === '/index.html') a.classList.add('is-active');
                return;
            }
            if (here === target || here.indexOf(target) === 0) a.classList.add('is-active');
        });
    }

    /* ─── 3. 平滑回到顶部 ─── */
    function bindTop() {
        document.addEventListener('click', function (e) {
            var a = e.target.closest && e.target.closest('a');
            if (!a) return;
            var href = a.getAttribute('href') || '';
            if (href === '#top' || href === '#') {
                e.preventDefault();
                window.scrollTo({ top: 0, behavior: 'smooth' });
            }
        });
    }

    /* ─── 4. 正文外链在新窗口打开 ─── */
    function externalLinks() {
        var scope = document.querySelector('.subpage-container, .main-content');
        if (!scope) return;
        scope.querySelectorAll('a[href^="http"]').forEach(function (a) {
            if (a.hostname && a.hostname === location.hostname) return;
            a.setAttribute('target', '_blank');
            a.setAttribute('rel', 'noopener noreferrer');
        });
    }

    function init() {
        markActive();
        bindTop();
        externalLinks();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
