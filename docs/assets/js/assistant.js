/* ============================================================
   assistant.js —— 小K · 站内智能助手
   ────────────────────────────────────────────────────────────
   静态站没有后端，所以走「浏览器直连 + 访客自带 Key（BYOK）」：
     · 统一使用各家的 OpenAI 兼容接口 /chat/completions
     · Key 只存在访客自己的 localStorage，不经过本站服务器
     · 如果站点配置了 proxy_url，则改走代理，浏览器不接触 Key
   站内知识：读 search.json 做工况很轻的关键词检索，把命中摘要
   塞进 system prompt，让回答带上原文链接。
   ============================================================ */
(function () {
    'use strict';

    var root = document.getElementById('kk-root');
    if (!root) return;

    var CFG = window.__KK_CONFIG__ || {};
    var LS_SET = 'kk.settings.v1';
    var LS_THREAD = 'kk.thread.v1';
    var MAX_KEEP = 30;

    /* ══════════════════════════════════════════
       0. 服务商清单（OpenAI 兼容）
       ══════════════════════════════════════════ */
    var PROVIDERS = {
        deepseek: {
            label: 'DeepSeek',
            hint: '深度求索',
            base: 'https://api.deepseek.com/v1',
            models: ['deepseek-chat', 'deepseek-reasoner'],
            keyUrl: 'https://platform.deepseek.com/api_keys'
        },
        moonshot: {
            label: 'Kimi',
            hint: '月之暗面',
            base: 'https://api.moonshot.cn/v1',
            models: ['kimi-latest', 'kimi-k2-0905-preview', 'moonshot-v1-32k', 'moonshot-v1-8k'],
            keyUrl: 'https://platform.moonshot.cn/console/api-keys'
        },
        zhipu: {
            label: '智谱 GLM',
            hint: 'bigmodel',
            base: 'https://open.bigmodel.cn/api/paas/v4',
            models: ['glm-4.5', 'glm-4.5-air', 'glm-4-flash'],
            keyUrl: 'https://open.bigmodel.cn/usercenter/apikeys'
        },
        dashscope: {
            label: '通义千问',
            hint: '阿里云百炼',
            base: 'https://dashscope.aliyuncs.com/compatible-mode/v1',
            models: ['qwen-plus', 'qwen-max', 'qwen-turbo'],
            keyUrl: 'https://bailian.console.aliyun.com/'
        },
        siliconflow: {
            label: '硅基流动',
            hint: '多模型聚合',
            base: 'https://api.siliconflow.cn/v1',
            models: ['deepseek-ai/DeepSeek-V3', 'Qwen/Qwen2.5-72B-Instruct', 'moonshotai/Kimi-K2-Instruct'],
            keyUrl: 'https://cloud.siliconflow.cn/account/ak'
        },
        openai: {
            label: 'OpenAI',
            hint: 'GPT 系列',
            base: 'https://api.openai.com/v1',
            models: ['gpt-4o-mini', 'gpt-4o', 'gpt-4.1-mini'],
            keyUrl: 'https://platform.openai.com/api-keys'
        },
        custom: {
            label: '自定义',
            hint: '任意兼容接口',
            base: '',
            models: [],
            keyUrl: ''
        }
    };

    var SYSTEM_PROMPT = [
        '你是「小K」，张子健（网名 K1n，摄像头驱动工程师）个人网站上的常驻助理。',
        '站长做的事：白天写摄像头驱动，盯着 sensor、ISP、MIPI 把一帧画面送到屏幕上；业余拍照，在北京，还在学。',
        '',
        '站内结构（回答"站里有没有 XX"这类问题时用得上）：',
        '- /gallery/ 照片墙：按系列筛选、灯箱浏览',
        '- /log/sensor/ 影像笔记·Sensor 与成像：由驱动/成像链路切入的技术文章',
        '- /log/aesthetic/ 影像笔记·摄影美学：构图、光线、审美',
        '- /life/ 旅记，/notes/ 旧技术笔记归档，/game/ 游戏攻略（带互动地图）',
        '- /assistant/ 就是你自己这一页',
        '',
        '说话方式：中文；先给结论，再给理由；短问短答，默认四句以内，用户说"详细讲讲"再展开；',
        '不用"作为一个 AI""很高兴为您解答"这类套话；技术问题该给公式、参数、代码就给。',
        '',
        '引用规则：只有【站内资料】里真实出现的文章才可以引用，格式 [文章标题](/路径.html)；',
        '资料里没有就直说"站里暂时没有写这个"，绝不编造标题、链接或文章内容。',
        '站内资料只是参考，观点不完整时可以用你自己的知识补充，但要说明哪部分来自站内。',
        '',
        '边界：与摄影、成像、这个站无关的政治、医疗、法律、投资等高风险问题，礼貌说明不太适合在这里展开，把话题拉回站点内容。'
    ].join('\n');

    /* ══════════════════════════════════════════
       1. 小工具
       ══════════════════════════════════════════ */
    function esc(s) {
        return String(s == null ? '' : s).replace(/[&<>"']/g, function (m) {
            return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[m];
        });
    }

    function readJSON(key, fallback) {
        try {
            var raw = localStorage.getItem(key);
            if (!raw) return fallback;
            var v = JSON.parse(raw);
            return v == null ? fallback : v;
        } catch (e) {
            return fallback;
        }
    }

    function writeJSON(key, value) {
        try { localStorage.setItem(key, JSON.stringify(value)); } catch (e) { /* 隐私模式忽略 */ }
    }

    function trimSlash(u) {
        return String(u || '').replace(/\/+$/, '');
    }

    function endpointOf(base) {
        var b = trimSlash(base);
        if (!b) return '';
        if (/\/chat\/completions$/.test(b)) return b;
        return b + '/chat/completions';
    }

    var ICONS = {
        plus: '<svg viewBox="0 0 24 24"><path d="M12 5v14M5 12h14"/></svg>',
        expand: '<svg viewBox="0 0 24 24"><path d="M14 4h6v6M20 4l-7 7M10 20H4v-6M4 20l7-7"/></svg>',
        gear: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3.2"/><path d="M19.4 15a1.6 1.6 0 0 0 .3 1.8l.1.1a2 2 0 1 1-2.8 2.8l-.1-.1a1.6 1.6 0 0 0-1.8-.3 1.6 1.6 0 0 0-1 1.5V21a2 2 0 1 1-4 0v-.1a1.6 1.6 0 0 0-1-1.5 1.6 1.6 0 0 0-1.8.3l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1a1.6 1.6 0 0 0 .3-1.8 1.6 1.6 0 0 0-1.5-1H3a2 2 0 1 1 0-4h.1a1.6 1.6 0 0 0 1.5-1 1.6 1.6 0 0 0-.3-1.8l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1a1.6 1.6 0 0 0 1.8.3H9a1.6 1.6 0 0 0 1-1.5V3a2 2 0 1 1 4 0v.1a1.6 1.6 0 0 0 1 1.5 1.6 1.6 0 0 0 1.8-.3l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1a1.6 1.6 0 0 0-.3 1.8V9a1.6 1.6 0 0 0 1.5 1H21a2 2 0 1 1 0 4h-.1a1.6 1.6 0 0 0-1.5 1z"/></svg>',
        close: '<svg viewBox="0 0 24 24"><path d="M6 6l12 12M18 6L6 18"/></svg>',
        send: '<svg viewBox="0 0 24 24"><path d="M4.5 12h14M13 6l6 6-6 6"/></svg>',
        stop: '<svg viewBox="0 0 24 24"><rect x="7" y="7" width="10" height="10" rx="2"/></svg>',
        copy: '<svg viewBox="0 0 24 24"><rect x="9" y="9" width="11" height="11" rx="2.5"/><path d="M5 15V6a2 2 0 0 1 2-2h8"/></svg>',
        check: '<svg viewBox="0 0 24 24"><path d="M4 12.5l5 5L20 6.5"/></svg>',
        eye: '<svg viewBox="0 0 24 24"><path d="M2 12s3.6-6.5 10-6.5S22 12 22 12s-3.6 6.5-10 6.5S2 12 2 12z"/><circle cx="12" cy="12" r="2.8"/></svg>',
        eyeOff: '<svg viewBox="0 0 24 24"><path d="M3 3l18 18M10.6 6.1A9.9 9.9 0 0 1 12 6c6.4 0 10 6 10 6a18 18 0 0 1-2.6 3.4M6.2 7.8A18 18 0 0 0 2 12s3.6 6.5 10 6.5a9.7 9.7 0 0 0 3.3-.6"/></svg>',
        info: '<svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 11v5M12 7.6v.6"/></svg>',
        refresh: '<svg viewBox="0 0 24 24"><path d="M20 12a8 8 0 1 1-2.6-5.9M20 4v4.5h-4.5"/></svg>'
    };

    /* ══════════════════════════════════════════
       2. Markdown（够用就行，且先转义再解析）
       ══════════════════════════════════════════ */
    function inline(text) {
        var store = [];
        var s = esc(text);

        // 行内代码先摘出来，避免被后续规则误伤
        s = s.replace(/`([^`]+)`/g, function (_, code) {
            store.push(code);
            return '\u0000' + (store.length - 1) + '\u0000';
        });

        // 链接：只放行 http(s) 与站内绝对路径
        s = s.replace(/\[([^\]]+)\]\(([^)\s]+)\)/g, function (whole, label, href) {
            if (!/^(https?:\/\/|\/)/i.test(href)) return whole;
            var ext = /^https?:\/\//i.test(href);
            return '<a href="' + href + '"' + (ext ? ' target="_blank" rel="noopener noreferrer"' : ' target="_top"') + '>' + label + '</a>';
        });

        // 裸链接
        s = s.replace(/(^|[\s(（])(https?:\/\/[^\s<)）]+)/g, function (_, pre, url) {
            return pre + '<a href="' + url + '" target="_blank" rel="noopener noreferrer">' + url + '</a>';
        });

        s = s.replace(/\*\*([^*\n]+)\*\*/g, '<strong>$1</strong>');
        s = s.replace(/~~([^~\n]+)~~/g, '<del>$1</del>');
        s = s.replace(/(^|[^*\w])\*([^*\n]+)\*(?!\*)/g, '$1<em>$2</em>');

        s = s.replace(/\u0000(\d+)\u0000/g, function (_, i) {
            return '<code>' + store[+i] + '</code>';
        });

        return s;
    }

    function renderMarkdown(src) {
        var lines = String(src == null ? '' : src).replace(/\r\n/g, '\n').split('\n');
        var out = [];
        var i = 0;

        function isBlockStart(line) {
            return /^\s*(```|#{1,6}\s|[-*+]\s|\d+[.)]\s|>\s?|(-{3,}|\*{3,})\s*$)/.test(line);
        }

        while (i < lines.length) {
            var line = lines[i];

            // 代码块（未闭合也照收，流式输出时会短暂处于这个状态）
            if (/^\s*```/.test(line)) {
                var lang = line.replace(/^\s*```/, '').trim();
                var buf = [];
                i++;
                while (i < lines.length && !/^\s*```/.test(lines[i])) {
                    buf.push(lines[i]);
                    i++;
                }
                i++;
                out.push(
                    '<pre><code' + (lang ? ' class="language-' + esc(lang) + '"' : '') + '>' +
                    esc(buf.join('\n')) + '</code></pre>'
                );
                continue;
            }

            if (!line.trim()) { i++; continue; }

            // 分割线
            if (/^\s*(-{3,}|\*{3,})\s*$/.test(line)) {
                out.push('<hr>');
                i++;
                continue;
            }

            // 标题
            var h = line.match(/^\s*(#{1,6})\s+(.*)$/);
            if (h) {
                var lv = Math.min(4, h[1].length);
                out.push('<h' + lv + '>' + inline(h[2]) + '</h' + lv + '>');
                i++;
                continue;
            }

            // 表格
            if (/\|/.test(line) && i + 1 < lines.length && /^\s*\|?[\s:|-]+\|[\s:|-]*$/.test(lines[i + 1])) {
                var head = line.split('|').map(function (c) { return c.trim(); });
                if (head[0] === '') head.shift();
                if (head[head.length - 1] === '') head.pop();
                i += 2;
                var rows = [];
                while (i < lines.length && /\|/.test(lines[i]) && lines[i].trim()) {
                    var cells = lines[i].split('|').map(function (c) { return c.trim(); });
                    if (cells[0] === '') cells.shift();
                    if (cells[cells.length - 1] === '') cells.pop();
                    rows.push(cells);
                    i++;
                }
                var tbl = '<table><thead><tr>' + head.map(function (c) { return '<th>' + inline(c) + '</th>'; }).join('') + '</tr></thead><tbody>';
                rows.forEach(function (r) {
                    tbl += '<tr>' + r.map(function (c) { return '<td>' + inline(c) + '</td>'; }).join('') + '</tr>';
                });
                out.push(tbl + '</tbody></table>');
                continue;
            }

            // 引用
            if (/^\s*>\s?/.test(line)) {
                var q = [];
                while (i < lines.length && /^\s*>\s?/.test(lines[i])) {
                    q.push(lines[i].replace(/^\s*>\s?/, ''));
                    i++;
                }
                out.push('<blockquote>' + inline(q.join(' ')) + '</blockquote>');
                continue;
            }

            // 列表
            if (/^\s*[-*+]\s+/.test(line) || /^\s*\d+[.)]\s+/.test(line)) {
                var ordered = /^\s*\d+[.)]\s+/.test(line);
                var items = [];
                while (i < lines.length && (/^\s*[-*+]\s+/.test(lines[i]) || /^\s*\d+[.)]\s+/.test(lines[i]))) {
                    items.push(lines[i].replace(/^\s*(?:[-*+]|\d+[.)])\s+/, ''));
                    i++;
                }
                var tag = ordered ? 'ol' : 'ul';
                out.push('<' + tag + '>' + items.map(function (it) { return '<li>' + inline(it) + '</li>'; }).join('') + '</' + tag + '>');
                continue;
            }

            // 段落
            var para = [];
            while (i < lines.length && lines[i].trim() && !isBlockStart(lines[i])) {
                para.push(lines[i]);
                i++;
            }
            out.push('<p>' + inline(para.join(' ')) + '</p>');
        }

        return out.join('');
    }

    /* ══════════════════════════════════════════
       3. 状态
       ══════════════════════════════════════════ */
    var settings = (function () {
        var saved = readJSON(LS_SET, null) || {};
        var pid = saved.provider || CFG.defaultProvider || 'deepseek';
        if (!PROVIDERS[pid]) pid = 'deepseek';
        return {
            provider: pid,
            baseUrl: saved.baseUrl || '',
            model: saved.model || CFG.defaultModel || PROVIDERS[pid].models[0] || '',
            apiKey: saved.apiKey || CFG.siteKey || '',
            proxy: saved.proxy != null ? saved.proxy : (CFG.proxyUrl || ''),
            useSearch: saved.useSearch != null ? saved.useSearch : (CFG.useSiteSearch !== false),
            remember: saved.remember !== false
        };
    })();

    var state = {
        messages: [],
        streaming: false,
        ctrl: null,
        docs: null,
        docsPromise: null,
        lastHits: [],
        open: false,
        pageMode: false
    };

    /* ══════════════════════════════════════════
       4. 建 DOM
       ══════════════════════════════════════════ */
    var SUGGESTIONS = Array.isArray(CFG.suggestions) ? CFG.suggestions.slice(0, 4) : [];
    var NAME = CFG.name || '小K';
    var MARK = CFG.mark || 'K';

    var els = {};

    function build() {
        var pageMount = document.getElementById('kk-page-mount');
        state.pageMode = !!pageMount;

        root.classList.toggle('kk-page', state.pageMode);
        root.innerHTML = [
            '<button class="kk-fab" type="button" aria-label="打开 ' + esc(NAME) + '" aria-expanded="false">',
            '  <span class="kk-fab-orb">' + esc(MARK) + '</span>',
            '  <span class="kk-fab-label">问' + esc(NAME) + '</span>',
            '</button>',
            '<section class="kk-panel" role="dialog" aria-label="' + esc(NAME) + ' 智能助手" aria-modal="false">',
            '  <header class="kk-head">',
            '    <div class="kk-id">',
            '      <span class="kk-avatar">' + esc(MARK) + '<i class="is-off"></i></span>',
            '      <div class="kk-id-text">',
            '        <strong class="kk-name">' + esc(NAME) + '</strong>',
            '        <span class="kk-sub">' + esc(CFG.tagline || '站内助理') + '</span>',
            '      </div>',
            '    </div>',
            '    <div class="kk-head-actions">',
            '      <button class="kk-icon-btn" type="button" data-act="new" title="新对话" aria-label="新对话">' + ICONS.plus + '</button>',
            (state.pageMode ? '' : '      <button class="kk-icon-btn" type="button" data-act="expand" title="独立页面打开" aria-label="独立页面打开">' + ICONS.expand + '</button>'),
            '      <button class="kk-icon-btn" type="button" data-act="settings" title="设置" aria-label="设置">' + ICONS.gear + '</button>',
            (state.pageMode ? '' : '      <button class="kk-icon-btn" type="button" data-act="close" title="收起" aria-label="收起">' + ICONS.close + '</button>'),
            '    </div>',
            '  </header>',
            '  <div class="kk-body">',
            '    <div class="kk-thread" role="log" aria-live="polite"></div>',
            '  </div>',
            '  <div class="kk-settings" aria-hidden="true">',
            '      <div class="kk-set-head">',
            '        <div class="kk-set-title"><strong>接入模型</strong><span>Key 只存在这台设备的浏览器里</span></div>',
            '        <button class="kk-icon-btn" type="button" data-act="set-close" aria-label="返回对话">' + ICONS.close + '</button>',
            '      </div>',
            '      <div class="kk-set-body">',
            '        <div class="kk-field">',
            '          <div class="kk-label">服务商</div>',
            '          <div class="kk-providers"></div>',
            '        </div>',
            '        <div class="kk-field">',
            '          <div class="kk-label"><span>API Key</span><a class="kk-key-link" href="#" target="_blank" rel="noopener noreferrer">去申请 →</a></div>',
            '          <div class="kk-key-row">',
            '            <input class="kk-input-t kk-key" type="password" placeholder="sk-..." autocomplete="off" spellcheck="false">',
            '            <button class="kk-eye" type="button" data-act="eye" aria-label="显示或隐藏 Key">' + ICONS.eye + '</button>',
            '          </div>',
            '          <p class="kk-note">静态站没有服务器，Key 直接由浏览器发给你选的服务商，<b>不会上传到本站</b>。公共电脑上用完建议清掉。</p>',
            '        </div>',
            '        <div class="kk-field">',
            '          <div class="kk-label"><span>模型</span></div>',
            '          <input class="kk-input-t kk-model" list="kk-models" type="text" placeholder="模型名，可手填" autocomplete="off" spellcheck="false">',
            '          <datalist id="kk-models"></datalist>',
            '        </div>',
            '        <div class="kk-field">',
            '          <div class="kk-label"><span>Base URL</span><span style="font-size:10px;letter-spacing:0;text-transform:none;font-weight:400">可选</span></div>',
            '          <input class="kk-input-t kk-base" type="text" placeholder="https://api.example.com/v1" autocomplete="off" spellcheck="false">',
            '          <p class="kk-note">换中转、本地 Ollama（<b>http://localhost:11434/v1</b>）或自建网关时填这里。</p>',
            '        </div>',
            '        <div class="kk-switch-row">',
            '          <div><strong>检索站内文章</strong><span>回答前先翻一遍 search.json，能给出原文链接</span></div>',
            '          <button class="kk-switch kk-sw-search" type="button" role="switch" aria-pressed="true" aria-label="检索站内文章"></button>',
            '        </div>',
            '        <div class="kk-switch-row">',
            '          <div><strong>记住 Key</strong><span>关掉则只在本次会话有效</span></div>',
            '          <button class="kk-switch kk-sw-remember" type="button" role="switch" aria-pressed="true" aria-label="记住 Key"></button>',
            '        </div>',
            '        <div class="kk-set-actions" style="margin-top:6px">',
            '          <button class="kk-btn ghost" type="button" data-act="wipe" style="flex:1 1 auto">清除对话记录</button>',
            '        </div>',
            '      </div>',
            '      <div class="kk-set-foot">',
            '        <p class="kk-test-out"></p>',
            '        <div class="kk-set-actions">',
            '          <button class="kk-btn primary" type="button" data-act="save">保存</button>',
            '          <button class="kk-btn ghost" type="button" data-act="test">测试连接</button>',
            '        </div>',
            '      </div>',
            '    </div>',
            '  <footer class="kk-composer">',
            '    <div class="kk-input-wrap">',
            '      <textarea class="kk-input" rows="1" placeholder="问点什么…" aria-label="输入问题"></textarea>',
            '      <button class="kk-send" type="button" aria-label="发送" disabled>' + ICONS.send + '</button>',
            '    </div>',
            '    <div class="kk-meta">',
            '      <button class="kk-model-chip" type="button" data-act="settings">未配置</button>',
            '      <span class="kk-hint">Enter 发送 · Shift+Enter 换行</span>',
            '    </div>',
            '  </footer>',
            '</section>'
        ].join('\n');

        els.fab = root.querySelector('.kk-fab');
        els.panel = root.querySelector('.kk-panel');
        els.thread = root.querySelector('.kk-thread');
        els.input = root.querySelector('.kk-input');
        els.send = root.querySelector('.kk-send');
        els.sub = root.querySelector('.kk-sub');
        els.chip = root.querySelector('.kk-model-chip');
        els.settings = root.querySelector('.kk-settings');
        els.providers = root.querySelector('.kk-providers');
        els.modelInput = root.querySelector('.kk-model');
        els.modelList = root.querySelector('#kk-models');
        els.keyInput = root.querySelector('.kk-key');
        els.baseInput = root.querySelector('.kk-base');
        els.keyLink = root.querySelector('.kk-key-link');
        els.swSearch = root.querySelector('.kk-sw-search');
        els.swRemember = root.querySelector('.kk-sw-remember');
        els.testOut = root.querySelector('.kk-test-out');

        if (state.pageMode) {
            pageMount.appendChild(root);
        }

        bind();
        restoreThread();
        paintStatus();
        if (state.pageMode) open(true);
        // 支持 /#kk 这类直达链接，方便把「去问小K」发给自己
        else if (location.hash === '#kk') setTimeout(function () { open(); }, 380);
    }

    /* ══════════════════════════════════════════
       5. 交互绑定
       ══════════════════════════════════════════ */
    function bind() {
        els.fab.addEventListener('click', function () { toggle(); });

        root.addEventListener('click', function (e) {
            var btn = e.target.closest('[data-act]');
            if (!btn) return;
            if (btn.tagName === 'A' && btn.getAttribute('href') === '#') e.preventDefault();
            var act = btn.getAttribute('data-act');
            if (act === 'close') close();
            else if (act === 'new') newChat();
            else if (act === 'settings') openSettings();
            else if (act === 'set-close') closeSettings();
            else if (act === 'expand') { e.preventDefault(); location.href = CFG.pageUrl || '/assistant/'; }
            else if (act === 'eye') toggleEye();
            else if (act === 'save') { saveSettings(true); }
            else if (act === 'test') testConnection();
            else if (act === 'wipe') wipe();
            else if (act === 'copy') copyMessage(btn);
        });

        els.send.addEventListener('click', function () {
            if (state.streaming) stop();
            else send(els.input.value);
        });

        els.input.addEventListener('input', function () {
            autoGrow();
            els.send.disabled = !els.input.value.trim() && !state.streaming;
        });

        els.input.addEventListener('keydown', function (e) {
            if (e.key === 'Enter' && !e.shiftKey && !e.isComposing) {
                e.preventDefault();
                if (!state.streaming) send(els.input.value);
            }
        });

        els.providers.addEventListener('click', function (e) {
            var b = e.target.closest('.kk-prov');
            if (b) pickProvider(b.getAttribute('data-id'));
        });

        els.swSearch.addEventListener('click', function () {
            var on = els.swSearch.getAttribute('aria-pressed') !== 'true';
            els.swSearch.setAttribute('aria-pressed', String(on));
        });

        els.swRemember.addEventListener('click', function () {
            var on = els.swRemember.getAttribute('aria-pressed') !== 'true';
            els.swRemember.setAttribute('aria-pressed', String(on));
        });

        document.addEventListener('keydown', function (e) {
            if (e.key === 'Escape' && state.open) {
                if (els.settings.classList.contains('is-open')) closeSettings();
                else if (!state.pageMode) close();
            }
            // Alt+K 唤醒
            if (e.altKey && (e.key === 'k' || e.key === 'K')) {
                e.preventDefault();
                toggle();
            }
        });

        // 任何页面元素都可以用 data-kk-open 唤起助手
        document.addEventListener('click', function (e) {
            var trigger = e.target.closest('[data-kk-open]');
            if (!trigger) return;
            e.preventDefault();
            var q = trigger.getAttribute('data-kk-open');
            open();
            if (q && q.length > 1) {
                els.input.value = q;
                autoGrow();
                els.send.disabled = false;
                els.input.focus();
            } else {
                setTimeout(function () { els.input.focus(); }, 260);
            }
        });

        window.addEventListener('resize', function () {
            if (!state.open || state.pageMode) return;
            document.body.classList.toggle('kk-lock', window.innerWidth <= 720);
        });
    }

    function toggle() {
        state.open ? close() : open();
    }

    function open(silent) {
        state.open = true;
        root.classList.add('is-open');
        els.fab.setAttribute('aria-expanded', 'true');
        els.panel.setAttribute('aria-hidden', 'false');
        if (!state.pageMode && window.innerWidth <= 720) document.body.classList.add('kk-lock');
        if (!silent) {
            setTimeout(function () { els.input.focus({ preventScroll: true }); }, 220);
        }
        if (settings.useSearch) ensureDocs();
        scrollToEnd(true);
    }

    function close() {
        state.open = false;
        root.classList.remove('is-open');
        els.fab.setAttribute('aria-expanded', 'false');
        els.panel.setAttribute('aria-hidden', 'true');
        document.body.classList.remove('kk-lock');
        closeSettings();
    }

    function openSettings() {
        fillSettings();
        els.settings.classList.add('is-open');
        els.settings.setAttribute('aria-hidden', 'false');
        root.querySelector('[data-act="settings"]').classList.add('is-active');
    }

    function closeSettings() {
        els.settings.classList.remove('is-open');
        els.settings.setAttribute('aria-hidden', 'true');
        var b = root.querySelector('[data-act="settings"]');
        if (b) b.classList.remove('is-active');
    }

    /* ══════════════════════════════════════════
       6. 设置读写
       ══════════════════════════════════════════ */
    function paintProviders() {
        els.providers.innerHTML = Object.keys(PROVIDERS).map(function (id) {
            var p = PROVIDERS[id];
            return '<button type="button" class="kk-prov' + (settings.provider === id ? ' is-active' : '') +
                '" data-id="' + id + '">' + esc(p.label) + '<small>' + esc(p.hint) + '</small></button>';
        }).join('');
    }

    function paintModels() {
        var p = PROVIDERS[settings.provider] || PROVIDERS.custom;
        els.modelList.innerHTML = (p.models || []).map(function (m) {
            return '<option value="' + esc(m) + '"></option>';
        }).join('');
    }

    function fillSettings() {
        paintProviders();
        paintModels();
        els.modelInput.value = settings.model || '';
        els.keyInput.value = settings.apiKey || '';
        els.keyInput.type = 'password';
        els.baseInput.value = settings.baseUrl || '';
        els.baseInput.placeholder = (PROVIDERS[settings.provider] || {}).base || 'https://api.example.com/v1';
        els.swSearch.setAttribute('aria-pressed', String(!!settings.useSearch));
        els.swRemember.setAttribute('aria-pressed', String(!!settings.remember));
        var p = PROVIDERS[settings.provider] || {};
        if (p.keyUrl) {
            els.keyLink.href = p.keyUrl;
            els.keyLink.hidden = false;
        } else {
            els.keyLink.hidden = true;
        }
        els.testOut.textContent = '';
        els.testOut.className = 'kk-test-out';
    }

    function pickProvider(id) {
        if (!PROVIDERS[id]) return;
        settings.provider = id;
        var p = PROVIDERS[id];
        els.providers.querySelectorAll('.kk-prov').forEach(function (b) {
            b.classList.toggle('is-active', b.getAttribute('data-id') === id);
        });
        // 换服务商时，模型与 Base 跟着切到默认值（用户手填过就尊重当前值）
        if (id !== 'custom') {
            els.modelInput.value = p.models[0] || '';
            els.baseInput.value = '';
            els.baseInput.placeholder = p.base;
        }
        els.keyInput.value = '';
        els.keyInput.focus();
        paintModels();
        if (p.keyUrl) {
            els.keyLink.href = p.keyUrl;
            els.keyLink.hidden = false;
        } else {
            els.keyLink.hidden = true;
        }
    }

    function toggleEye() {
        var show = els.keyInput.type === 'password';
        els.keyInput.type = show ? 'text' : 'password';
        root.querySelector('[data-act="eye"]').innerHTML = show ? ICONS.eyeOff : ICONS.eye;
    }

    function currentBase() {
        var b = (els.baseInput.value || '').trim();
        if (b) return b;
        var p = PROVIDERS[settings.provider];
        return p ? p.base : '';
    }

    function saveSettings(announce) {
        settings.model = (els.modelInput.value || '').trim();
        settings.apiKey = (els.keyInput.value || '').trim();
        settings.baseUrl = (els.baseInput.value || '').trim();
        settings.useSearch = els.swSearch.getAttribute('aria-pressed') === 'true';
        settings.remember = els.swRemember.getAttribute('aria-pressed') === 'true';

        if (settings.remember) {
            writeJSON(LS_SET, {
                provider: settings.provider,
                model: settings.model,
                apiKey: settings.apiKey,
                baseUrl: settings.baseUrl,
                proxy: settings.proxy,
                useSearch: settings.useSearch,
                remember: true
            });
        } else {
            try { localStorage.removeItem(LS_SET); } catch (e) {}
        }

        paintStatus();
        if (announce) {
            els.testOut.textContent = '已保存，可以开始提问了。';
            els.testOut.className = 'kk-test-out ok';
            setTimeout(function () { closeSettings(); }, 520);
        }
        if (settings.useSearch && state.docs) updateHeroCount();
    }

    function paintStatus() {
        var ready = !!(settings.apiKey || settings.proxy);
        var p = PROVIDERS[settings.provider] || {};
        els.chip.classList.toggle('is-empty', !ready);
        els.chip.textContent = ready
            ? (p.label || settings.provider) + ' · ' + (settings.model || '默认模型')
            : '未配置 Key · 点此设置';
        els.sub.textContent = ready
            ? (CFG.tagline || '站内助理')
            : '未接入模型 · 点右上角设置';
        root.querySelector('.kk-avatar i').classList.toggle('is-off', !ready);
    }

    function wipe() {
        state.messages = [];
        state.lastHits = [];
        try { localStorage.removeItem(LS_THREAD); } catch (e) {}
        renderThread();
        els.testOut.textContent = '对话记录已清除。';
        els.testOut.className = 'kk-test-out ok';
    }

    /* ══════════════════════════════════════════
       7. 站内检索（search.json）
       ══════════════════════════════════════════ */
    function ensureDocs() {
        if (state.docs) return Promise.resolve(state.docs);
        if (state.docsPromise) return state.docsPromise;
        var url = CFG.searchIndex || '/search.json';
        state.docsPromise = fetch(url, { cache: 'force-cache' })
            .then(function (r) { return r.ok ? r.json() : []; })
            .then(function (list) {
                state.docs = Array.isArray(list) ? list : [];
                updateHeroCount();
                return state.docs;
            })
            .catch(function () {
                state.docs = [];
                return [];
            });
        return state.docsPromise;
    }

    function tokens(q) {
        var s = String(q || '').toLowerCase();
        var out = [];
        var latin = s.match(/[a-z0-9][a-z0-9+#._-]{1,}/g) || [];
        latin.forEach(function (w) { out.push(w); });
        var cjk = s.match(/[\u4e00-\u9fa5]+/g) || [];
        cjk.forEach(function (run) {
            if (run.length <= 2) { out.push(run); return; }
            for (var i = 0; i < run.length - 1; i++) out.push(run.slice(i, i + 2));
        });
        var seen = {};
        return out.filter(function (t) {
            if (t.length < 2 || seen[t]) return false;
            seen[t] = 1;
            return true;
        });
    }

    function searchDocs(query, limit) {
        if (!state.docs || !state.docs.length) return [];
        var ts = tokens(query);
        if (!ts.length) return [];
        var q = String(query).toLowerCase();
        var scored = [];

        state.docs.forEach(function (d) {
            var title = String(d.title || '').toLowerCase();
            var excerpt = String(d.excerpt || '').toLowerCase();
            var group = String(d.group || '');
            var score = 0;
            ts.forEach(function (t) {
                if (title.indexOf(t) >= 0) score += 8;
                if (group.toLowerCase().indexOf(t) >= 0) score += 2;
                if (excerpt.indexOf(t) >= 0) score += 1.5;
            });
            if (title && q.indexOf(title) >= 0) score += 10;
            if (score > 0) scored.push({ doc: d, score: score });
        });

        scored.sort(function (a, b) { return b.score - a.score; });
        var top = scored.slice(0, limit || 3);
        if (!top.length) return [];
        var cut = Math.max(5, top[0].score * 0.34);
        return top.filter(function (x) { return x.score >= cut; }).map(function (x) {
            return {
                title: x.doc.title || '未命名',
                url: x.doc.url || '#',
                excerpt: String(x.doc.excerpt || '').slice(0, 260),
                score: x.score
            };
        });
    }

    function contextFor(hits) {
        if (!hits.length) return '';
        var lines = ['【站内资料】以下是本站 search.json 里检索到的相关文章（可能不完全对题，自行判断是否引用）：'];
        hits.forEach(function (h, i) {
            lines.push((i + 1) + '. 《' + h.title + '》 ' + h.url);
            if (h.excerpt) lines.push('   摘要：' + h.excerpt.replace(/\s+/g, ' '));
        });
        lines.push('引用时请用 [标题](/路径.html) 的形式，链接必须来自上面的列表。');
        return lines.join('\n');
    }

    /* ══════════════════════════════════════════
       8. 消息渲染
       ══════════════════════════════════════════ */
    function renderThread() {
        els.thread.innerHTML = '';
        if (!state.messages.length) {
            els.thread.appendChild(heroEl());
            return;
        }
        state.messages.forEach(function (m, i) {
            els.thread.appendChild(messageEl(m, i));
        });
        scrollToEnd();
    }

    function heroEl() {
        var wrap = document.createElement('div');
        wrap.className = 'kk-hero';
        var ready = !!(settings.apiKey || settings.proxy);
        var tip = ready
            ? '<div class="kk-tip">' + ICONS.info + '<span class="kk-tip-text">可以让我在站内找文章，也可以直接聊技术。按 <b>Esc</b> 收起。</span></div>'
            : '<div class="kk-tip">' + ICONS.info + '<span class="kk-tip-text">还差一步：<button type="button" data-act="settings">填入 API Key</button>（DeepSeek 免费额度就够用），Key 只留在这台设备的浏览器里。</span></div>';

        wrap.innerHTML = [
            '<div class="kk-hero-mark"><span>' + esc(MARK) + '</span></div>',
            '<p>' + esc(CFG.welcome || ('我是' + NAME + '，这个站的随身助理。')) + '</p>',
            SUGGESTIONS.length ? '<div class="kk-sug">' + SUGGESTIONS.map(function (s) {
                return '<button type="button" class="kk-sug-btn" data-sug="' + esc(s) + '">' + esc(s) + '</button>';
            }).join('') + '</div>' : '',
            tip
        ].join('');

        wrap.querySelectorAll('.kk-sug-btn').forEach(function (b) {
            b.addEventListener('click', function () {
                send(b.getAttribute('data-sug'));
            });
        });

        return wrap;
    }

    function messageEl(m, index) {
        var div = document.createElement('div');
        div.className = 'kk-msg ' + (m.role === 'user' ? 'user' : 'bot') + (m.error ? ' is-error' : '');
        div.setAttribute('data-i', String(index));

        var body = '<div class="kk-bubble"><div class="kk-md">' + renderMarkdown(m.content) + '</div></div>';
        if (m.role === 'user') {
            body = '<div class="kk-bubble">' + esc(m.content).replace(/\n/g, '<br>') + '</div>';
        }

        var foot = m.role === 'user' ? '' :
            '<div class="kk-msg-foot">' +
            '<button type="button" data-act="copy">' + ICONS.copy + '复制</button>' +
            (m.error ? '' : '') +
            '</div>';

        div.innerHTML = (m.role === 'user' ? '' : '<span class="kk-badge">' + esc(MARK) + '</span>') +
            '<div class="kk-msg-main">' + body + foot + '</div>';
        return div;
    }

    function lastBubble() {
        var nodes = els.thread.querySelectorAll('.kk-msg.bot .kk-bubble');
        return nodes.length ? nodes[nodes.length - 1] : null;
    }

    function scrollToEnd(force) {
        var el = els.thread;
        if (!el) return;
        if (force || isNearBottom(el)) {
            requestAnimationFrame(function () { el.scrollTop = el.scrollHeight; });
        }
    }

    function isNearBottom(el) {
        return el.scrollHeight - el.scrollTop - el.clientHeight < 110;
    }

    function copyMessage(btn) {
        var msg = btn.closest('.kk-msg');
        var i = +msg.getAttribute('data-i');
        var m = state.messages[i];
        if (!m) return;
        var text = m.content;
        var done = function () {
            btn.innerHTML = ICONS.check + '已复制';
            setTimeout(function () { btn.innerHTML = ICONS.copy + '复制'; }, 1600);
        };
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(text).then(done, function () {});
        } else {
            var ta = document.createElement('textarea');
            ta.value = text;
            document.body.appendChild(ta);
            ta.select();
            try { document.execCommand('copy'); done(); } catch (e) {}
            ta.remove();
        }
    }

    function autoGrow() {
        els.input.style.height = 'auto';
        els.input.style.height = Math.min(132, els.input.scrollHeight) + 'px';
    }

    function restoreThread() {
        var saved = readJSON(LS_THREAD, null);
        if (Array.isArray(saved) && saved.length) {
            state.messages = saved.filter(function (m) {
                if (!m || (m.role !== 'user' && m.role !== 'assistant')) return false;
                if (typeof m.content !== 'string') return false;
                // 上次在生成途中关掉页面，会留下空的助手消息，丢掉
                return !(m.role === 'assistant' && !m.content.trim());
            }).slice(-MAX_KEEP);
        }
        renderThread();
    }

    function persist() {
        writeJSON(LS_THREAD, state.messages.slice(-MAX_KEEP).map(function (m) {
            return { role: m.role, content: m.content, error: !!m.error };
        }));
    }

    function newChat() {
        if (state.streaming) stop();
        state.messages = [];
        state.lastHits = [];
        persist();
        renderThread();
        closeSettings();
        els.input.focus();
    }

    function updateHeroCount() {
        var t = els.thread.querySelector('.kk-tip-text');
        if (!t || !state.docs) return;
        var base = t.dataset.base || t.innerHTML;
        t.dataset.base = base;
        if (state.docs.length) {
            t.innerHTML = base + ' 站内已有 <b>' + state.docs.length + '</b> 篇文章可检索。';
        }
    }

    /* ══════════════════════════════════════════
       9. 发送 / 流式
       ══════════════════════════════════════════ */
    function setStreaming(on) {
        state.streaming = on;
        els.send.classList.toggle('is-stop', on);
        els.send.innerHTML = on ? ICONS.stop : ICONS.send;
        els.send.setAttribute('aria-label', on ? '停止生成' : '发送');
        els.send.disabled = on ? false : !els.input.value.trim();
        els.input.disabled = false;
    }

    function bubbleFor(m, isNew) {
        if (isNew) {
            els.thread.appendChild(messageEl(m, state.messages.length - 1));
            scrollToEnd(true);
        }
        var nodes = els.thread.querySelectorAll('.kk-msg');
        var node = nodes[nodes.length - 1];
        var b = node.querySelector('.kk-bubble');
        b.innerHTML = '<div class="kk-md"><span class="kk-dots"><i></i><i></i><i></i></span></div><div class="kk-think" hidden></div>';
        return b;
    }

    function send(raw) {
        var text = String(raw || '').trim();
        if (!text) return;
        if (state.streaming) return;

        if (!settings.apiKey && !settings.proxy) {
            openSettings();
            els.testOut.textContent = '先填一个 API Key（或代理地址）才能对话。';
            els.testOut.className = 'kk-test-out bad';
            return;
        }

        // 清空首屏 hero
        var hero = els.thread.querySelector('.kk-hero');
        if (hero) hero.remove();

        state.messages.push({ role: 'user', content: text });
        els.thread.appendChild(messageEl(state.messages[state.messages.length - 1], state.messages.length - 1));
        scrollToEnd(true);

        els.input.value = '';
        autoGrow();
        els.send.disabled = true;

        var reply = { role: 'assistant', content: '', reasoning: '' };
        state.messages.push(reply);
        var bubble = bubbleFor(reply, true);

        persist();
        run(text, reply, bubble);
    }

    function stop() {
        if (state.ctrl) {
            try { state.ctrl.abort(); } catch (e) {}
        }
    }

    function run(question, reply, bubble) {
        setStreaming(true);
        state.ctrl = new AbortController();
        var ctrl = state.ctrl;

        var prep = settings.useSearch ? ensureDocs().then(function () {
            var hits = searchDocs(question, 3);
            state.lastHits = hits;
            return hits;
        }) : Promise.resolve([]);

        prep.then(function (hits) {
            var msgs = [{ role: 'system', content: SYSTEM_PROMPT }];
            var ctx = contextFor(hits);
            if (ctx) msgs.push({ role: 'system', content: ctx });

            var history = state.messages.slice(0, -1).filter(function (m) {
                return !m.error && m.content;
            }).slice(-Math.max(2, CFG.maxHistory || 12));

            history.forEach(function (m) {
                msgs.push({ role: m.role, content: m.content });
            });

            var body = {
                model: settings.model || (PROVIDERS[settings.provider] || {}).models[0] || '',
                messages: msgs,
                stream: true,
                temperature: typeof CFG.temperature === 'number' ? CFG.temperature : 0.4
            };

            var url = settings.proxy ? settings.proxy : endpointOf(currentBase());
            var headers = { 'Content-Type': 'application/json' };
            if (!settings.proxy && settings.apiKey) {
                headers['Authorization'] = 'Bearer ' + settings.apiKey;
            }

            return fetch(url, {
                method: 'POST',
                headers: headers,
                body: JSON.stringify(body),
                signal: ctrl.signal
            }).then(function (res) {
                if (!res.ok) {
                    return res.text().then(function (t) {
                        throw new Error(kindError(res.status, t));
                    });
                }
                var ctype = res.headers.get('content-type') || '';
                if (ctype.indexOf('text/event-stream') < 0 || !res.body) {
                    return res.json().then(function (j) {
                        var msg = (j.choices && j.choices[0] && j.choices[0].message) || {};
                        reply.content = msg.content || '';
                        reply.reasoning = msg.reasoning_content || '';
                        paint(reply, bubble, false);
                    });
                }
                return stream(res, reply, bubble);
            });
        }).then(function () {
            finish(reply, bubble);
        }).catch(function (err) {
            if (err && err.name === 'AbortError') {
                reply.content = reply.content || '（已停止）';
                finish(reply, bubble);
                return;
            }
            reply.error = true;
            reply.content = reply.content
                ? reply.content + '\n\n> ' + err.message
                : err.message;
            paint(reply, bubble, false);
            finish(reply, bubble);
        });
    }

    function stream(res, reply, bubble) {
        var reader = res.body.getReader();
        var decoder = new TextDecoder('utf-8');
        var buffer = '';
        var lastPaint = 0;

        function handleLine(line) {
            line = line.trim();
            if (!line || line.charAt(0) === ':') return false;
            if (line.indexOf('data:') !== 0) return false;
            var data = line.slice(5).trim();
            if (!data) return false;
            if (data === '[DONE]') return true;
            var j;
            try { j = JSON.parse(data); } catch (e) { return false; }
            if (j.error) throw new Error(j.error.message || '服务商返回了错误');
            var ch = j.choices && j.choices[0];
            if (!ch) return false;
            var d = ch.delta || ch.message || {};
            if (d.reasoning_content) reply.reasoning += d.reasoning_content;
            if (d.content) reply.content += d.content;
            return false;
        }

        function pump() {
            return reader.read().then(function (r) {
                if (r.done) {
                    if (buffer) handleLine(buffer);
                    if (!reply.content) {
                        reply.error = true;
                        reply.content = '模型没有返回内容。可能是模型名不对、额度用尽，或被 CORS 拦掉了——换个服务商或填代理地址试试。';
                    }
                    return;
                }
                buffer += decoder.decode(r.value, { stream: true });
                var parts = buffer.split('\n');
                buffer = parts.pop();
                var done = false;
                for (var i = 0; i < parts.length; i++) {
                    if (handleLine(parts[i])) { done = true; break; }
                }
                var now = Date.now();
                if (now - lastPaint > 55) {
                    lastPaint = now;
                    paint(reply, bubble, true);
                }
                if (done) return;
                return pump();
            });
        }

        return pump();
    }

    function paint(reply, bubble, streaming) {
        if (!bubble || !bubble.isConnected) bubble = lastBubble();
        if (!bubble) return;
        var pinned = isNearBottom(els.thread);
        var html = '';
        if (reply.reasoning) {
            html += '<div class="kk-think"><b>思考过程</b>' + esc(reply.reasoning) + '</div>';
        }
        html += '<div class="kk-md">' + renderMarkdown(reply.content) + '</div>';
        if (streaming && !reply.content) {
            html += '<span class="kk-dots"><i></i><i></i><i></i></span>';
        }
        bubble.innerHTML = html;
        if (pinned) scrollToEnd(true);
    }

    function finish(reply, bubble) {
        setStreaming(false);
        state.ctrl = null;

        // 流式过程中渲染是节流的，收尾必须补最后一次，否则末尾几个字符永远不上屏
        if (bubble && bubble.isConnected) paint(reply, bubble, false);

        // 只挂真实被引用到的站内文章
        var used = (state.lastHits || []).filter(function (h) {
            return h.title && reply.content.indexOf(h.title) >= 0 || reply.content.indexOf(h.url) >= 0;
        });

        if (bubble && bubble.isConnected) {
            var wrap = bubble.closest('.kk-msg-main');
            if (used.length && !reply.error) {
                var src = document.createElement('div');
                src.className = 'kk-src';
                src.innerHTML = '<span class="kk-src-label">站内</span>' + used.map(function (h) {
                    return '<a href="' + esc(h.url) + '" target="_top">' + esc(h.title) + '</a>';
                }).join('');
                var foot = wrap.querySelector('.kk-msg-foot');
                if (foot) wrap.insertBefore(src, foot);
                else wrap.appendChild(src);
            }
            if (!reply.content) {
                reply.error = true;
                reply.content = '这次没拿到内容，稍后再试一次。';
                paint(reply, bubble, false);
            }
        }

        persist();
        paintStatus();
        els.send.disabled = !els.input.value.trim();
    }

    function kindError(status, text) {
        var detail = '';
        try {
            var j = JSON.parse(text);
            detail = (j.error && (j.error.message || j.error.type)) || j.message || '';
        } catch (e) {
            detail = String(text || '').slice(0, 160);
        }
        var head;
        if (status === 401 || status === 403) head = 'Key 无效或没有权限（' + status + '）。去设置里核对一下，注意别把空格带进去。';
        else if (status === 404) head = '接口或模型名找不到（404）。检查 Base URL 与模型名是否匹配。';
        else if (status === 429) head = '请求太频繁或额度用完了（429），等一会儿再试。';
        else if (status >= 500) head = '服务商那边出错了（' + status + '），不是你的问题，稍后再试。';
        else head = '请求失败（' + status + '）。';
        return head + (detail ? '\n\n原始信息：' + detail : '') +
            '\n\n如果浏览器控制台报 CORS / Failed to fetch，说明这家接口不允许前端直连，换 DeepSeek、硅基流动，或在设置里填代理地址。';
    }

    /* ══════════════════════════════════════════
       10. 测试连接
       ══════════════════════════════════════════ */
    function testConnection() {
        var key = (els.keyInput.value || '').trim();
        var base = currentBase();
        var model = (els.modelInput.value || '').trim() || (PROVIDERS[settings.provider] || {}).models[0];
        var proxy = settings.proxy || CFG.proxyUrl || '';

        if (!key && !proxy) {
            els.testOut.textContent = '还没有 Key，也没配代理。';
            els.testOut.className = 'kk-test-out bad';
            return;
        }

        var url = proxy || endpointOf(base);
        if (!url) {
            els.testOut.textContent = 'Base URL 是空的。';
            els.testOut.className = 'kk-test-out bad';
            return;
        }

        els.testOut.textContent = '正在握手…';
        els.testOut.className = 'kk-test-out';
        var started = Date.now();

        var headers = { 'Content-Type': 'application/json' };
        if (!proxy && key) headers['Authorization'] = 'Bearer ' + key;

        fetch(url, {
            method: 'POST',
            headers: headers,
            body: JSON.stringify({
                model: model,
                messages: [{ role: 'user', content: '只回两个字：收到' }],
                stream: false
            })
        }).then(function (res) {
            return res.text().then(function (t) {
                if (!res.ok) throw new Error(kindError(res.status, t));
                var j = JSON.parse(t);
                var msg = j.choices && j.choices[0] && j.choices[0].message;
                if (!msg) throw new Error('返回结构不认识：' + t.slice(0, 140));
                els.testOut.textContent = '通了。' + (msg.content || '').trim().slice(0, 20) +
                    ' · ' + (Date.now() - started) + ' ms';
                els.testOut.className = 'kk-test-out ok';
            });
        }).catch(function (err) {
            els.testOut.textContent = err.message;
            els.testOut.className = 'kk-test-out bad';
        });
    }

    /* ══════════════════════════════════════════
       11. 启动
       ══════════════════════════════════════════ */
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', build);
    } else {
        build();
    }

    // 给外部留个口子：KK.ask('问题') / KK.open()
    window.KK = {
        open: function () { open(); },
        close: function () { close(); },
        ask: function (q) {
            open();
            setTimeout(function () { send(q); }, 60);
        },
        reset: newChat
    };
})();
