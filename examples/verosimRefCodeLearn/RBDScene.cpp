#include "../RBDScene.h"

#include <omp.h>

//#include "../../../Lib/VSL/VSLPerformanceMeasurement.h"
#include "../../../Lib/VSM/VSMVectorN.h"
#include "../../../Lib/VSM/VSMMatrixBool.h"

// #include "../DynamXCollisionGrid.h"
#include "../RBDMaterial.h"
#include "../RBDJointRigid.h"
#include "../RBDShape.h"
#include "../RBDRigidBody.h"
#include "../RBDRigidBodyList.h"

// integration algorithm modularity
#include "../RBDDynamXState.h"
#include "../RBDDynamXStateDerivative.h"
#include "../RBDIntegrationAlgorithmEulerExplicit.h"
#include "../RBDIntegrationAlgorithmEulerSemiImplicit.h"
#include "../RBDIntegrationAlgorithmHeun.h"
#include "../RBDIntegrationAlgorithmLeapfrog.h"
#include "../RBDIntegrationAlgorithmRK4.h"
#include "../RBDIntegrationAlgorithmAdamsBashforth.h"
#include "../RBDIntegrationAlgorithmAdamsMoulton.h"
#include "../RBDIntegrationAlgorithmStepWidthControl.h"

#include "../RBDRigidBodyVariableMass.h"
#include "../RBDClusterLagrangeDebugObserver.h"

#include "./RBDColliderGroup.h"
#include "./RBDClusterLagrangeMultipliers.h"
#include "./RBDClusterLagrangeMultipliersInteractive.h"
#include "./RBDClusterImpulseBased.h"
#include "./RBDClusterPenalty.h"
#include "../RBDClusterSequentialImpulse.h"
#include "../RBDSolverAPGD.h"
#include "../RBDSolverAPGD_MF.h"
#include "../RBDChronoNSC.h"


#include "../RBDCollisionDetectionAlgorithm.h"
#include "../RBDCollisionDetectionBroadPhaseAlgorithmAABB.h"
#include "../RBDCollisionDetectionBroadPhaseAlgorithmGrid.h"
#include "../RBDCollisionDetectionBroadPhaseAlgorithmSweepAndPrune.h"

#include "./RBDCollisionDetectionNarrowPhaseBoxBox.h"
#include "./RBDCollisionDetectionNarrowPhaseBoxSphere.h"
#include "./RBDCollisionDetectionNarrowPhaseBoxPlane.h"
#include "./RBDCollisionDetectionNarrowPhaseBoxCylsphere.h"
#include "./RBDCollisionDetectionNarrowPhaseCylsphereCylsphere.h"
#include "./RBDCollisionDetectionNarrowPhaseCylsphereCylsphereHole.h"
#include "./RBDCollisionDetectionNarrowPhaseCylsphereSphere.h"
#include "./RBDCollisionDetectionNarrowPhaseCylspherePlane.h"
#include "./RBDCollisionDetectionNarrowPhaseEllipsoidPlane.h"
#include "./RBDCollisionDetectionNarrowPhasePolyhedronPolyhedron.h"
#if !defined(Q_OS_QNX)
#include "./RBDCollisionDetectionNarrowPhaseKdopBvhKdopBvh.h"
#endif
#include "./RBDCollisionDetectionNarrowPhaseSpherePlane.h"
#include "./RBDCollisionDetectionNarrowPhaseSphereSphere.h"

#include "../../Lib/VSD/VSDManager.h"
#include <chrono>

using namespace VSLibRBDynamX;

//#define PERFORMANCESUITE_ENABLED
#include "../../VSLibOpenCV/VSLibOpenCVPerformanceSuite.h"

#include <QSemaphore>
#include <QTime>
#include <QMutex>
#include <QtConcurrent>

#ifdef RBD_DEBUG
QAtomicInt RBDScene::insts = 0;
#endif

QHash<int, RBDScene*> RBDScene::staticSimStateSceneMap = QHash<int, RBDScene*>();

RBDScene::RBDScene()
   : SceneElement()
   , myGravity(0, 0, double(-9.8065))
   , myConstraintSolverType(CST_LAGRANGEDANTZIG)
   , myNumberIterations(100)
   , myGlobalCfm(double(0.0))
   , myStribeckVelocity(0.001)
   , myGaussianExponent(2.0)
   , myTau(0.0)
   , myHysteresisFactor(1.0)
   , myBaumgarteStabilizationConstant(double(0.1))
   , myGenerateCollisionSignals(0)
   , myMinimumCollisionVelocityForSignal(double(0.1))
   , mySupportAirDrag(false)
   , myResizeGridAtRuntime(false)
   , myErrorCorrectionMethod(RBDScene::ECM_BAUMGARTE)
   , myIgnoreGyroTerm(false)
   , myFlags(0x0)
   , myNumberFoundContacts(0)
   , myCurrentDt(0.0)
   , myCurrentNewTime(0.0)
   , myFrictionModel(FRICTIONTYPE::FT_DEFAULT)
   , myContactModel(CONTACTMODELTYPE::CMT_DEFAULT)
   , myLinkageToLCP(LINKAGE_TO_LCP::LINEARISATION_CFM)
   , maxDistContactMatching(0.1)
   , timestempsSinceResetCalled(0)
   , timestepsToWaitAfterReset(0)
   , currentSimulationState(nullptr)
   , mySimState(nullptr)
   , useBilateralFrictionCone(true)
   , calculateConditionNumber(false)
   , conditionNumber(0.0)
   , conditionNumberLastStep(0.0)
   , numberOfThreads(1)
{
#ifdef DOTIMING
   QueryPerformanceFrequency(&ticksPerSecond);
#endif
   myColliderGroups = new RBDColliderGroupPtrSet();
   myCDAlgorithm = new VSLibRBDynamX::RBDCollisionDetectionAlgorithm(this);
   //   increaseContactsBufferBy(2000);

#ifdef RBD_DEBUG
   insts.ref();
#endif

#ifdef DOTIMING
   myNumberDoneSteps = 0;
   myNumberStepsPerMean = 100;
#endif

//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseBoxBox(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePlane"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseBoxPlane(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseBoxSphere(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseBoxCylsphere(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseCylsphereCylsphere(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseCylsphereSphere(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphereHole"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseCylsphereCylsphereHole(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePlane"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseCylspherePlane(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseSphereSphere(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePlane"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseSpherePlane(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeEllipsoid"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePlane"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseEllipsoidPlane(this));
//
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePolyhedron"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePolyhedron"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhasePolyhedronPolyhedron(this));
//
//#if !defined(Q_OS_QNX)
//   registerCDNarrowPhaseAlgorithm(
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeKdopBvh"),
//      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeKdopBvh"),
//      new VSLibRBDynamX::RBDCollisionDetectionNarrowPhaseKdopBvhKdopBvh(this));
//#endif
}

RBDScene::~RBDScene()
{
#ifdef RBD_DEBUG
   insts.deref();
#endif

   delete myColliderGroups;

   static QAtomicPointer<QMutex> mutex = 0; //qt5? Noch mal gucken, ob das mit dem QAtomicPointer und den load()s so bleiben soll.
   if (!mutex.loadRelaxed())
   {
      QMutex* m = new QMutex();
      if (!mutex.testAndSetOrdered(0, m))
         delete m;
   }

   mutex.loadRelaxed()->lock();
   QHash<int, RBDScene*>::iterator it = staticSimStateSceneMap.begin();
   while (it != staticSimStateSceneMap.end())
   {
      if ((*it) == this)
      {
         staticSimStateSceneMap.erase(it);
         break;
      }

      ++it;
   }

   mutex.loadRelaxed()->unlock();
   QMutex* m = mutex.fetchAndStoreOrdered(0);
   if (m)
      delete m;

   //for (RBDCollisionDetectionNarrowPhaseAlgorithm* algo : myCDNarrowPhaseAlgos)
   //{
   //   VSL_DELETE(algo);
   //}

   //if (!myCDNarrowPhaseAlgos.empty())
   //   myCDNarrowPhaseAlgos.clear();

   delete myCDAlgorithm;
   myCDAlgorithm = nullptr;

   //if (myCollisionCandidatesMap)
   //   delete myCollisionCandidatesMap;
}

//void RBDScene::insertColliderGroupsIntoCollisionDetection(bool resizeGrid)
//{
//   Q_UNUSED(resizeGrid);
//
//   if (!myCDAlgorithm)
//      return;
//
//   if (myColliderGroups->isEmpty())
//      return;
//
//   for (RBDColliderGroup* cg : *myColliderGroups)
//   {
//      myCDAlgorithm->insertElement(cg);
//   }
//   myCDAlgorithm->elementSetHasChanged();
//}

void RBDScene::postAssembly()
{
   storeCurrentAsInitialSimulationState();

   compileRootRigidBodySet();
   compileMainActorsSet();
   compileColliderGroupSet();
   //insertColliderGroupsIntoCollisionDetection(true);

   //if (myCDAlgorithm)
   //   myCDAlgorithm->compileCollisionCandidatesMap();

   myCDAlgorithm->updateCDBroadPhaseAlgorithm();
   currentSimulationState = new DynamXState(myRootRigidBodies);

   integrationAlgorithm = createIntegrationAlgorithm(myIntegrationAlgorithm);

   myIntegrationAlgorithmStepWidthControl = new IntegrationAlgorithmStepWidthControl(0.01, false, true);
}

void RBDScene::generateCollisionSignals(RBDContact** contacts, int count)
{
   //   VSM::MatrixBool alreadySent(myColliderGroups.size(), false);

   RBDColliderGroup* cg1 = 0;
   RBDColliderGroup* cg2 = 0;

   QSet<QPair<RBDColliderGroup*, RBDColliderGroup*>> pairsSentThisCycle;

   for (int i = 0; i < count; ++i)
   {
      RBDContact* c = contacts[i];

      cg1 = c->body(0)->getColliderGroup();
      cg2 = c->body(1)->getColliderGroup();

      if (fabs(c->getRelativeNormalSpeedNorm()) < myMinimumCollisionVelocityForSignal) // ab 5 cm/s Kollisionsgeschwindigkeit wird gefeuert.
         continue;

      if (cg1 > cg2)
      {
         RBDColliderGroup* h = cg2;
         cg2 = cg1;
         cg1 = h;
      }

      if (pairsSentThisCycle.contains(QPair<RBDColliderGroup*, RBDColliderGroup*>(cg1, cg2)))
         continue;

      if (myCollisionPairsSentLastCycle.contains(QPair<RBDColliderGroup*, RBDColliderGroup*>(cg1, cg2)))
         continue;

      emit signalCollision(c->body(0), c->body(1), c->position(), fabs(c->getRelativeNormalSpeedNorm()));

      pairsSentThisCycle.insert(QPair<RBDColliderGroup*, RBDColliderGroup*>(cg1, cg2));
   }

   myCollisionPairsSentLastCycle = pairsSentThisCycle;
}

// 被委托/转发函数：积分器基类不自己算导数，而是把活儿交给场景 RBDScene 去算
// 来自IntegrationAlgorithmBase::calculateDerivative这个基类实现的转发函数
DynamXStateDerivative* RBDScene::calculateDerivative(DynamXState* currentState, double dt, double newTime)
{
   // 新建“状态导数/增量”的容器（加速度、广义速度增量、等式/不等式约束残差等最终都会写进它）。
   // 本行只是分配，还没填内容。
   DynamXStateDerivative* currentDerivative = new DynamXStateDerivative();

   // 让每个刚体开启“一帧的新循环”：清外力/外矩累加、清上一帧标志/缓存（比如接触“已处理”标记）、准备本帧暂存区。
   // 确保导数计算用的是干净基线。
   for (int i = 0; i < (int)myRootRigidBodies.size(); ++i)
   {
      myRootRigidBodies[i]->initNewCycle();
   }

   // 把 currentState 里的 q、v（位置/姿态、线/角速度）反写到刚体对象。
   // 这一步把“状态快照”与“场景里的实体刚体”对齐，后续所有基于刚体的计算才有统一基准。
   // get current simulation state at the beginning of every simulation step
   currentState->setCurrentStateToPtrList(myRootRigidBodies);

   // 用当前速度做一次位姿预测
   doBodiesPositionUpdate(dt);
   // 更新所有约束资源（接触、关节、限位等）
   doConstraintResourcesUpdate(dt, newTime, false);

   // Qt 信号：告诉外部“导数计算开始了”（UI、记录器、插件可据此挂钩）。
   emit signalPreDerivativeCalculation(dt);

   // 若外部改了刚体的碰撞形状、质量、所属“碰撞分组”等，把 BodyConfigDirty 置位
   // 统一重新分配/更新刚体的 colliderGroup、联合体等资源，保证这一帧前配置一致。
   if (getFlag(RBDScene::BodyConfigDirty))
   {
      slotDoConfigUpdate();
   }

   //for (int i=0; i<(int)myRootRigidBodies.size(); ++i)
   //{
   //   myRootRigidBodies[i]->initNewCycle();
   //}

   for (RBDRigidBodyPtrSet::iterator it = myRootRigidBodies.begin(); it != myRootRigidBodies.end(); it++)
   {
      // Check if current RB is a variable mass body
      RBDRigidBody* curBody = (*it);
      if (curBody->instanceCast<RBDRigidBodyVariableMass*>())
      {
         RBDRigidBodyVariableMass* varMassBody = curBody->instanceCast<RBDRigidBodyVariableMass*>();

         double currentMass = varMassBody->getMass();

         // check if current Mass is greater then minimal mass
         if (currentMass >= varMassBody->getMinMass())
         {
            QList<RBDRigidBodyAdapterVariableMass*> varMassAdapters = varMassBody->getVarMassAdapter();

            // iterate over all adapters attachted to rigid bodies
            double dmdtSum = 0.0;
            for (QList<RBDRigidBodyAdapterVariableMass*>::iterator it = varMassAdapters.begin(); it != varMassAdapters.end(); it++)
               dmdtSum += (*it)->getCurrentDeltaMass();

            double scalingFactorDmdt = 1.0;
            if (currentMass + dmdtSum * dt < varMassBody->getMinMass())
            {
               double dmdt_possible = (currentMass - varMassBody->getMinMass()) / dt;
               scalingFactorDmdt = fabs(dmdt_possible / dmdtSum);
            }

            for (QList<RBDRigidBodyAdapterVariableMass*>::iterator it = varMassAdapters.begin(); it != varMassAdapters.end(); it++)
            {
               double v_rel = (*it)->getCurrentDeltaV();
               double dmdt = scalingFactorDmdt * (*it)->getCurrentDeltaMass();

               VSM::Frame frameAdapter = varMassBody->getWorldFrameOfAdapterVariableMass((*it));
               VSM::Vector3 position = frameAdapter.getPosition();
               VSM::Vector3 dirForce = frameAdapter.baseTrafo(VSM::Vector3(0, 0, 1));
               varMassBody->addForceOffCenter(position, v_rel * dmdt * dirForce);
            }
         }
      }
   }

   {
      //VSL_PERFORMANCE_MEASUREMENT_CHILD_BLOCK(QStringList() << __FUNCTION__ << "Collision detection");
      //static long long totaltime = 0;
      //auto start = std::chrono::high_resolution_clock::now();
      myNumberFoundContacts = doCollisionDetection(dt, newTime);
      //auto end = std::chrono::high_resolution_clock::now();
      //auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
      //totaltime += duration;
      //qDebug() << "CD Time: " << totaltime << " µs" ;
   }

   myFoundContacts = myCDAlgorithm->getContacts();

   /*emit signalCollisionDetectionReady();*/

   if (myGenerateCollisionSignals && myCDAlgorithm)
      generateCollisionSignals(myCDAlgorithm->getContacts(), myNumberFoundContacts);

   DXForAllDirectChildren(RBDConstraintResource*, res, this)
   {
      res->updatePreInverseDynamics(dt, newTime);
   }

   if (calcPathDistancesToMainActors())
   {
      initNewGraphAlgoRun();

      foreach(RBDRigidBody * mainActor, myMainActors)
      {
         QList<VSLibRBDynamX::RBDRigidBody*> list = mainActor->bsCalculateDistances(Edge::SameCluster, Edge::SameCluster, false, false);
         foreach(VSLibRBDynamX::RBDRigidBody * b, list) b->initNewGraphAlgorithmRun();
      }
   }

   // Bestimmung der Höhe in einem Stapel
   doStackHeightComputation();

   // Add the rigid bodies to the cluster list.
   doClustering();

   //VSL_PERFORMANCE_MEASUREMENT_CHILD_BLOCK(QStringList() << __FUNCTION__ << "Inverse dynamics");

   // Use LCP solver to calculate the lambda and velocity
   doInverseDynamics(dt, newTime);

   // Reset the clusters
   foreach(RBDCluster * cluster, myClusters)
   {
      delete cluster;
   }
   myClusters.clear();

   // Rücksetzen der Kontakte (Stack, nicht Heap!):
   if (myCDAlgorithm)
   {
      for (int i = 0; i < myNumberFoundContacts; ++i)
      {
         myFoundContacts[i]->setDisabled(false);
         myFoundContacts[i]->setHandled(false);
      }
   }

   //get values from the root rigid bodies to calculate the derivative
   for (RBDRigidBodyPtrSet::iterator it = myRootRigidBodies.begin(); it != myRootRigidBodies.end(); it++)
   {
      unsigned int currentID = (*it)->getRigidBodyID();

      //qDebug() << "ID: " << currentID << " previousVelocity: " << (*it)->getLastTwist();
      VSM::Vector6 currentVelocity = (*it)->getTwist();
      //qDebug() << "ID: " << currentID << " currentVelocity: " << currentVelocity;

      VSM::Vector6 previousVelocity = (*it)->getLastTwist();

      VSM::Vector3 deltaPosition = previousVelocity.getV();

      VSM::Quaternion previousOrientation = (*it)->getAbsPose().getOrientation();
      VSM::Quaternion deltaOrientation = 0.5 * VSM::Quaternion(0, previousVelocity.getW()) * previousOrientation;

      //qDebug() << "ID: " << currentID << " dt = " << dt << " delta velocity: " << currentVelocity - previousVelocity;
      VSM::Vector6 deltaVelocity = (currentVelocity - previousVelocity) / dt;
      //qDebug() << "ID: " << currentID << " dt = " << dt << " delta velocity: " << deltaVelocity;

      rigidBodyDynamXStateDerivative currentRBDerivative;
      currentRBDerivative.deltaPosition = deltaPosition;
      currentRBDerivative.deltaOrientation = deltaOrientation;
      currentRBDerivative.deltaVelocity = deltaVelocity;

      // Check if current RB is a variable mass body
      RBDRigidBody* curBody = (*it);
      if (curBody->instanceCast<RBDRigidBodyVariableMass*>())
      {
         RBDRigidBodyVariableMass* varMassBody = curBody->instanceCast<RBDRigidBodyVariableMass*>();

         double currentMass = varMassBody->getMass();

         // check if current Mass is greater then minimal mass
         if (currentMass >= varMassBody->getMinMass())
         {
            QList<RBDRigidBodyAdapterVariableMass*> varMassAdapters = varMassBody->getVarMassAdapter();

            // iterate over all adapters attachted to rigid bodies
            double dmdtSum = 0.0;
            for (QList<RBDRigidBodyAdapterVariableMass*>::iterator it = varMassAdapters.begin(); it != varMassAdapters.end(); it++)
               dmdtSum += (*it)->getCurrentDeltaMass();

            double scalingFactorDmdt = 1.0;
            if (currentMass + dmdtSum * dt < varMassBody->getMinMass())
            {
               double dmdt_possible = (currentMass - varMassBody->getMinMass()) / dt;
               scalingFactorDmdt = fabs(dmdt_possible / dmdtSum);
            }
            for (QList<RBDRigidBodyAdapterVariableMass*>::iterator it = varMassAdapters.begin(); it != varMassAdapters.end(); it++)
            {
               double dmdt = scalingFactorDmdt * (*it)->getCurrentDeltaMass();
               VSM::Matrix3x3 currentInertiaWorld = varMassBody->getInertiaW();
               double factor = dmdt / currentMass;
               VSM::Matrix3x3 deltaInertia = currentInertiaWorld * factor;

               currentRBDerivative.deltaMass += dmdt;
               currentRBDerivative.deltaInertia += deltaInertia;
            }
         }
      }
      else
      {
         currentRBDerivative.deltaMass = 0;
         currentRBDerivative.deltaInertia = VSM::Matrix3x3();
      }

      currentDerivative->setDynamXStateDerivative(currentID, currentRBDerivative);
   }

   return currentDerivative;
}

void RBDScene::simulate(
   double delta_t,
   double newTime)
{
   //VSL_PERFORMANCE_MEASUREMENT_ROOT_BLOCK(QStringList() << __FUNCTION__);
   //VSL_PERFOMANCE_MEASUREMENT_OUTPUT(100);

   //时间步长 和 绝对时间戳 存到场景成员里，方便其他子系统（日志/回调/可视化）查询。
   myCurrentDt = delta_t;
   myCurrentNewTime = newTime;

   // Qt 信号：告诉外部“这一帧的仿真开始了”（UI、记录器、插件可据此挂钩）。
   emit beginSimulateFunction();

   // get current simulation state at the beginning of every simulation step
   //currentSimulationState->setCurrentStateToPtrList(myRootRigidBodies);
   // 把散落在各个 RBDRigidBody 里的 q/v 打包成一个大状态向量，供积分器使用
   currentSimulationState->updateCurrentStateFromPtrList(myRootRigidBodies);
   //currentSimulationState->qDebugDynamXState();

   // 若外部改了刚体的碰撞形状、质量、所属“碰撞分组”等，把 BodyConfigDirty 置位
   // 统一重新分配/更新刚体的 colliderGroup、联合体等资源，保证这一帧前配置一致。
   // When BodyConfigDirty is set, the rigid body's union and colliderGroup are reallocated in the simulate function.
   if (getFlag(RBDScene::BodyConfigDirty))
   {
      slotDoConfigUpdate();
   }

   // 帧内初始化（清累计量、统计器、缓冲等）
   for (int i = 0; i < (int)myRootRigidBodies.size(); ++i)
   {
      myRootRigidBodies[i]->initNewCycle();
   }

   //myNumberFoundContacts = doCollisionDetection(delta_t, newTime);
   //myFoundContacts = myCDBroadPhaseAlgorithm->myContacts;

   // Qt 信号：告诉外部“碰撞检测算好了”（UI、记录器、插件   
   emit signalCollisionDetectionReady();

   // 自适应步长控制（可选）
   // 这里是预留的“自适应步长控制”入口（目前被硬编码为 false 未启用）。
   // check if adaptive stepwidth control should be used
   if (false)
      delta_t = myIntegrationAlgorithmStepWidthControl->adjustStepsize(currentSimulationState);

   // 被注释的）基于约束误差的粗略 dt 调整
   // 应该是自适应步长失败，所以这部分暂且就没开启了。
   //if (constraintError > 5e-4)
   //{
   //   timeStepFactor /= 2;
   //   timeStepFactor = qMax(timeStepFactor, 0.1);
   //   delta_t = delta_t*timeStepFactor;
   //}
   //else
   //{
   //   if (timeStepFactor != 1)
   //      timeStepFactor *= 2;
   //   timeStepFactor = qMin(timeStepFactor, 0.1);
   //}

   // 调用积分器做“状态推进”
   // 
   // calculate new state, based on integration algorithm
   DynamXState* newState = integrationAlgorithm->performeIntegrationStep(currentSimulationState, delta_t, newTime);

   // 用新状态写回刚体对象
   currentSimulationState = newState;
   currentSimulationState->setCurrentStateToPtrList(myRootRigidBodies);

   // 统一做位姿（位置/姿态）更新，积分位置
   doBodiesPositionUpdate(delta_t);

   // 更新所有约束资源（接触、关节、限位等）
   // Standardlösung - schön aber langsam? --> Lauf über ALLE Kontakt instanzen, ob in use oder nicht!
   doConstraintResourcesUpdate(delta_t, newTime, true);

   // 结束信号
   emit endSimulateFunction();
}

void RBDScene::undoStep()
{
   for (int i = 0; i < (int)myRootRigidBodies.size(); ++i)
   {
      myRootRigidBodies[i]->undoLastPositionUpdate();
      myRootRigidBodies[i]->undoLastVelocityUpdate();
   }
}

void RBDScene::saveStep()
{
   //PERFORMANCESUITE_TIC("Save step / updateVisu");
   for (int i = 0; i < myRootRigidBodies.size(); ++i)
   {
      myRootRigidBodies[i]->updateVisu();
   }

#ifdef RBD_DEBUG
   //   qDebug() << "VSLibRBDynamX: SaveStep finished!\n";
#endif

   //PERFORMANCESUITE_TOC("Save step / updateVisu");
#ifdef DOTIMING
   signalEndCycle();
#endif
}

void RBDScene::addMaterial(RBDMaterial* mat)
{
   myMaterials.add(mat);
}

RBDRigidBody* RBDScene::findRigidBodyByName(const QString& name)
{
   for (RBDRigidBody* b : myRootRigidBodies)
   {
      if (b->getName() == name)
         return b;
   }
   return 0;
}

void RBDScene::removeRootRigidBody(RBDRigidBody* rb)
{
   myRootRigidBodies.remove(rb);

   if (rb->isMainActor())
   {
      myMainActors.remove(rb);
   }
}

void RBDScene::addRootRigidBody(RBDRigidBody* rb)
{
   myRootRigidBodies.add(rb);

   if (rb->isMainActor())
   {
      myMainActors.add(rb);
   }
}

void RBDScene::addColliderGroup(RBDColliderGroup* cg)
{
   myColliderGroups->add(cg);
}

void RBDScene::removeColliderGroup(RBDColliderGroup* cg)
{
   myColliderGroups->remove(cg);
}

RBDScene* RBDScene::instanceFor(VSD::SimState* simState)
{
   int simstateID = simState->getID();

   //static QMutex mutex;
   //QMutexLocker locker(&mutex);
   if (staticSimStateSceneMap[simstateID] == nullptr)
   {
      RBDScene* newScene = new RBDScene();
      ((SceneElement*)newScene)->setName("RBDScene");
      newScene->simStateID = simstateID;
      newScene->mySimState = simState;
      staticSimStateSceneMap[simstateID] = newScene;
      return newScene;
   }
   else
      return staticSimStateSceneMap[simstateID];
}

void RBDScene::removeInstanceFor(VSD::SimState* simState)
{
   int simstateID = simState->getID();

   //static QMutex mutex;
   //QMutexLocker locker(&mutex);

   if (staticSimStateSceneMap[simstateID] != nullptr)
      delete staticSimStateSceneMap.value(simstateID);
   staticSimStateSceneMap.remove(simstateID);
}

RBDRigidBody* RBDScene::fixRigidBody() const
{
   //PERFORMANCESUITE_TIC("825 RBDScene::fixRigidBody()");

   /* Q_FOREACH expansion:
   for ( const QForeachContainerBase &_container_ = qForeachContainerNew(container);
         qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();
         ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)
      {
      for ( variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i;
            qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;
            --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)
      {
      }
   }
   */

   // originally:
   //
   //   DXForAllFixRootRigidBodies(fix)
   //   {
   // now: replaced with

   const QList<RBDRigidBody*>& list = myRootRigidBodies.getList();

   for (QList<RBDRigidBody*>::const_iterator itFix = list.constBegin();
      itFix != list.constEnd();
      ++itFix)
   {
      RBDRigidBody* const fix = *(itFix);

      if (!fix->isFix())
      {
         continue;
      }
      else

      {
         ////PERFORMANCESUITE_TIC("825 RBDScene::fixRigidBody, fix->getParent");
         RBDRigidBody* p = fix->getParentRigidBody(true);
         ////PERFORMANCESUITE_TOC("825 RBDScene::fixRigidBody, fix->getParent");

         if (p)
         {
            //PERFORMANCESUITE_TOC("825 RBDScene::fixRigidBody()");
            return p;
         }

         //PERFORMANCESUITE_TOC("825 RBDScene::fixRigidBody()");
         return fix;
      }
   }//end for

   //PERFORMANCESUITE_TOC("825 RBDScene::fixRigidBody()");
   return 0;
}

void RBDScene::emitSignalShowInfo(
   const QString& msg,
   bool asMessageBox)
{
   emit signalShowInfo(msg, asMessageBox);
}

void RBDScene::emitSignalShowWarning(
   const QString& msg,
   bool asMessageBox)
{
   emit signalShowWarning(msg, asMessageBox);
}

int RBDScene::limitPathLengthToMainActorForDeactivation() const
{
   return 10;// myLimitPathLengthToMainActorForDeactivation;
}

void RBDScene::setLimitPathLengthToMainActorForDeactivation(const int& value)
{
   myLimitPathLengthToMainActorForDeactivation = value;
}

bool RBDScene::clustering() const
{
   return myClustering;
}

void RBDScene::setClustering(bool value)
{
   myClustering = value;
}

void RBDScene::setShockPropagation(SHOCKPROPAGATION value)
{
   myShockPropagation = value;
}

void RBDScene::setShockPropagation(const QString& value)
{
   if (value.toLower().trimmed() == "correct")
   {
      setShockPropagation(RBDScene::SP_CORRECT);
   }
   else if (value.toLower().trimmed() == "simple")
   {
      setShockPropagation(RBDScene::SP_SIMPLE);
   }
   else
   {
      setShockPropagation(RBDScene::SP_NONE);
   }
}

RBDScene::SHOCKPROPAGATION RBDScene::getShockPropagationMethod() const
{
   return myShockPropagation;
}

RBDScene::CONTACTMODELTYPE RBDScene::getContactModelType()
{
   return myContactModel;
}

RBDScene::FRICTIONTYPE RBDScene::getFrictionModel()
{
   return myFrictionModel;
}

void RBDScene::setContactModelType(QString str)
{
   str = str.toLower().trimmed();

   if (str == "default")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_DEFAULT);
   }
   else if (str == "hard")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_HARDCONTACT);
   }
   else if (str == "springdamper")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_SPRINGDAMPER);
   }
   else if (str == "hertz")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_HERTZ);
   }
   else if (str == "nonlinearspringdamper")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_NONLINEARSPRINGDAMPER);
   }
   else if (str == "anagnostopoulos")
   {
      // TO DO
      setContactModelType(CONTACTMODELTYPE::CMT_ANAGNOSTOPOULOS);
   }
   else if (str == "huntcrossley")
   {
      // TO DO
      setContactModelType(CONTACTMODELTYPE::CMT_HUNTCROSSLEY);
   }
   else if (str == "flores")
   {
      setContactModelType(CONTACTMODELTYPE::CMT_FLORES);
   }
   else
   {
      setContactModelType(CONTACTMODELTYPE::CMT_DEFAULT);
   }
}

void RBDScene::setFrictionModelType(QString str)
{
   str = str.toLower().trimmed();

   if (str == "default")
   {
      setFrictionModelType(FRICTIONTYPE::FT_DEFAULT);
   }
   else if (str == "gaussian")
   {
      setFrictionModelType(FRICTIONTYPE::FT_GAUSSIAN);
   }
   else if (str == "dynamic")
   {
      setFrictionModelType(FRICTIONTYPE::FT_DYNAMIC);
   }
   else if (str == "tustin")
   {
      setFrictionModelType(FRICTIONTYPE::FT_TUSTIN);
   }
   else if (str == "hesssoom")
   {
      setFrictionModelType(FRICTIONTYPE::FT_HESS_SOOM);
   }
   else if (str == "viscous")
   {
      setFrictionModelType(FRICTIONTYPE::FT_VISCOUS);
   }
   else if (str == "coulombstrict")
   {
      setFrictionModelType(FRICTIONTYPE::FT_COULOMB_STRICT);
   }
   else
   {
      setFrictionModelType(FRICTIONTYPE::FT_DEFAULT);
   }
}

void RBDScene::setFrictionModelType(FRICTIONTYPE type)
{
   myFrictionModel = type;
}

void RBDScene::setContactModelType(CONTACTMODELTYPE type)
{
   myContactModel = type;
}

RBDScene::LINKAGE_TO_LCP RBDScene::getLinkageToLCP()
{
   return myLinkageToLCP;
}

double RBDScene::getCFMWeighting()
{
   return myCFMWeighting;
}

void RBDScene::setLinkageToLCP(QString str)
{
   str = str.toLower().trimmed();

   if (str == "cfm")
   {
      setLinkageToLCP(LINKAGE_TO_LCP::LINEARISATION_CFM);
   }
   else if (str == "relativevelocity")
   {
      setLinkageToLCP(LINKAGE_TO_LCP::COMPLEMENTARITY_CONSTRAINT);
   }
   else if (str == "externalforce")
   {
      setLinkageToLCP(LINKAGE_TO_LCP::EXTERNAL_FORCE);
   }
}
void RBDScene::setLinkageToLCP(LINKAGE_TO_LCP linkage)
{
   myLinkageToLCP = linkage;
}

void RBDScene::setCFMWeighting(double w)
{
   myCFMWeighting = w;
}

void RBDScene::setMaxAllowedDistanceForContactMatching(double maxDist)
{
   maxDistContactMatching = maxDist;
}

double RBDScene::getMaxAllowedDistanceForContactMatching()
{
   return maxDistContactMatching;
}

bool RBDScene::calcPathDistancesToMainActors() const
{
   return myCalcPathDistanceToMainActors;
}

void RBDScene::setCalcPathDistancesToMainActors(bool value)
{
   myCalcPathDistanceToMainActors = value;
}

void RBDScene::setFrictionConeApproximationParam(int value)
{
   if (value >= 2)
      myFrictionConeApproximationParam = value;
}

void RBDScene::setNumberThreads(int n)
{
#if defined(__unix) || defined(__QNXNTO__)
   ; //VSL_ASSERT_ELSE_RETURN(0);
#else
   //omp_set_nested(1);
   if (n > 0)
   {
      numberOfThreads = n;
      //omp_set_num_threads(n);
   }
#endif

   //if (myContactsThreadWise)
   //   delete[] myContactsThreadWise;

   //myContactsThreadWise = new RBDContact**[n];
   //for (int i=0; i<n; ++i)
   //{
   //   int k = i * myContactsBufferSize / n;
   //   myContactsThreadWise[i] = &myContacts[k];
   //}
}

void RBDScene::doBodiesPositionUpdate(double dt)
{
#if !defined(__unix) && !defined(__QNXNTO__)
#ifdef VSLIBRBDYNAMX_USE_OMP
#pragma omp parallel sections num_threads(3)
   //omp_set_num_threads(2);
   //#pragma omp parallel for schedule(static,(int)myRootRigidBodies.size()%2+1)
#endif
#endif
   {
#if !defined(__unix) && !defined(__QNXNTO__)
#pragma omp section
#endif
      {
         for (int i = 0; i < (int)myRootRigidBodies.size(); ++i)
         {
            //int id = omp_get_thread_num();
            //int numberThreads = omp_get_num_threads();
            //qDebug() << "current thread ID:" << id << "of total threads:" << numberThreads;
            myRootRigidBodies[i]->evolvePose(dt);
         }
      }

      // Innerhalb der DynamX Kollisionserkennung werden nur die ColliderGroups in
      // das Grid eingefügt. Prinzipiell kann aber jedes CollisionDetectionElement
      // eingefügt werden - die VSPluginRBHarvester verwaltet ein eigenes, zusätuliches
      // Grid, in das sie bodies der trees einfügt. Daher muss die Methode hier
      // auf für die boidies aufegrufen werdenm.
#if !defined(__unix) && !defined(__QNXNTO__)
#pragma omp section
#endif
      {
         for (RBDRigidBody* b : myRootRigidBodies)
         {
            b->updateBroadPhaseCollisionDetectionData();
         }
      }

#if !defined(__unix) && !defined(__QNXNTO__)
#pragma omp section
#endif
      {
         for (RBDColliderGroup* cg : *myColliderGroups)
         {
            cg->updateBroadPhaseCollisionDetectionData();
         }
      }
   }
}

void RBDScene::undoBodiesPositionUpdate()
{
   for (int i = 0; i < (int)myRootRigidBodies.size(); ++i)
   {
      myRootRigidBodies[i]->undoLastPositionUpdate();
   }

   for (RBDColliderGroup* cg : *myColliderGroups)
   {
      cg->updateBroadPhaseCollisionDetectionData();
   }
}

void RBDScene::initNewGraphAlgoRun()
{
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->initNewGraphAlgorithmRun();
   }
}

void RBDScene::setGenerateCollisionSignals(bool value)
{
   myGenerateCollisionSignals = value;
}

bool RBDScene::getGenerateCollisionSignals() const
{
   return myGenerateCollisionSignals;
}

#ifdef DOTIMING
void RBDScene::addMeasurement(const QString& name)
{
   myTimeMeasurements[name] += getTimeDiff(name);
}

void RBDScene::signalEndCycle()
{
   ++myNumberDoneSteps;

   if (myNumberDoneSteps % myNumberStepsPerMean)
      return;

   myNumberDoneSteps = 0;

   QString msg;
   msg += "-----------------\n";

   double avgSumTime = 0.0;

   QMap<QString, double>::const_iterator it;
   for (it = myTimeMeasurements.constBegin(); it != myTimeMeasurements.constEnd(); ++it)
   {
      QString line = (it.key()) + ": ";

      double mean = it.value() / (double)myNumberStepsPerMean;
      avgSumTime += mean;

      line += QString::number(mean);
      line += '\n';

      msg += line;
   }
   msg += "Average sum time: ";
   msg += QString::number(avgSumTime);
   msg += '\n';
   msg += "       ---       ";
   msg += '\n';

   myTimeMeasurements.clear();
   emitSignalShowInfo(msg, false);
}
#endif

void RBDScene::setTimestampsToWaitAfterReset(int i)
{
   timestepsToWaitAfterReset = i;
}

RBDContact** RBDScene::getCurrentContacts()
{
   return myFoundContacts;
}

int RBDScene::getNumberCurrentContacts()
{
   return myNumberFoundContacts;
}

void RBDScene::resetCurrentNumberOfContacts()
{
   myNumberFoundContacts = 0;
}

void RBDScene::addRigidBodyInstance(RBDRigidBody* rb)
{
   myRigidBodyInstances.add(rb);
   //int i=0;
   //for(RBDRigidBody* b, myRigidBodyInstances)
   //{
   //   b->setSceneIndex(i++);
   //}
}

void RBDScene::removeRigidBodyInstance(RBDRigidBody* rb)
{
   myRigidBodyInstances.remove(rb);
   //int i=0;
   //for(RBDRigidBody* b, myRigidBodyInstances)
   //{
   //   b->setSceneIndex(i++);
   //}
}

//!(ci) attaches a RBDClusterLagrangeDebugObserver
void RBDScene::attachDebugInstance(RBDClusterLagrangeDebugObserver* LDO)
{
   myLDOPtr = LDO;
}

//!(ci) detaches a RBDClusterLagrangeDebugObserver
void RBDScene::detachDebugInstance()
{
   myLDOPtr = NULL;
}

void RBDScene::compileRootRigidBodySet()
{
   // Delete bodies with corr. flag
   QList<RBDRigidBody*> toBeDeleted;
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      if (b->getFlag(RBDRigidBody::DeleteMe))
      {
         toBeDeleted.append(b);
         continue;
      }
   }

   for (RBDRigidBody* b : toBeDeleted)
      delete b;

   // Aufräumen:
   if (!myRootRigidBodies.isEmpty())
      myRootRigidBodies.clear();

   initNewGraphAlgoRun();

   // alle bodies aus unions (existenzabhängig) direkt unter die szene hängen
   for (RBDRigidBody* bodyUnion : myBodyUnions)
   {
      // Reparent the unions children:
      QList<RBDRigidBody*> helper;
      DXForAllDirectChildren(RBDRigidBody*, child, bodyUnion)
      {
         helper.append(child);
      }

      for (RBDRigidBody* b : helper)
      {
         addChild(b);
      }

      // Delete union
      delete bodyUnion;
   }

   if (!myBodyUnions.isEmpty())
      myBodyUnions.clear();

   // Tiefensuche über die Bodies: Komponenten zusammensuchen
   QList<QList<RBDRigidBody*>*> components;
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      if (b->getInactive())
         continue;

      QList<RBDRigidBody*>* bodysComponent = 0;
      if (!b->getVisited())
      {
         bodysComponent = new QList<RBDRigidBody*>();
         b->dsFindConnectedComponent(bodysComponent, Edge::SameBody, true);
         components.append(bodysComponent);
      }
   }

   // Komponenten, die nur einen Body enthalten: Body direkt in RootBody List einfügen
   // Komponenten, die mehr als einen Body enthalten: Union erzeugen, bodies unterhängen
   for (QList<RBDRigidBody*>* bodysComponent : components)
   {
      if (bodysComponent->empty())
      {
         qDebug() << "Komponentenliste leer! "; // Sollte nicht passieren
      }
      else if (bodysComponent->size() == 1)
      {
         RBDRigidBody* b = (RBDRigidBody*)bodysComponent->first();
         myRootRigidBodies.add(b);
         b->getRigidBodyList()->createRootRigidBodyList();
      }
      else
      {
         RBDRigidBody* bodyUnion = new RBDRigidBody(this, 0);

         myBodyUnions.add(bodyUnion);
         bodyUnion->setName("Union");

         VSM::Vector3 aabbExtension;

         bool collisionDetection = false;
         for (RBDRigidBody* n : *bodysComponent)
         {
            RBDRigidBody* b = (RBDRigidBody*)n;
            bodyUnion->addChild(b);
            collisionDetection |= b->getCollisionDetection();

            if (b->isFix())
            {
               bodyUnion->setIsFix(true);
            }

            // compilek aabb extension:
            for (int i = 0; i < 3; ++i)
            {
               aabbExtension[i] = qMax(b->getCollisionAabbExtension()[i], aabbExtension[i]);
            }
         }

         bodyUnion->updateCogAndMassAndInertiaAndTwist();
         bodyUnion->setCollisionDetection(collisionDetection);
         bodyUnion->setCollisionAabbExtension(aabbExtension);

         myRootRigidBodies.add(bodyUnion);

         RBDRigidBody* b = (RBDRigidBody*)bodysComponent->first();
         b->getRigidBodyList()->createRootRigidBodyList();
      }

      delete bodysComponent;
   }

#ifdef RBD_DEBUG
   //qDebug() << "\nSceneConfig: Root bodies\n";
   //QString conf;
   //for(RBDRigidBody* b: myRootRigidBodies)
   //{
   //   conf += b->getName() + "\n";
   //   conf += "Masse: " + QString::number(b->getMass()) + "\n";
   //   conf += "Inertia: " + b->getInertia().toString()+ "\n";
   //}
   //qDebug() << conf;
#endif

   // Den Bodies ihren Index zuweisen, der 1:1 der Position in myRigidBodyInstances entspricht
   int index = 0;
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->setSceneIndex(index++);
   }
      }

void RBDScene::compileColliderGroupSet()
{
   while (!myColliderGroups->isEmpty())
   {
      delete myColliderGroups->getFirstElement<RBDColliderGroup>();
   }

   // Alle "visited"-flags rücksetzen
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->setVisited(false);
   }

   QList<QList<RBDRigidBody*>*> nodeLists;
   for (RBDRigidBody* b : myRootRigidBodies)
   {
      if (b->getInactive())
         continue;

      if (!b->getVisited())
      {
         QList<RBDRigidBody*>* nodeList = new QList<RBDRigidBody*>();
         b->dsFindConnectedComponent(nodeList, Edge::SameColliderGroup, true);
         nodeLists.append(nodeList);
      }
   }

   for (QList<RBDRigidBody*>* nodeList : nodeLists)
   {
      RBDColliderGroup* cg = new RBDColliderGroup(this);
      cg->setCollisionAabbExtension(VSM::Vector3(.1, .1, .1));

      for (RBDRigidBody* node : *nodeList)
      {
         RBDRigidBody* body = (RBDRigidBody*)node;

         if (!body->getCollisionDetection())
            continue;

         cg->addRigidBody(body);

#ifdef RBD_DEBUG
         cg->setName(cg->getName() + " " + body->getName());
#endif

         DXForAllDirectChildren(RBDRigidBody*, child, body)
         {
            if (child->getCollisionDetection())
               cg->addRigidBody(child);
         }

         //         cg->setCollisionDetection(collisionDetection);
      }

      if (cg->getRigidBodies().isEmpty())
         delete cg;
      else
         myColliderGroups->add(cg);
   }

   /*
   #ifdef RBD_DEBUG
      qDebug() << "\n";
      qDebug() << "ColliderGroups: ";
   #endif
   */
   for (unsigned int i = 0; i < myColliderGroups->size(); ++i)
   {
      (*myColliderGroups)[i]->setSceneIndex(i);

#ifdef RBD_DEBUG
      /*
            qDebug() << "CG: " << myColliderGroups[i]->getName();
            for(RBDRigidBody* b: myColliderGroups[i]->getRigidBodies())
            {
               qDebug() << b->getName();
            }
      */
#endif
}
#ifdef RBD_DEBUG
   //   qDebug() << "\n";
#endif
   }

void RBDScene::storeCurrentAsInitialSimulationState()
{
   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->storeInitialState();
   }

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->storeInitialState();
   }
}

void RBDScene::storeCurrentAsIntermediateSimulationState()
{
   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->storeIntermediateState();
   }

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->storeIntermediateState();
   }
}

void RBDScene::restoreInitialSimulationState()
{
   // Call this function after reset.
   // Unions entfernen
   for (RBDRigidBody* bodyUnion : myBodyUnions)
   {
      // Reparent the unions children:
      QList<RBDRigidBody*> helper;
      DXForAllDirectChildren(RBDRigidBody*, child, bodyUnion)
      {
         helper.append(child);
      }

      for (RBDRigidBody* b : helper)
      {
         addChild(b);
      }

      // Delete union
      delete bodyUnion;
      //      myRigidBodyInstances.remove(bodyUnion);
   }

   if (!myBodyUnions.isEmpty())
      myBodyUnions.clear();

   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->restoreInitialState();
   }

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->restoreInitialState();
   }

   // Rekonfigurieren:
   slotDoConfigUpdate();
}

void RBDScene::restoreIntermediateSimulationState()
{
   // Unions entfernen
   for (RBDRigidBody* bodyUnion : myBodyUnions)
   {
      // Reparent the unions children:
      QList<RBDRigidBody*> helper;
      DXForAllDirectChildren(RBDRigidBody*, child, bodyUnion)
      {
         helper.append(child);
      }

      for (RBDRigidBody* b : helper)
      {
         addChild(b);
      }

      // Delete union
      delete bodyUnion;
      //      myRigidBodyInstances.remove(bodyUnion);
   }

   if (!myBodyUnions.isEmpty())
      myBodyUnions.clear();

   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->restoreIntermediateState();
   }

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->restoreIntermediateState();
   }

   // Rekonfigurieren:
   slotDoConfigUpdate();
}

void RBDScene::restoreInitialSimulationStateDuringRuntime()
{
   timestempsSinceResetCalled = 0;

   // Unions entfernen
   for (RBDRigidBody* bodyUnion : myBodyUnions)
   {
      // Reparent the unions children:
      QList<RBDRigidBody*> helper;
      DXForAllDirectChildren(RBDRigidBody*, child, bodyUnion)
      {
         helper.append(child);
      }

      for (RBDRigidBody* b : helper)
      {
         addChild(b);
      }

      // Delete union
      delete bodyUnion;
      //      myRigidBodyInstances.remove(bodyUnion);
   }

   if (!myBodyUnions.isEmpty())
      myBodyUnions.clear();

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->restoreIntialStateDuringRuntime();
   }

   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->restoreInitialState();
      joint->updatePostBodiesPoseUpdate(myCurrentDt, myCurrentNewTime, true);
   }

   // Rekonfigurieren:
   slotDoConfigUpdate();
}

void RBDScene::restoreIntermediateSimulationStateDuringRuntime()
{
   timestempsSinceResetCalled = 0;

   // Unions entfernen
   for (RBDRigidBody* bodyUnion : myBodyUnions)
   {
      // Reparent the unions children:
      QList<RBDRigidBody*> helper;
      DXForAllDirectChildren(RBDRigidBody*, child, bodyUnion)
      {
         helper.append(child);
      }

      for (RBDRigidBody* b : helper)
      {
         addChild(b);
      }

      // Delete union
      delete bodyUnion;
      //      myRigidBodyInstances.remove(bodyUnion);
   }

   if (!myBodyUnions.isEmpty())
      myBodyUnions.clear();

   // Bodies zurücksetzen:
   for (RBDRigidBody* b : myRigidBodyInstances)
   {
      b->restoreIntermediateStateDuringRuntime();
   }

   // Joints zurücksetzen
   DXForAllDirectChildren(RBDJoint*, joint, this)
   {
      joint->restoreIntermediateState();
      joint->updatePostBodiesPoseUpdate(myCurrentDt, myCurrentNewTime, true);
   }

   // Rekonfigurieren:
   slotDoConfigUpdate();
}

void RBDScene::createJointsBetweenAllFixBodies()
{
   RBDRigidBody* firstFix = 0;
   DXForAllFixRootRigidBodies(fix)
   {
      if (!firstFix)
      {
         firstFix = fix;
         continue;
      }

      RBDJointRigid* rigid = new RBDJointRigid(VSM::PoseVector3Quaternion(), VSM::PoseVector3Quaternion(), firstFix, fix, 0);
      rigid->setFlag(RBDJoint::Unite, true);
   }
}

RBDScene::CONSTRAINTSOLVERTYPE RBDScene::constraintSolverType() const
{
   return myConstraintSolverType;
}

void RBDScene::setConstraintSolverType(const RBDScene::CONSTRAINTSOLVERTYPE& value)
{
   myConstraintSolverType = value;
}

VSG::BoundingBox RBDScene::compileSceneAABB(bool ignoreInfiniteElements) const
{
   VSG::BoundingBox universe;
   universe.setToUniverse();

   VSG::BoundingBox aabb;
   aabb.invalidate();

   for (RBDRigidBody* b : myRootRigidBodies)
   {
      //      b->loadCollisionHullFor(&universe, 0);

      if (!b->getAABB()->isFinite() && ignoreInfiniteElements)
      {
         continue;
      }

      aabb.expandBy(*b->getAABB());
      //      b->unloadCollisionHull();
   }

   return aabb;
}

void RBDScene::setCollisionBroadPhase(COLLISIONBROADPHASE cb)
{
   myCDAlgorithm->chooseCDBroadPhaseAlgorithm(cb);
   myCDAlgorithm->updateCDBroadPhaseAlgorithm();
}

int RBDScene::getFrictionConeApproximationParam() const
{
   return myFrictionConeApproximationParam;
}

RBDScene::ERRORCORRECTIONMETHOD RBDScene::getErrorCorrectionMethod() const
{
   return myErrorCorrectionMethod;
}

void RBDScene::setErrorCorrectionMethod(RBDScene::ERRORCORRECTIONMETHOD m)
{
   myErrorCorrectionMethod = m;
}

int RBDScene::doCollisionDetection(double dt, double newTime)
{
   //if (!myCDAlgorithm)
   //   return 0;
   ////qDebug() << "CD is working now" << newTime << "current time";
   //int result = myCDAlgorithm->findContacts(2000, dt, newTime);
   //for (int i=0; i<result; ++i)
   //{
   //   myCDAlgorithm->getContacts()[i]->compile(dt, newTime);
   //}
   ////qDebug() << "CD is finished now" << newTime << "current time";

   //return result;
   return myCDAlgorithm->doCollisionDetection(2000, dt, newTime);
}

void RBDScene::doSaveStateVariables()
{
   if (!myCDAlgorithm)
      return;
   myCDAlgorithm->saveContactStateVariables();
}

void RBDScene::doStackHeightComputation()
{
   // TBD
   // Nimmt gegenwärtig an, dass es nur einen fixen Body (den Boden) gibt.
   // Gibt es mehrere, ist das Verhalten nicht deterministisch, weil nicht klar
   // ist, welcher fixe Body genutzt wird.
   if (getShockPropagationMethod() != SP_NONE)
   {
      initNewGraphAlgoRun();

      RBDRigidBody* fix = fixRigidBody();
      if (fix)
      {
         QList<VSLibRBDynamX::RBDRigidBody*> list = fix->bsCalculateDistances(Edge::IsContact, Edge::IsContact, true);
         for (VSLibRBDynamX::RBDRigidBody* b : list) b->initNewGraphAlgorithmRun();
      }
   }
}

RBDCluster* RBDScene::createNewCluster()
{
   const CONSTRAINTSOLVERTYPE t = this->constraintSolverType();
   if (t == CST_IMPULSEBASED)
   {
      return new RBDClusterImpulseBased(this);
   }
   else if (t == CST_PENALTY)
   {
      return new RBDClusterPenalty(this);
   }
   else if (
      t == CST_LAGRANGEINTERACTIVEDANTZIG
      || t == CST_LAGRANGEINTERACTIVEGS
      || t == CST_LAGRANGEINTERACTIVEAUTO)
   {
      return new RBDClusterLagrangeMultipliersInteractive(this);
   }
   //(ci) use the attached LagrangeDebugObserver
   else if (myLDOPtr)
   {
      return new RBDClusterLagrangeMultipliers(this, myLDOPtr);
   }
   else if (t == CST_SEQUENTIALIMPULSE) {
      return new RBDClusterSequentialImpulse(this);
   }
   else if (t == CST_LAGRANGEAPGD)
   {
       // 这里直接 new RBDClusterLagrangeMultipliers 而不是 Interactive
       return new RBDClusterLagrangeMultipliers(this);
   }
   else if (t == CST_MFAPGD) {
       return new RBDChronoNSC(this);
   }
   else
   {
      return new RBDClusterLagrangeMultipliers(this);
   }
}

void RBDScene::doClustering()
{
   if (!clustering())
   {
      RBDCluster* cluster = createNewCluster();

      myClusters.append(cluster);

      for (RBDRigidBody* b : myRootRigidBodies)
      {
         if (!b->getInactive())
            cluster->addRigidBody(b);
      }

      // Shock Propagation fehlt noch bei !clustering
   }
   else
   {
      initNewGraphAlgoRun();

      DXForAllFixRootRigidBodies(fix)
      {
         // Bei der Breitensuche darf der Algorithmus nicht mehr über die fix laufen! -->
         fix->setColor(RBDRigidBody::BLACK);
      }

      DXForAllFixRootRigidBodies(fix)
      {
         PtrList<RBDContact> fixBodyContacts = fix->getContacts(true);
         PtrList<RBDContact>::iterator it = fixBodyContacts.begin();

         //         fix->setColor(RBDRigidBody::BLACK);

         while (it != fixBodyContacts.end())
         {
            RBDContact* contact = *it;

            RBDRigidBody* other = contact->getLeftNode(true);
            if (other == fix)
               other = contact->getRightNode(true);

            QList<RBDRigidBody*> component;
            other->bsFindConnectedComponent(&component, Edge::SameCluster, true);

            if (component.size() > 0)
            {
               RBDCluster* cluster = createNewCluster();

               myClusters.push_back(cluster);

               // Fix body einfügen:
               cluster->addRigidBody(fix);
               // Alle bodies aus component in cluster einfügen:
               for (RBDRigidBody* n : component)
               {
                  cluster->addRigidBody(n);
               }
            }

            ++it;
         }
      }

      QSet<RBDRigidBody*> leftBodies; // Hier hinein kommen alle die bodies, die keinerlei constraint unterliegen

      // Die jetzt verbliebenen Bodies werden ebenfalls geclustert - mangels Fix jedoch
      // ohne shock propagation!
      for (RBDRigidBody* b : myRootRigidBodies)
      {
         if (b->getCluster())
            continue;

         //         initNewGraphAlgoRun();

         QList<RBDRigidBody*> comp;
         b->bsFindConnectedComponent(&comp, Edge::SameCluster, true);

         if (comp.size() < 1)
         {
            continue;
         }
         else if (comp.size() == 1)
         {
            leftBodies.insert(comp.first());
         }
         else
         {
            RBDCluster* cluster = createNewCluster();

            myClusters.push_back(cluster);

            // Alle bodies aus comp in cluster einfügen:
            for (RBDRigidBody* n : comp)
            {
               cluster->addRigidBody(n);
            }
         }
      }

      if (leftBodies.size() > 0)
      {
         for (RBDRigidBody* b : leftBodies)
         {
            RBDCluster* cluster = createNewCluster();

            myClusters.push_back(cluster);
            cluster->addRigidBody(b);
         }
      }
   }
}

void RBDScene::doConstraintResourcesUpdate(double dt, const double& new_t, bool ifDisplay)
{
   Q_UNUSED(dt);
#ifdef VSLIBRBDYNAMX_USE_OMP
   //omp_set_num_threads(1);
   //#pragma omp parallel for schedule(dynamic)
#endif

   //int numberConstraintResource = getChildren().size();
   //qDebug() << "numberConstraintResource" << numberConstraintResource;
   for (int i = 0; i < getChildren().size(); ++i)
   {
      if (RBDConstraintResource* res = getChildren()[i]->dynamicCast<RBDConstraintResource*>())
      {
         //PERFORMANCESUITE_TIC("911 calculateInternalStates");
         //slow iteration, strage construct
         res->calculateInternalStates(dt, new_t);
         //PERFORMANCESUITE_TOC("911 calculateInternalStates");
         //PERFORMANCESUITE_TIC("912 updatePostBodiesPoseUpdate");
         res->updatePostBodiesPoseUpdate(dt, new_t, ifDisplay);
         //PERFORMANCESUITE_TOC("912 updatePostBodiesPoseUpdate");
      }
   }
}

//void RBDScene::doInverseDynamics(double dt, double t)
//{
//   //PERFORMANCESUITE_VALUE("801 Cluster count", myClusters.size());
//   if (myClusters.size() == 1)
//   {
//      RBDCluster* cluster = myClusters[0];
//      cluster->doTimeStep(t, dt);
//   }
//   else
//   {
//   #ifdef VSLIBRBDYNAMX_USE_OMP
//      #pragma omp parallel for schedule(static,1)
//   #endif
//      int numberThreads = 6;
//
//      for (int i = 0; i < myClusters.size(); i += numberThreads)
//      {
//         QFuture<void>* futures = new QFuture<void>[numberThreads];
//
//         for (unsigned int j = 0; ((j < numberThreads) && (i + j < myClusters.size())); j++)
//         {
//            QFuture<void> f = QtConcurrent::run(this, &RBDScene::doInverseDynamicsForCluster, myClusters[i + j], t, dt);
//            futures[j] = f;
//         }
//         for (unsigned int j = 0; ((j < numberThreads) && (i + j < myClusters.size())); j++)
//         {
//            futures[j].waitForFinished();
//         }
//         delete[] futures;
//      }
//   }
//}

void RBDScene::doInverseDynamics(double dt, double t)
{
   //PERFORMANCESUITE_VALUE("801 Cluster count", myClusters.size());

   if (myClusters.size() == 1)
   {
      RBDCluster* cluster = myClusters[0];

      if (calculateConditionNumber)
         cluster->setDoCalculateConditionNumber(calculateConditionNumber);

      /*static long long totaltime = 0;
      auto start = std::chrono::high_resolution_clock::now();*/
      cluster->doTimeStep(t, dt);

      /*auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
      totaltime += duration;
      qDebug() << "CD Time: " << totaltime << "μs";*/

      constraintError = cluster->getConstraintError();
      //qDebug() << "constraintErrorMSE = " << constraintError;
      //if (constraintError > 1e-3)
         //qDebug() << "ConstraintError is over the limit 1e-3, time step should be reduced!";

      if (calculateConditionNumber)
      {
         conditionNumber = cluster->getCNDNr();

         if (t > 0)
         {  // the conditionNumberLastStep has no value at t = 0
            double error_conditionNumber = fabs(conditionNumber - conditionNumberLastStep);

            //if (error_conditionNumber > 1000000)
            //   qDebug() << "Condition number changes too drastically, the system will crash!";
         }

         conditionNumberLastStep = conditionNumber;
      }
   }
   else
   {
      // Use OMP to parallelize the computation of each cluster.
#if !defined(__unix) && !defined(__QNXNTO__)
#ifdef VSLIBRBDYNAMX_USE_OMP
      int num_threads = VSM::minValue(myClusters.size(), omp_get_max_threads());
      omp_set_num_threads(num_threads);
#pragma omp parallel for schedule(static,1)
#endif
#endif
      for (int i = 0; i < myClusters.size(); ++i)
      {
         RBDCluster* cluster = myClusters[i];

         if (calculateConditionNumber)
            cluster->setDoCalculateConditionNumber(calculateConditionNumber);

         // Solve the LCP for each cluster
         /*static long long totaltime = 0;
         auto start = std::chrono::high_resolution_clock::now();*/
         cluster->doTimeStep(t, dt);
         /*auto end = std::chrono::high_resolution_clock::now();
         auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
         totaltime += duration;
         qDebug() << "CD Time: " << totaltime << "μs";*/

         if (calculateConditionNumber)
            conditionNumber = cluster->getCNDNr();
      }
   }
   //qDebug() << "conditionNumber" << conditionNumber;
}

void RBDScene::doInverseDynamicsForCluster(RBDCluster* cluster, double newTime, double dt)
{
   if (cluster)
      cluster->doTimeStep(newTime, dt);
}

bool RBDScene::getIgnoreGyroTerm() const
{
   return myIgnoreGyroTerm;
}

void RBDScene::setIgnoreGyroTerm(bool value)
{
   myIgnoreGyroTerm = value;
}

void RBDScene::slotDoConfigUpdate()
{
   // 1. Bestimme die Menge logischen Bodies, die simuliert werden müssen.
   // Keine fixen, nicht solche, die Teil einer UNion sind usw. ...
   compileRootRigidBodySet();

   // MainActor sind z.B. die Greiferteile am Forwarder. Brauchte hier ein
   // Identifikationsmerkmal, um Pfadabstände zwischen liegenden Stämmen und dem
   // Greifer berechnen zu können als Kriterium für Aktivierung / Deaktivierung
   compileMainActorsSet();

   // Bestimmung der Grundmenge für die Kollisionserkennung
   compileColliderGroupSet();

   //
   //insertColliderGroupsIntoCollisionDetection(myResizeGridAtRuntime);

   //if (myCDAlgorithm)
   //   myCDAlgorithm->compileCollisionCandidatesMap();
   myCDAlgorithm->updateCDBroadPhaseAlgorithm();

   setFlag(RBDScene::BodyConfigDirty, false);

   // update current state according to new root rigid body set
   currentSimulationState = new DynamXState(myRootRigidBodies);
}

void RBDScene::compileMainActorsSet()
{
   if (!myMainActors.isEmpty())
      myMainActors.clear();

   for (RBDRigidBody* rb : myRootRigidBodies)
   {
      if (rb->isMainActor())
      {
         myMainActors.add(rb);
      }
   }
}

void RBDScene::setFlag(Flags flag, bool value)
{
   value ? myFlags |= flag : myFlags &= ~flag;
}

bool RBDScene::getFlag(Flags flag) const
{
   return myFlags & flag;
}

void RBDScene::setMinimumCollisionVelocityForSignal(double value)
{
   myMinimumCollisionVelocityForSignal = value;
}

double RBDScene::getGlobalCfm() const
{
   return myGlobalCfm;
}

void RBDScene::setGlobalCfm(const double& value)
{
   myGlobalCfm = value;
}

void RBDScene::setCellActivatingBody(RBDRigidBody* b)
{
   myCellActivatingBody = b;
}

#ifdef DOTIMING
void RBDScene::startTimer()
{
   mySimpleTimer.QuadPart = 0;
   QueryPerformanceCounter(&mySimpleTimer);
}

void RBDScene::printTime(const QString& text)
{
   LARGE_INTEGER newCounterValue;
   QueryPerformanceCounter(&newCounterValue);

   newCounterValue.QuadPart -= mySimpleTimer.QuadPart;

   double result = (double)newCounterValue.QuadPart;
   result /= (double)ticksPerSecond.QuadPart;
   result *= 1000;

   qDebug() << text << " " << result;
}
#endif

void RBDScene::setResizeGridAtRuntime(bool value)
{
   myResizeGridAtRuntime = value;
}

RBDMaterial* RBDScene::findMaterialByName(const QString& name)
{
   for (RBDMaterial* mat : myMaterials)
   {
      if (mat->getName() == name)
         return mat;
   }

   return 0;
}

//bool RBDScene::registerCDNarrowPhaseAlgorithm(
//   const VSD::MetaInstance* metaInstanceShape1,
//   const VSD::MetaInstance* metaInstanceShape2,
//   RBDCollisionDetectionNarrowPhaseAlgorithm* na)
//{
//   if (myCDNarrowPhaseAlgos.value(qMakePair(metaInstanceShape1, metaInstanceShape2)))
//      return false;
//
//   myCDNarrowPhaseAlgos.insert(
//      qMakePair(metaInstanceShape1, metaInstanceShape2),
//      VSD::Ref<RBDCollisionDetectionNarrowPhaseAlgorithm>(na, true));
//
//   if (metaInstanceShape1 != metaInstanceShape2)
//   {
//      myCDNarrowPhaseAlgos.insert(
//         qMakePair(metaInstanceShape2, metaInstanceShape1),
//         VSD::Ref<RBDCollisionDetectionNarrowPhaseAlgorithm>(na, false));
//   }
//
//   return true;
//}

const PtrList<RBDRigidBody>& RBDScene::getRigidBodyInstances()
{
   return myRigidBodyInstances;
}

const PtrList<RBDRigidBody>& RBDScene::getRootRigidBodyInstances()
{
   return myRootRigidBodies;
}

void RBDScene::createRigidBodyList(RBDRigidBody* tcp, RBDRigidBody* successor)
{
   // init new graphAlgoRun, all colors are reseted
   initNewGraphAlgoRun();

   // output
   QList<RBDRigidBody*> rigidBodyList;

   // only bodies in the same cluster can be combined to a RBDRigidBodyList
   VSLibRBDynamX::Edge::Enforcement enf = VSLibRBDynamX::Edge::Enforcement::SameCluster;

   // depth search should work, even if unification is enabled, so set getMostParentBodies = false
   bool getMostParentBodies = false;

   // performe directed depth search, starting at the tcp in direction of the successor
   tcp->DirectedDSFindConnectedComponent(&rigidBodyList, successor, enf, getMostParentBodies);

   // create RBDRigidBodyList
   RBDRigidBodyList* tmp = new RBDRigidBodyList(rigidBodyList);

   tmp->createRootRigidBodyList();

   // store the rigidBodyList in the map, if a RBDRigidBodyList for the tcp already exists, it will be replaced by the new one
   if (myRigidBodyListMemory.count(tcp) == 0)
   {
      myRigidBodyListMemory.insert(tcp, tmp);
   }
   else
   {
      myRigidBodyListMemory.erase(myRigidBodyListMemory.find(tcp));
      myRigidBodyListMemory.insert(tcp, tmp);
   }
}

void RBDScene::deleteRigidBodyList(RBDRigidBody* tcp)
{
   if (myRigidBodyListMemory.count(tcp))
   {
      delete myRigidBodyListMemory.find(tcp).value();
   }
}

void RBDScene::setUseStaticVariablesToRigidBodyList(bool b)
{
   typedef QMap<RBDRigidBody*, RBDRigidBodyList*> rbList;

   if (myRigidBodyListMemory.size() > 0)
   {
      for (rbList::iterator it = myRigidBodyListMemory.begin(); it != myRigidBodyListMemory.end(); it++)
      {
         (*it)->setUseStaticVariables(b);
      }
   }
}

void RBDScene::setIntegrationAlgorithm(QString algo)
{
   algo = algo.remove(" ");
   algo = algo.toLower();

   if (algo == "eulerexplicit")
      myIntegrationAlgorithm = IA_EULEREXPLICIT;

   if (algo == "eulersemiimplicit")
      myIntegrationAlgorithm = IA_EULERSEMIIMPLICIT;

   if (algo == "leapfrog")
      myIntegrationAlgorithm = IA_LEAPFROG;

   if (algo == "heun")
      myIntegrationAlgorithm = IA_HEUN;

   if (algo == "rk4")
      myIntegrationAlgorithm = IA_RK4;

   if (algo == "adamsbashforth")
      myIntegrationAlgorithm = IA_ADAMSBASHFORTH;

   if (algo == "adamsmoulton")
      myIntegrationAlgorithm = IA_ADAMSMOULTON;
}

IntegrationAlgorithmBase* RBDScene::createIntegrationAlgorithm(INTEGRATIONALGORITHM algo)
{
   IntegrationAlgorithmBase* newIntegrationAlgorithm = nullptr;

   switch (algo)
   {
   case (IA_EULEREXPLICIT):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmEulerExplicit(this);
      break;
   }
   case (IA_EULERSEMIIMPLICIT):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmEulerSemiImplicit(this);
      break;
   }
   /*case (IA_LEAPFROG) :
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmLeapfrog(this);
      break;
   }*/
   case (IA_HEUN):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmHeun(this);
      break;
   }
   case (IA_RK4):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmRK4(this);
      break;
   }
   case (IA_ADAMSBASHFORTH):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmAdamsBashforth(this);
      break;
   }
   case (IA_ADAMSMOULTON):
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmAdamsMoulton(this);
      break;
   }
   default:
   {
      newIntegrationAlgorithm = new IntegrationAlgorithmEulerSemiImplicit(this);
      break;
   }
   }
   return newIntegrationAlgorithm;
}

bool VSLibRBDynamX::RBDScene::getUseBilateralFrictionCone()
{
   return useBilateralFrictionCone;
}

double VSLibRBDynamX::RBDScene::getConditionNumber()
{
   return conditionNumber;
}

void VSLibRBDynamX::RBDScene::setCalculateConditionNumber(bool b)
{
   calculateConditionNumber = b;
}