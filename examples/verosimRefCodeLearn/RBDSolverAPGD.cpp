/**
 * @file RBDSolverAPGD.cpp
 * @brief Implementation of the RBDLcpAPGD class using Accelerated Projected Gradient Descent (APGD) for solving mixed Linear Complementarity Problems (LCPs).
 * @author Zijian Zhang
 * @date 2025-07-24
 *
 * This file contains the implementation of the RBDLcpAPGD solver, which extends
 * the RBMMixedLCP interface and provides warm-start capability, Lipschitz constant estimation,
 * line search with Armijo condition, and friction handling for rigid-body dynamics problems.
 */

#include "../RBDSolverAPGD.h"
#include <cstring>
#include <algorithm>

#include "../../Lib/VSM/VSMVectorNDynamic.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>

#include <limits>   // for std::numeric_limits
#include <cmath>    // for std::sqrt

#include <vector>


using namespace VSLibRBDynamX;

/**
 * @brief Enable or disable warm start functionality.
 * @param flag If true, the solver will use the previous solution as an initial guess.
 * 这部分是关于热启动的，使用上一次迭代/上一次时间步的解作为初值，加速收敛。
 * @param flag true 则启用热启动；false 则每次从默认初值开始（通常是 0 向量）。
 */
void RBDLcpAPGD::EnableWarmStart(bool flag) {
	useWarmStart = flag;
}

/**
 * @brief Set the initial guess for the Lagrange multipliers (warm start).
 * @param new_lambda Vector of previous solution values to initialize xprev.
 * 设置拉格朗日乘子的初值（用于 warm start）。
 */
void RBDLcpAPGD::SetLambda(const std::vector<double>& new_lambda) {
    xprev = new_lambda;
}

/**
 * @brief Retrieve the stored warm-start values of the Lagrange multipliers.
 * @return Vector containing the previous solution (xprev).
 * 读取当前缓存的 warm-start 乘子。
 * 这部分这个vector是不是要使用其他的VSM::VectorN 后面可以再做探讨。
 */
std::vector<double> RBDLcpAPGD::GetLambda() const {
	return xprev;
}

/**
 * @brief Load the right-hand side vector for the LCP from a VSM::VectorN object.
 * @param rhs Input vector containing the right-hand side values.
 * 来自建模阶段的读取右端项的值。
 */
void RBDLcpAPGD::setValuesInRightSide(const VSM::VectorN& rhs) {
	if (b) delete[] b;
	b = new double[size()];
	memcpy(b, rhs.getData().data(), size() * sizeof(double));
	// 仅作调试用的拷贝
	// 等价于做了一次“深拷贝”。
	rhs_.assign(rhs.getData().begin(), rhs.getData().end());
}


/**
 * @brief 计算 APGD 的 Res4 残差（Chrono 风格）：以固定步长 gdiff=1/n^2 做一次“投影-梯度”差分，
 *        返回 || (γ - Proj(γ - gdiff * ∇f(γ))) || / gdiff 的 2-范数。
 *
 * 数学背景：
 *   最小化 f(γ) = 0.5 * γᵀ A γ - bᵀ γ，∇f(γ) = Aγ - b。
 *   Res4(gamma) = || (gamma - Π(gamma - gdiff * (A*gamma - b))) ||_2 / gdiff，
 *   其中 Π(·) 为约束投影（盒约束 + 摩擦圆锥投影）。
 *
 * 设计动机：
 *   - 采用固定的 gdiff 与线搜索步长解耦，避免当实际步长很小时残差被“缩小”而虚假收敛。
 *   - 与 Project Chrono 的 APGD 实现保持一致的残差定义，数值更平滑、鲁棒性更好。
 *
 * @param gamma  当前拉格朗日乘子/约束反力向量（长度为 n）。
 * @return double 残差标量（收敛判据用）。值越小表示越接近投影梯度不动点。
 *
 * 复杂度：O(n^2)（密集 A 的一次 SpMV + 一次投影遍历；若 A 为稀疏/算子式可降至 O(nnz)）。
 * 前置条件：
 *   - size() == n，A 为 n×n 的行主序连续存储（步幅 myPadSize），b 为长度 n。
 *   - projectBounds / projectFriction 定义良好且与 gamma 维度匹配。
 * 线程安全：非线程安全（读写临时向量本地 OK，但 A/b/内部状态并未声明为只读或互斥）。
 */

double RBDLcpAPGD::ResidualRes4(const std::vector<double>& gamma) const {
	const int n = gamma.size();
	// z时gamma的深拷贝，完整的复制一份
	std::vector<double> g(n, 0.0), z(gamma);
	const double gdiff = 1.0 / std::max(1.0, double(n) * n);

	// g = A*gamma - b   （与你上面 grad 的号一致）
	for (int i = 0; i < n; ++i) {                 // 遍历矩阵 A 的第 i 行
		const double* row = &A[i * myPadSize];    // 取出第 i 行的首地址（行主序 + 行步幅 myPadSize）
		double s = 0.0;                           // 累加器 s = 0
		for (int j = 0; j < n; ++j)               // 行·列做点积：A[i,:] · gamma
			s += row[j] * gamma[j];               // s = Σ_j A[i,j] * gamma[j]
		g[i] = s - b[i];                          // g[i] = (A*gamma)[i] - b[i]
	}
	// z = Proj( gamma - alpha * g )
	for (int i = 0; i < n; ++i) z[i] = gamma[i] - alpha * g[i];
	projectBounds(z);
	if (useFriction) projectFriction(z);

	// 3) 残差 = ||(gamma - z)|| / gdiff
	const double inv_g = 1.0 / gdiff;
	// const double inva = 1.0 / std::max(alpha, 1e-16);
	// double gdiff = 1.0 / std::max(1, n * n);
	double accum = 0.0;
	for (int i = 0; i < n; ++i) {
		double d = (gamma[i] - z[i]) * inv_g;
		accum += d * d;
	}
	return std::sqrt(accum);
}


/**
 * @brief 将无填充（unpadded）的系统矩阵按行复制到内部存储，并在每行尾部补零到对齐宽度。
 * 十分重要的函数，加上之后立竿见影的快
 *
 * 该函数把 @p values 的每一行连续内存拷贝到内部线性数组 @c A 中，
 * 并将每行的尾部用 0 填充至固定步幅 @c myPadSize 。这样可使后续
 * 矩阵-向量乘法 (A*vec) 具备固定上界、对齐良好的访问模式，便于 SIMD 向量化。
 *
 * @param values 输入矩阵（无填充、行主序），尺寸应为 size() × size()（或列数 ≤ myPadSize）。
 *
 * @pre
 *  - 已为 @c A 分配至少 @c values.rows()*myPadSize 个 @c double 的连续内存；
 *  - @c myPadSize >= values.columns()，且通常为 8/16 的整数倍以利于对齐；
 *  - @c values 每行的元素在内存中连续存放（能通过 &values[r][0] 取得行首指针）。
 *
 * @post
 *  - @c A 的第 r 行区间 [r*myPadSize, r*myPadSize+values.columns()) 拷贝自 @p values 第 r 行；
 *  - 每行尾部区间 [r*myPadSize+values.columns(), (r+1)*myPadSize) 被置 0。
 *
 * @note 复杂度为 O(rows * myPadSize)。补零不会影响数值正确性（额外乘以 γ 的那一段是 0）。
 * @warning 本函数不做边界检查或内存分配；调用方需保证前置条件满足，否则可能越界。
 * @thread_safety 非线程安全；若在多线程环境下调用，请为不同求解器实例各自维护独立缓冲。
 */
inline void RBDLcpAPGD::setValuesInMatrix(const VSM::MatrixNxM& values)
{
	// 确保 A 已经分配过内存 to size()×myPadSize
	// 然后按行拷贝并在每行后面填充 pad zeros
	unsigned int rows = values.rows();
	unsigned int cols = values.columns();
	size_t p = 0;

	for (unsigned int r = 0; r < rows; ++r) {
		// 把有用的 cols 列复制进 A[p..]
		std::memcpy(&A[p], &values[r][0], cols * sizeof(double));
		// p是偏移量
		p += cols;
		// 剩下的位置补 0
		std::memset(&A[p], 0, (myPadSize - cols) * sizeof(double));
		p += (myPadSize - cols);
	}
}

/**
 * @brief APGD 求解 “SOCCP” 的求解器。
 *
 * RBDLcpAPGD(int size, int nub, int maxIters_, double tol_, double accel_)
 *
 * @param size       变量总维数 n（约束未知量 λ 的长度），包含法向+切向等全部分量。
 * @param nub        前缀自由变量（unbounded / equality）的个数；下标 [0..nub-1] 为等式行，
 *                   它们没有上下界（或等价为 lo=-inf, hi=+inf），用于建模双边/驱动等等式约束。
 *                   从 nub 到 n-1 的变量是带界的互补变量（接触/摩擦等）。
 * @param maxIters_  迭代上限（每次 Solve 的最大步数）。
 * @param tol_       收敛阈值（残差/投影梯度范数/互补违背量允许的最大值）。
 * @param accel_     加速因子（Nesterov/FISTA 动量的权重或开关；0 退化为 PGD，1 为默认全加速）。没有用到。
 */
// RBDLcpAPGD::RBDLcpAPGD(int size, int nub, int maxIters_, double tol_, double accel_)
RBDLcpAPGD::RBDLcpAPGD(int size, int nub, int maxIters_, double tol_)
	: VSLibRBDynMath::RBMMixedLCP(size, nub),
	x(nullptr), w(nullptr),
	A(nullptr), b(nullptr),
	myHi(nullptr), myLo(nullptr),
	addFriction(nullptr), frictionIndices(nullptr),
	myPadSize(0), myNub(nub),
	maxIters(maxIters_), tol(tol_), accel(accel_),
	Lk(1.0), tk(1.0), theta(1.0),L(1.0)
{
	// 分配 pad-aligned A，及解向量
	myPadSize = this->size();
	// 改成先把 size 转成 size_t，再乘：
	size_t total = static_cast<size_t>(size) * static_cast<size_t>(myPadSize);
	A = new double[total];
	b = new double[size];
	x = new VSM::VectorN(size);
	w = new VSM::VectorN(size);

	// 分配并初始化上下界 lo, hi（默认无限制）
	myLo = new double[size];
	myHi = new double[size];
	for (int i = 0; i < size; ++i) {
		myLo[i] = -1e10;  // 对于等式约束，可视为 [-∞, +∞]
		myHi[i] = +1e10;
	}
	// 临时数组
	xk.resize(size);
	yk.resize(size);
	xprev.resize(size);
	grad.resize(size);
}

/**
 * @brief Destructor: release allocated memory.
 */
RBDLcpAPGD::~RBDLcpAPGD()
{
	delete[] A;
	delete[] b;
	delete[] myHi;
	delete[] myLo;
	delete[] addFriction;
	delete[] frictionIndices;
	delete x;
	delete w;
}

/**
 * @brief Set the lower bounds for the solution variables.
 * @param value Dynamic vector containing new lower bounds.
 */
void RBDLcpAPGD::setLowVector(const VSM::VectorNDynamic& value)
{
	if (myLo) delete[] myLo;
	unsigned n = size();
	myLo = new double[n];
	// 直接用 getData() 返回的指针，不要再写 .data()
	memcpy(myLo, value.getData(), n * sizeof(double));
}

/**
 * @brief Set the upper bounds for the solution variables.
 * @param value Dynamic vector containing new upper bounds.
 */
void RBDLcpAPGD::setHighVector(const VSM::VectorNDynamic& value)
{
	if (myHi) delete[] myHi;
	unsigned n = size();
	myHi = new double[n];
	memcpy(myHi, value.getData(), n * sizeof(double));
}

/**
 * @brief Define friction index blocks for tangential constraints.
 * @param value Integer template vector specifying block indices.
 */
void RBDLcpAPGD::setFrictionIndex(const VSM::VectorNDynamicTemplate<int>& value)
{
	// 用 size_t 接大小，避免 int(4 字节) 被当成元素个数
	size_t n = static_cast<size_t>(size());

	delete[] frictionIndices;
	frictionIndices = new int[n];

	for (size_t i = 0; i < n; ++i) {
		frictionIndices[i] = value[i];
	}
}

/**
 * @brief Set additional friction coefficients for each constraint.
 * @param value Dynamic vector of friction scaling factors.
 */
void RBDLcpAPGD::setAddFriction(const VSM::VectorNDynamic& value)
{
	// 先释放旧内存
	if (addFriction)
		delete[] addFriction;

	// 分配新数组
	unsigned n = size();
	addFriction = new double[n];

	// 逐元素拷贝，避免所有底层指针调用
	for (unsigned i = 0; i < n; ++i) {
		addFriction[i] = value[i];
	}
}


///// 估算 Lipschitz 常数 L = max_i ∑_j |A_{ij}|（这里简单取对角线最大值）
//void RBDLcpAPGD::computeLipschitz()
//{
//    L = 0;
//    for (int i = 0; i < size(); ++i)
//    {
//        double d = std::abs(A[i * myPadSize + i]);
//        if (d > L) L = d;
//    }
//    // 避免过大步长
//    if (L < 1e-8) L = 1e-8;
//}


/**
 * @brief Project vector entries within [myLo, myHi] bounds.
 * @param v Vector to be clamped in-place.
 */

//投影边界
void RBDLcpAPGD::projectBounds(std::vector<double>& v) const
{
	for (int i = 0; i < size(); ++i)
		v[i] = std::min(std::max(v[i], myLo[i]), myHi[i]);
}

/**
 * @brief Project tangential friction components onto the friction cone.
 * @param v Vector of current multipliers (normal and tangential) to project.
 */
void RBDLcpAPGD::projectFriction(std::vector<double>& v) const
{
	if (!frictionIndices) return;

	// 从 myNub 开始，每遇到一个 >=0 的 frictionIndices 就是 new block
	int blk = -1;
	for (int i = myNub; i < size(); ++i)
	{
		if (frictionIndices[i] != blk)
		{
			blk = frictionIndices[i];
			if (blk < 0) continue;
			// 正向分量
			double lambda_n = v[i];
			if (lambda_n <= 0.0)
				continue;  // 没有法向力支撑的接触点不应投影摩擦力

			// 收集同一 blk 的所有切向分量
			std::vector<int> T;
			int j = i + 1;
			while (j < size() && frictionIndices[j] == blk) { T.push_back(j); ++j; }
			// 计算切向合量
			double normT2 = 0;
			for (int id : T) normT2 += v[id] * v[id];

			double mu = myHi[i];                 // 基础摩擦系数
			if (addFriction) mu += addFriction[i];   // 可选：附加/各点不同 μ
			double bound = mu * lambda_n;        // 最终圆锥半径

			if (normT2 > bound * bound && normT2 > 0)
			{
				double s = bound / std::sqrt(normT2);
				for (int id : T) v[id] *= s;
			}
		}
	}
}


/**
 * @brief Solve the mixed LCP using the APGD method.
 * @return True if the solution converged within tolerance; false otherwise.
 */
bool RBDLcpAPGD::solve()
{
	static bool first = true;
	if (first) {
		first = false;
		QFile::remove("apgd_lambda.dat");
		QFile::remove("apgd_w.dat");
	}

	/*qDebug() << "rhs_ =" << QVector<double>::fromStdVector(rhs_);*/
	/*qDebug() << "initial grad =" << QVector<double>::fromStdVector(grad);*/

	const int n = size();


	// === 1) 初始化 & Warm-start ===
 /*   computeLipschitz();*/
	// … 之前的删掉 computeLipschitz() 那行 …

	if (!useWarmStart) {
		xprev.assign(n, 0.0);     // 冷启动：全零
	}

	// —— sanitize & 投影到可行域 ——
	// 先清掉 warm start 里潜在的 NaN/Inf，并按盒约束裁剪
//#pragma omp parallel for
	for (int i = 0; i < n; ++i) {
		if (!std::isfinite(xprev[i])) xprev[i] = 0.0;
		xprev[i] = std::min(std::max(xprev[i], myLo[i]), myHi[i]);
	}
	// 如果启用摩擦，进一次摩擦圆锥投影，避免一开始就不可行
	if (useFriction) {
		projectFriction(xprev);
	}

	yk = xprev;                    // y0 = γ0


	// 用 A 正确估计 Lk、tk（按 chrono 的初始化方式）
	// —— 在这里插入用 A 正确计算 L₀, t₀ ——  
	{
		std::vector<double> gamma0 = xprev;
		std::vector<double> gamma_hat0(n, 1.0);
		std::vector<double> diff(n), tmp(n);

		for (int i = 0; i < n; ++i)
			diff[i] = gamma0[i] - gamma_hat0[i];

		// tmp = A * diff
//#pragma omp parallel for
		for (int i = 0; i < n; ++i) {
			double s = 0;
			const double* row = &A[i * myPadSize];
			for (int j = 0; j < n; ++j)
				s += row[j] * diff[j];
			tmp[i] = s;
		}

		// 计算范数
		double norm_diff = 0, norm_tmp = 0;
		for (int i = 0; i < n; ++i) { norm_diff += diff[i] * diff[i]; norm_tmp += tmp[i] * tmp[i]; }
		norm_diff = std::sqrt(std::max(1e-16, norm_diff));
		norm_tmp = std::sqrt(norm_tmp);

		Lk = (norm_tmp > 0.0 ? norm_tmp / norm_diff : 1.0);
		if (Lk < 1e-8) Lk = 1e-8;
		tk = 1.0 / Lk;
		/*qDebug() << "[APGD] initial Lk =" << Lk << ", tk =" << tk;*/
	}


	// —— 新增: 初始化 Algorithm 1 中的 θ、Lk、tk ——
	theta = 1.0;                // θ₀ = 1
	grad.resize(n);
	std::vector<double> xnew(n);

	double last_r4 = std::numeric_limits<double>::infinity();
	// —— 新增: 记录历史最小残差与对应解 —— 
	double rmin = std::numeric_limits<double>::infinity();
	std::vector<double> gamma_hat = xprev;   // 先用当前初值占位


	// ========== 2) 迭代 ==========
	for (int k = 0; k < maxIters; ++k) {
		// 2.1) grad = A*yk - b
		for (int i = 0; i < n; ++i) {
			const double* row = &A[i * myPadSize];
			double s = 0.0;
			for (int j = 0; j < n; ++j) s += row[j] * yk[j];
			grad[i] = s - b[i];
		}

		/*qDebug() << "grad =" << QVector<double>::fromStdVector(grad);*/

		// 2.2) 回溯 line-search: 不断调整 Lk, tk
		double L_temp = Lk;
		double t = tk;
		const int BT_MAX = 20;   // 最多回溯 20 次
		int bt = 0;
		while (true) {
			// 试步: γ = proj_C(yk - t*grad)
			for (int i = 0; i < n; ++i)
				xnew[i] = yk[i] - t * grad[i];
			projectBounds(xnew);
			/*projectFriction(xnew);*/

			//// 打印 useFriction 的值
			//qDebug() << "useFriction =" << useFriction;

			if (useFriction) {
				projectFriction(xnew);
			}

			// —— 能量下降判断 —— 
			// Armijo 条件（f(x) = 0.5 x^T A x - b^T x）
			double f_y = 0.0, f_x = 0.0, dot = 0.0, sqnorm = 0.0;

			for (int i = 0; i < n; ++i) {
				// Ay
				const double* row = &A[i * myPadSize];
				double Ay_i = 0.0, Ax_i = 0.0;
				for (int j = 0; j < n; ++j) {
					Ay_i += row[j] * yk[j];
					Ax_i += row[j] * xnew[j];
				}
				f_y += 0.5 * yk[i] * Ay_i - yk[i] * b[i];
				f_x += 0.5 * xnew[i] * Ax_i - xnew[i] * b[i];
			}
			for (int i = 0; i < n; ++i) {
				double d = xnew[i] - yk[i];
				dot += grad[i] * d;
				sqnorm += d * d;
			}

			//// **在这里加打印**
			//qDebug()
			//    << "bt=" << bt       // 当前已回溯次数
			//    << " L_temp=" << L_temp   // 当前 L_k estimate
			//    << " t=" << t        // 当前步长 t = 1/L_temp
			//    << " f_y=" << f_y      // 能量在 yk 处：½·yᵀAy + yᵀb
			//    << " f_x=" << f_x      // 能量在 xnew 处：½·xnewᵀAxnew + xnewᵀb
			//    << " dot=" << dot      // ⟨∇f(yk), xnew−yk⟩
			//    << " ||dx||²=" << sqnorm;  // ‖xnew−yk‖²

			// Armijo 条件
			if (f_x <= f_y + dot + 0.5 * L_temp * sqnorm) {
				//qDebug() << "  --> Armijo satisfied";
				break;
			}
			// 否则 back-off
			L_temp *= 2;
			t = 1.0 / L_temp;

			if (++bt >= BT_MAX) {
				/*qDebug() << "APGD backtrack reached BT_MAX, revert to yk instead of force accept";*/
				// 1) 回退到上一次加速点
				xnew = yk;
				break;
			}
		}
		// 结束回溯

		// 2.3) 接受步长: γ_{k+1} = xnew
		// （在做 Nesterov 加速、更新 yk 之前做收敛判断）
		// === 使用 RES4 作为收敛判据 ===
		last_r4 = ResidualRes4(xnew);

		// 维护“历史最好”：更小就刷新 gamma_hat
		if (last_r4 < rmin) {
			rmin = last_r4;
			gamma_hat = xnew;
		}

		// 若达到容差，提前结束（并确保 best 已经是 xnew）
		if (last_r4 < tol) {
			xprev = xnew;   // 你若想保留“上一解”，也可以不写；下面会用 gamma_hat 输出
			break;
		}




		// 2.4) Nesterov 参数更新
		double theta_prev = theta;
		theta = 0.5 * (1.0 + std::sqrt(1.0 + 4.0 * theta_prev * theta_prev));
		double beta = (theta_prev - 1.0) / theta;  // β_{k+1}

		// 2.5) 加速点 y_{k+1}
		for (int i = 0; i < n; ++i) {
			yk[i] = xnew[i] + beta * (xnew[i] - xprev[i]);
		}

		// 2.6) 非单调保护：g^T (x_{k+1}-x_k) > 0 则重置
		double gdot = 0.0;
		for (int i = 0; i < n; ++i) gdot += grad[i] * (xnew[i] - xprev[i]);
		if (gdot > 0.0) {
			yk = xnew;
			theta = 1.0;
		}

		// 2.7) 更新 Lk, tk，并准备下一步
		Lk = 0.9 * L_temp;
		tk = 1.0 / Lk;
		xprev = xnew;

		//// 原有: 每隔30步打印调试信息
		//if (k % 100 == 0) {
		//    qDebug() << "iter" << k
		//        << " LA[0]=" << xprev[0]
		//        << " err=" << r;
		//}
	}

	// ========== 3) 输出解 ==========
	// 选用历史最好解作为最终解；若极端情况下一次都没更新，就回退 xprev
	const bool have_best = (rmin < std::numeric_limits<double>::infinity());
	const std::vector<double>& sol = have_best ? gamma_hat : xprev;

	// 同步到成员以便 DumpLambda() 导出一致的最终解
	xprev = sol;

	for (int i = 0; i < n; ++i)
		(*x)[i] = sol[i];


	// w = A*x - b

	for (int i = 0; i < n; ++i) {
		double sum = 0;
		const double* row = &A[i * myPadSize];
		for (int j = 0; j < n; ++j)
			sum += row[j] * (*x)[j];
		(*w)[i] = sum - b[i];
	}

	//// —— 在这里新增“写文件”——

	//// 0) 打印当前工作目录，方便你去找文件
	//qDebug() << "[RBDLcpAPGD] Current working dir:" << QDir::currentPath();

	//static int step_id = 0;
	//// 1) Dump APGD λ (x) 到 apgd_lambda.dat
	//{
	//	QFile f("apgd_lambda.dat");
	//	if (!f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
	//		qWarning() << "[RBDLcpAPGD] ERROR: cannot open apgd_lambda.dat for writing";
	//	}
	//	else {
	//		QTextStream out(&f);
	//		out << "# time_step: " << step_id << "\n";
	//		out.setRealNumberPrecision(6);
	//		out.setRealNumberNotation(QTextStream::FixedNotation);
	//		for (int i = 0; i < n; ++i) {
	//			out << (*x)[i] << "\n";
	//		}
	//		f.close();
	//		qDebug() << "[RBDLcpAPGD] Wrote" << n << "values to apgd_lambda.dat";
	//	}
	//}

	//// 2) （可选）Dump APGD w 到 apgd_w.dat
	//{
	//	QFile f("apgd_w.dat");
	//	if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
	//		QTextStream out(&f);
	//		out << "# time_step: " << step_id << "\n";
	//		out.setRealNumberPrecision(6);
	//		out.setRealNumberNotation(QTextStream::FixedNotation);
	//		for (int i = 0; i < n; ++i) {
	//			out << (*w)[i] << "\n";
	//		}
	//		f.close();
	//		qDebug() << "[RBDLcpAPGD] Wrote" << n << "values to apgd_w.dat";
	//	}
	//}
	//step_id++;  // 准备下一个时间步

	//// 3) （可选）在控制台再 qDebug 一次
	//{
	//	QString xs; xs.reserve(n * 16);
	//	for (int i = 0; i < n; ++i)
	//		xs += QString(" %1").arg((*x)[i], 0, 'f', 6);
	//	qDebug() << "[RBDLcpAPGD] Final x:" << xs;

	//	QString ws; ws.reserve(n * 16);
	//	for (int i = 0; i < n; ++i)
	//		ws += QString(" %1").arg((*w)[i], 0, 'f', 6);
	//	qDebug() << "[RBDLcpAPGD] Final w:" << ws;
	//}

	//// —— 写文件结束 ——  

	return (rmin < tol);

}


void RBDLcpAPGD::EnableDump(bool on) { dump_enabled = on; }

void RBDLcpAPGD::DumpRhs(std::vector<double>& out, bool physical) const {
	const int n = size();
	out.resize(n);
	if (!b) { std::fill(out.begin(), out.end(), 0.0); return; }
	if (!physical) {
		// 内部使用的 b（与你的 grad = A*y - b / w = A*x - b 一致）
		for (int i = 0; i < n; ++i) out[i] = b[i];
	}
	else {
		// 物理意义上的 RHS（外层 setValuesInRightSide(-b) 前的号）
		for (int i = 0; i < n; ++i) out[i] = -b[i];
	}
}

void RBDLcpAPGD::DumpLambda(std::vector<double>& out) const {
	// 你在 solve() 结束前把 gamma_hat 写回 xprev，然后 (*x)[i] = xprev[i]
	// 这里直接导出 xprev（或 *x），两者一致
	out = xprev;  // O(n) 拷贝；不用 push_back，避免累积
}
