/* ============================================================
   lightbox.js —— 照片灯箱 + 系列筛选
   无依赖，纯原生。图为空时整段直接跳过。
   ============================================================ */
(function () {
    'use strict';

    var box = document.getElementById('lightbox');
    if (!box) return;

    var img = document.getElementById('lbImg');
    var elTitle = document.getElementById('lbTitle');
    var elPlace = document.getElementById('lbPlace');
    var elNote = document.getElementById('lbNote');
    var elExif = document.getElementById('lbExif');
    var elCount = document.getElementById('lbCount');

    /* 摄影页（.photo-btn / .gallery-hero-btn）和首页（.hp-shot）共用 */
    var allBtns = Array.prototype.slice.call(
        document.querySelectorAll('.photo-btn, .gallery-hero-btn, .hp-shot')
    );
    var grid = document.getElementById('photoGrid');

    /* 当前可见的一组 —— 筛选之后只在这些之间切换 */
    var set = allBtns.slice();
    var cur = 0;
    var lastFocus = null;

    function visible(el) {
        return el && el.offsetParent !== null;
    }

    function refreshSet() {
        set = allBtns.filter(visible);
    }

    function fill(btn) {
        var d = btn.dataset;
        img.src = d.full || '';
        img.alt = d.title || '';
        elTitle.textContent = d.title || '';
        elPlace.textContent = d.place || '';
        elNote.textContent = d.note || '';
        elExif.textContent = [d.camera, d.settings].filter(Boolean).join('  ·  ');
        elPlace.hidden = !d.place;
        elNote.hidden = !d.note;
        elExif.hidden = !elExif.textContent;
        elCount.textContent = set.length > 1 ? (cur + 1) + ' / ' + set.length : '';
    }

    function preload(i) {
        var b = set[i];
        if (!b || !b.dataset.full) return;
        var p = new Image();
        p.src = b.dataset.full;
    }

    function open(btn) {
        refreshSet();
        cur = set.indexOf(btn);
        if (cur < 0) { set = [btn]; cur = 0; }
        lastFocus = btn;
        box.hidden = false;
        document.documentElement.classList.add('lb-open');
        fill(set[cur]);
        elExif.setAttribute('aria-live', 'polite');
        if (set.length > 1) {
            preload((cur + 1) % set.length);
            preload((cur - 1 + set.length) % set.length);
        }
        box.querySelector('.lb-close').focus();
    }

    function close() {
        box.hidden = true;
        document.documentElement.classList.remove('lb-open');
        img.removeAttribute('src');
        if (lastFocus) lastFocus.focus();
    }

    function step(n) {
        if (set.length < 2) return;
        cur = (cur + n + set.length) % set.length;
        fill(set[cur]);
        preload((cur + n + set.length) % set.length);
    }

    /* ── 事件 ── */
    document.addEventListener('click', function (ev) {
        var trigger = ev.target.closest('.photo-btn, .gallery-hero-btn');
        if (trigger) { ev.preventDefault(); open(trigger); return; }

        if (box.hidden) return;
        var t = ev.target;
        if (t.closest('[data-lb-close]')) { close(); return; }
        if (t.closest('[data-lb-prev]')) { step(-1); return; }
        if (t.closest('[data-lb-next]')) { step(1); }
    });

    document.addEventListener('keydown', function (ev) {
        if (box.hidden) return;
        if (ev.key === 'Escape') { ev.preventDefault(); close(); }
        else if (ev.key === 'ArrowLeft') { ev.preventDefault(); step(-1); }
        else if (ev.key === 'ArrowRight') { ev.preventDefault(); step(1); }
        else if (ev.key === 'Tab') {
            /* 焦点锁在灯箱里，别跑到底层页面 */
            var f = box.querySelectorAll('button');
            if (!f.length) return;
            var first = f[0], last = f[f.length - 1];
            if (ev.shiftKey && document.activeElement === first) { ev.preventDefault(); last.focus(); }
            else if (!ev.shiftKey && document.activeElement === last) { ev.preventDefault(); first.focus(); }
        }
    });

    /* 触摸左右滑动 */
    var startX = 0, startY = 0, tracking = false;
    box.addEventListener('touchstart', function (e) {
        if (e.touches.length !== 1) return;
        startX = e.touches[0].clientX;
        startY = e.touches[0].clientY;
        tracking = true;
    }, { passive: true });

    box.addEventListener('touchend', function (e) {
        if (!tracking) return;
        tracking = false;
        var dx = e.changedTouches[0].clientX - startX;
        var dy = e.changedTouches[0].clientY - startY;
        if (Math.abs(dx) > 60 && Math.abs(dx) > Math.abs(dy) * 1.6) step(dx < 0 ? 1 : -1);
    }, { passive: true });

    /* ── 系列筛选 ── */
    var chips = Array.prototype.slice.call(document.querySelectorAll('.gf-chip'));
    var empty = document.getElementById('galleryEmpty');

    function applyFilter(name) {
        var shown = 0;
        Array.prototype.forEach.call(grid.children, function (card) {
            var hit = name === 'all' || card.dataset.series === name;
            card.hidden = !hit;
            if (hit) shown++;
        });
        /* 筛选时收起首图，避免「封面」和筛选结果对不上 */
        var hero = document.querySelector('.gallery-hero');
        if (hero) hero.hidden = name !== 'all';
        if (empty) empty.hidden = shown > 0;
        refreshSet();
    }

    chips.forEach(function (chip) {
        chip.addEventListener('click', function () {
            chips.forEach(function (c) { c.classList.toggle('is-on', c === chip); });
            applyFilter(chip.dataset.filter);
        });
    });
})();
