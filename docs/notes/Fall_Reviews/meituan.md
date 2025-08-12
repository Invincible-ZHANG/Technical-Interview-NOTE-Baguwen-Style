---
title: "美团 硬件笔试"
date: 2025-08-09
categories: Fall_Reviews
tags: [笔试,美团, 嵌入式软件]
layout: note
excerpt: 2小时，20道选择加两道编程。

---
## 第一次笔试（2025-08-09）

### 编程题



~~~
#include <bits/stdc++.h>  // 一次性包含几乎所有C++标准库（竞赛常用）
using namespace std;

int main() {
    ios::sync_with_stdio(false); // 关闭C和C++标准流的同步，加快cin/cout速度
    cin.tie(nullptr);            // 解除cin和cout的绑定，防止每次cin前强制刷新cout

    int n;
    // 如果无法成功读入n（例如输入结束EOF），直接退出程序
    if (!(cin >> n)) return 0;

    // a[i] 表示第i只怪兽的血条
    vector<long long> a(n + 1);
    for (int i = 1; i <= n; ++i) cin >> a[i];

    // NEG 表示“无效状态”，初始化时用这个值防止错误转移
    const long long NEG = LLONG_MIN / 4;

    // cur[t]   表示当前击杀数 mod 10 == t 时的最大经验值
    // nxt[t]   表示下一轮（处理完第i+1只怪后）的状态
    array<long long, 10> cur, nxt;

    // 初始化cur数组，全部设为无效值NEG
    for (int t = 0; t < 10; ++t) cur[t] = NEG;
    cur[0] = 0; // 处理0只怪时，击杀数mod10=0，经验为0

    // 枚举每一只怪兽
    for (int i = 0; i < n; ++i) {
        // 每次处理新怪兽前，清空下一轮状态
        for (int t = 0; t < 10; ++t) nxt[t] = NEG;

        // 枚举当前所有可能的击杀数mod10状态
        for (int t = 0; t < 10; ++t) {
            long long val = cur[t]; // 当前状态的最大经验值
            if (val == NEG) continue; // 无效状态跳过

            // 1) 放走第 i+1 只怪
            //   经验增加 (i+1)，击杀数不变（mod10状态不变）
            nxt[t] = max(nxt[t], val + (i + 1));

            // 2) 击杀第 i+1 只怪
            //   经验增加 ((当前击杀数mod10) + 1) * a[i+1]
            //   击杀数mod10 状态更新为 (t+1) % 10
            long long gain = (t + 1) * a[i + 1];
            nxt[(t + 1) % 10] = max(nxt[(t + 1) % 10], val + gain);
        }

        // 更新当前状态
        cur = nxt;
    }

    // 最终答案是处理完n只怪后，所有可能的mod10状态中的最大值
    cout << *max_element(cur.begin(), cur.end()) << "\n";
    return 0;
}

~~~




第二题：