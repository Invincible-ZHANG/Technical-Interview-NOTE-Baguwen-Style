; (function () {
    const values = [
        "Harmony", "Peace", "Love",
        "Joy", "Hope", "Wisdom",
        "Serenity", "Grace", "Kindness",
        "Courage", "Passion", "Freedom"
    ];
    // 一组柔和的“粉彩”色
    const colors = [
        "#FFD1DC", "#FFE4B5", "#E0FFFF",
        "#E6E6FA", "#FFFACD", "#F0FFF0"
    ];
    // 一组可选字体大小（px）
    const sizes = [16, 18, 20, 22, 24];

    document.addEventListener('click', function (e) {
        const txt = values[Math.floor(Math.random() * values.length)];
        const color = colors[Math.floor(Math.random() * colors.length)];
        const size = sizes[Math.floor(Math.random() * sizes.length)];
        const angle = (Math.random() - 0.5) * 30;       // -15° 到 +15° 随机旋转
        const dx = (Math.random() - 0.5) * 40;       // -20px 到 +20px 随机横移

        const span = document.createElement('span');
        span.className = 'click-text';
        span.textContent = txt;

        // 注入样式变量
        span.style.setProperty('--rotate', `${angle}deg`);
        span.style.setProperty('--dx', `${dx}px`);
        span.style.color = color;
        span.style.fontSize = `${size}px`;

        // 定位到点击位置（做一下水平/垂直居中偏移）
        span.style.left = `${e.pageX - size * txt.length * 0.15}px`;
        span.style.top = `${e.pageY - size * 0.6}px`;

        document.body.appendChild(span);
        // 动画结束后移除元素
        span.addEventListener('animationend', () => span.remove());
    });
})();
