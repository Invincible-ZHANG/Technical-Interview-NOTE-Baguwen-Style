(function () {
    // 只提示一次
    if (localStorage.getItem('mobileAlertShown')) return;

    // 常见移动设备标识
    var mobileAgents = [
        "iphone", "ipod", "ipad",
        "android", "mobile", "blackberry",
        "webos", "incognito", "webmate",
        "bada", "nokia", "lg", "ucweb", "skyfire"
    ];
    var ua = navigator.userAgent.toLowerCase();

    for (var i = 0; i < mobileAgents.length; i++) {
        if (ua.indexOf(mobileAgents[i]) !== -1) {
            alert(
                "😅 嘿嘿，因为我太懒，还没给手机端做好适配～\n" +
                "💻 最佳观赏体验请切换到电脑哦！\n\n" +
                "👉 点“确定”继续用手机浏览吧，加油鸭！"
            );
            // 标记已提示
            localStorage.setItem('mobileAlertShown', '1');
            break;
        }
    }
})();
