// 自动将 <pre><code> 包装为带有 .code-wrapper 样式的容器，保留 Markdown 中的语言类型
document.addEventListener("DOMContentLoaded", () => {
    document.querySelectorAll("pre code").forEach((codeBlock) => {
        const pre = codeBlock.parentElement;
        if (!pre || pre.classList.contains("code-wrapper")) return;

        // 获取语言名（比如 language-cpp、language-python）
        const langMatch = codeBlock.className.match(/language-(\w+)/);
        const langLabel = langMatch ? langMatch[1] : "code";

        // 创建 Mac 风格 wrapper
        const wrapper = document.createElement("div");
        wrapper.className = "code-wrapper";
        wrapper.setAttribute("data-rel", langLabel);  // 显示文件语言名（可选）

        // 包装替换结构
        pre.parentNode.replaceChild(wrapper, pre);
        wrapper.appendChild(pre);
    });
});
