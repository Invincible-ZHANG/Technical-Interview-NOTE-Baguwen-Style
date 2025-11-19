# 基于可操作度的伸缩臂奇异性判定——实现说明书（用于汇报）

> 版本：v2.0（报告版）
> 适用对象：伐木车/起重臂等“伸缩 + 指向”机构的在线奇异性监测
> 目标：在不明确构造全雅可比的情况下，给出**实时、可解释、可标定**的奇异性判定与调参与验证方法

---

## 1. 问题定义与判定目标

**问题**：伸缩臂在某些构型下（过短/过长/过竖），末端对任务空间速度/力的可达性退化（雅可比秩亏或条件数恶化），出现**奇异/近奇异**风险。
**目标**：在线计算一个介于 ([0,1]) 的可操作度评分 (w)，并据此产生**稳健且不过敏**的“进入/退出奇异”判定信号，提供**主因诊断**（过短、过长、过竖）。

---

## 2. 理论基础（为何可行）

### 2.1 可操作度与奇异性

经典可操作度（Yoshikawa）定义为
[
w_Y(q) ;=; \sqrt{\det\big(J(q),J(q)^\top\big)} ;=; \prod_{i}\sigma_i(q),
]
其中 (\sigma_i) 为雅可比奇异值。**越接近奇异**（最小奇异值 (\sigma_{\min}\to 0)），(w_Y\to 0)。

### 2.2 伸缩 + 指向机构的主导近似

对“（偏航 (\phi)、俯仰 (\theta)、伸缩 (L)）(\to) 末端位置 (\mathbf{r})”的常见几何，可推得
[
\det(JJ^\top) ;\propto; L^{2}\sin^{2}\theta,
]
即体积与“尺度 (L) × 方向正交度 (\sin\theta)”主导相关。
**工程近似**：用**长度分量** (w_L(L)) 与**方向分量** (w_D(\theta)) 的乘积近似 (w_Y) 的主导结构，再做归一化与权重融合。

---

## 3. 评分模型与计算流程

### 3.1 输入量（每帧）

* 基端/末端位置：(\mathbf{p}*\text{base}, \mathbf{p}*\text{end})
* 重力向量：(\mathbf{g})（若不可得，退化用世界 (\hat z)）
* 伸缩轴（可选更稳）：单位向量 (\hat a)

### 3.2 基本几何

* **臂向量与长度**
  [
  \mathbf{r}=\mathbf{p}*\text{end}-\mathbf{p}*\text{base},\qquad
  L=|\mathbf{r}|\quad\text{或}\quad L=|\mathbf{r}\cdot\hat a|
  ]
* **单位方向**：(\hat n=\mathbf{r}/|\mathbf{r}|)
* **“上方向”**：(\hat u=-\mathbf{g}/|\mathbf{g}|)（如果 (|\mathbf{g}|\approx 0)，用 (\hat z)）

> 推荐 **轴向投影** 定义 (L)，更贴近“伸缩自由度”。

### 3.3 长度分量（区间窗，严格无量纲）

设软限 (L_{\min},L_{\max})（见 §5），定义
[
t=\frac{L-L_{\min}}{L_{\max}-L_{\min}}\in[0,1],\qquad
w_L = \big(4,t(1-t)\big)^{\kappa}\in[0,1],
]
(\kappa\in[1.0,1.8]) 为“钝化指数”，(\kappa>1) 时中段更平坦、端点更温和。
（两端 (w_L\to 0)，中段 (w_L\approx 1)）

### 3.4 方向分量（不使用 acos，数值更稳）

[
w_D ;=; \sin\theta ;=; |\hat n\times \hat u|;=;\sqrt{,1-(\hat n\cdot \hat u)^2,}\in[0,1].
]
**可选降敏**：

* **死区抬升**：(w_D\leftarrow \max\big(0,\frac{w_D-\delta_D}{1-\delta_D}\big))，(\delta_D=0.05\sim0.1)
* **温度软饱和**：(w_D\leftarrow \frac{w_D}{w_D+\tau_D})，(\tau_D=0.05\sim0.2)

### 3.5 旋转分量（可扩展钩子）

若暂不纳入台座旋转软限：(w_R\equiv 1)。
如加入旋转角 (\psi) 的软限 ([\psi_{\min},\psi_{\max}])，同样用区间窗：
[
\tilde t=\frac{\psi-\psi_{\min}}{\psi_{\max}-\psi_{\min}},\quad
w_R=4,\tilde t(1-\tilde t).
]

### 3.6 分量融合（加权几何平均）

[
w ;=; w_L^{\alpha}, w_D^{\beta}, w_R^{\gamma},\qquad
\alpha+\beta+\gamma=1,
]
推荐先验：((\alpha,\beta,\gamma)=(0.6,,0.3,,0.1))。

> 解释：几何平均**短板敏感**；在 (\log) 域线性可加，(\alpha) 是“弹性”：(\partial\log w/\partial\log w_L=\alpha)。

### 3.7 判决（滞后 + 去抖 + 低通）

* **低通（可选）**：(w_f\leftarrow (1-\lambda),w_f+\lambda,w)，(\lambda=0.1\sim0.3)
* **双阈值（施密特触发）**：
  [
  \text{enter: } w_f<e_w\quad;\quad \text{exit: } w_f>x_w,\quad x_w>e_w
  ]
  经验起点：(e_w=0.20,\ x_w=0.35)（过敏时可拉大间隙，如 0.15/0.45）
* **连续帧去抖**：进入/退出各需 N 帧（5～12）

### 3.8 主因诊断（用于人机交互/日志）

* 若 (w_L<\tau_L)：

  * (L<\frac{L_{\min}+L_{\max}}{2}\Rightarrow) **过短**
  * 否则 **过长**
* 若 (w_D<\tau_D)：**过竖**
  将主因打印到日志/界面，方便操作者快速调整。

---

## 4. 参数获取与标定

### 4.1 (L_{\min}, L_{\max})（软限，而非机械硬限）

* **主动“软触边”标定**：低速靠近两端的**软件限位**，记录稳定段均值 (\mu) 与方差 (\sigma)
  [
  L_{\min}=\mu_{\min}+\Delta_{\text{delay}}+\Delta_{\text{over}}+k_\sigma \sigma_{\min}
  ]
  [
  L_{\max}=\mu_{\max}-\Delta_{\text{delay}}-\Delta_{\text{over}}-k_\sigma \sigma_{\max}
  ]
  其中 (\Delta_{\text{delay}}=v_{\max}\tau)（控制延迟），(\Delta_{\text{over}})（超调），(k_\sigma=3\sim5)。
* **被动统计**：运行期维护 1%/99% 分位 + 噪声裕度，收敛后固化。
* **关节映射**：若已知伸缩关节 (\ell) 及 ([\ell_{\min},\ell_{\max}]) 且 (L=L_0+c,\ell)，直接映射后再加裕度。

### 4.2 权重与阈值数据化

* **权重**：对已打标签数据（安全/奇异），在 (\log) 域做逻辑回归：
  (\Pr(\text{奇异})=\sigma!\big(b_0+a\log w_L+b\log w_D+c\log w_R\big))，将 ((a,b,c)) 归一成 ((\alpha,\beta,\gamma))。
* **阈值**：扫描 ((e_w,x_w,\text{连续帧}N))，以“漏报代价≫误报代价”为目标做网格搜索，选 ROC/PR 最优点。

---

## 5. 数值稳定与“不过敏”设计

* **方向项不使用 acos**；改用 (\sqrt{1-(\hat n\cdot \hat u)^2})；必要时加**死区 + 温度软饱和**。
* **长度项用区间窗**并加指数钝化；可再加小死区（行程 2–5%）。
* **融合前地板保护**：(w_i\leftarrow \max(w_i,\varepsilon_i))，(\varepsilon_i=0.02\sim0.05)。
* **滞后 + 连续帧 + 低通**：用于抑制边界抖动。
* **预热帧**：启动后前 5～20 帧不判定，避免初始化假阳性。

---

## 6. 可选增强：与最小奇异值混合（更“保守稳定”）

当需要更稳健的“离奇异距离”度量，可引入**简化雅可比**的 (\sigma_{\min})（3×3 或 3×2，小矩阵计算量极低）：

* 构造 (J(q))（([L,\theta,\phi]\to[x,y,z]) 的位置雅可比），计算
  (\lambda_{\min}(JJ^\top)=\sigma_{\min}^2)。
* **混合判据**：
  [
  \textbf{进入}:\ (w<e_w) \ \textbf{or}\ (\sigma_{\min}<e_\sigma),\qquad
  \textbf{退出}:\ (w>x_w)\ \textbf{and}\ (\sigma_{\min}>x_\sigma).
  ]

> 这一步与 Dantzig/LCP 无关，仅是**运动学层**的增强判据。

---

## 7. 复杂度与实时性

* 所有分量与融合：**O(1)** 标量运算（点/叉乘、开方、幂），典型单帧 (<1,\mu s)。
* 低通/去抖/日志：常数开销。
* 可选 (\sigma_{\min})：仅 3×3 对称特征或 SVD，亦可在**每 N 帧**做一次。

---

## 8. 对外接口与代码嵌入（最小改动版）

**建议将以下项做成可配置属性**（UI/配置文件）：

* 行程软限：`L_min, L_max`（含标定工具）
* 权重：`alpha, beta, gamma`
* 降敏：`kappa, deltaL, deltaD, tauD, epsFloor`
* 判决：`enter, exit, needDangerFrames, needSafeFrames, emaLambda`
* 预热：`warmupFrames`

**关键实现点（与你现有代码一致的替换/增强）**

* 长度分量（替换 3.2）：区间窗 + 指数 + 小死区
* 方向分量（替换 3.3）：`sqrt(1-(dot)^2)` + 死区 + 温度
* 融合前地板保护 + 减小对应权重
* 判决层：双阈值 + 连续帧 + EMA
* 诊断输出：打印 (w_L,w_D,w_R,w) 与主因标签

---

## 9. 验证与验收指标（建议）

* **静态验证**：

  * (L\to L_{\min}/L_{\max}\Rightarrow w\to 0)；(\theta\to 0/\pi\Rightarrow w\to 0)。
  * (L\approx(L_{\min}+L_{\max})/2,\ \theta\approx\pi/2\Rightarrow w\approx 1)。
* **动态回放**：

  * **误报率**（正常工况中无意义报警比例）
  * **漏报率**（已知近奇异片段未报警比例）
  * **边界抖动次数**（单位时间内进/出状态翻转次数）
* **调参目标**：

  * 漏报≈0；误报尽量低；边界抖动接近 0；报警触发到操作调整的平均时间受控。
  * 若引入 (\sigma_{\min})，以“漏报权重=10×误报”做加权优化。

---

## 10. FAQ（与 Dantzig 的关系）

* **是否等价于做 SVD？** 不是。当前评分是雅可比可操作度的**主导近似**，不做 SVD。
* **Dantzig（LCP 枢轴）内部是否用 SVD？** 否。Dantzig 用 **LDLᵀ/LU** 等线性分解；其矩阵与“运动学奇异”不是同一对象。
* **要更稳就必须做 SVD 吗？** 不需要。先按本文的**降敏 + 滞后 + 去抖 + 标定**即可；必要时仅对**小型雅可比**补充 (\sigma_{\min}) 判据。

---

## 11. 附：伪代码（可直接对照改造你现有函数）

```cpp
// 每帧：输入 p_base, p_end, gravity g
Vector3 r = p_end - p_base;
double L = use_axis ? fabs(dot(r, a_hat)) : norm(r);
if (L < 1e-6) { singular=true; return; }

Vector3 n = r / max(L,1e-9);
Vector3 u = (norm(g)>1e-9) ? (-g / norm(g)) : Vector3(0,0,1);

// --- 方向分量（稳健）
double c  = clamp(dot(n, u), -1.0, 1.0);
double wD = sqrt(max(0.0, 1.0 - c*c));
wD = max(0.0, (wD - deltaD) / (1.0 - deltaD));   // 死区抬升
wD = wD / (wD + tauD);                            // 温度软饱和

// --- 长度分量（区间窗 + 钝化 + 小死区）
double wL = 0.0;
if (L > L_min && L < L_max) {
    double t = (L - L_min) / (L_max - L_min);
    wL = pow(max(0.0, 4.0 * t * (1.0 - t)), kappa);
    wL = max(0.0, (wL - deltaL) / (1.0 - deltaL));
}
double wR = 1.0; // 暂不纳入旋转

// --- 地板保护 + 几何平均融合
wL = max(wL, epsFloor);
wD = max(wD, epsFloor);
double w_raw = pow(wL, alpha) * pow(wD, beta) * pow(wR, gamma);

// --- EMA 低通
w_f = (1.0 - emaLambda) * w_f + emaLambda * w_raw;

// --- 双阈值 + 连续帧去抖
if (w_f < enterTh) { danger_cnt++; safe_cnt=0; }
else if (w_f > exitTh) { safe_cnt++; danger_cnt=0; }
else { danger_cnt=0; safe_cnt=0; }

if (danger_cnt >= needDangerFrames) singular = true;
if (safe_cnt   >= needSafeFrames)   singular = false;

// --- 诊断
if (first_enter) {
   log << "w="<<w_f<<" wL="<<wL<<" wD="<<wD;
   if (wL < tauL) log << ((L < 0.5*(L_min+L_max)) ? "too short" : "too long");
   if (wD < tauD_diag) log << "too vertical";
}
```

---

## 12. 结论

* 本方法以**雅可比可操作度**为理论根基，提取“**长度 × 方向**”两大主因，构成**低成本、可解释**的在线评分；
* 通过“**区间窗 + 稳健方向项 + 几何平均 + 双阈值 + 去抖 + 低通 + 标定**”实现“**准确、不过敏**”的奇异性判定；
* 需要更稳时，可**混合小型雅可比的 (\sigma_{\min})** 作为辅判；
* 与 Dantzig/LCP 求解内核解耦，适合在现有仿真/控制栈中无缝植入与扩展。

---

如需，我可以把上述参数做成一份**配置模板**（INI/JSON）与**日志标定脚本**（从运行日志学习 (\alpha,\beta,\gamma) 与阈值），以便在不同机型/作业场景间快速迁移。
