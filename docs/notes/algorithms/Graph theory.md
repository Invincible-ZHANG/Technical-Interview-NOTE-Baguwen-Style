---
layout: note
title: "图论"
date: 2025-07-11
excerpt: "图是一种非常重要的数据结构，图论是算法当中一个非常大的专题。"
categories: algorithms
tags:
  - 算法
  - 图论
  - 贪心算法
---


# 图

当然可以！下面是 **LeetCode 200. 岛屿数量** 的 README 题解模板，风格适合你放在 ACM/LeetCode 题解仓库：

---

## 200. 岛屿数量（Number of Islands）

题目链接：[LeetCode 200. Number of Islands](https://leetcode.cn/problems/number-of-islands/)

## 题目描述

给定一个由 `'1'`（陆地）和 `'0'`（水）组成的二维网格，计算网格中岛屿的数量。岛屿总是被水包围，
并且每个岛屿只能由水平方向或竖直方向相邻的陆地连接形成。假设网格的四个边均被水包围。

---

## 示例

**示例 1:**

```
输入:
grid = [
  ["1","1","1","1","0"],
  ["1","1","0","1","0"],
  ["1","1","0","0","0"],
  ["0","0","0","0","0"]
]
输出: 1
```

**示例 2:**

```
输入:
grid = [
  ["1","1","0","0","0"],
  ["1","1","0","0","0"],
  ["0","0","1","0","0"],
  ["0","0","0","1","1"]
]
输出: 3
```

---

## 解题思路

这题本质上是经典的 **连通块计数问题**，可以使用**深度优先搜索（DFS）** 或 **广度优先搜索（BFS）** 实现。

* **遍历整个网格**，每遇到一个 `'1'`，就将其所在的连通块全部遍历并标记为 `'0'`（已访问）。
* 每遇到一个未被访问的 `'1'`，岛屿数量加一。

常用的做法有：

1. **DFS 递归遍历**（推荐，思路简洁）
2. BFS 队列遍历

---

## 代码实现（C++）

### DFS写法

```cpp
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// 四个方向
const int dx[4] = {0, 0, 1, -1};
const int dy[4] = {1, -1, 0, 0};

void dfs(vector<vector<char>>& grid, int x, int y, int m, int n) {
    grid[x][y] = '0'; // 标记为已访问
    for (int d = 0; d < 4; ++d) {
        int nx = x + dx[d], ny = y + dy[d];
        if (nx >= 0 && nx < m && ny >= 0 && ny < n && grid[nx][ny] == '1') {
            dfs(grid, nx, ny, m, n);
        }
    }
}

int main() {
    int m, n;
    cin >> m >> n;
    vector<vector<char>> grid(m, vector<char>(n));
    // 读入
    for (int i = 0; i < m; ++i) {
        string row;
        cin >> row;
        for (int j = 0; j < n; ++j)
            grid[i][j] = row[j];
    }

    int cnt = 0;
    // 枚举每个格子
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            if (grid[i][j] == '1') {
                dfs(grid, i, j, m, n);
                ++cnt;
            }
    cout << cnt << endl;
    return 0;
}

```

---

## 复杂度分析

* **时间复杂度：** \$O(m \times n)\$，每个格子最多被访问一次。
* **空间复杂度：** \$O(m \times n)\$（递归栈/队列最大深度，最坏是全部为陆地）。

---

## 总结

* 连通块问题，典型模板，掌握 DFS/BFS 的套路即可。
* 注意：每个点最多访问一次，不需要额外的 visited 数组，可以直接在原数组上做标记。

---

如需 BFS 或 Python 题解、复杂度推导、面试考点分析等可随时补充！
