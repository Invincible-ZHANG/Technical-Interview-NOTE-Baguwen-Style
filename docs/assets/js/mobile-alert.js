(function () {
    // 常见移动设备标识
    const mobileAgents = [
        "iphone", "ipod", "ipad",
        "android", "mobile", "blackberry",
        "webos", "incognito", "webmate",
        "bada", "nokia", "lg", "ucweb", "skyfire"
    ];
    const ua = navigator.userAgent.toLowerCase();

    // 先判断是不是手机
    const isMobile = mobileAgents.some(agent => ua.includes(agent));
    if (!isMobile) return;  // 不是手机，直接不提示

    // 再判断本次会话里有没有提示过
    if (!sessionStorage.getItem('mobileAlertShown')) {
        alert(
            "😅 嘿嘿，因为我太懒，还没给手机端做好适配～\n" +
            "💻 最佳观赏体验请切换到电脑哦！\n\n" +
            "👉 点“确定”继续用手机浏览吧，加油鸭！"
        );
        sessionStorage.setItem('mobileAlertShown', '1');
    }
})();
