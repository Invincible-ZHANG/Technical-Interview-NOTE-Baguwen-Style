//(function () {
//    // 常见移动设备标识
//    const mobileAgents = [
//        "iphone", "ipod", "ipad",
//        "android", "mobile", "blackberry",
//        "webos", "incognito", "webmate",
//        "bada", "nokia", "lg", "ucweb", "skyfire"
//    ];
//    const ua = navigator.userAgent.toLowerCase();

//    // 先判断是不是手机
//    const isMobile = mobileAgents.some(agent => ua.includes(agent));
//    if (!isMobile) return;  // 不是手机，直接不提示

//    // 再判断本次会话里有没有提示过
//    if (!sessionStorage.getItem('mobileAlertShown')) {
//        alert(
//            "😅 嘿嘿，因为我太懒，还没给手机端做好适配～\n" +
//            "💻 最佳观赏体验请切换到电脑哦！\n\n" +
//            "👉 点“确定”继续用手机浏览吧，加油鸭！"
//        );
//        sessionStorage.setItem('mobileAlertShown', '1');
//    }
//})();



// docs/assets/js/mobile-alert-with-egg.js
(function () {
    // 常见移动设备标识
    const mobileAgents = [
        "iphone", "ipod", "ipad", "android", "mobile", "blackberry",
        "webos", "incognito", "webmate", "bada", "nokia", "lg", "ucweb", "skyfire"
    ];
    const ua = navigator.userAgent.toLowerCase();
    const isMobile = mobileAgents.some(agent => ua.includes(agent));
    if (!isMobile) return;

    // 会话内只提示一次
    if (sessionStorage.getItem('mobileAlertShown')) return;

    // 1) 先弹适配提示
    alert(
        "😅 我还没给手机端做好适配～\n" +
        "💻 最佳观赏体验请用电脑访问！\n\n" +
        "👉 点“确定”继续用手机浏览吧～"
    );

    // 2) 再来彩蛋互动
    const normalize = s => (s || '').replace(/\s|\u3000/g, '').toLowerCase();
    const answer = normalize(prompt("🎁 小彩蛋：你今天学习了吗？（是 / 不是）"));

    // 可识别关键词
    const yesSet = new Set(['是', 'shi', 'y', 'yes', '1']);
    const noSet = new Set(['不是', 'bushi', 'n', 'no', '否', '0']);

    if (answer) {
        if (yesSet.has(answer)) {
            alert('🎉 记录成功：你是卷王，建议休息！');
        } else if (noSet.has(answer)) {
            alert('📎 收到：趁早躺平！');
        } else {
            alert('我只认识“是 / 不是”。下次再来玩～');
        }
    }

    sessionStorage.setItem('mobileAlertShown', '1');
})();
