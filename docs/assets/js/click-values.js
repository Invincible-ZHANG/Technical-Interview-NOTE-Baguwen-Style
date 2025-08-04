; (function () {
    const values = [
        "Harmony", "Peace", "Love",
        "Joy", "Hope", "Wisdom",
        "Serenity", "Grace", "Kindness",
        "Courage", "Passion", "Freedom"
    ];
    // һ����͵ġ��۲ʡ�ɫ
    const colors = [
        "#FFD1DC", "#FFE4B5", "#E0FFFF",
        "#E6E6FA", "#FFFACD", "#F0FFF0"
    ];
    // һ���ѡ�����С��px��
    const sizes = [16, 18, 20, 22, 24];

    document.addEventListener('click', function (e) {
        const txt = values[Math.floor(Math.random() * values.length)];
        const color = colors[Math.floor(Math.random() * colors.length)];
        const size = sizes[Math.floor(Math.random() * sizes.length)];
        const angle = (Math.random() - 0.5) * 30;       // -15�� �� +15�� �����ת
        const dx = (Math.random() - 0.5) * 40;       // -20px �� +20px �������

        const span = document.createElement('span');
        span.className = 'click-text';
        span.textContent = txt;

        // ע����ʽ����
        span.style.setProperty('--rotate', `${angle}deg`);
        span.style.setProperty('--dx', `${dx}px`);
        span.style.color = color;
        span.style.fontSize = `${size}px`;

        // ��λ�����λ�ã���һ��ˮƽ/��ֱ����ƫ�ƣ�
        span.style.left = `${e.pageX - size * txt.length * 0.15}px`;
        span.style.top = `${e.pageY - size * 0.6}px`;

        document.body.appendChild(span);
        // �����������Ƴ�Ԫ��
        span.addEventListener('animationend', () => span.remove());
    });
})();
