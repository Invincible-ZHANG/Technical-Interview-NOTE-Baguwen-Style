// �Զ��� <pre><code> ��װΪ���� .code-wrapper ��ʽ������������ Markdown �е���������
document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("pre code").forEach((codeBlock) => {
        const pre = codeBlock.parentElement;
        if (!pre || pre.classList.contains("code-wrapper")) return;

        const langMatch = codeBlock.className.match(/language-(\w+)/);
        const langLabel = langMatch ? langMatch[1] : "code";

        const wrapper = document.createElement("div");
        wrapper.className = "code-wrapper";
        wrapper.setAttribute("data-rel", langLabel);

        // 顶栏右侧：复制按钮
        const copyBtn = document.createElement("button");
        copyBtn.className = "code-copy-btn";
        copyBtn.type = "button";
        copyBtn.textContent = "Copy";
        copyBtn.addEventListener("click", async () => {
            try {
                await navigator.clipboard.writeText(codeBlock.innerText);
                copyBtn.textContent = "Copied";
                setTimeout(() => (copyBtn.textContent = "Copy"), 1200);
            } catch (e) {
                copyBtn.textContent = "Failed";
                setTimeout(() => (copyBtn.textContent = "Copy"), 1200);
            }
        });

        // 构建结构：wrapper -> 顶栏(伪元素) + 按钮 + pre
        pre.parentNode.replaceChild(wrapper, pre);
        wrapper.appendChild(pre);
        wrapper.appendChild(copyBtn);
    });
});
