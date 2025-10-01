#include "./RBDClusterLagrangeMultipliers.h"

#include <iostream>

//头文件排序略有点不合理


// QT thread library resource allocation
// Qt 线程同步原语
#include <QWaitCondition>
#include <QSemaphore>
#include <QMutex>

// 求解器
#include "./RBDLcpGS.h"
#include "./RBDLcpDantzig.h"
#include "./RBDLcpDantzigInternal.h"
#include "./RBDLcpSor.h"
#include "../RBDSolverAPGD.h"

// 逆动力学
#include "../RBDInverseDynamicsFormulation.h"
#include "../RBDInverseDynamicsSolverLCP.h"

// 场景/材料/关节（约束来源 & 全局配置）
#include "../RBDMaterial.h"
#include "../RBDScene.h"
#include "../RBDJoint.h"

// 向量/矩阵与通用数学
#include "../../../Lib/VSM/VSMVectorNDynamic.h"

//我个人觉得没用到，属于冗余包含，调试/观测（他有观察版本的代码，在另一个文件里）
#include "../RBDClusterLagrangeDebugObserver.h"

// 通用数值与线代“杂物包”，跨模块都会用到的数学常量与小工具函数
#include "../../VSLibRBDynMath/VSLibRBDynMathCommon.h"

//通用 N×M 矩阵类（稠密）
#include "Lib/VSM/VSMMatrixNxM.h"


// 这两个是编译期开关（宏），用来切换功能分支/是否插入性能计时。现在被注释掉=未启用。
//#define PERFORMANCESUITE_ENABLED
//#define USE_QP_IMPLEMENTATION

// 项目内的性能计时/指标上报套件
#include "../../VSLibOpenCV/VSLibOpenCVPerformanceSuite.h"

// Qt 的文件 I/O（跨平台封装）
#include <QFile>
#include <QtConcurrent>   //“简单并行”工具 没用到
#include "omp.h"   // OpenMP 多线程并行编程支持 没用到
#include <chrono>   // C++11 标准库的时间工具

// ===== Contact logging （这里是我后加的aicoding，很多头文件重复，nocare） =====
#include <QDir>
#include <QTextStream>
#include <QMutex>
#include <QMutexLocker>
#include <QFileInfo>


static QFile        g_cpFile;   // 每个接触点坐标
static QFile        g_ccFile;   // 每步接触点数量
static QTextStream  g_cpOut;
static QTextStream  g_ccOut;
static QMutex       g_logMutex;
static bool         g_logsInit = false;

static void ensureLogsOpen() {
	if (g_logsInit) return;
	QMutexLocker lock(&g_logMutex);
	if (g_logsInit) return;

	QDir().mkpath("logs");
	g_cpFile.setFileName("logs/contact_points.dat");
	g_ccFile.setFileName("logs/contact_count.dat");

	if (!g_cpFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
		qWarning() << "[contact] open failed:" << g_cpFile.errorString();
	if (!g_ccFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
		qWarning() << "[count] open failed:" << g_ccFile.errorString();

	g_cpOut.setDevice(&g_cpFile);
	g_ccOut.setDevice(&g_ccFile);

	g_cpOut.setRealNumberNotation(QTextStream::FixedNotation);
	g_ccOut.setRealNumberNotation(QTextStream::FixedNotation);
	g_cpOut.setRealNumberPrecision(8);
	g_ccOut.setRealNumberPrecision(8);

	// 表头（可选）
	g_cpOut << "# time  contact_ptr  px  py  pz  [fn]\n";
	g_ccOut << "# time  count\n";
	g_cpOut.flush();
	g_ccOut.flush();

	g_logsInit = true;
}

// end ===== Contact logging =====

using namespace VSLibRBDynamX;

// 每一帧记录一次当前所有约束解算结果（如残差/λ值等）
static QVector<ConstraintsDebug> history;
static ConstraintsDebug actualStep;   //当前帧的约束/求解快照


//实例计数器（instance counter）记录当前类 RBDClusterSequentialImpulse 被创建了多少个实例，检查资源泄漏、对象重复创建
#ifdef RBD_DEBUG
QAtomicInt RBDClusterLagrangeMultipliers::insts = 0;
#endif


// 调试导出函数（目前被注释掉）。
// 作用很单纯：把你在内存里累积的 history（一帧一份的 ConstraintsDebug 快照）导出成一个 MATLAB 脚本 D:/LagrangeMultipliers.m
//	调试信息
// DELETEME if it works....
//static void printHistory() {
//   QFile file("D:/LagrangeMultipliers.m");
//   file.open(QIODevice::WriteOnly);
//
//   file.write(QString("function S = dynamxData()\n\n"));
//
//   int i = 0;
//   for(ConstraintsDebug d: history) {
//      d.print(file, i++);
//   }
//
//   file.write(QString("end\n\n"));
//   file.close();
//}

// Transform the system jacobian matrix from VSLibRBDynMath::RBMJacobeanMatrix to VSM::MatrixNxM
// 把稀疏的雅可比矩阵 J 转换成稠密矩阵，把所有块放在一个大矩阵A里,JMJT里要用到
static VSM::MatrixNxM calcMatrix(const VSLibRBDynMath::RBMJacobeanMatrix& J, int sizeOfProblem)
{
	int targetRow = 0;
	VSM::MatrixNxM matrix(J.getRowCount(), sizeOfProblem);
	for (VSLibRBDynMath::RBMJacobeanMatrix::JacobeanRow* jRow : J.getRows())
	{
		for (VSLibRBDynMath::RBMJacobeanMatrix::JacobeanEntry* jEntry : *jRow) // 98% der Fälle 2 Stück!
		{
			for (int i = 0; i < 6; i++)
				matrix.setElement(targetRow, jEntry->column + i, jEntry->values[i]);
		}
		++targetRow;
	}
	return matrix;
}
//
//static QString printMatrix(VSM::MatrixNxM m) {
//   QString s("[");
//   for (unsigned int c = 0; c < m.columns(); c++) {
//      for (unsigned int r = 0; r < m.rows(); r++) {
//         s += QString::number(m.element(r,c));
//         if (r < m.rows() - 1)
//            s += " ";
//      }
//      if (c < m.columns()- 1)
//         s += ";";
//   }
//   s += "]";
//   return s;
//}
//
//template <class T> static QString printMatrix(T v) {
//   QString s("[");
//   for (unsigned int c = 0; c < v.size(); c++) {
//      s += QString::number(v[c]);
//      if (c < v.size()- 1)
//         s += ";";
//   }
//   s += "]";
//   return s;
//}

// (ci) LM mit Debuggingmöglichkeit
//

// 构造函数  带“调试观察者”指针
RBDClusterLagrangeMultipliers::RBDClusterLagrangeMultipliers(RBDScene* scene, VSLibRBDynamX::RBDClusterLagrangeDebugObserver* LagrangeDebugObserver)
	: RBDCluster(scene)
	//, M_Inv(0)
	, myScene(scene)
	, f_ext(0)
	, myLcp(0)
	, myLagrangeDebugObserver(LagrangeDebugObserver)
	//, mySizeOfJacobean(0)
{
#ifdef RBD_DEBUG
	insts.ref();
#endif
}

// // 构造函数  不带“调试观察者”指针
RBDClusterLagrangeMultipliers::RBDClusterLagrangeMultipliers(RBDScene* scene)
	// : RBDCluster(scene)   // ① 先调用“基类”构造，把场景交给基类层
	//, M_Inv(0)
	, myScene(scene)   // 把基类 RBDCluster 用当前的 scene 初始化；  ② 初始化成员：保存场景指针
	, f_ext(0)     // 把本类的关键指针成员设成空（延迟到后续 doTimeStep() 再分配/使用）；
	, myLcp(0)
	//, mySizeOfJacobean(0)
{
#ifdef RBD_DEBUG
	insts.ref();   //在开启 RBD_DEBUG 时，用一个静态的原子计数器统计实例数，便于内存/对象泄漏排查。
#endif
}

// 析构函数 用来统计当前活跃的 RBDClusterLagrangeMultipliers 实例数量，析构后减1
RBDClusterLagrangeMultipliers::~RBDClusterLagrangeMultipliers()
{
#ifdef RBD_DEBUG
	insts.deref();
#endif
}

// 更新刚体的速度
void updateRigidBodyTwist(RBDRigidBody* rb, VSM::VectorN v_new)
{
	VSM::Vector6 twist;
	for (int i = 0; i < 6; ++i)
		twist[i] = v_new[rb->matrixOffset() + i];

	rb->setTwist(twist);
}

// “做一帧时间推进”
// 本帧推进后的绝对时间戳 newTime
// 本帧时间步长 delta_t > 0
void RBDClusterLagrangeMultipliers::doTimeStep(
	double newTime,
	double delta_t)
{
	// 这是我当时专门为了测试UseBilateralFrictionCone 是否生效而加的调试输出
	// 主要是为了看他后面写的关于摩擦的语句是否生效了
	//// UseBilateralFrictionCone 是否生效
	//const bool useCone = getScene()->getUseBilateralFrictionCone();
	//qDebug() << "[Scene] UseBilateralFrictionCone =" << useCone;


	// 应该是前朝遗物，不知道留这个开关是做什么的，猜测是有关“旧版实现”的功能分支
	bool useOldImplementation = false;
#ifndef USE_QP_IMPLEMENTATION   // 这个宏没被定义 ，就走LCP

	Q_UNUSED(newTime);  // Qt 的小宏

	// delta_t 必须大于0，否则不推进
	if (delta_t <= 0)		
		return;

	// 观察者模式的记录但是，至于是否存在线程的安全问题，不清楚
	if (myLagrangeDebugObserver)
	{
		actualStep.newTime = newTime;
	}

	// 性能采样/打点的计时宏，开一个秒表
	PERFORMANCESUITE_TIC("810 RBDCluster, Collect constraints");

	int numRBodies = 0;   //用来统计当前 cluster 里有多少个刚体，并作为编号的起点。

	// “从当前 cluster 的刚体集合里，拿一个只读的起始迭代器 it”
	RBDRigidBodyPtrSet::const_iterator it = getRigidBodies().begin();

	// 打印刚体信息的调试代码，被注释掉了 好像整个函数都没用到
	//#ifdef RBD_DEBUG
	//  qDebug() << "Cluster: " << this << " Größe: " << getRigidBodies().size();
	//  for (; it != getRigidBodies().end(); ++it)
	//  {
	  //	RBDRigidBody* rBody = *it;

	//     qDebug() << rBody->getName() << ": " << rBody->getAbsPose().getPosition().toString();

	//     QString bName = rBody->getName();
	//  }
	//  it = getRigidBodies().begin();
	//#endif


	// cluster 内的局部索引
	for (; it != getRigidBodies().end(); ++it)
	{
		RBDRigidBody* rBody = *it;

		rBody->setClusterIndex(numRBodies);
		++numRBodies;
	}

	// 整团刚体的全局状态向量”预分配内存
	// The size of the state vector
	int sizeOfProblem = 6 * numRBodies;

	// Allokation des Gesamtspeedvektors
	VSM::VectorN v_old(sizeOfProblem); // 旧速度
	VSM::VectorN pos_old(sizeOfProblem); // 旧位置
	VSM::VectorN mass(sizeOfProblem);	// 质量

	//int numberFoundContacts = 0;

	// 在当前 Cluster 里收集所有“属于本集群”的约束资源

	//本帧临时表：记录本集群里已处理过的接触指针。
	std::list<RBDContact*> allHandledContactsInCluster;

	
	// Store all the constraints of each rigid body in a list myConstraintResources.
	for (int i = 0; i < getRigidBodies().size(); ++i)
	{
		// 遍历本集群里的刚体集合
		RBDRigidBody* rBody = getRigidBodies()[i];   //for(RBDRigidBody* rBody: getRigidBodies())

		//遍历该刚体挂接的“约束资源”（关节、接触、限位等）
		const RBDConstraintResourcePtrSet& bodiesConstrRes = rBody->getConstraintResources();

		for (int i = 0; i < bodiesConstrRes.size(); ++i)   //for(RBDConstraintResource* joint: )
		{ // In Differenzialen wird derzeit nur der 0te Body gesetzt - body (1) wird immer 0 returnieren.
			RBDConstraintResource* res = bodiesConstrRes[i];

			// 跳过禁用的约束
			if (res->getDisabled())
				continue;
				
			// 跳过没有挂接刚体0的约束
			if (!res->body(0))
				continue;

			// 只有当约束两端的“非固定刚体”都属于当前这个 Cluster 时，才纳入本集群的装配。
			// Check whether body 0 and body 1 of the constraints are both in this cluster
			if (!res->body(0, true)->isFix() && res->body(0, true)->getCluster() != this)
				continue;

			if (res->body(1, true) && !res->body(1, true)->isFix() && res->body(1, true)->getCluster() != this)
				continue;

			// 把这个约束资源加入本集群的总表。
			myConstraintResources.add(res);

			// 如果这个约束资源是一个接触，就要做额外的处理, 防止被重复添加
			RBDContact* contact = res->dynamicCast<RBDContact*>();
			if (contact)
			{
				if (contact->getHandled())
					continue;

				//++numberFoundContacts;
				contact->setHandled(true);

				allHandledContactsInCluster.push_back(contact);  // 这个就用到了 306行定义的本帧临时表
			}
		}
	}

	// 记录雅可比矩阵 𝐽的“行数”（也就是约束变量的总数 = 等式约束数 + 互补/不等式约束数）
	// 可能在之前老的实现中涉及了用这个变量分配内存，但现在已经不需要了。
	//   mySizeOfJacobean = numberEqualityConstraints + numberComplementaryConstraints;

	// 这里我存在一定的疑惑 ，下一个时间帧不一定是接触的了吧？
	// 这里是对于之前的接触登记表进行复位的，防止影响下一帧的计算，至于为什么去掉，可能在别的地方被包含了，我没有找到？？
	   //// Reset handled-flag in contacts
	   //for (std::list<RBDContact*>::const_iterator it = allHandledContactsInCluster.begin(); it != allHandledContactsInCluster.end(); ++it)
	   //{
	   //   (*it)->setHandled(false);
	   //}

	PERFORMANCESUITE_TOC("810 RBDCluster, Collect constraints");
	// 给性能打点分段计时
	PERFORMANCESUITE_TIC("820 RBDCluster, Build J"); // 820 构建雅可比 J”的总耗时
	PERFORMANCESUITE_TIC("821 J, add eq. constr.");  // 821 向 J 添加等式约束”的耗时

	/* Now start to calculate J,lambdaLow,lambdaHigh,addFriction,constraintsRightSide
	   -lambdaLow,lambdaHigh: limits of lambda
	   -constraintsRightSide: baumgarte term
	   -First calculate with equality constraints, then with complementarity constraints
	*/

	// 进入装配阶段前，把“约束求解需要的所有容器与计数器”初始化好
	VSLibRBDynMath::RBMJacobeanMatrix J;  // 雅可比矩阵 J
	VSM::VectorNDynamic constraintsRightSide(0.0);  // 约束右端项（等式约束的 Baumgarte 项）
	VSM::VectorNDynamic lambdaLow(-VSM::maxDouble);  // λ 的下界
	VSM::VectorNDynamic lambdaHigh(VSM::maxDouble);	// λ 的上界
	VSM::VectorNDynamic addFriction(0.0);  // 摩擦相关的附加项占位
	VSM::VectorNDynamicTemplate<int> frictionNormalIndices(-1); // 摩擦相关的法向量索引	
	VSM::VectorN v_new(sizeOfProblem); // 新速度

	int numberEqualityConstraints = 0;  // 统计等式约束的行数（关节约束、锁定等）
	int numberComplementaryConstraints = 0; // 统计互补约束的行数（接触、摩擦等）
	int numberFrictionConstraints = 0;   // 统计摩擦的“附加约束”产生的额外行数。
	
	// 用于误差统计
	int numberEqConstraintsForPoseCorrection = 0; // velocity based motor with unlimited force is not considered.

	int currentRow = 0;  // 当前正在装配的约束行号，J 的每一行对应一条约束
	constraintError = 0; // 本帧约束误差累加器，用于计算均方根误差

	// First run the joints and add the equality constraints to the system
	// 首先处理关节等约束，把等式约束装配进系统
	for (RBDConstraintResource* constraintRes : myConstraintResources)
	{
		// 1) 告诉这个约束：你的等式约束从全局J的哪一行开始
		constraintRes->setEqualityConstraintsOffset(currentRow);

		// 2) 让它把自己的等式约束写进系统
		int number = constraintRes->addEqualityConstraintsToSystem(
			J,
			constraintsRightSide,
			delta_t,
			currentRow);

		// 3) 写完后推进“行光标”和计数器	
		currentRow += number;  // // 下一条约束该从哪一行写
		numberEqualityConstraints += number;  // 统计等式约束的总行数
		// 位姿修正（Pose Correction） 做误差的均方根（RMSE）归一化
		numberEqConstraintsForPoseCorrection += constraintRes->getNumberEqConstraintsForPoseCorrection();

		// 4) 误差统计（平方和，稍后会做均方根）
		// calculate the square sum of constraint error for this cluster at this time point
		constraintError += constraintRes->getConstraintError();
	}
	// calculate the Mean Squared Error for this cluster at this time point
	// 统计本帧集群的等式约束误差（做位姿纠正的那部分）RMSE（均方根误差）
	if (numberEqConstraintsForPoseCorrection > 0)
		constraintError = VSM::sqrt(constraintError / numberEqConstraintsForPoseCorrection);
	else
		constraintError = 0;

	//qDebug() << "The constraintError of this cluster is" << constraintError << "The total number of equality constraints are" << numberEqConstraintsForPoseCorrection;
	// 添加等式约束的子阶段（在这段结束时 TOC）
	PERFORMANCESUITE_TOC("821 J, add eq. constr.");
	// 添加互补（不等式）约束的子阶段, 计时开始
	PERFORMANCESUITE_TIC("822 J, add compl. constr.");

	// Update frictionNormalIndices，lambdaLow，lambdaHigh，numberFrictionConstraints，addFriction with complementarity constraints
	// 把“非接触类”的互补约束统统装进线性系统
	// 含雅可比行、右端项、拉格朗日变量的上下界、以及与摩擦相关的索引映射
	for (RBDConstraintResource* constraintRes : myConstraintResources)
	{
		// 单独分阶段处理，避免和一般“非接触”的单边约束（比如关节限位、单向止挡、单边电机等）混在一起。
		if (constraintRes->dynamicCast<RBDContact*>())
			continue;

		// 当前全局“约束行号”的起始位置记到此资源里，方便后续反查某个资源的变量/残差在全局向量里的位置。
		constraintRes->setComplementaryConstraintsOffset(currentRow);

		// 把“非接触互补约束”写进系统
		int number = constraintRes->addComplementaryConstraintsToSystem(
			J,
			lambdaLow,
			lambdaHigh,
			constraintsRightSide,
			frictionNormalIndices,
			delta_t,
			currentRow);

		numberComplementaryConstraints += number;
		currentRow += number;
	}
	// 非接触互补约束 完成
	PERFORMANCESUITE_TOC("822 J, add compl. constr.");
	// 处理接触类的互补约束，计时开始
	PERFORMANCESUITE_TIC("823 J, add compl. constr. for contacts");

	// 遍历当前簇里“已确认属于本簇”的接触点
	// allHandledContactsInCluster 前面收集过并打了 handled 标记，确保每个接触只加一次。
	// 给这个接触的互补约束记录一个起始行偏移，便于后续反查。
	for (std::list<RBDContact*>::const_iterator it = allHandledContactsInCluster.begin(); it != allHandledContactsInCluster.end(); ++it)
	{
		RBDContact* contact = (*it);
		contact->setComplementaryConstraintsOffset(currentRow);

		int number = contact->addComplementaryConstraintsToSystem(
			J,
			lambdaLow,
			lambdaHigh,
			constraintsRightSide,
			frictionNormalIndices,
			delta_t,
			currentRow,
			addFriction);

		currentRow += number;
		if (number > 1)
			numberFrictionConstraints += (number - 1);

		numberComplementaryConstraints += number;
	}

	// Transform the system jacobian matrix from VSLibRBDynMath::RBMJacobeanMatrix to VSM::MatrixNxM
	// 把稀疏的雅可比矩阵 J 转换成稠密矩阵，把所有块放在一个大矩阵A里,JMJT里要用到
	// svd 分解
	VSM::MatrixNxM jMatrix(calcMatrix(J, sizeOfProblem));

	// SVD 奇异性探测（默认关闭） 对每帧的大矩阵做全 SVD 会拖慢仿真。
	bool acitivateSingularityDetector = false;
	// Singularity detector
	if (acitivateSingularityDetector)
	{
		VSM::MatrixNxM U;
		VSM::VectorN w;
		VSM::MatrixNxM V;

		VSM::MatrixNxM::compileSVDecomposition(jMatrix, U, w, V);

		double minSingularValue = w.getElement(w.size() - 1);
		qDebug() << "minSingularValue" << minSingularValue;
		
		//calc condition number
		// 2-范数条件数。条件数越大，矩阵（或约束系统）越病态
		CND_Nr = w.getElement(0) / w.getElement(w.size() - 1);
		qDebug() << "CND_Nr" << CND_Nr;
	}


	// we have a constrained system --> build the LCP system:
	// A*x = b + w, x[i]*w[i] >= lo[i], x[i]*w[i] < hi[i], hi, lo

	// A = J * M^-1 * J^T   系统矩阵
	// b = -J * ( dt * M^-1 * f_ext + v_jetzt ) + constraintsRightSide + b_speed 右端项
	// x = lambda * dt  未知量的物理意义

	// 带上下界的互补条件（混合 LCP）
	//qDebug() << "RBDClusterLagrangeMultipliers::doTimeStep J:" << J.toString();

	// 结束“接触互补约束（法向 + 摩擦）”阶段
	PERFORMANCESUITE_TOC("823 J, add compl. constr. for contacts");
	// 结束“整个 J 的构建阶段”
	PERFORMANCESUITE_TOC("820 RBDCluster, Build J");

	if (J.getRowCount() > 0)
	{
		PERFORMANCESUITE_TIC("830 RBDCluster, before multiplications");

		//qDebug() << J.toString(true);

		VSM::MatrixNxM MInv_JT(sizeOfProblem, J.getRowCount()); // M^-1 * J^T
		VSM::MatrixNxM matA(J.getRowCount(), MInv_JT.columns(), false); // J * M^-1 * J^T
		VSM::VectorN MInv_dt_fext(sizeOfProblem); // dt * M^-1 * f_ext

		it = getRigidBodies().begin();
		for (; it != getRigidBodies().end(); ++it)
		{
			RBDRigidBody* rBody = *it;

#ifdef RBD_DEBUG
			if (fabs(rBody->getMass()) < VSM::epsilon)  // 认为质量数值上接近 0。
			{
				qDebug() << "RBDClusterLagrangeMultipliers: Body " << rBody->getName() << " has mass close to zero!";
			}
#endif
			v_old.setElements(rBody->matrixOffset(), rBody->getTwist().getV());  // 3 线速度
			v_old.setElements(rBody->matrixOffset() + 3, rBody->getTwist().getW()); // 3 角速度
			
			// 调试/观测器使用的“旁路数据”：一帧里每个刚体的位置 + 旋转向量 + 质量 + 惯量对角
			if (myLagrangeDebugObserver)
			{
				pos_old.setElements(rBody->matrixOffset(), rBody->getAbsPose().getPosition());
				pos_old.setElements(rBody->matrixOffset() + 3, rBody->getAbsPose().getOrientation().getRVAAngle() * rBody->getAbsPose().getOrientation().getRVAVector());

				mass.setElements(rBody->matrixOffset(), VSM::Vector3(rBody->getMass(), rBody->getMass(), rBody->getMass()));
				mass.setElements(rBody->matrixOffset() + 3, VSM::Vector3(
					rBody->getInertia().getElement(0, 0),
					rBody->getInertia().getElement(1, 1),
					rBody->getInertia().getElement(2, 2)));
			}
		}

		// Calculate the vector of extern forces
		// 构造外力/外矩广义向量
		// 每个刚体占 6 个槽位：前三个是力 𝐹，后三个是力矩 𝑇
		f_ext = new VSM::VectorN(sizeOfProblem); // 6n 维外力向量
		if (!getScene()->getIgnoreGyroTerm()) // 是否忽略陀螺力矩项
		{	
			// 情况A：启用陀螺项
			for (int i = 0; i < getRigidBodies().size(); ++i)
			{
				RBDRigidBody* rBody = getRigidBodies()[i];
				//for(RBDRigidBody* rBody: getRigidBodies())

				// 线部分：外力
				f_ext->setElements(rBody->matrixOffset(), rBody->getExternalForce());
				// 角部分：外矩 - 角速度 × (世界惯量 * 角速度)
				f_ext->setElements(rBody->matrixOffset() + 3, rBody->getExternalTorque() - cross(rBody->getTwist().getW(), (rBody->getInertiaW() * rBody->getTwist().getW())));

				// “消费”后清零
				// reset the extern forces and torques on rigid bodies to zero
				rBody->resetForceAndTorque();
			}
		}
		else
		{
			// 情况B：忽略陀螺项（更简单）
			for (int i = 0; i < getRigidBodies().size(); ++i)
			{
				RBDRigidBody* rBody = getRigidBodies()[i];
				//for(RBDRigidBody* rBody: getRigidBodies())

				f_ext->setElements(rBody->matrixOffset(), rBody->getExternalForce());
				f_ext->setElements(rBody->matrixOffset() + 3, rBody->getExternalTorque());//- rBody->getTwist().getW() % ( rBody->getInertiaW() * rBody->getTwist().getW() ) );

				// reset the extern forces and torques on rigid bodies to zero
				rBody->resetForceAndTorque();
			}
		}

		// “做大矩阵乘法之前”的准备阶段 结束
		PERFORMANCESUITE_TOC("830 RBDCluster, before multiplications");
		// 计算 M^-1 * J^T 的阶段 开始
		PERFORMANCESUITE_TIC("840 RBDCluster, M_Inv*JT");

		// 计算 M^-1 * J^T
		// 这个函数里会自己构造 M^-1
		this->multiplyMinvImplJT(MInv_JT, getRigidBodies(), J);

		// 结束 M^-1 * J^T 阶段
		PERFORMANCESUITE_TOC("840 RBDCluster, M_Inv*JT");

		// 把当前约束行数 记录到性能日志里，方便后面把耗时与规模关联起来。
		PERFORMANCESUITE_VALUE("J.getRowCount()", J.getRowCount());
		
		// 计算 J * M^-1 * J^T 的阶段 开始
		PERFORMANCESUITE_TIC("850 RBDCluster, J * M_InvJT (sym.)");

		//		VSLibRBDynMath::RBMMatrix matA = J * MInv_JT;
		// 对称优化的乘法
		J.multiplySymmetricFast(matA, MInv_JT);

		// 完成了 𝐴
		PERFORMANCESUITE_TOC("850 RBDCluster, J * M_InvJT (sym.)");

		// 开始下一阶段——给每条关节/约束添加 CFM（Constraint Force Mixing）
		PERFORMANCESUITE_TIC("860 RBDCluster, add Joints CFM");

		// Add constraintForceMixing to system matrix JMJT
		// 每条约束自己的 CFM（Constraint Force Mixing，约束柔度/对角正则）加进系统矩阵 A
		for (RBDConstraintResource* constraintRes : myConstraintResources)
		{
			if (constraintRes->getDisabled())
				continue;

			constraintRes->addConstraintForceMixing(&matA, constraintsRightSide, delta_t);
		}

		//  不用启动addAuxiliaryConstraintsForFriction，只用求解器的内部处理
		if (!getScene()->getUseBilateralFrictionCone())
			matA = addAuxiliaryConstraintsForFriction(matA, lambdaLow, lambdaHigh, frictionNormalIndices, addFriction);

		PERFORMANCESUITE_TOC("860 RBDCluster, add Joints CFM");
		// 在调用求解器之前的其余步骤 开始计时
		PERFORMANCESUITE_TIC("870 RBDCluster, Rest bis vor solve");

		// Add global constraint force mixing to increase systems stability at least for ODE Solver
		// 全局 CFM，增加系统稳定性
		if (myScene->getGlobalCfm() > .0)
		{
			VSLibRBDynMath::addToMatrixNxMDiag(&matA, J.getRowCount(), myScene->getGlobalCfm());
			//qDebug() << QString("Global CFM: ") << myScene->getGlobalCfm();
		}

		// Choose lcp solver
		// TBD: SolverFactory bauen
		// --> myLcp = SolverFactory->getSolver(myScene->constraintSolverType())
		// 选择 LCP 求解器
		switch (myScene->constraintSolverType())
		{
		case(RBDScene::CST_LAGRANGEDANTZIG):
		{
			myLcp = new RBDLcpDantzig(matA.rows(), numberEqualityConstraints);
			break;
		}
		case(RBDScene::CST_LAGRANGEDANTZIGEXP):
		{
			myLcp = new RBDLcpDantzigExperimental(matA.rows(), numberEqualityConstraints, myScene->numberIterations());
			break;
		}
		case (RBDScene::CST_LAGRANGEGS):
		{
			myLcp = new RBDLcpGS(matA.rows(), numberEqualityConstraints, myScene->numberIterations());
			break;
		}
		case RBDScene::CST_LAGRANGEAPGD:
		{
			// 1) 用派生类指针接收 new 出来的对象
			RBDLcpAPGD* apgd = new RBDLcpAPGD(
				matA.rows(),                 // LCP 维度
				numberEqualityConstraints,   // nub
				myScene->numberIterations(), // maxIters
				1e-5,                        // tol
				0.9                          // accel
			);
			// 2) 在派生类指针上启用 warm-start
			apgd->EnableWarmStart(true);
			/*apgd->EnableWarmStart(false);*/
			//摩擦
			apgd->EnableFriction(true);    // <— 这一行，关闭摩擦
			// 3) 再把它赋给基类指针
			myLcp = apgd;
			break;
		}
		default:
		{
			// 没有等式约束 → 用 Gauss–Seidel（RBDLcpGS）
			if (numberEqualityConstraints <= 0)
				myLcp = new RBDLcpGS(matA.rows(), numberEqualityConstraints, myScene->numberIterations());
			// 有等式约束 → 用 Dantzig（RBDLcpDantzig）
			else
				myLcp = new RBDLcpDantzig(matA.rows(), numberEqualityConstraints);
		}
		}

		// Set the JMJT matrix in lcp solver
		// 把系数矩阵塞进求解器
		myLcp->setValuesInMatrix(matA);

		// 若启用调试观察者，记录本帧快照
		if (myLagrangeDebugObserver)
		{
			actualStep.J = jMatrix;
			actualStep.matA = matA;
			actualStep.v_old = v_old;
			actualStep.pos_old = pos_old;
			actualStep.m = mass;
			actualStep.lambdaLow = lambdaLow;
			actualStep.lambdaHigh = lambdaHigh;
			actualStep.constraintsRightSide = constraintsRightSide;
		}

		// 给上下界向量兜底（危险写法，建议修正）不太严谨，建议改成 resize() + fill()
		if (lambdaLow.size() < matA.rows())
			lambdaLow[matA.rows() - 1] = -VSM::maxDouble;

		if (lambdaHigh.size() < matA.rows())
			lambdaHigh[matA.rows() - 1] = VSM::maxDouble;



		// （A）调试打印摩擦相关索引/参数，（B）只针对 APGD 的“无界约束夹紧补丁”。关于APGD调试的，可以不用理会
		//// —— 上面已经填充好了 frictionNormalIndices 和 addFriction ——

		//// ===== 在这里插入调试打印 =====
		//qDebug() << ">>> frictionNormalIndices:";
		//for (int i = 0; i < frictionNormalIndices.size(); ++i)
		//	qDebug() << i << ":" << frictionNormalIndices[i];

		//qDebug() << ">>> addFriction:";
		//for (int i = 0; i < addFriction.size(); ++i)
		//	qDebug() << i << ":" << addFriction[i];
		//// ===== 调试打印结束 =====


		//// --- Only patch for APGD: Fix unbounded constraints (e.g., motors) ---
		//if (dynamic_cast<RBDLcpAPGD*>(myLcp)) {
		//	const double kMotorClamp = 1e6;
		//	for (int i = 0; i < lambdaLow.size(); ++i) {
		//		// 检查是否是“无边界”约束（motor、LOCK类型常见）
		//		if (lambdaLow[i] < -1e308 && lambdaHigh[i] > 1e308) {
		//			lambdaLow[i] = -kMotorClamp;
		//			lambdaHigh[i] = +kMotorClamp;
		//		}
		//	}
		//}


		//Set the frictionNormalIndices, lambdaLow, lambdaHigh, numberFrictionConstraints and addFriction in lcp solver
		// 把摩擦/边界信息完整交给 LCP 求解器，
		myLcp->setFrictionIndex(frictionNormalIndices);
		myLcp->setLowVector(lambdaLow);
		myLcp->setHighVector(lambdaHigh);
		myLcp->setNumberFrictionConstraints(numberFrictionConstraints);
		myLcp->setAddFriction(addFriction);

		// Calculate dt * M^-1 * f_ext
		this->multiplyMinvImplVector(MInv_dt_fext, getRigidBodies(), delta_t * (*f_ext));

		// Calculate the right side of the system equation and set it in the lcp solver
		// b = J* ( v_old + M^-1*dt*fext ) + constraintsRightSide
		// 计算右端项 b，并把它交给 LCP 求解器
		VSM::VectorN b(J.getRowCount());
		J.multiply(b, MInv_dt_fext += v_old);

		if (myLagrangeDebugObserver)
		{
			actualStep.JvOld = b;
		}

		b += constraintsRightSide;

		if (myLagrangeDebugObserver)
		{
			actualStep.b = b;
		}

		//======关于APGD求解器的 warm-start 机制 START=======
		// warm-start 相关
		// ——— 在这里加载上一步的 λ ———
		if (!first_step) {
			if (auto* apgd = dynamic_cast<RBDLcpAPGD*>(myLcp)) {
				apgd->SetLambda(prev_lambda);
			}
		}
		else {
			// 第一次也要初始化
			if (auto* apgd = dynamic_cast<RBDLcpAPGD*>(myLcp)) {
				prev_lambda.assign(matA.rows(), 1.0);
				apgd->SetLambda(prev_lambda);
			}
			first_step = false;
		}
		// ======关于APGD求解器的 warm-start 机制 END=======


		// Add the baumgarte stabilization to the lcp solver
		myLcp->setValuesInRightSide(-b);

		// 读取全局 CFM
		double globalCFM = myScene->getGlobalCfm();

		PERFORMANCESUITE_TOC("870 RBDCluster, Rest bis vor solve");
		// 开启“求解循环”阶段的计时
		PERFORMANCESUITE_TIC("880 RBDCluster, Solver loop");

		solverIterations = 1;

		// Calculate the condition number
		// SVD 分解计算条件数
		// 奇异值分解计算条件数非常耗时，默认关闭
		if (false)
		{
			VSM::MatrixNxM U;
			VSM::VectorN w;
			VSM::MatrixNxM V;

			//QFile file("C:/Users/Kurokawa/Desktop/conditionNR.txt");
			VSM::MatrixNxM::compileSVDecomposition(matA, U, w, V);

			QDebug debug = qDebug();
			int countLarger = 0; // Count the number of values greater than or equal to 0.001

			for (int i = 1; i < w.size(); i++) {
				double value = w.getElement(i - 1);
				if (value < 0.001) {
					//debug << value;  // Only output the values less than 0.001
				}
				else {
					countLarger++;  // Count the number of values greater than or equal to 0.001
				}
			}
			qDebug() << "countLarger" << countLarger;

			//calc condition number
			CND_Nr = w.getElement(0) / w.getElement(w.size() - 1);
			qDebug() << "CND_Nr" << CND_Nr;
		}
		else
		{
			CND_Nr = -1.0;
		}
		// Solve the LCP! :)
		// 求解失败时的“自救兜底”循环
		while (!myLcp->solve() || !myLcp->validSolution())
		{
			qDebug() << "solveMsg";
			// Okay, this is some dirty trick: To increase the prpability of solvability of the system,
			// in case when the solving fails the values on the main diagonal of A ar increased, until
			// it works...
			// And it does great! :)

			// 增大全局 CFM，期望把 A 的对角抬高、改善条件数，让系统更“好解”
			globalCFM = qMax(2 * globalCFM, 0.0001);

			if (globalCFM > .1)
			{
				myScene->emitSignalShowWarning(
					QObject::tr("VSLibRBDynamX::Cluster: Unable to solve for constraints at t=%1s. Continue with default solution for complementarity problem").arg(newTime),
					false);  // 弹警告：这一帧约束没能解好

				// Just take 0-vector as solution and continue.
				// 兜底：把 λ 设为 0（相当于不施加约束冲量），避免卡死
				myLcp->getLambda().setAllElementsToZero();
				break;

				// 另一种思路：直接用 PGS 做一步，看看能不能行得通
				// 但实际上没必要，因为上面已经把 CFM 提升到
				// 0.1 了，基本上不太可能解不出来了。
				// 反正 CFM 大了，约束就软了，解不
				// 出来也没啥好说的了。
				// Do one step using PGS

			  //  globalCFM = myScene->getGlobalCfm();
			  //  addToMatrixNxMDiag(&matA, myScene->getGlobalCfm());

			  //  delete myLcp;
			  //  myLcp = new RBDLcpGS( J.getRowCount(), numberEqualityConstraints, myScene->numberIterations() );

			  //  myLcp->setValuesInMatrix(0,0,matA); // Matrix A im LCP System setzen

			  //  myLcp->setFrictionIndex(frictionNormalIndices);
			  //  myLcp->setLowVector(lambdaLow);
			  //  myLcp->setHighVector(lambdaHigh);
			  //  myLcp->setNumberFrictionConstraints(numberFrictionConstraints);
			  //  myLcp->setValuesInRightSide(0, -b);

			  //  continue;
			}//end if


//仅在调试版弹警告：这帧约束没解成，把 globalCFM 增大再试（日志带时间与当前 CFM 值）。
#ifdef RBD_DEBUG
			myScene->emitSignalShowWarning(
				QObject::tr("Unable to solve for constraints at t=%1s. Increasing global cfm value to %2").arg(newTime).arg(globalCFM),
				false);
#endif
			// 给 A 的对角线加上 globalCFM，这会抬高最小特征值，改善条件数，让迭代更容易收敛。
			VSLibRBDynMath::addToMatrixNxMDiag(&matA, J.getRowCount(), globalCFM);

			// Diese Werte werden vom Solver überschrieben,
			// müssen daher neu gesetzt werden.
			// 这些值会被求解器覆盖，所以重试前必须重新设置
			myLcp->setLowVector(lambdaLow);
			myLcp->setHighVector(lambdaHigh);

			myLcp->setValuesInMatrix(matA);
			myLcp->setValuesInRightSide(-b);

			++solverIterations;
		}//end while

		// 统计尝试次数，方便分析
		PERFORMANCESUITE_VALUE("RBDCluster, Solver iterations", solverIterations);

		PERFORMANCESUITE_TOC("880 RBDCluster, Solver loop");
		PERFORMANCESUITE_TIC("890 RBDCluster, After solving");


// == STRAT : warm-start 相关机制的配置，不用太在意 ==
		// warm-start 相关
		// Solve 完成后，你已有了新的 λ 向量

		// —— 在这里保存本次求解后的 λ ——  
		if (auto* apgd = dynamic_cast<RBDLcpAPGD*>(myLcp)) {
			prev_lambda = apgd->GetLambda();
		}
		// ————————————————————————
// == END : warm-start 相关机制的配置，不用太在意 ==
		if (myLagrangeDebugObserver)
		{
			actualStep.l = myLcp->getLambda();
		}

		// 把本帧求出来的拉格朗日乘子 𝜆（对应每条约束/接触的解）还原成“有效力/力矩（effective force/torque）”并存回各约束对象，
		// The constraint resources store their current lagrangian multipliers
		for (RBDConstraintResource* constraintRes : myConstraintResources)
		{
			constraintRes->updateEffectiveForce(myLcp->getLambda(), delta_t);
		}

		for (std::list<RBDContact*>::iterator it = allHandledContactsInCluster.begin(); it != allHandledContactsInCluster.end(); ++it)
		{
			(*it)->updateEffectiveForce(myLcp->getLambda(), delta_t);
		}



		//==START:  为了判定接触带是否偏移打出来的调试代码，可以忽略 ==
		//    （确保你已把 ensureLogsOpen() 和全局日志对象加到文件顶部）
		ensureLogsOpen();
		{
			QMutexLocker lock(&g_logMutex);

			// 本步接触点个数
			const int cnum = static_cast<int>(allHandledContactsInCluster.size());
			g_ccOut << QString::number(newTime, 'f', 6) << "  " << cnum << "\n";

			// 逐接触点记录坐标（可选追加法向力）
			for (auto it = allHandledContactsInCluster.begin();
				it != allHandledContactsInCluster.end(); ++it) {

				RBDContact* c = *it;

				// 世界坐标
				const VSM::Vector3& P = c->position();
				// 你的 VSM::Vector3 没有 x()/y()/z()，用 getElement(i) 或 []
				const double px = P[0];
				const double py = P[1];
				const double pz = P[2];


				// 如需“值”=法向力（更直观看变化），打开下面一行
				// const double fn = c->getContactNormalForce();

				// 指针地址作为接触点ID，便于跨步追踪
				g_cpOut << QString::number(newTime, 'f', 6) << "  "
					<< quintptr(c) << "  "
					<< px << "  " << py << "  " << pz
					// << "  " << fn
					<< "\n";
			}

			// 用空行分隔一个时间步的数据块
			g_cpOut << "\n";

			// —— 只在首次写入时打印路径（避免每帧刷屏）——
			static bool s_pathPrinted = false;
			if (!s_pathPrinted) {
				const QString cpPath = QFileInfo(g_cpFile).absoluteFilePath();
				const QString ccPath = QFileInfo(g_ccFile).absoluteFilePath();
				qDebug() << "[contact_points.dat] =>" << cpPath;
				qDebug() << "[contact_count.dat ] =>" << ccPath;
				s_pathPrinted = true;
			}


			g_cpOut.flush();
			g_ccOut.flush();
		}

		//==END:  为了判定接触带是否偏移打出来的调试代码，可以忽略 ==

		//qDebug() << "matA = " << matA;
		//qDebug() << "b = " << b;/*
		//qDebug() << "lambdaLow = " << lambdaLow;
		//qDebug() << "lambdaHigh = " << lambdaHigh;*/
		//qDebug() << "lambda = " << myLcp->getLambda();
		// 回代得到新速度
		v_new = MInv_JT * myLcp->getLambda() + MInv_dt_fext;

		if (myLagrangeDebugObserver)
		{
			actualStep.v_new = v_new;
		}

		// Den einzelnen Bodies ihre neue Geschwindigkeit mitteilen
		// Das Update auf die neue Position wird wieder zentral in der Scene vorgenommen.
		// 把新速度写回各个刚体
		for (int i = 0; i < getRigidBodies().size(); ++i)
		{
			//QtConcurrent::run(&pool, updateRigidBodyTwist, currentRigidBodies[i], v_new);
			RBDRigidBody* rBody = getRigidBodies()[i];

			VSM::Vector6 twist;
			for (int i = 0; i < 6; ++i)
				twist[i] = v_new[rBody->matrixOffset() + i];

			rBody->setTwist(twist);
		}

		delete f_ext;

		PERFORMANCESUITE_TOC("890 RBDCluster, After solving");
	}
	else
	{
		// In this situation, there are no constraints in the system
		// 没有任何约束行（J 为空）时的快捷通道
		PERFORMANCESUITE_TIC("825 RBDCluster, J empty");

		if (myLagrangeDebugObserver)
		{
			actualStep.J = VSM::MatrixNxM(0, 0);
			actualStep.matA = VSM::MatrixNxM(0, 0);
			actualStep.v_old = VSM::VectorN(0);
			actualStep.pos_old = VSM::VectorN(0);
			actualStep.m = VSM::VectorN(0);
			actualStep.lambdaLow = VSM::VectorNDynamic();
			actualStep.lambdaHigh = VSM::VectorNDynamic();
			actualStep.constraintsRightSide = VSM::VectorNDynamic();
		}

		//	Update the velocities of each rigid body with v_new = v_old + dt * ( M^-1 * f_ext )   { ~ v_new = v_alt + a * dt )
		// 当前没有任何约束时，逐个刚体用“自由运动”更新一步，然后把本帧外力/外矩清零
		for (int i = 0; i < getRigidBodies().size(); ++i)
		{
			RBDRigidBody* rBody = getRigidBodies()[i];
			rBody->evolveStateUnconstrained(delta_t);
			rBody->resetForceAndTorque();
		}

		PERFORMANCESUITE_TOC("825 RBDCluster, J empty");
	}

	//DELETEME if it works
   // if (!history.isEmpty() && newTime < history.last().newTime)
   // {
	   //if(myLagrangeDebugObserver){
	   //	 myScene->detachDebugInstance();
	   //}
   //    printHistory();
   //    history.clear();
   // }

	if (myLagrangeDebugObserver)
	{
		history.append(actualStep);
		myLagrangeDebugObserver->update(actualStep);
	}
	//notifyDebugObserver(); //(ci)

	delete myLcp;
#endif

//Quadratic Programming——二次规划
// 有一说一这个QP我没看懂他想做什么，虽然注释掉了
#ifdef USE_QP_IMPLEMENTATION
	RBDInverseDynamicsFormulation* inverseDynamics = new RBDInverseDynamicsFormulation(myScene, this, getRigidBodies(), delta_t);
	if (inverseDynamics->getDimension() > 0)
	{
		RBDInverseDynamicsSolverLCP* solver = new RBDInverseDynamicsSolverLCP(myScene, inverseDynamics);
		VSM::VectorN  lambda = solver->solve();

		VSM::VectorN v_new =
			inverseDynamics->getMatrixMInv_JT() * lambda
			+ inverseDynamics->getVectorMInv_dt_fext();

		for (int i = 0; i < getRigidBodies().size(); ++i)
		{
			{
				//QtConcurrent::run(&pool, updateRigidBodyTwist, currentRigidBodies[i], v_new);
				RBDRigidBody* rBody = getRigidBodies()[i];

				VSM::Vector6 twist;
				for (int i = 0; i < 6; ++i)
					twist[i] = v_new[rBody->matrixOffset() + i];

				rBody->setTwist(twist);
			}
		}
	else
	{
		//	we have an unconstrained system, new speedvector simply determined by:
		//	v_neu = v_alt + dt * ( M^-1 * f_ext )   { ~ v_neu = v_alt + a * dt )
		for (int i = 0; i < getRigidBodies().size(); ++i)
		{
			RBDRigidBody* rBody = getRigidBodies()[i];
			rBody->evolveStateUnconstrained(delta_t);
			rBody->resetForceAndTorque();
		}
	}
#endif
}
	// 为摩擦添加辅助约束的函数，但是没被用到，一般都是求解器自己处理摩擦
	VSM::MatrixNxM RBDClusterLagrangeMultipliers::addAuxiliaryConstraintsForFriction(
		VSM::MatrixNxM & A,
		VSM::VectorNDynamic & lambdaLow,
		VSM::VectorNDynamic & lambdaHigh,
		VSM::VectorNDynamicTemplate<int>&frictionIndices,
		VSM::VectorNDynamic & constFrictionForces)
	{
		int numberFrictionBlocks = 0;

		for (int i = 1; i < frictionIndices.size(); i++)
		{
			if (frictionIndices[i - 1] == -1 && frictionIndices[i] >= 0)
				numberFrictionBlocks++;
		}

		VSM::MatrixNxM matA(A.rows() + numberFrictionBlocks, A.columns() + numberFrictionBlocks);

		for (int i = 0; i < A.rows(); i++)
		{
			for (int j = 0; j < A.columns(); j++)
			{
				matA[i][j] = A[i][j];
			}
		}

		int idxAuxConstr = 0;
		for (int i = 1; i < frictionIndices.size(); i++)
		{
			if (frictionIndices[i - 1] == -1 && frictionIndices[i] >= 0)
			{
				// index i is pointing to the normal force
				matA[A.rows() + idxAuxConstr][i - 1] = lambdaHigh[i];

				// add friction auxiliary constraint for current contact point
				while (i < frictionIndices.size() && frictionIndices[i] >= 0)
				{
					lambdaLow.setElement(i, 0);
					lambdaHigh.setElement(i, VSM::maxDouble);

					matA[i][A.columns() + idxAuxConstr] = 1;
					matA[A.rows() + idxAuxConstr][i] = -1;

					i++;
				}

				lambdaLow.setElement(A.rows() + idxAuxConstr, 0);
				lambdaHigh.setElement(A.rows() + idxAuxConstr, VSM::maxDouble);

				idxAuxConstr++;
			}
		}

		for (int i = 0; i < matA.rows(); i++)
		{
			frictionIndices.setElement(i, -1);

			if (i >= matA.rows())
				constFrictionForces.setElement(i, 0);
		}
		//qDebug() << "A = " << matA;
		return matA;
	}

	/*
	   Matrix multiplication M^(-1) * J^T in "nearly" linear runtime, utilizing sparsity.
	*/
    // 乘法 M^-1 * J^T 的优化实现
	void RBDClusterLagrangeMultipliers::multiplyMinvImplJT(
		VSM::MatrixNxM & result,
		const RBDRigidBodyPtrSet & bodies,
		const VSLibRBDynMath::RBMJacobeanMatrix & J)
	{
		int targetColumn = 0;
		for (VSLibRBDynMath::RBMJacobeanMatrix::JacobeanRow* jRow : J.getRows())
		{
			for (VSLibRBDynMath::RBMJacobeanMatrix::JacobeanEntry* jEntry : *jRow) // 98% der Fälle 2 Stück!
			{
				RBDRigidBody* b = bodies[jEntry->column / 6];

				result[b->matrixOffset() + 0][targetColumn] +=
					b->getMassInv() * jEntry->values[0];
				result[b->matrixOffset() + 1][targetColumn] +=
					b->getMassInv() * jEntry->values[1];
				result[b->matrixOffset() + 2][targetColumn] +=
					b->getMassInv() * jEntry->values[2];

				result[b->matrixOffset() + 3][targetColumn] +=
					b->getInertiaInvW().getElement(0, 0) * jEntry->values[3]
					+ b->getInertiaInvW().getElement(0, 1) * jEntry->values[4]
					+ b->getInertiaInvW().getElement(0, 2) * jEntry->values[5];

				result[b->matrixOffset() + 4][targetColumn] +=
					b->getInertiaInvW().getElement(1, 0) * jEntry->values[3]
					+ b->getInertiaInvW().getElement(1, 1) * jEntry->values[4]
					+ b->getInertiaInvW().getElement(1, 2) * jEntry->values[5];

				result[b->matrixOffset() + 5][targetColumn] +=
					b->getInertiaInvW().getElement(2, 0) * jEntry->values[3]
					+ b->getInertiaInvW().getElement(2, 1) * jEntry->values[4]
					+ b->getInertiaInvW().getElement(2, 2) * jEntry->values[5];
			}
			++targetColumn;
		}
	}

	/*
	   Optimized method to calculate the product of the mass matrix and the jacobean^T.
	   Makes use of the sparse form of M AND the sparse form of the jacobean.
	*/
	// 乘法 M^-1 * v 的优化实现
	void RBDClusterLagrangeMultipliers::multiplyMinvImplVector(
		VSM::VectorN & result,
		const RBDRigidBodyPtrSet & bodies,
		const VSM::VectorN & mult)
	{
		int bodyOffset = 0;
		unsigned int r = 0;
		int l = 0;

		RBDRigidBodyPtrSet::const_iterator itRb = bodies.constBegin();
		while (r < result.size())
		{
			RBDRigidBody* body = *itRb;

			unsigned int end = r + 3;
			for (; r < end; ++r)
			{
				//         double minv = 1/body->getMass();
				result[r] = mult[r] / body->getMass();
			}
			/*
				  int lowerBorder = bodyOffset + 3;
				  int upperBorder = lowerBorder + 3;
			*/
			end += 3;

			const VSLibRBDynMath::RBMInertiaTensor& inertiaInv = body->getInertiaInvW();
			l = 0;

			for (; r < end; ++r)
			{
				int inertiaRow = r % 3;
				result[r] += inertiaInv.getElement(inertiaRow, l) * mult[bodyOffset + 3];
				result[r] += inertiaInv.getElement(inertiaRow, l + 1) * mult[bodyOffset + 4];
				result[r] += inertiaInv.getElement(inertiaRow, l + 2) * mult[bodyOffset + 5];
			}

			bodyOffset += 6;
			++itRb;
		}
	}

	// 获取条件数 SVD用的
	double RBDClusterLagrangeMultipliers::getCNDNr()
	{
		return CND_Nr;
	}

	// 获得误差
	double RBDClusterLagrangeMultipliers::getConstraintError()
	{
		return constraintError;
	}

	/*
	   method to notify Observer class for Lagrange Debugging
	*/
	//void notifyDebugObserver(){
	//	 if(myLagrangeDebugObserver){
	//	 myLagrangeDebugObserver->update(actualStep);
	//	 }
	//}

	/*
	void VSLibRBDynamX::RBDClusterLagrangeMultipliers::doErrorProjection()
	{
	   VSLibRBDynMath::RBMMatrix *M_Inv = 0;
	   myConstraintResources.clear();

	   int numRBodies = 0;
	   RBDRigidBodyPtrSet::const_iterator it = getRigidBodies().begin();

	   for (; it != getRigidBodies().end(); ++it)
	   {
		  RBDRigidBody* rBody = *it;

		  QString bName = rBody->getName();

		  rBody->setClusterIndex(numRBodies);
		  ++numRBodies;
	   }

	   // Zur Größenbestimmung des Systems:
	   int sizeOfProblem = 6*numRBodies;

	   int numberEqualityConstraints = 0;
	   int numberComplementaryConstraints = 0;

	   // Erzeuge Liste aller Constraint dieses Clusters
	   // RBDConResPtrList myConstraintResources;

	   for(RBDRigidBody* body: getRigidBodies())
	   {
		  for(RBDConstraintResource* joint: body->getConstraintResources())
		  {
			 if (joint->getDisabled())
				continue;

			 myConstraintResources.add(joint);
		  }
	   }

	   for(RBDConstraintResource* joint: myConstraintResources)
	   {
		  if (joint->getDisabled())
			 continue;

		  numberEqualityConstraints += joint->getNumberEqConstraintsForPoseCorrection();
		  numberComplementaryConstraints += joint->getNumberIneqConstraintsForPoseCorrection();
	   }

	   int numberFoundContacts = 0;

	   std::list<RBDContact*> allContacts;
	   RBDRigidBodyPtrSet::const_iterator itRb = getRigidBodies().constBegin();
	   for (; itRb != getRigidBodies().constEnd(); ++itRb)
	   {
		  RBDRigidBody* body = *itRb;

		  const QList<RBDContact*>& contacts = body->getContacts(true);
		  QList<RBDContact*>::const_iterator itContact = contacts.constBegin();
		  for (; itContact != contacts.constEnd(); ++itContact)
		  {
			 RBDContact* contact = *itContact;

			 if (!contact->getHandled() && !contact->getDisabled())
			 {
				// Wenn eine Szene einen fixen Körper enthält (Boden), so wird dieser gegebenenfalls
				// in mehrere Cluster eingefügt. Daher können hier Kontakte auftauchen, von denen einer der
				// beiden beteiligten Körper nich in diesem Cluster liegt.
				if (!contact->body(0)->isFix() && contact->body(0)->getCluster() != this)
				   continue;

				if (!contact->body(1)->isFix() && contact->body(1)->getCluster() != this)
				   continue;

				++numberFoundContacts;
				myConstraintResources.add(contact);
				allContacts.push_back(contact);
				contact->setHandled(true);

	//            contact->calculate();
				numberComplementaryConstraints += contact->getNumberIneqConstraintsForPoseCorrection();
			 }
		  }
	   }

	   for (std::list<RBDContact*>::const_iterator it = allContacts.begin(); it != allContacts.end(); ++it)
	   {
		  RBDContact* contact = *it;
		  contact->setHandled(false);
	   }

	   mySizeOfJacobean = numberEqualityConstraints + numberComplementaryConstraints;

	   if (mySizeOfJacobean > 0)
	   {
		  // Welcher Solver soll genutzt werden?
		  if (myScene->constraintSolverType() == RBDScene::CST_LAGRANGEDANTZIG)
			 myLcp = new RBDLcpDantzig( mySizeOfJacobean, numberEqualityConstraints );
		  else if (myScene->constraintSolverType() == RBDScene::CST_LAGRANGEDANTZIGEXP )
			 myLcp = new RBDLcpDantzigExperimental( mySizeOfJacobean, numberEqualityConstraints, myScene->numberIterations());
		  else if (myScene->constraintSolverType() == RBDScene::CST_LAGRANGESOR)
			 myLcp = new RBDLcpSor( mySizeOfJacobean, numberEqualityConstraints, myScene->numberIterations());
		  else if (myScene->constraintSolverType() == RBDScene::CST_LAGRANGEGS)
			 myLcp = new RBDLcpGS( mySizeOfJacobean, numberEqualityConstraints, myScene->numberIterations() );
		  else
		  {
			 myScene->emitSignalShowWarning(QObject::tr("VSLibRBDynamX::RBDClusterLagrangeMultipliers: The lcp-solver %1 is unknown. Default GS used instead!").arg(myScene->constraintSolverType()), false);
			 myLcp = new RBDLcpGS( mySizeOfJacobean, numberEqualityConstraints, myScene->numberIterations() );
		  }

		  VSLibRBDynMath::RBMJacobeanMatrix J(mySizeOfJacobean, sizeOfProblem);

		  // Zunächst die Joints ablaufen und deren Constraints in J einfügen
		  VSM::VectorN constraintsRightSide(mySizeOfJacobean);

		  int currentRow = 0;

		  for( RBDConstraintResource* constraintRes: myConstraintResources )
		  {
			 if (constraintRes->getDisabled())
				continue;

			 constraintRes->setEqualityConstraintsOffset(currentRow);

			 if ( !constraintRes->body(0)->isFix() && constraintRes->body(0)->getCluster() != this )
				continue;

			 if ( constraintRes->body(1) && !constraintRes->body(1)->isFix() && constraintRes->body(1)->getCluster() != this )
				continue;

			 currentRow += constraintRes->insertConstraintsInPostStabJacobean(
				J,
				lamb
				constraintsRightSide,
				.01, //TBD
				currentRow);
		  }

	//      qDebug() << J.toString();

		  f_ext = new VSM::VectorN(sizeOfProblem);
		  VSM::VectorN v_new(sizeOfProblem);

		  VSLibRBDynMath::RBMMatrix MInv_JT(M_Inv->getRowCount(), J.getRowCount(), false);
		  this->multiplyMinvImplJT(MInv_JT, getRigidBodies(), J);

		  VSLibRBDynMath::RBMMatrix matA(J.getRowCount(), MInv_JT.getColCount(), false);

		  J.multiply(matA, MInv_JT);

		  // Add global constraint force mixing to increase
		  // systems stability at least for ODE Solver
		  if (myScene->getGlobalCfm() > .0)
		  {
			 for (int i = 0; i < matA.getRowCount(); ++i)
			 {
				matA[i][i] += myScene->getGlobalCfm();
			 }
		  }

		  // TODO Wenn immer die vollständige Matrix überbeben
		  // wird, optimier das hier!
		  myLcp->setValuesInMatrix(0,0,matA); // Matrix A im LCP System setzen

		  myLcp->setValuesInRightSide(0, -constraintsRightSide);

		  VSM::VectorN myHi = *myLcp->hi();
		  VSM::VectorN myLo = *myLcp->lo();

		  double globalCFM = myScene->getGlobalCfm();

		  // Solve the LCP! :)
		  while ( !myLcp->solve() || !myLcp->validSolution() )
		  {
			 if (globalCFM > 0.0512)
			 {
				myScene->emitSignalShowWarning(
				   QObject::tr("VSLibRBDynamX::Cluster: Unable to solve for constraints at t=%1s. Continue with valid default solution for complementarity problem").arg(999),
				   false);

				// Just take 0-vector as solution and continue.
				myLcp->getLambda().setAllElementsToZero();
				break;
			 }

			 // Okay, this is some dirty trick: To increase the prpability of solvability of the system,
			 // in case when the solving fails the values on the main diagonal of A ar increased, until
			 // it works...
			 // And it does great! :)
			 globalCFM = max(2*globalCFM, 0.0001);

			 addToMatrixNxMDiag(&matA, globalCFM);

			 mylambdaHigh.setValues(0, myHi);
			 mylambdaLow.setValues(0, myLo);
			 myLcp->setValuesInMatrix(0,0,matA);
		  }

		  v_new = MInv_JT * myLcp->getLambda();

	//      myScene->printTimeDiff("Nach Solve: ");

		  // Den einzelnen Bodies ihre neue Geschwindigkeit mitteilen
		  // Das Update auf die neue Position wird wieder zentral in der Scene vorgenommen.
		  for(RBDRigidBody* rBody: getRigidBodies())
		  {
			 VSM::Vector6 twist(v_new, rBody->matrixOffset());
			 VSM::PoseVector3Quaternion p = rBody->getAbsPose();
			 p.evolve(.01, twist);
			 rBody->setAbsPose(p);
		  }
	   }

	   myConstraintResources.clear();
	}

	*/