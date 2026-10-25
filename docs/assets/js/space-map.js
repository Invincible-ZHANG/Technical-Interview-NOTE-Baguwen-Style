/* ============================================================
   space-map.js —— 航天基地互动地图（Leaflet 版）
   坐标系：底图 space-base.svg 的像素坐标，1200 x 900，原点左上。
   Leaflet 的 CRS.Simple 用 [y, x]，所以要翻转 y（见 yx()）。

   点位的 x / y 直接对着底图量，改底图就要跟着改这里。
   数据依据 2026-10 检索的公开攻略（官方地图详解为主），
   游戏平衡会调整，数值以局内为准。
   ============================================================ */

(function () {
    'use strict';

    if (typeof L === 'undefined') return;

    var MAP_W = 1200;
    var MAP_H = 900;

    var P = {

        /* ─── 核心区五房 ─── */
        centrifuge: {
            name: '离心机室',
            short: '离心机室',
            type: 'loot',
            dir: 'b',
            x: 420, y: 540,
            sub: '核心区西侧 · 31 个固定物资点 · 一楼有拉闸点',
            jm: '核心区最肥的一间，也是一楼大厅那个<strong>拉闸点</strong>所在。拉闸会响，拉完立刻上楼或离开原地，别站在闸口等人。',
            js: '绝密里这声闸响传得更远。刷得远就别硬挤——绝密死一次的成本比多摸一间高得多。'
        },

        buoyancy: {
            name: '浮力室',
            short: '浮力室',
            type: 'loot',
            dir: 't',
            x: 580, y: 460,
            sub: '核心区西北 · 22 点 · 深潜池 + 二层平台',
            jm: '水池结构。深潜池里有水下物资点和保险箱，得潜下去拿；二层平台有<strong>滑索直连总裁室</strong>，是进中央最安全的一条路。',
            js: '潜下去就听不见岸上的脚步。绝密里最容易在水下被人堵住，下水前先确认岸上干净。'
        },

        blue: {
            name: '蓝室',
            short: '蓝室',
            type: 'loot',
            dir: 'r',
            x: 750, y: 535,
            sub: '核心区东侧 · 双闸撤离的另一处闸',
            jm: '双闸撤离的第二处闸在这儿。两处都拉完，喷泉广场才开，然后再等 5 分钟。',
            js: '拉完两个闸，全图大概都知道你在撤了。绝密里别从闸口直奔撤离点，先绕一下看有没有人跟。'
        },

        black: {
            name: '黑室',
            short: '黑室',
            type: 'loot',
            dir: 'b',
            x: 585, y: 665,
            sub: '核心区南侧 · 滑索连总裁室',
            jm: '从西北门卡通道进来后的第一站，和总裁室之间有滑索，是西线最顺的开局。',
            js: '西线进核心区的口子只有一两个。绝密里落地先在口子外面听两秒再进。'
        },

        president: {
            name: '总裁室',
            short: '总裁室',
            type: 'loot',
            dir: 'r',
            x: 615, y: 580,
            sub: '核心区中央悬空 · 11 点 · 只能靠滑索进出',
            jm: '悬在半空，只能从浮力室或黑室滑索过来。点少但档次高，开柜慢，而且<strong>索道一次只能过一个人</strong>，得留人守两头。',
            js: '这地方是死胡同——进来容易出去难，索道就是唯一的路。绝密里另一头没人接就别进。'
        },

        /* ─── 外围上区 ─── */
        control: {
            name: '中控楼',
            short: '',
            type: 'loot',
            dir: 'r',
            x: 385, y: 170,
            sub: '北侧 · 29 点 · 外围最肥',
            jm: '外围物资最多的一栋，二楼保险柜优先。更值钱的是视野——二楼平台正对中控桥，是这张图的架桥位。',
            js: '绝密里中控楼二楼就是制高点。想安全过桥，先确认楼上是自己人。'
        },

        dorm: {
            name: '宿舍区',
            short: '',
            type: 'loot',
            dir: 'r',
            x: 165, y: 185,
            sub: '西北 · 21 点 · 常见出生点',
            jm: '停车场、中庭、三间两层宿舍楼。总量不多，但桌面和置物架常刷电子产品，路过顺手摸。',
            js: '西北最常见的出生点。绝密里落这儿算运气好——离核心区远，可以先吃干净再决定进不进。'
        },

        print: {
            name: '工业区 · 打印室',
            short: '打印室',
            type: 'loot',
            dir: 'r',
            x: 622, y: 172,
            sub: '东侧 · 西侧流水线',
            jm: '工业区 26 个点里比较集中的一块，工具盒多。搜完往组装室走，别在吊装室和罐装区耗。',
            js: '工业区是东边唯一的入口方向，来的人不少。绝密里进门先清角落。'
        },

        assemble: {
            name: '工业区 · 组装室',
            short: '组装室',
            type: 'loot',
            dir: 'r',
            x: 845, y: 172,
            sub: '东侧 · 东侧二层办公区',
            jm: '和打印室同一套结构，资源集中在东侧二层。这里离东区吊桥最近，搜完直接上桥进核心区。',
            js: '二层窗口能看到吊桥。绝密里过桥前先抬头看一眼有没有人架着。'
        },

        hangar: {
            name: '工业区 · 吊装室',
            short: '吊装室',
            type: 'loot',
            dir: 'b',
            x: 622, y: 275,
            sub: '东侧 · 物资少',
            jm: '这间穷，路过看一眼就行，时间留给打印室和组装室。',
            js: '绝密里更没必要在这耗——多待一分钟，核心区就多一队人。'
        },

        /* ─── 撤离点 ─── */
        workshop: {
            name: '水平试车间 · 丢包撤离',
            short: '',
            type: 'exit',
            dir: 'r',
            x: 905, y: 828,
            sub: '东南角 · 开局约 9 分钟开放 · 弃包等 10 秒',
            jm: '这张图最稳的兜底。背包丢下去永久留在战场，<strong>值钱的要提前塞进安全箱、胸挂和口袋</strong>。名额 3 个，三人队正好一人一个。',
            js: '名额只有 <strong>1 个</strong>。这是两个模式最大的差别：单排来晚了就是白跑一趟，出发前先确认名额还在。',
            slot: { jm: '3', js: '1' }
        },

        fountain: {
            name: '喷泉广场 · 双闸撤离',
            short: '喷泉广场',
            type: 'exit',
            dir: 'r',
            x: 800, y: 675,
            sub: '离心机室 + 蓝室 各拉一处闸 · 等 5 分钟',
            jm: '两处闸都拉完才会开，然后等 5 分钟。关键判断：<strong>开局 10 分钟内没拉第一个闸，基本来不及拉第二个</strong>，直接改丢包。',
            js: '拉闸动静全图能听。绝密里拉完别往这边直走，先在附近架一会儿，等闻声过来的人先露头。'
        },

        rocket: {
            name: '火箭平台 · 飞升撤离',
            short: '火箭平台',
            type: 'exit',
            dir: 'r',
            x: 400, y: 820,
            sub: '发射区 · 约 15 分钟接任务 · 全图通知',
            jm: '在发射区大桥接「飞升者行动」，接完全图可见。全程大约 270 秒防守 + 60 秒撤离，做完火箭上还会刷一个物资点。',
            js: '绝密里接飞升等于广播自己的位置。三个人装备都齐、都愿意守，才值得做。'
        },

        /* ─── 通道 ─── */
        bridge: {
            name: '中控桥',
            short: '中控桥',
            type: 'gate',
            dir: 'l',
            x: 385, y: 317,
            sub: '中控楼 ↔ 核心区 · 主通道',
            jm: '正门过桥最快，也最容易被架——中控楼二楼正对着整座桥。没把握就走西北的门卡通道。',
            js: '绝密里这座桥基本都有人看。过桥前在桥头停三秒听枪声，不要一边跑一边看。'
        },

        cable: {
            name: '东区吊桥',
            short: '东区吊桥',
            type: 'gate',
            dir: 'r',
            x: 643, y: 352,
            sub: '工业区 ↔ 核心区',
            jm: '东边进核心区的路。搜完工业区直接上桥，比绕到中控桥近一大截。',
            js: '桥面窄。绝密里被人堵在桥上基本没得跑，过之前先看对岸。'
        },

        tunnel: {
            name: '地下通道',
            short: '地下通道',
            type: 'gate',
            dir: 't',
            x: 535, y: 275,
            sub: '工业区 ↔ 中控楼',
            jm: '连接东边和北边的暗路，转移时不容易被看见。',
            js: '通道里没有掩体。绝密里迎面撞上就是硬碰硬，进去前先听脚步。'
        },

        keycard: {
            name: '门卡通道',
            short: '门卡通道',
            type: 'gate',
            dir: 'l',
            x: 315, y: 460,
            sub: '核心区西北 · 需门卡',
            jm: '有员工通道一类的门卡，就能绕开中控桥正面，直接从西北进核心区。没卡只能走桥或吊桥。',
            js: '开卡有声音。绝密里开完别马上进，等三秒听反应。'
        }
    };

    /* 两条固定打法路线 */
    var ROUTES = [
        { color: '#8fd6c4', keys: ['cable', 'centrifuge', 'buoyancy', 'president', 'fountain', 'workshop'] },
        { color: '#e8c477', keys: ['keycard', 'black', 'president', 'fountain', 'workshop'] }
    ];

    var TYPE_LABEL = { loot: '资源点', exit: '撤离点', gate: '通道' };

    var MODE = 'jm';
    var SELECTED = 'centrifuge';

    var el = document.getElementById('sbMap');
    var panel = document.getElementById('sbPanel');
    if (!el || !panel) return;

    var modeBtns = document.querySelectorAll('.sb-btn[data-mode]');
    var filterBtns = document.querySelectorAll('.sb-btn[data-filter]');

    /* ─── Leaflet 初始化 ─── */
    var bounds = [[0, 0], [MAP_H, MAP_W]];

    var map = L.map(el, {
        crs: L.CRS.Simple,
        minZoom: -1.5,
        maxZoom: 2,
        zoomSnap: 0.25,
        zoomControl: true,
        attributionControl: false,
        doubleClickZoom: true,
        scrollWheelZoom: false      /* 页面里滚动时不劫持滚轮，点过地图才启用 */
    });

    function yx(x, y) {
        return L.latLng(MAP_H - y, x);
    }

    var overlay = L.imageOverlay(el.getAttribute('data-base'), bounds).addTo(map);

    /* 容器高度由 CSS 决定，defer 执行时已就绪；仍包一层 setTimeout 兜底，
       避免容器尺寸还没算出来时 fitBounds 算出错误的缩放 */
    setTimeout(function () {
        map.invalidateSize();
        map.fitBounds(bounds, { padding: [6, 6] });
    }, 0);

    overlay.on('load', function () {
        map.fitBounds(bounds, { padding: [6, 6] });
    });

    map.setMaxBounds([[-200, -200], [MAP_H + 200, MAP_W + 200]]);

    map.on('click', function () { map.scrollWheelZoom.enable(); });

    /* ─── 点位 ─── */
    var markers = {};

    Object.keys(P).forEach(function (key) {
        var p = P[key];
        var icon = L.divIcon({
            className: 'sb-pin sb-pin--' + (p.dir || 'r') + ' sb-t-' + p.type,
            html: '<i></i>' +
                (p.short ? '<b>' + p.short + '</b>' : '') +
                (p.slot ? '<u class="sb-slot" data-slot="' + key + '"></u>' : ''),
            iconSize: [0, 0],
            iconAnchor: [0, 0]
        });

        var m = L.marker(yx(p.x, p.y), {
            icon: icon,
            keyboard: true,
            title: p.name,
            riseOnHover: true
        }).addTo(map);

        m.on('click', function () { select(key); });
        markers[key] = m;
    });

    /* ─── 路线 ─── */
    ROUTES.forEach(function (r) {
        var pts = r.keys.map(function (k) {
            return yx(P[k].x, P[k].y);
        });
        L.polyline(pts, {
            color: r.color,
            weight: 1.6,
            opacity: 0.5,
            dashArray: '7 6',
            interactive: false
        }).addTo(map);
    });

    /* ─── 面板渲染 ─── */
    function render() {
        var p = P[SELECTED];
        if (!p) return;

        var isJs = MODE === 'js';
        var cur = isJs ? p.js : p.jm;
        var alt = isJs ? p.jm : p.js;
        var curName = isJs ? '绝密' : '机密';
        var altName = isJs ? '机密' : '绝密';

        var slotHtml = p.slot
            ? '<p class="sb-note"><strong>丢包名额：</strong>' + p.slot[MODE] + ' 人</p>'
            : '';

        panel.innerHTML =
            '<span class="sb-panel-type">' + TYPE_LABEL[p.type] + '</span>' +
            '<div class="sb-panel-name" role="heading" aria-level="3">' + p.name + '</div>' +
            '<p class="sb-panel-sub">' + p.sub + '</p>' +
            '<div class="sb-panel-body">' +
            '<div class="sb-mode-row"><span class="sb-mode-chip' + (isJs ? ' sb-mode-chip--js' : '') + '">' + curName + '模式</span></div>' +
            '<p class="sb-note">' + cur + '</p>' +
            slotHtml +
            '<p class="sb-note sb-note--alt"><strong>' + altName + '：</strong>' + alt + '</p>' +
            '</div>';
    }

    /* ─── 选中 ─── */
    function select(key) {
        if (!P[key]) return;
        SELECTED = key;

        Object.keys(markers).forEach(function (k) {
            var node = markers[k].getElement();
            if (!node) return;
            if (k === key) {
                L.DomUtil.addClass(node, 'is-active');
            } else {
                L.DomUtil.removeClass(node, 'is-active');
            }
        });

        render();
    }

    /* ─── 撤离点角标随模式变化 ─── */
    function updateSlots() {
        Object.keys(P).forEach(function (k) {
            if (!P[k].slot) return;
            var node = markers[k].getElement();
            if (!node) return;
            var u = node.querySelector('.sb-slot');
            if (u) u.textContent = P[k].slot[MODE];
        });
    }

    /* ─── 工具栏 ─── */
    function each(list, fn) {
        for (var i = 0; i < list.length; i++) fn(list[i], i);
    }

    function setPressed(list, active) {
        each(list, function (b) {
            var on = b === active;
            b.setAttribute('aria-pressed', on ? 'true' : 'false');
            if (on) {
                b.classList.add('is-on');
            } else {
                b.classList.remove('is-on');
            }
        });
    }

    each(modeBtns, function (btn) {
        btn.addEventListener('click', function () {
            MODE = btn.getAttribute('data-mode');
            el.setAttribute('data-mode', MODE);
            setPressed(modeBtns, btn);
            updateSlots();
            render();
        });
    });

    each(filterBtns, function (btn) {
        btn.addEventListener('click', function () {
            el.setAttribute('data-filter', btn.getAttribute('data-filter'));
            setPressed(filterBtns, btn);
        });
    });

    /* ─── 初始化 ─── */
    setPressed(modeBtns, modeBtns[0]);
    setPressed(filterBtns, filterBtns[0]);
    updateSlots();
    select(SELECTED);

    /* 响应式：容器尺寸变了要让 Leaflet 重算 */
    var rt;
    window.addEventListener('resize', function () {
        clearTimeout(rt);
        rt = setTimeout(function () {
            map.invalidateSize();
        }, 160);
    });

})();
