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

RBDClusterLagrangeMultipliers::RBDClusterLagrangeMultipliers(RBDScene* scene)
	: RBDCluster(scene)
	//, M_Inv(0)
	, myScene(scene)
	, f_ext(0)
	, myLcp(0)
	//, mySizeOfJacobean(0)
{
#ifdef RBD_DEBUG
	insts.ref();
#endif
}

RBDClusterLagrangeMultipliers::~RBDClusterLagrangeMultipliers()
{
#ifdef RBD_DEBUG
	insts.deref();
#endif
}

void updateRigidBodyTwist(RBDRigidBody* rb, VSM::VectorN v_new)
{
	VSM::Vector6 twist;
	for (int i = 0; i < 6; ++i)
		twist[i] = v_new[rb->matrixOffset() + i];

	rb->setTwist(twist);
}

void RBDClusterLagrangeMultipliers::doTimeStep(
	double newTime,
	double delta_t)
{
	//// UseBilateralFrictionCone 是否生效
	//const bool useCone = getScene()->getUseBilateralFrictionCone();
	//qDebug() << "[Scene] UseBilateralFrictionCone =" << useCone;


	bool useOldImplementation = false;
#ifndef USE_QP_IMPLEMENTATION

	Q_UNUSED(newTime);

	if (delta_t <= 0)
		return;

	if (myLagrangeDebugObserver)
	{
		actualStep.newTime = newTime;
	}

	PERFORMANCESUITE_TIC("810 RBDCluster, Collect constraints");

	int numRBodies = 0;
	RBDRigidBodyPtrSet::const_iterator it = getRigidBodies().begin();

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

	for (; it != getRigidBodies().end(); ++it)
	{
		RBDRigidBody* rBody = *it;

		rBody->setClusterIndex(numRBodies);
		++numRBodies;
	}

	// The size of the state vector
	int sizeOfProblem = 6 * numRBodies;

	// Allokation des Gesamtspeedvektors
	VSM::VectorN v_old(sizeOfProblem);
	VSM::VectorN pos_old(sizeOfProblem);
	VSM::VectorN mass(sizeOfProblem);

	//int numberFoundContacts = 0;

	std::list<RBDContact*> allHandledContactsInCluster;

	// Store all the constraints of each rigid body in a list myConstraintResources.
	for (int i = 0; i < getRigidBodies().size(); ++i)
	{
		RBDRigidBody* rBody = getRigidBodies()[i];   //for(RBDRigidBody* rBody: getRigidBodies())

		const RBDConstraintResourcePtrSet& bodiesConstrRes = rBody->getConstraintResources();

		for (int i = 0; i < bodiesConstrRes.size(); ++i)   //for(RBDConstraintResource* joint: )
		{ // In Differenzialen wird derzeit nur der 0te Body gesetzt - body (1) wird immer 0 returnieren.
			RBDConstraintResource* res = bodiesConstrRes[i];

			if (res->getDisabled())
				continue;

			if (!res->body(0))
				continue;

			// Check whether body 0 and body 1 of the constraints are both in this cluster
			if (!res->body(0, true)->isFix() && res->body(0, true)->getCluster() != this)
				continue;

			if (res->body(1, true) && !res->body(1, true)->isFix() && res->body(1, true)->getCluster() != this)
				continue;

			myConstraintResources.add(res);

			RBDContact* contact = res->dynamicCast<RBDContact*>();
			if (contact)
			{
				if (contact->getHandled())
					continue;

				//++numberFoundContacts;
				contact->setHandled(true);

				allHandledContactsInCluster.push_back(contact);
			}
		}
	}

	//   mySizeOfJacobean = numberEqualityConstraints + numberComplementaryConstraints;

	   //// Reset handled-flag in contacts
	   //for (std::list<RBDContact*>::const_iterator it = allHandledContactsInCluster.begin(); it != allHandledContactsInCluster.end(); ++it)
	   //{
	   //   (*it)->setHandled(false);
	   //}

	PERFORMANCESUITE_TOC("810 RBDCluster, Collect constraints");
	PERFORMANCESUITE_TIC("820 RBDCluster, Build J");
	PERFORMANCESUITE_TIC("821 J, add eq. constr.");

	/* Now start to calculate J,lambdaLow,lambdaHigh,addFriction,constraintsRightSide
	   -lambdaLow,lambdaHigh: limits of lambda
	   -constraintsRightSide: baumgarte term
	   -First calculate with equality constraints, then with complementarity constraints
	*/
	VSLibRBDynMath::RBMJacobeanMatrix J;
	VSM::VectorNDynamic constraintsRightSide(0.0);
	VSM::VectorNDynamic lambdaLow(-VSM::maxDouble);
	VSM::VectorNDynamic lambdaHigh(VSM::maxDouble);
	VSM::VectorNDynamic addFriction(0.0);
	VSM::VectorNDynamicTemplate<int> frictionNormalIndices(-1);
	VSM::VectorN v_new(sizeOfProblem);

	int numberEqualityConstraints = 0;
	int numberComplementaryConstraints = 0;
	int numberFrictionConstraints = 0;
	int numberEqConstraintsForPoseCorrection = 0; // velocity based motor with unlimited force is not considered.

	int currentRow = 0;
	constraintError = 0;

	// First run the joints and add the equality constraints to the system
	for (RBDConstraintResource* constraintRes : myConstraintResources)
	{
		constraintRes->setEqualityConstraintsOffset(currentRow);

		int number = constraintRes->addEqualityConstraintsToSystem(
			J,
			constraintsRightSide,
			delta_t,
			currentRow);

		currentRow += number;
		numberEqualityConstraints += number;
		numberEqConstraintsForPoseCorrection += constraintRes->getNumberEqConstraintsForPoseCorrection();

		// calculate the square sum of constraint error for this cluster at this time point
		constraintError += constraintRes->getConstraintError();
	}
	// calculate the Mean Squared Error for this cluster at this time point
	if (numberEqConstraintsForPoseCorrection > 0)
		constraintError = VSM::sqrt(constraintError / numberEqConstraintsForPoseCorrection);
	else
		constraintError = 0;

	//qDebug() << "The constraintError of this cluster is" << constraintError << "The total number of equality constraints are" << numberEqConstraintsForPoseCorrection;

	PERFORMANCESUITE_TOC("821 J, add eq. constr.");
	PERFORMANCESUITE_TIC("822 J, add compl. constr.");

	// Update frictionNormalIndices，lambdaLow，lambdaHigh，numberFrictionConstraints，addFriction with complementarity constraints
	for (RBDConstraintResource* constraintRes : myConstraintResources)
	{
		if (constraintRes->dynamicCast<RBDContact*>())
			continue;

		constraintRes->setComplementaryConstraintsOffset(currentRow);

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

	PERFORMANCESUITE_TOC("822 J, add compl. constr.");
	PERFORMANCESUITE_TIC("823 J, add compl. constr. for contacts");

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
	VSM::MatrixNxM jMatrix(calcMatrix(J, sizeOfProblem));

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
		CND_Nr = w.getElement(0) / w.getElement(w.size() - 1);
		qDebug() << "CND_Nr" << CND_Nr;
	}


	// we have a constrained system --> build the LCP system:
	// A*x = b + w, x[i]*w[i] >= lo[i], x[i]*w[i] < hi[i], hi, lo

	// A = J * M^-1 * J^T
	// b = -J * ( dt * M^-1 * f_ext + v_jetzt ) + constraintsRightSide + b_speed
	// x = lambda * dt

	//qDebug() << "RBDClusterLagrangeMultipliers::doTimeStep J:" << J.toString();

	PERFORMANCESUITE_TOC("823 J, add compl. constr. for contacts");
	PERFORMANCESUITE_TOC("820 RBDCluster, Build J");

	if (J.getRowCount() > 0)
	{
		PERFORMANCESUITE_TIC("830 RBDCluster, before multiplications");

		//qDebug() << J.toString(true);

		VSM::MatrixNxM MInv_JT(sizeOfProblem, J.getRowCount());
		VSM::MatrixNxM matA(J.getRowCount(), MInv_JT.columns(), false);
		VSM::VectorN MInv_dt_fext(sizeOfProblem);

		it = getRigidBodies().begin();
		for (; it != getRigidBodies().end(); ++it)
		{
			RBDRigidBody* rBody = *it;

#ifdef RBD_DEBUG
			if (fabs(rBody->getMass()) < VSM::epsilon)
			{
				qDebug() << "RBDClusterLagrangeMultipliers: Body " << rBody->getName() << " has mass close to zero!";
			}
#endif
			v_old.setElements(rBody->matrixOffset(), rBody->getTwist().getV());
			v_old.setElements(rBody->matrixOffset() + 3, rBody->getTwist().getW());

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
		f_ext = new VSM::VectorN(sizeOfProblem);
		if (!getScene()->getIgnoreGyroTerm())
		{
			for (int i = 0; i < getRigidBodies().size(); ++i)
			{
				RBDRigidBody* rBody = getRigidBodies()[i];
				//for(RBDRigidBody* rBody: getRigidBodies())

				f_ext->setElements(rBody->matrixOffset(), rBody->getExternalForce());
				f_ext->setElements(rBody->matrixOffset() + 3, rBody->getExternalTorque() - cross(rBody->getTwist().getW(), (rBody->getInertiaW() * rBody->getTwist().getW())));

				// reset the extern forces and torques on rigid bodies to zero
				rBody->resetForceAndTorque();
			}
		}
		else
		{
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

		PERFORMANCESUITE_TOC("830 RBDCluster, before multiplications");
		PERFORMANCESUITE_TIC("840 RBDCluster, M_Inv*JT");

		this->multiplyMinvImplJT(MInv_JT, getRigidBodies(), J);

		PERFORMANCESUITE_TOC("840 RBDCluster, M_Inv*JT");
		PERFORMANCESUITE_VALUE("J.getRowCount()", J.getRowCount());
		PERFORMANCESUITE_TIC("850 RBDCluster, J * M_InvJT (sym.)");

		//		VSLibRBDynMath::RBMMatrix matA = J * MInv_JT;
		J.multiplySymmetricFast(matA, MInv_JT);

		PERFORMANCESUITE_TOC("850 RBDCluster, J * M_InvJT (sym.)");
		PERFORMANCESUITE_TIC("860 RBDCluster, add Joints CFM");

		// Add constraintForceMixing to system matrix JMJT
		for (RBDConstraintResource* constraintRes : myConstraintResources)
		{
			if (constraintRes->getDisabled())
				continue;

			constraintRes->addConstraintForceMixing(&matA, constraintsRightSide, delta_t);
		}

		if (!getScene()->getUseBilateralFrictionCone())
			matA = addAuxiliaryConstraintsForFriction(matA, lambdaLow, lambdaHigh, frictionNormalIndices, addFriction);

		PERFORMANCESUITE_TOC("860 RBDCluster, add Joints CFM");
		PERFORMANCESUITE_TIC("870 RBDCluster, Rest bis vor solve");

		// Add global constraint force mixing to increase systems stability at least for ODE Solver
		if (myScene->getGlobalCfm() > .0)
		{
			VSLibRBDynMath::addToMatrixNxMDiag(&matA, J.getRowCount(), myScene->getGlobalCfm());
			//qDebug() << QString("Global CFM: ") << myScene->getGlobalCfm();
		}

		// Choose lcp solver
		// TBD: SolverFactory bauen
		// --> myLcp = SolverFactory->getSolver(myScene->constraintSolverType())
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
			if (numberEqualityConstraints <= 0)
				myLcp = new RBDLcpGS(matA.rows(), numberEqualityConstraints, myScene->numberIterations());
			else
				myLcp = new RBDLcpDantzig(matA.rows(), numberEqualityConstraints);
		}
		}

		// Set the JMJT matrix in lcp solver
		myLcp->setValuesInMatrix(matA);

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

		if (lambdaLow.size() < matA.rows())
			lambdaLow[matA.rows() - 1] = -VSM::maxDouble;

		if (lambdaHigh.size() < matA.rows())
			lambdaHigh[matA.rows() - 1] = VSM::maxDouble;


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
		myLcp->setFrictionIndex(frictionNormalIndices);
		myLcp->setLowVector(lambdaLow);
		myLcp->setHighVector(lambdaHigh);
		myLcp->setNumberFrictionConstraints(numberFrictionConstraints);
		myLcp->setAddFriction(addFriction);

		this->multiplyMinvImplVector(MInv_dt_fext, getRigidBodies(), delta_t * (*f_ext));

		// Calculate the right side of the system equation and set it in the lcp solver
		// b = J* ( v_old + M^-1*dt*fext ) + constraintsRightSide
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
		// ——————————————————————


		// Add the baumgarte stabilization to the lcp solver
		myLcp->setValuesInRightSide(-b);

		double globalCFM = myScene->getGlobalCfm();

		PERFORMANCESUITE_TOC("870 RBDCluster, Rest bis vor solve");
		PERFORMANCESUITE_TIC("880 RBDCluster, Solver loop");

		solverIterations = 1;

		// Calculate the condition number
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
		while (!myLcp->solve() || !myLcp->validSolution())
		{
			qDebug() << "solveMsg";
			// Okay, this is some dirty trick: To increase the prpability of solvability of the system,
			// in case when the solving fails the values on the main diagonal of A ar increased, until
			// it works...
			// And it does great! :)

			globalCFM = qMax(2 * globalCFM, 0.0001);

			if (globalCFM > .1)
			{
				myScene->emitSignalShowWarning(
					QObject::tr("VSLibRBDynamX::Cluster: Unable to solve for constraints at t=%1s. Continue with default solution for complementarity problem").arg(newTime),
					false);

				// Just take 0-vector as solution and continue.
				myLcp->getLambda().setAllElementsToZero();
				break;

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

#ifdef RBD_DEBUG
			myScene->emitSignalShowWarning(
				QObject::tr("Unable to solve for constraints at t=%1s. Increasing global cfm value to %2").arg(newTime).arg(globalCFM),
				false);
#endif

			VSLibRBDynMath::addToMatrixNxMDiag(&matA, J.getRowCount(), globalCFM);

			// Diese Werte werden vom Solver überschrieben,
			// müssen daher neu gesetzt werden.
			myLcp->setLowVector(lambdaLow);
			myLcp->setHighVector(lambdaHigh);

			myLcp->setValuesInMatrix(matA);
			myLcp->setValuesInRightSide(-b);

			++solverIterations;
		}//end while

		PERFORMANCESUITE_VALUE("RBDCluster, Solver iterations", solverIterations);

		PERFORMANCESUITE_TOC("880 RBDCluster, Solver loop");
		PERFORMANCESUITE_TIC("890 RBDCluster, After solving");

		// warm-start 相关
		// Solve 完成后，你已有了新的 λ 向量

		// —— 在这里保存本次求解后的 λ ——  
		if (auto* apgd = dynamic_cast<RBDLcpAPGD*>(myLcp)) {
			prev_lambda = apgd->GetLambda();
		}
		// ————————————————————————

		if (myLagrangeDebugObserver)
		{
			actualStep.l = myLcp->getLambda();
		}

		// The constraint resources store their current lagrangian multipliers
		for (RBDConstraintResource* constraintRes : myConstraintResources)
		{
			constraintRes->updateEffectiveForce(myLcp->getLambda(), delta_t);
		}

		for (std::list<RBDContact*>::iterator it = allHandledContactsInCluster.begin(); it != allHandledContactsInCluster.end(); ++it)
		{
			(*it)->updateEffectiveForce(myLcp->getLambda(), delta_t);
		}



		// 2) —— 紧跟其后：统计并落盘 ——
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
		//qDebug() << "matA = " << matA;
		//qDebug() << "b = " << b;/*
		//qDebug() << "lambdaLow = " << lambdaLow;
		//qDebug() << "lambdaHigh = " << lambdaHigh;*/
		//qDebug() << "lambda = " << myLcp->getLambda();
		v_new = MInv_JT * myLcp->getLambda() + MInv_dt_fext;

		if (myLagrangeDebugObserver)
		{
			actualStep.v_new = v_new;
		}

		// Den einzelnen Bodies ihre neue Geschwindigkeit mitteilen
		// Das Update auf die neue Position wird wieder zentral in der Scene vorgenommen.
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

	double RBDClusterLagrangeMultipliers::getCNDNr()
	{
		return CND_Nr;
	}

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