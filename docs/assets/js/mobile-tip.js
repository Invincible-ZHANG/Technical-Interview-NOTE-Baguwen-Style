/* 移动端提示：改为非阻断的顶部横幅（可关闭，会话内只出现一次）
   替换原先的 alert() 硬阻断 */
(function () {
    var ua = navigator.userAgent.toLowerCase();
    var isMobile = /iphone|ipod|ipad|android|mobile|blackberry|webos|bada|nokia|ucweb|skyfire/.test(ua);
    if (!isMobile) return;
    try {
        if (sessionStorage.getItem('mobileTipDismissed') === '1') return;
    } catch (e) { /* 隐私模式下忽略 */ }

    function build() {
        var tip = document.createElement('div');
        tip.className = 'mobile-tip';
        tip.setAttribute('role', 'note');
        tip.innerHTML =
            '<span class="mobile-tip-icon">📱</span>' +
            '<span class="mobile-tip-text">本站为桌面端排版优化，手机上建议横屏或切换到电脑浏览～</span>' +
            '<button class="mobile-tip-close" type="button" aria-label="关闭提示">×</button>';
        document.body.appendChild(tip);

        // 入场动画
        requestAnimationFrame(function () { tip.classList.add('is-visible'); });

        tip.querySelector('.mobile-tip-close').addEventListener('click', function () {
            tip.classList.remove('is-visible');
            setTimeout(function () { tip.remove(); }, 260);
            try { sessionStorage.setItem('mobileTipDismissed', '1'); } catch (e) {}
        });

        // 8 秒后自动收起
        setTimeout(function () {
            if (!tip.isConnected) return;
            tip.classList.remove('is-visible');
            setTimeout(function () { tip.remove(); }, 260);
            try { sessionStorage.setItem('mobileTipDismissed', '1'); } catch (e) {}
        }, 8000);
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', build);
    } else {
        build();
    }
})();
