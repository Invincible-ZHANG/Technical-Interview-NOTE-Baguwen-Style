// �Զ��� <pre><code> ��װΪ���� .code-wrapper ��ʽ������������ Markdown �е���������
document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("pre code").forEach((codeBlock) => {
        const pre = codeBlock.parentElement;
        if (!pre || pre.classList.contains("code-wrapper")) return;

        // ��ȡ������������ language-cpp��language-python��
        const langMatch = codeBlock.className.match(/language-(\w+)/);
        const langLabel = langMatch ? langMatch[1] : "code";

        // ���� Mac ��� wrapper
        const wrapper = document.createElement("div");
        wrapper.className = "code-wrapper";
        wrapper.setAttribute("data-rel", langLabel);  // ��ʾ�ļ�����������ѡ��

        // ��װ�滻�ṹ
        pre.parentNode.replaceChild(wrapper, pre);
        wrapper.appendChild(pre);
    });
});
