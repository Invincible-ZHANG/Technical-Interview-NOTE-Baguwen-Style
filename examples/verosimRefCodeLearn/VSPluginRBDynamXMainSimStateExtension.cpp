#include "../VSPluginRBDynamXMainSimStateExtension.h"
#include "../VSPluginRBDynamXProject.h"

// standard
#include "../../Main/VEROSIM/VEROSIMApplication.h"
#include "Main/VEROSIM/VEROSIMProject.h"

#include "../../Lib/VSL/VSLMacros.h"
#include "Lib/VSL/VSLMiscTools.h"

#include "../../Lib/VSLibSignalSlotSpy/VSLibSignalSlotSpyBlock.h"

#ifdef VS_WIDGETS
   #include <QInputDialog>
#endif
#include <QString>

//#include <Plugin/VSPluginRBDynamX/VSPluginRBDynamXExtensionVelocityBasedMotor.h>

#include "../../../Lib/VSD/VSDModel.h"
#include "../../../Lib/VSD/VSDElementIndex.h"
#include "../../../Lib/VSD/VSDEnvironment.h"
#include "../../../Lib/VSD/VSDDatabase.h"
#include "../../../Lib/VSD/VSDNode.h"
#include "../../../Lib/VSD/VSDElementContainer.h"
#include "../../../Lib/VSD3D/VSD3DMaterialNode.h"
#include "../../../Lib/VSD3D/VSD3DNode.h"
#include "../../../Lib/VSD3D/VSD3DGeometryCylsphere.h"
#include "../../../Lib/VSD3D/VSD3DGeometryCylinder.h"
#include "../../../Lib/VSD3D/VSD3DGeometryFace.h"
#include "../../../Lib/VSD3D/VSD3DGeometrySphere.h"
#include "../../../Lib/VSD3D/VSD3DGeometryBox.h"
#include "../../../Lib/VSD3D/VSD3DParamTransform.h"
#include "../../../Lib/VSD3D/VSD3DFrameTransform.h"
#include "Lib/VCI/VCIDirectInput.h"
#include <Lib/VSD/VSDConnection.h>

#ifdef VS_WIDGETS
#include "../../../Lib/VSLibGUI/VSLibGUIAction.h"
#endif

#ifdef VS_DEPENDENCY_VSLibRenderGL
#include "../../VSLibRenderGL/VSLibRenderGLNormalCalculator.h"
#endif

#include "../../VSLibSelection/VSLibSelectionComponentSimStateInstance.h"
#include "../../VSPluginCommands/VSPluginCommandsProject.h"

#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDProject.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandCopySimStateInstances.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandMoveSimStateInstances.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandAppendNewExtension.h"
//#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandAppendNewHullNode.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandModifyPropertyVal.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandAppendSimStateInstanceToPropertyRef.h"
#include "../../VSPluginCommandsVSD/VSPluginCommandsVSDCommandRemoveSimStateInstances.h"

#include "../../VSLibTree/VSLibTreeTreeBase.h"

// header
#include "./VSPluginRBDynamXCollisionDetectionOctSphereTreeBox.h"
#include "./VSPluginRBDynamXCollisionDetectionOctSphereTreeCylsphere.h"
#include "./VSPluginRBDynamXCollisionDetectionOctSphereTreePlane.h"
#include "./VSPluginRBDynamXCollisionDetectionOctSphereTreeSphere.h"
#include "./VSPluginRBDynamXCollisionDetectionOctSphereTree.h"
#include "./VSPluginRBDynamXCollisionDetectionTriangleMeshOctSphereTree.h"
#include "./VSPluginRBDynamXCollisionDetectionTriangleMeshCylsphere.h"
#include "./VSPluginRBDynamXCollisionDetectionTriangleMeshEllipsoid.h"
#include "./VSPluginRBDynamXCollisionDetectionTriangleMeshSphere.h"

#include "../VSPluginRBDynamXCommandSwitchVisibility.h"
#include "../VSPluginRBDynamXCommandActivateRigidBodies.h"
#include "../VSPluginRBDynamXExtensionDebugLagrangeMFile.h"
#include "../VSPluginRBDynamXExtensionContactDebugger.h"
#include "../VSPluginRBDynamXExtensionDynamXScene.h"
#include "../VSPluginRBDynamXExtensionAerodynamicDrag.h"
#include "../VSPluginRBDynamXExtensionJoint.h"
#include "../VSPluginRBDynamXExtensionJointGeneral.h"
#include "../VSPluginRBDynamXExtensionJointBallInSocket.h"
#include "../VSPluginRBDynamXExtensionJointPrismatic.h"
//#include "../VSPluginRBDynamXExtensionMeltingJoint.h"
#include "../VSPluginRBDynamXExtensionTwoBodiesJoint.h"
#include "../VSPluginRBDynamXExtensionJointDifferential.h"
#undef Status // Irgendwo in der XLib definiert ...
//#include "../VSPluginRBDynamXExtensionJointPathConstraint.h"
#include "../VSPluginRBDynamXExtensionMotor.h"
#include "../VSPluginRBDynamXExtensionPositionControlledMotor.h"
#include "../VSPluginRBDynamXExtensionOverGround.h"
#include "../VSPluginRBDynamXExtensionRigidBody.h"
#include "../VSPluginRBDynamXExtensionRigidBodyList.h"
#include "../VSPluginRBDynamXExtensionRigidBodyShapeCylsphere.h"
#include "../VSPluginRBDynamXExtensionRigidBodyShapeSphere.h"
#include "../VSPluginRBDynamXExtensionRigidBodyShapeOctSphereTree.h"
#include "../VSPluginRBDynamXExtensionRigidBodyShapeBox.h"
//#include "../VSPluginRBDynamXProject.h"
#include "../VSPluginRBDynamXRigidBodyDamping.h"
#include "../VSPluginRBDynamXExtensionMaterial.h"
#include "../VSPluginRBDynamXJointSteeredWheelSuspension.h"
#include "../VSPluginRBDynamXJointWheelSuspension.h"
#include "../VSPluginRBDynamXInputAdapter.h"
#include "../VSPluginRBDynamXCollisionEvent.h"
#include "../VSPluginRBDynamXMainWindow.h"
#include "../VSPluginRBDynamXExtensionCompoundInitializer.h"
#include "../VSPluginRBDynamXExtensionSetRigidBodyChain.h"
//#include "../VSPluginRBDynamXExtensionRigidBodyDirectControl.h"
#include "../VSPluginRBDynamXExtensionJointDifferential.h"
#include "../VSPluginRBDynamXExtensionTwoHingeGear.h"
#include "../VSPluginRBDynamXExtensionHingePrismaticGear.h"
#include "../VSPluginRBDynamXSimStateContextFilter.h"
#include "../VSPluginRBDynamXExtensionConditionNumber.h"
#include "../VSPluginRBDynamXExtensionHydraulic.h"

#include "../../VSLibRBDynamX/RBDShapePlane.h"
#include "../../VSLibRBDynamX/RBDShapeCylsphere.h"
#include "../../VSLibRBDynamX/RBDCollisionDetectionAlgorithm.h"

#include "../VSPluginRBDynamXMainSimStateExtension.h"
#include "VSPluginRBDynamXTTGluableDynamicModel.h"

#include <Lib/VSD3D/VSD3DSpatialView.h>
#include <Lib/VSS2/VSS2Scheduler.h>
#include <Lib/VSS2/VSS2SimConfig.h>

#include <Lib/VSS2/VSS2ResourceVSD.h>

VSPluginRBDynamX::MainSimStateExtension* VSPluginRBDynamX::MainSimStateExtension::findOrCreate(VSD::SimState* simState)
{
   if (!simState)
      return 0;

   //ist nicht GUI SimState und gehört nicht zum Dynmax SimState Context
   if (simState->getID() > 0 && !(simState->getSimContexts() & VSD::SimContext::ContextDynamX))
   {
      return nullptr;
   }

   MainSimStateExtension* extension = simState->findFirstExtensionInherits<VSPluginRBDynamX::MainSimStateExtension*>();

   if (!extension)
   {
      extension = simState->newSimStateInstance<MainSimStateExtension>();
   }

   return extension;
}

// main class<

//VSPluginRBDynamX::MainSimStateExtension::MainSimStateExtension()
//   : VSD::ModelInstanceExtension(nullptr, nullptr)
//   , VSS2::TaskStep("VSPluginRBDynamX::MainSimStateExtension", VSS2::Scheduler::the().getDefaultStep(), VSS2::EventHandler::PRIORITY_DEPENDENCY_SORTED_DEFAULT, VSS2::DependencySpecification::noDependencies(), VSS2::DependencySpecification::noDependencies())
//{
//}

VSPluginRBDynamX::MainSimStateExtension::MainSimStateExtension(VSD::SimState* simState, VSD::SimStateInstance* otherSimStateInstance)
   : VSD::SimStateExtension(simState, otherSimStateInstance)
   , VSS2::TaskStep(nullptr, *simState, "VSPluginRBDynamX::MainSimStateExtension")
   , myScene(0)
   , mySpeedTreesBody(0)
   , myPhysicModelIsVisible(false)
   , timeToBeDone(0)
   , doCalculateConditionNumber(false)
{
   setActive(false);

   VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
   if (dynamXProject)
   {
      VSD::Connection::connect(
         dynamXProject,
         SIGNAL(signalUpdateSettings()),
         this,
         SLOT(slotSyncSettings()));

      VSD::Connection::connect(
         dynamXProject,
         SIGNAL(signalResetDynamX()),
         this,
         SLOT(slotResetDynamX()));
   }

   QObject *vssProject = (QObject*)VSL::MiscTools::FindChild(getMyEnvironment()->getMyProject(), 0, "VSPluginVSS::Project");
   if (vssProject)
   {
      VSD::Connection::connect(vssProject, SIGNAL(signalSimConfigHasBeenCreated(VSS2::SimConfig*))
         , this, SLOT(slotSimConfigHasBeenCreated(VSS2::SimConfig*)));

      VSD::Connection::connect(vssProject, SIGNAL(signalCreateSimConfigConnections(VSS2::SimConfig*))
         , this, SLOT(slotCreateSimConfigConnections(VSS2::SimConfig*)));
   }

   postHasBeenAddedToSimState();

   setSimContext(VSD::SimContext::ContextSimTask, false);
   setSimContext(VSD::SimContext::ContextDynamX, true);
}

VSPluginRBDynamX::MainSimStateExtension::~MainSimStateExtension()
{
   slotFreeRBDScene();

   if (myScene)
      delete myScene;
}

void VSPluginRBDynamX::MainSimStateExtension::postHasBeenAddedToSimState()
{
   //TODO (wah): Parallelisierungskontext reparieren falls noch notwendig
   //this->addToContext(VSS::Context::Predefined::ContextDynamX);

   // initialize DynamX when project is loaded --> DynamXScene is available for requests during modeling, e.g. getCOG();
   VSL::SettingsGroup mySettings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);

   //VSD3D::SpatialView* sv = VSD3D::SpatialView::the(getMySimState());
   //sv->updateAllWorldFrames();

   if (getMySimState() == getMyEnvironment()->getGuiSimState())
   {
      //Vorinitialisierung Parallele Ausführung damit wird es threadsafe für diese Anzahl an SimState Threads
      int threadCount = QThread::idealThreadCount() * 2;
      for (int i = 0; i <= threadCount; i++)
      {
         VSLibRBDynamX::RBDScene::staticSimStateSceneMap.insert(i, nullptr);
      }
   }
}

void VSPluginRBDynamX::MainSimStateExtension::initResources()
{
   VSS2::TaskStep::initResources();

   addReadResource(VSS2::ResourceVSD::allPropertiesInNamespace("VSPluginRBDynamX"));
   addWriteResource(VSS2::ResourceVSD::allPropertiesInNamespace("VSPluginRBDynamX"));
   

   addReadResource(VSS2::ResourceVSD::allPropertiesInNamespace("VSPluginRobotAxisControl"));
   addWriteResource(VSS2::ResourceVSD::allPropertiesInNamespace("VSPluginRobotAxisControl"));

   //addReadResource(VSS2::ResourceVSD::allPropertiesWithSameNamespaceAsInstances("VSLibKinematics::KinematicRootController"));
   //addWriteResource(VSS2::ResourceVSD::allPropertiesWithSameNamespaceAsInstances("VSLibKinematics::KinematicRootController"));

   //addReadResource(VSS2::ResourceVSD::allPropertiesWithSameNamespaceAsInstances("VSLibKinematics::KinematicRootControllerManager"));
   //addWriteResource(VSS2::ResourceVSD::allPropertiesWithSameNamespaceAsInstances("VSLibKinematics::KinematicRootControllerManager"));

   addReadResource(VSS2::ResourceVSD::propertyOfAllInstances(*VSD3D::HullNode::getThisMetaInstance(), *VSD3D::HullNode::getMetaPropertyAxisAlignedBoundingBox()));
   addReadResource(VSS2::ResourceVSD::propertyOfAllInstances(*VSD3D::HullNode::getThisMetaInstance(), *VSD3D::HullNode::getMetaPropertyAxisAlignedBoundingBoxState()));

   // necessary for collision detection with VSPluginCollision to initialize collision data
   addReadResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*VSD3D::Geometry::getThisMetaInstance()));

   // during execution the ports of motor will be read/written
   VSDForAllElementsInherits(ExtensionMotor*, motor, getMySimState()->getDatabase()) {
      if (motor->getOutputCurrentVelocity())
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*motor->getOutputCurrentVelocity()->instanceCast<VSD::SimStateInstance*>()));
      if (motor->getOutputExertedForceOrTorque())
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*motor->getOutputExertedForceOrTorque()->instanceCast<VSD::SimStateInstance*>()));
      if (motor->getOutputCurrentPower())
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*motor->getOutputCurrentPower()->instanceCast<VSD::SimStateInstance*>()));
   }

   addReadResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionVelocityBasedMotor::getThisMetaInstance()));
   addWriteResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionVelocityBasedMotor::getThisMetaInstance()));


   addReadResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionForceBasedMotor::getThisMetaInstance()));
   addWriteResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionForceBasedMotor::getThisMetaInstance()));


   VSDForAllElements(ExtensionForceBasedMotor*, motor, getMySimState()->getDatabase())
   {
      if (motor->getOutputVelocity())
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*motor->getOutputVelocity()->instanceCast<VSD::SimStateInstance*>()));
      if (motor->getInputForce())
         addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*motor->getInputForce()->instanceCast<VSD::SimStateInstance*>()));
   }

   addReadResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionRigidBody::getThisMetaInstance()));
   addWriteResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionRigidBody::getThisMetaInstance()));

   // during save, the new positions of RBs will be written
   VSDForAllElements(ExtensionRigidBody*, rb, getMySimState()->getDatabase())
   {
      VSLibRBDynamX::RBDRigidBody* body = rb->getRBDRigidBody();
      if (body) {
         addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*body->instanceCast<VSD::SimStateInstance*>()));
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*body->instanceCast<VSD::SimStateInstance*>()));
      }

      VSD3D::Node* myNode = rb->getMyModelInstance()->instanceCast<VSD3D::Node*>();
      if (myNode) {
         addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*myNode->instanceCast<VSD::SimStateInstance*>()));
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*myNode->instanceCast<VSD::SimStateInstance*>()));

         for (VSD3D::Node* parent = myNode->getSpatialParentNode(); parent != nullptr; parent = parent->getSpatialParentNode()) {
            addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*parent->instanceCast<VSD::SimStateInstance*>()));
            addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*parent->instanceCast<VSD::SimStateInstance*>()));
         }
      }
   }

   addReadResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionJoint::getThisMetaInstance()));
   addWriteResource(VSS2::ResourceVSD::allPropertiesOfAllInstances(*ExtensionJoint::getThisMetaInstance()));

   VSDForAllElementsInherits(ExtensionJoint*, rb, getMySimState()->getDatabase())
   {
      VSD3D::Node* node = rb->getParent<VSD3D::Node*>();
      if (node) {
         addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*node->instanceCast<VSD::SimStateInstance*>()));
         addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*node->instanceCast<VSD::SimStateInstance*>()));

         VSD3D::Transform* rel = node->getRelFrameTransform();
         if (rel) {
            addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*rel->instanceCast<VSD::SimStateInstance*>()));
            addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*rel->instanceCast<VSD::SimStateInstance*>()));
         }

         VSD3D::SpatialTransform* spatial = node->getMySpatialTransform();
         if (spatial) {
            addReadResource(VSS2::ResourceVSD::allPropertiesOfInstance(*spatial->instanceCast<VSD::SimStateInstance*>()));
            addWriteResource(VSS2::ResourceVSD::allPropertiesOfInstance(*spatial->instanceCast<VSD::SimStateInstance*>()));
         }
      }
   }
}

void VSPluginRBDynamX::MainSimStateExtension::load(const VSS2::Event& evt)
{

}

void VSPluginRBDynamX::MainSimStateExtension::calculate(const VSS2::Event& evt)
{
   //!!! see pragma message in save()
   /*if (!myScene)
      return;

   if (!getEnableRBDynamX())
      return;


   for (ExtensionRigidBody* extRb : bodiesToBeInitialized)
   {
      extRb->initializeDynamX(myScene);
   }

   if (!bodiesToBeInitialized.empty())
      bodiesToBeInitialized.clear();

   VSS2::Duration dt = getStepTime();
   double dtSeconds = VSS2::toSeconds(dt);

   emit signalDynamXPreCalcStep(dt);

   // Bei 3dGroundSupport: Wenn es eine ExtensionOverGround an einer
   // RBDExtensionRigidBody gibt, deren RigidBody nicht mehr über 3D-Boden
   // ist, wird diese nicht simuliert!
   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
   if (settings.ReadBool("extOverGroundSupport"))
   {
      VSDForAllElements(ExtensionOverGround*, eog, getMySimState()->getDatabase())
      {
         if (!eog->isOverGround())
         {
            ExtensionRigidBody* extRb = eog->getMyModelInstance()->instanceCast<ExtensionRigidBody*>();
            if (extRb)
            {
               extRb->setIsFix(true);
            }
         }
         else
         {
            ExtensionRigidBody* extRb = eog->getMyModelInstance()->instanceCast<ExtensionRigidBody*>();
            if (extRb)
            {
               extRb->setIsFix(false);
            }
         }
      }

      if (settings.ReadBool("speedTreeSupport"))
      {
         this->updateSpeedTreesBody();
      }
   }

   myExtensionDynamXScene->emitSignalPrePhysicsAdvance(dt);
   myExtensionDynamXScene->emitSignalPreInternalAdvance(dt);

   double absTimeSeconds = VSS2::toSeconds(evt.executionTime);

   if (dt.count() > 0)
      myScene->simulate(dtSeconds, absTimeSeconds);

   //if (getMySimState()->getID() == 2)
   //{
   //	double time = timerTotal.time() / 1000.0;
   //	qDebug() << "02 :" << time;
   //	qDebug() << "__";
   //	qDebug() << "__";
   //	timerTotal.start();
   //}
   bool done = true;

   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      mod->doStep(dtSeconds);
   }
   int iterations = 0;

   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      if (mod->getMaxNumberIterations() > iterations++)
         done &= mod->compliesWithConstraints();
   }

   while (!done)
   {
      myScene->undoStep();

      done = true;
      for (TTGluableDynamicModel* mod : myGluedDynamicModels)
      {
         mod->undoStep();
         mod->doStep(dtSeconds);
      }

      myScene->simulate(dtSeconds, absTimeSeconds);

      for (TTGluableDynamicModel* mod : myGluedDynamicModels)
      {
         if (mod->getMaxNumberIterations() > iterations++)
            done &= mod->compliesWithConstraints();
      }
   }

   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      mod->confirmStep();
   }

   signalDynamXInternalAdvance(dt);
   myExtensionDynamXScene->emitSignalPostInternalAdvance(dt);

   // I'm done
   emit signalDynamXPostCalcStep(dt);
   myExtensionDynamXScene->emitSignalPostCalcStep(dt);

   if (doCalculateConditionNumber)
   {
      VSDForAllElements(VSPluginRBDynamX::ExtensionConditionNumber*, ext, getMySimState()->getDatabase())
      {
         ext->setConditionNumber(myExtensionDynamXScene->getConditionNumber());
      }
   }*/
}

// 主仿真循环里“每一步保存前的统筹调度器”
void VSPluginRBDynamX::MainSimStateExtension::save(const VSS2::Event& evt)
{
   // 历史遗留问题，到现在还没来得及重构
#pragma message("!!!FIX ME: move back to calculate() and load() after reworking; only load() is allowed to read from the VSD and only save() may write!!!")
   if (!myScene)
      return;

   if (!getEnableRBDynamX())
      return;

   // 把还没加入物理世界的刚体，逐个初始化并注册到 myScene，使其从“定义阶段”进入“参与仿真阶段”。
   for (ExtensionRigidBody* extRb : bodiesToBeInitialized)
   {
      extRb->initializeDynamX(myScene);
   }

   // 清空待初始化队列
   if (!bodiesToBeInitialized.empty())
      bodiesToBeInitialized.clear();

   // 取本步时间步长并转成秒   
   VSS2::Duration dt = getStepTime();
   double dtSeconds = VSS2::toSeconds(dt);

   // 这是一发 Qt 信号, 发出“预计算”信号
   emit signalDynamXPreCalcStep(dt);

   // Bei 3dGroundSupport: Wenn es eine ExtensionOverGround an einer
   // RBDExtensionRigidBody gibt, deren RigidBody nicht mehr über 3D-Boden
   // ist, wird diese nicht simuliert!
   // 取项目配置组，后面通过布尔开关控制功能。
   // 每一帧开始前，根据“是否还在地面上方”的判定，动态把相关刚体设为固定/解锁，以减少不必要的仿真或防止“穿地下”的异常传播；同时按需更新 SpeedTree 相关刚体的状态。
   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);

   // 如果开启了“地面支撑”功能
   if (settings.ReadBool("extOverGroundSupport"))
   {
      // 遍历所有的 ExtensionOverGround 扩展
      VSDForAllElements(ExtensionOverGround*, eog, getMySimState()->getDatabase())
      {
         // 如果该扩展检测到其所附加的刚体已经不在地面上
         if (!eog->isOverGround())
         {
            // 那么把该刚体设置为“固定”，即不参与仿真
            ExtensionRigidBody* extRb = eog->getMyModelInstance()->instanceCast<ExtensionRigidBody*>();
            
            if (extRb)
            {
               extRb->setIsFix(true);
            }
         }
         else
         {
            // 否则把该刚体设置为“非固定”，即参与仿真
            ExtensionRigidBody* extRb = eog->getMyModelInstance()->instanceCast<ExtensionRigidBody*>();
            if (extRb)
            {
               extRb->setIsFix(false);
            }
         }
      }
      // 如果开启了“SpeedTree 支撑”功能
      // 植被系统的适配/同步，eg:风吹摆动
      if (settings.ReadBool("speedTreeSupport"))
      {
         this->updateSpeedTreesBody();
      }
   }

   // 发出“物理计算前”信号
   myExtensionDynamXScene->emitSignalPrePhysicsAdvance(dt);
   // 发出“内部计算前”信号
   myExtensionDynamXScene->emitSignalPreInternalAdvance(dt);

   // 计算绝对时间，推进场景一步
   double absTimeSeconds = VSS2::toSeconds(evt.executionTime);

   // 守卫避免零步长或负步长时错误推进  
   if (dt.count() > 0)
      myScene->simulate(dtSeconds, absTimeSeconds);

   //if (getMySimState()->getID() == 2)
   //{
   //	double time = timerTotal.time() / 1000.0;
   //	qDebug() << "02 :" << time;
   //	qDebug() << "__";
   //	qDebug() << "__";
   //	timerTotal.start();
   //}


   bool done = true;

   // 让每个子模型先跟着这一步时间 dt 做一次自身更新（内部状态推进到当前帧）。
   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      mod->doStep(dtSeconds);
   }
   // 迭代计数器清零
   int iterations = 0;

   // 检查每个子模型是否都满足约束条件
   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      // 检查它是否已经满足自身约束 compliesWithConstraints()
      if (mod->getMaxNumberIterations() > iterations++)
         done &= mod->compliesWithConstraints();
   }

   // 如果有任一子模型不满足约束条件，则需要回滚并重新推进
   while (!done)
   {
      // 回滚场景到上一步
      myScene->undoStep();
      // 重新尝试
      done = true;
      // 让每个子模型先跟着这一步时间 dt 做一次自身更新（内部状态推进到当前帧）。
      for (TTGluableDynamicModel* mod : myGluedDynamicModels)
      {
         mod->undoStep();
         mod->doStep(dtSeconds);
      }
      // 场景重新推进一步
      myScene->simulate(dtSeconds, absTimeSeconds);
      // 再次检查每个子模型是否都满足约束条件
      for (TTGluableDynamicModel* mod : myGluedDynamicModels)
      {
         if (mod->getMaxNumberIterations() > iterations++)
            done &= mod->compliesWithConstraints();
      }
   }
   // 全部满足约束条件，确认这一步的推进
   for (TTGluableDynamicModel* mod : myGluedDynamicModels)
   {
      mod->confirmStep();
   }
   // 发出“内部计算后”信号
   signalDynamXInternalAdvance(dt);
   myExtensionDynamXScene->emitSignalPostInternalAdvance(dt);

   // I'm done
   emit signalDynamXPostCalcStep(dt);
   myExtensionDynamXScene->emitSignalPostCalcStep(dt);

   if (doCalculateConditionNumber)
   {
      VSDForAllElements(VSPluginRBDynamX::ExtensionConditionNumber*, ext, getMySimState()->getDatabase())
      {
         ext->setConditionNumber(myExtensionDynamXScene->getConditionNumber());
      }
   }
   /*------------------------------------------ end FIX ME ------------------------------------------*/


   // 最后一步，把刚体的最新位置和姿态写回 VSD
   saveStep();
}

void VSPluginRBDynamX::MainSimStateExtension::setDoConfigUpdate(VSD::SimState* simState)
{
   myScene->setFlag(VSLibRBDynamX::RBDScene::BodyConfigDirty, true);

   if (!mySpeedTreeColliderBodies.isEmpty())
      mySpeedTreeColliderBodies.clear();

   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
   if (settings.ReadBool("speedTreeSupport"))
   {
      VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionRigidBody*, rbExt, simState->getDatabase())
      {
         if (rbExt->getCollidesWithSpeedTrees())
            mySpeedTreeColliderBodies.append(rbExt->getRBDRigidBody());
      }
   }
}

bool VSPluginRBDynamX::MainSimStateExtension::createRBDScene(VSD::SimState* simState)
{
   Q_UNUSED(simState);
   if (myScene)
   {
      VSLibRBDynamX::RBDScene::removeInstanceFor(getMySimState());
   }

   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
   if (!settings.ReadBool("dynamX", false))
      return false;

   myScene = VSLibRBDynamX::RBDScene::instanceFor(getMySimState());

   // Bekanntmachung der Kollisionsalgorithmen aus VSPluginRBDynamX  
   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeBox"),
      new CollisionDetectionOctSphereTreeBox(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
      new CollisionDetectionOctSphereTreeCylsphere(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapePlane"),
      new CollisionDetectionOctSphereTreePlane(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
      new CollisionDetectionOctSphereTreeSphere(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      new CollisionDetectionOctSphereTree(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeTriangleMesh"),
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeOctSphereTree"),
      new CollisionDetectionTriangleMeshOctSphereTree(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeTriangleMesh"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeCylsphere"),
      new CollisionDetectionTriangleMeshCylsphere(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeTriangleMesh"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeEllipsoid"),
      new CollisionDetectionTriangleMeshEllipsoid(myScene));

   myScene->myCDAlgorithm->registerCDNarrowPhaseAlgorithm(
      VSD::Manager::the()->findMetaInstance("VSPluginRBDynamX::RBDShapeTriangleMesh"),
      VSD::Manager::the()->findMetaInstance("VSLibRBDynamX::RBDShapeSphere"),
      new CollisionDetectionTriangleMeshSphere(myScene));

   if (settings.ReadBool("generateCollisionSignals"))
   {
      connect(myScene, SIGNAL(signalCollision(VSLibRBDynamX::RBDRigidBody*, VSLibRBDynamX::RBDRigidBody*, const VSM::Vector3&, double)),
         this, SLOT(slotHandleCollision(VSLibRBDynamX::RBDRigidBody*, VSLibRBDynamX::RBDRigidBody*, const VSM::Vector3&, double)));
      myScene->setGenerateCollisionSignals(true);
   }

   slotSyncSettings();
   //connect(myScene, SIGNAL(signalShowInfo(const QString&, bool)),
   //   this, SLOT(ShowInfo(const QString&, bool)));

   //connect(myScene, SIGNAL(signalShowWarning(const QString&, bool)),
   //   this, SLOT(ShowWarning(const QString&, bool)));

   bool success = connect(myScene, SIGNAL(signalCollisionDetectionReady()), this, SLOT(slotUpdateIsCollidingFlag()));

   return true;
}

void VSPluginRBDynamX::MainSimStateExtension::slotCreatePrimitiveGeometries()
{
   if (!getMySimState())
      return;

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionRigidBodyShape*, extShape, getMySimState()->getDatabase())
   {
      if (ExtensionRigidBodyShapeCylsphere* extShapeCys = extShape->instanceCast<ExtensionRigidBodyShapeCylsphere*>())
      {
         VSD3D::HullNode* hull = extShape->getMyModelInstance()->instanceCast<VSD3D::HullNode*>();

         if (!hull)
            continue;

         VSD3D::GeometryCylsphere* geomCys = hull->getGeometryCast<VSD3D::GeometryCylsphere*>();

         if (!geomCys)
         {
            // Eine GeomCys anlegen und das Shape umhängen.
            VSD3D::HullNode* clone = getMySimState()->newSimStateInstance<VSD3D::HullNode>();
            clone->setRelFrame(hull->getRelFrame());
            clone->setName(hull->getName());
            clone->setMaterial(hull->getMaterial());
            geomCys = getMySimState()->newSimStateInstance<VSD3D::GeometryCylsphere>();
            geomCys->setRadius(extShapeCys->getRadius());
            geomCys->setHeight(extShapeCys->getHeight());
            geomCys->setApproxParam(32);
            clone->setGeometry(geomCys);
            hull->getParent<VSD::Node*>()->getChildNodes().append(clone);

            VSD::Ref<ExtensionRigidBodyShape> ref(extShape, true);
            extShape->removeFromParents();
            clone->getExtensions().append(extShape);
         }
      }
      else if (ExtensionRigidBodyShapeSphere* extShapeSphere = extShape->instanceCast<ExtensionRigidBodyShapeSphere*>())
      {
         VSD3D::HullNode* hull = extShape->getMyModelInstance()->instanceCast<VSD3D::HullNode*>();

         if (!hull)
            continue;

         VSD3D::GeometrySphere* geomSphere = hull->getGeometryCast<VSD3D::GeometrySphere*>();

         if (!geomSphere)
         {
            // Eine GeomCys anlegen und das Shape umhängen.
            VSD3D::HullNode* clone = getMySimState()->newSimStateInstance<VSD3D::HullNode>();
            clone->setRelFrame(hull->getRelFrame());
            clone->setName(hull->getName());
            clone->setMaterial(hull->getMaterial());
            geomSphere = getMySimState()->newSimStateInstance<VSD3D::GeometrySphere>();
            geomSphere->setRadius(extShapeSphere->getRadius());
            geomSphere->setApproxParam(32);
            clone->setGeometry(geomSphere);
            hull->getParent<VSD::Node*>()->getChildNodes().append(clone);

            VSD::Ref<ExtensionRigidBodyShape> ref(extShape, true);
            extShape->removeFromParents();
            clone->getExtensions().append(extShape);
         }
      }
      else if (ExtensionRigidBodyShapeBox* extShapeBox = extShape->instanceCast<ExtensionRigidBodyShapeBox*>())
      {
         VSD3D::HullNode* hull = extShape->getMyModelInstance()->instanceCast<VSD3D::HullNode*>();

         if (!hull)
            continue;

         VSD3D::GeometryBox* geomBox = hull->getGeometryCast<VSD3D::GeometryBox*>();

         if (!geomBox)
         {
            // Eine GeomCys anlegen und das Shape umhängen.
            VSD3D::HullNode* clone = getMySimState()->newSimStateInstance<VSD3D::HullNode>();
            clone->setRelFrame(hull->getRelFrame());
            clone->setName(hull->getName());
            clone->setMaterial(hull->getMaterial());
            geomBox = getMySimState()->newSimStateInstance<VSD3D::GeometryBox>();

            geomBox->setSize(extShapeBox->getSizes());

            clone->setGeometry(geomBox);
            hull->getParent<VSD::Node*>()->getChildNodes().append(clone);

            VSD::Ref<ExtensionRigidBodyShape> ref(extShape, true);
            extShape->removeFromParents();
            clone->getExtensions().append(extShape);
         }
      }
   }
}

void VSPluginRBDynamX::MainSimStateExtension::prepareMultiSimStateSim(VSS2::SimConfigSimState* configSimState)
{
   TaskStep::prepareMultiSimStateSim(configSimState);

   if (getMySimState()->getSimContexts() & VSD::SimContext::ContextDynamX)
   {
      DynamXSimStateContextFilter* contextFilter = getMySimState()->newSimStateInstance<DynamXSimStateContextFilter>();
      getMySimState()->addMultiSimStateContextFilter(contextFilter);
   }
}

void VSPluginRBDynamX::MainSimStateExtension::slotSimConfigHasBeenCreated(VSS2::SimConfig* simConfig)
{
   // Create SimConfigSimState and configure it
   VSS2::SimConfigSimState* mySimStateConfig = getMySimState()->newSimStateInstance<VSS2::SimConfigSimState>();
   simConfig->getDataSimStates()->append(mySimStateConfig);
   mySimStateConfig->setContext(VSD::SimContext::ContextDynamX);
   VSL::SettingsGroup mySettings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
   double steppingInMS = mySettings.ReadDouble("timeStepping", 10.0);
   mySimStateConfig->setSyncIntervall(steppingInMS / 1000);
   mySimStateConfig->setName("DynamX");

   // Append this EventHandler to this SimConfigSimState and remove it from the GUI SimConfigSimState
   VSS2::SimConfigSimState* guiSimStateConfig = simConfig->getGUISimStateConfig();
   QString baseNameLoad = this->getLabel();
   QString baseNameCalc = this->getLabel().replace("[load]", "[calc]");
   QString baseNameSave = this->getLabel().replace("[load]", "[save]");
   VSS2::EventHandlerModelInstance* eventHandlerModelInstanceLoad = guiSimStateConfig->findEventHandlerModelInstanceByLabel(baseNameLoad);
   mySimStateConfig->getDataEventHandlers()->append(eventHandlerModelInstanceLoad);
   guiSimStateConfig->getDataEventHandlers()->remove(eventHandlerModelInstanceLoad);
   VSS2::EventHandlerModelInstance* eventHandlerModelInstanceCalc = guiSimStateConfig->findEventHandlerModelInstanceByLabel(baseNameCalc);
   mySimStateConfig->getDataEventHandlers()->append(eventHandlerModelInstanceCalc);
   guiSimStateConfig->getDataEventHandlers()->remove(eventHandlerModelInstanceCalc);
   VSS2::EventHandlerModelInstance* eventHandlerModelInstanceSave = guiSimStateConfig->findEventHandlerModelInstanceByLabel(baseNameSave);
   mySimStateConfig->getDataEventHandlers()->append(eventHandlerModelInstanceSave);
   guiSimStateConfig->getDataEventHandlers()->remove(eventHandlerModelInstanceSave);
}

void VSPluginRBDynamX::MainSimStateExtension::slotCreateSimConfigConnections(VSS2::SimConfig* simConfig)
{
   // Connections (later from definied DependencySpecification as soon as they are available)

   // Find my SimConfigSimState
   VSS2::SimConfigSimState* mySimStateConfig = nullptr;
   VSDForAllRefListInstances(VSS2::SimConfigSimState*, configSimState, simConfig->getSimStates())
   {
      VSD::SimContexts simContext = configSimState->getContext();
      simContext |= VSD::SimContext::ContextSimTask;
      if (simContext & VSD::SimContext::ContextDynamX)
      {
         mySimStateConfig = configSimState;
      }
   }

   // Create Sync Connection to WFU
   VSDForAllRefListInstances(VSS2::SimConfigSimState*, configSimState, simConfig->getSimStates())
   {
      VSD::SimContexts simContext = configSimState->getContext();
      simContext |= VSD::SimContext::ContextSimTask;

      // WFU depends on DynamX
      if (simContext & VSD::SimContext::ContextWFU)
      {
         VSS2::SimConfigConnection* newConnection = getMySimState()->newSimStateInstance<VSS2::SimConfigConnection>();
         newConnection->setSendingSimState(mySimStateConfig);
         newConnection->setReceivingSimState(configSimState);
         QString connectionName = QString(mySimStateConfig->getName()).append("->").append(configSimState->getName());
         newConnection->setName(connectionName);
         simConfig->getDataConnetions()->append(newConnection);

         // dajust Sync-Intervall to Intervall of WFU (not necessary to update more often than its itervall)
         mySimStateConfig->setSyncIntervall(configSimState->getSyncIntervall());
      }
   }

   // Create Sync Connection to GUI
   VSDForAllRefListInstances(VSS2::SimConfigSimState*, configSimState, simConfig->getSimStates())
   {
      VSD::SimContexts simContext = configSimState->getContext();
      simContext |= VSD::SimContext::ContextSimTask;

      // GUI Thread needs updated DynamX data
      if (simContext & VSD::SimContext::ContextGUI)
      {
         VSS2::SimConfigConnection* newConnection = getMySimState()->newSimStateInstance<VSS2::SimConfigConnection>();
         newConnection->setSendingSimState(mySimStateConfig);
         newConnection->setReceivingSimState(configSimState);
         QString connectionName = QString(mySimStateConfig->getName()).append("->").append(configSimState->getName());
         newConnection->setName(connectionName);
         simConfig->getDataConnetions()->append(newConnection);
      }
   }

   // Create Sync Connection to Kinematics
   VSDForAllRefListInstances(VSS2::SimConfigSimState*, configSimState, simConfig->getSimStates())
   {
      VSD::SimContexts simContext = configSimState->getContext();
      simContext |= VSD::SimContext::ContextSimTask;

      // GUI Thread needs updated DynamX data
      if (simContext & VSD::SimContext::ContextKinematic)
      {
         VSS2::SimConfigConnection* newConnection = getMySimState()->newSimStateInstance<VSS2::SimConfigConnection>();
         newConnection->setSendingSimState(mySimStateConfig);
         newConnection->setReceivingSimState(configSimState);
         QString connectionName = QString(mySimStateConfig->getName()).append("->").append(configSimState->getName());
         newConnection->setName(connectionName);
         simConfig->getDataConnetions()->append(newConnection);
      }
   }
}

void VSPluginRBDynamX::MainSimStateExtension::slotSyncSettings()
{
   if (!getMyEnvironment())
      return;
   if (!getMyEnvironment()->getMyProject())
      return;
   if (!getMyEnvironment()->getMyProject()->GetSettings())
      return;

   VSL::SettingsGroup mySettings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);

   //if ((getMySimState()->getSyncMode() != VSD::SimState::SyncMode::MultiSimState) || ((getMySimState()->getSyncMode() == VSD::SimState::MultiSimState) && (getMySimState()->getSimContexts() & VSD::SimContext::ContextDynamX)))
   //{
   setActive(mySettings.ReadBool("dynamX"));
   /*}*/

   VSLibRBDynamX::RBDScene* myRBDScene = VSLibRBDynamX::RBDScene::instanceFor(getMySimState());

   if (!myRBDScene)
      return;

   myRBDScene->setBaumgarteStabilizationConstant(mySettings.ReadDouble("baumgarteContactStabilizationConstant", 0.15));
   myRBDScene->setStribeckVelocity(mySettings.ReadDouble("stribeckVelocityConstant", 0.001));
   myRBDScene->setGaussianExponent(mySettings.ReadDouble("gaussianExponent", 2.0));
   myRBDScene->setTau(mySettings.ReadDouble("tau_lag", 0.0));
   myRBDScene->setDeltaFs(mySettings.ReadDouble("deltaFs"));
   myRBDScene->setHysteresisFactor(mySettings.ReadDouble("hysteresisFactor", 1.0));
   myRBDScene->setCalcPathDistancesToMainActors(mySettings.ReadBool("calculatePathDistanceToMainActors", false));
   myRBDScene->setClustering(mySettings.ReadBool("clustering", false));
   myRBDScene->setUnderwaterSimulation(mySettings.ReadBool("underwaterSimulation", false));
   myRBDScene->setGlobalCfm(mySettings.ReadDouble("globalCfm", 0.0001));
   //   myRBDScene->setContactCfm(mySettings.ReadDouble("contactCfm"));
   myRBDScene->setLimitPathLengthToMainActorForDeactivation(mySettings.ReadInt("limitPathLengthToMainActorForDeactivation"));
   myRBDScene->setNumberIterations(mySettings.ReadInt("numberIterations", 100));
   myRBDScene->setNumberThreads(mySettings.ReadInt("numberThreads", 1));
   myRBDScene->setShockPropagation(mySettings.ReadString("shockPropagation", "none"));
   myRBDScene->setContactModelType(mySettings.ReadString("contactModel", "default"));
   myRBDScene->setFrictionModelType(mySettings.ReadString("frictionModel", "default"));
   myRBDScene->setMaxAllowedDistanceForContactMatching(mySettings.ReadDouble("maximalAllowedDistanceForContactMatching", 0.1));
   myRBDScene->setLinkageToLCP(mySettings.ReadString("linkageToLCP"));
   myRBDScene->setCFMWeighting(mySettings.ReadDouble("spinBoxCFMWeighting", 0));
   myRBDScene->setIntegrationAlgorithm(mySettings.ReadString("integrationAlgorithm", "eulersemiimplicit"));

   myRBDScene->setFrictionConeApproximationParam(mySettings.ReadInt("frictionConeApproximationParam", 2));
   myRBDScene->setGenerateCollisionSignals(mySettings.ReadBool("generateCollisionSignals", false));
   myRBDScene->setIgnoreGyroTerm(mySettings.ReadBool("ignoreGyroTerm", false));
   myRBDScene->setMinimumCollisionVelocityForSignal(mySettings.ReadDouble("minimumCollisionVelocityForSignal"));
   myRBDScene->setResizeGridAtRuntime(mySettings.ReadBool("resizeGridAtRuntime"));

   myRBDScene->setGravity(ExtensionDynamXScene::the(getMySimState())->getGravity());

   VSDForAllElements(VSPluginRBDynamX::ExtensionConditionNumber*, ext, getMySimState()->getDatabase())
   {
      myRBDScene->setCalculateConditionNumber(true);
      doCalculateConditionNumber = true;
   }

   QString solverName = mySettings.ReadString("constraintSolver", "lagrangeauto");
   if (solverName.toLower() == "dantziginternal")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEDANTZIGEXP);
   }
   else if (solverName.toLower() == "lagrangeinteractivedantzig")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEINTERACTIVEDANTZIG);
   }
   else if (solverName.toLower() == "lagrangeinteractivegs")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEINTERACTIVEGS);
   }
   else if (solverName.toLower() == "lagrangeinteractiveauto")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEINTERACTIVEAUTO);
   }
   else if (solverName.toLower() == "lagrangedantzig")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEDANTZIG);
   }
   else if (solverName.toLower() == "lagrangegs")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEGS);
   }
   else if (solverName.toLower() == "lagrangeauto")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEAUTO);
   }
   else if (solverName.toLower() == "penalty")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_PENALTY);
   }
   else if (solverName.toLower() == "impulsebased")
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_IMPULSEBASED);
   }
   else if (solverName.toLower() == "sequentialimpulse") {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_SEQUENTIALIMPULSE);
   }
   else if (solverName.toLower() == "apgd") {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEAPGD);
   }
   else if (solverName.toLower() == "apgd_MF") {
       myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_MFAPGD);
   }
   else
   {
      myRBDScene->setConstraintSolverType(VSLibRBDynamX::RBDScene::CST_LAGRANGEAUTO);
   }

   double steppingInMS = mySettings.ReadDouble("timeStepping", 10.0);
   if (steppingInMS > 0.0)
   {
      setStepTime(VSS2::fromSeconds(steppingInMS / 1000));
      //setInitialOffset(steppingInMS/1000.0)
   }
   //double durationInMS = mySettings.ReadDouble("duration", 0.0);
   //this->setDuration(durationInMS/1000.0);
   //double initialOffsetInMS = mySettings.ReadDouble("initialOffset", 0.0);
   //this->setInitialOffset(initialOffsetInMS/1000.0);

   QString broadPhaseCDName = mySettings.ReadString("collisionBroadPhase", "sweepAndPrune");
   if (broadPhaseCDName.toLower() == "collidergroups")
   {
      myRBDScene->setCollisionBroadPhase(VSLibRBDynamX::RBDScene::CB_COLLIDERGROUPS);
   }
   else if (broadPhaseCDName.toLower() == "collisiongrid")
   {
      myRBDScene->setCollisionBroadPhase(VSLibRBDynamX::RBDScene::CB_COLLISIONGRID);
   }
   else
   {
      myRBDScene->setCollisionBroadPhase(VSLibRBDynamX::RBDScene::CB_SWEEPANDPRUNE);
   }

   QString midPhaseCDName = mySettings.ReadString("collisionMidPhase", "traversal");
   if (midPhaseCDName.toLower() == "recursivebvh")
   {
       myRBDScene->myCDAlgorithm->myMidPhaseAlgo = VSLibRBDynamX::RBDScene::CM_RECURSIVEBVH;
   }
   else if (midPhaseCDName.toLower() == "flatbvh")
   {
       myRBDScene->myCDAlgorithm->myMidPhaseAlgo = VSLibRBDynamX::RBDScene::CM_FLATBVH;
   }
   else
   {
       myRBDScene->myCDAlgorithm->myMidPhaseAlgo = VSLibRBDynamX::RBDScene::CM_TRAVERSAL;
   }

   //if (myRBDScene->getCollisionGrid())
   //{
   //   myRBDScene->setCollisionGridCellLength(mySettings.ReadDouble("gridCellSideLength"));
   //   myRBDScene->insertColliderGroupsIntoCollisionDetection(true);
   //}

   if (mySettings.ReadBool("createPhysicalBoundaries", false))
   {
      this->createPhysicalBoundaries(myRBDScene);
   }
   else
   {
      if (myRigidBodyBoundary)
         delete myRigidBodyBoundary;
   }

   myRBDScene->setFlag(VSLibRBDynamX::RBDScene::BodyConfigDirty, false);
}

void VSPluginRBDynamX::MainSimStateExtension::slotInitializeNewClasses(bool storeInitialFrames /*= true*/)
{
   if (!(getMyEnvironment()->getOptions() & VSD::Environment::IsMaster))
      return;

   if (!myScene)
      return;

   if (!getMySimState())
      return;

   VSD::Database* database = getMySimState()->getDatabase();

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionMaterial*, matExt, database)
   {
      matExt->initializeDynamX(myScene);
   }

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionRigidBody*, rbExt, database)
   {
      rbExt->initializeDynamX(myScene, true, storeInitialFrames);
   }

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionRigidBody*, rbExt, database)
   {
      VSDForAllRefListInstances(VSPluginRBDynamX::ExtensionRigidBody*, rbExt2, rbExt->getNoCollisionTo())
      {
         if (rbExt->getRBDRigidBody() && rbExt2->getRBDRigidBody())
         {
            rbExt->getRBDRigidBody()->addNotCollidingRigidBody(rbExt2->getRBDRigidBody());
            rbExt2->getRBDRigidBody()->addNotCollidingRigidBody(rbExt->getRBDRigidBody());
         }
      }
   }

   // for FEM
   //VSD::SimState* simstate = this->getExtensionDynamXScene()->getMyEnvironment()->getGuiSimState();

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionJoint*, jointExt, database)
   {
      /*006
      if (jointExt->inherits<ExtensionRigidBodyDirectControl*>()) // evtl isA ???
         continue;
         */

      if (jointExt->inherits<ExtensionJointDifferential*>())
         continue;

      if (jointExt->inherits<ExtensionHingePrismaticGear*>())
         continue;

      //if (jointExt->inherits<ExtensionJointGeneral*>())
      //   continue;


      jointExt->initializeDynamX(myScene);

   }

   /*
   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionTwoBodiesJoint*, jointExt, database)
   {
      jointExt->initializeDynamX(myScene);
   }

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionJointGeneral*, jointExt, database)
   {
      jointExt->initializeDynamX(myScene);
   }
   */

   // works only, if it is called extra

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionJointDifferential*, diffExt, database)
   {
      diffExt->initializeDynamX(myScene);
   }

   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionHingePrismaticGear*, gearExt, database)
   {
      gearExt->initializeDynamX(myScene);
   }

   // doesn't inherit from joint, call extra

   /*006
   VSDForAllElements(ExtensionMeltingJoint*, ext, database)
   {
      ext->initializeDynamX(myExtensionDynamXScene, this);
   }
   */


   /*
   VSDForAllElements(RigidBodyDamping*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }

   VSDForAllElements(ExtensionJointPathConstraint*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }
   */



   //(ci) needed for initialization
   VSDForAllElements(ExtensionDebugLagrangeMFile*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }


   VSDForAllElements(ExtensionContactDebugger*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }

   VSDForAllElements(ExtensionRigidBodyList*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }

   VSDForAllElements(VSPluginRBDynamX::ExtensionAerodynamicDrag*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }

   VSDForAllElements(ExtensionHydraulic*, ext, database)
   {
      ext->initializeDynamX(myScene);
   }

   myScene->setFlag(VSLibRBDynamX::RBDScene::BodyConfigDirty, true);

}

void VSPluginRBDynamX::MainSimStateExtension::createSpatiallySortedListOfExtensionRigidBodies()
{
   if (!getMySimState())
      return;

   if (!myExtRBsOrdered.isEmpty())
      myExtRBsOrdered.clear();

   QSet<ExtensionRigidBody*> ausgangsMenge;

   VSDForAllElements(ExtensionRigidBody*, extRb, getMySimState()->getDatabase())
   {
      ausgangsMenge.insert(extRb);
   }

   while (!ausgangsMenge.isEmpty())
   {
      // Nimm einen aus der Ausgangsmenge und hänge ihn hinten an
      ExtensionRigidBody* extRb = *ausgangsMenge.begin();

      myExtRBsOrdered.append(extRb);
      ausgangsMenge.remove(extRb);

      // position ist die Position in der Liste, an der die zuletzt angehängte
      // ExtRB hängt:
      int position = myExtRBsOrdered.count() - 1;

      // Gehe spatial nach oben und suche dort weitere ExtRBs
      VSD3D::Node* spatialParent = extRb->getMyModelInstance()->instanceCast<VSD3D::Node*>();
      if (!spatialParent)
         continue;

      while (spatialParent = spatialParent->getSpatialParentNode())
      {
         extRb = spatialParent->findFirstExtensionInherits<ExtensionRigidBody*>();
         if (extRb)
         {
            // Sortiere diesen extRb direkt vor den zuletzt einsortierten:
            myExtRBsOrdered.insert(position, extRb);
            ausgangsMenge.remove(extRb);
         }
      }
   }
}

void VSPluginRBDynamX::MainSimStateExtension::slotInitDynamX(bool storeInitialFrames /*= true*/)
{
   //VSL::SettingsGroup mySettings = getProject()->GetSettings()->Group(settingsGroupName);
   //mySettings.WriteEntry("dynamX", true);

   VEROSIM::Project* p = getMyEnvironment()->getMyProject();
   VSD::SimState* simState = getMySimState();
   slotInitDynamX(p, simState, storeInitialFrames);
   slotResetIsCollidingFlag();
}

bool VSPluginRBDynamX::MainSimStateExtension::getEnableRBDynamX()
{
   return getIsActive();
}

void VSPluginRBDynamX::MainSimStateExtension::setEnableRBDynamX(bool enable)
{
   setActive(enable);
}

void VSPluginRBDynamX::MainSimStateExtension::slotInitDynamX(VEROSIM::Project* p, VSD::SimState* simState, bool storeInitialFrames /*= true*/)
{
   if (!(simState->getMyEnvironment()->getOptions() & VSD::Environment::IsMaster))
      return;

   emit signalPreDynamXInitialized(simState);

   slotSyncSettings();

   if (!(simState->getMyEnvironment()->getOptions() & VSD::Environment::IsMaster))
      return;

   myExtensionDynamXScene = ExtensionDynamXScene::the(simState);

   createRBDScene(simState);

   if (!myScene)
      return;

   myExtensionDynamXScene->setRBDScene(myScene);

   // Der neue Ansatz:
   slotInitializeNewClasses(storeInitialFrames);
   // Ende neuer Ansatz

   VSL::SettingsGroup settings = p->GetSettings()->Group(settingsGroupName);

   // SpeedTree Collision Support:
   settings = p->GetSettings()->Group(settingsGroupName);
   if (settings.ReadBool("speedTreeSupport") && !mySpeedTreesBody)
   {
      mySpeedTreesBody = new VSLibRBDynamX::RBDRigidBody(myScene, 0);
      mySpeedTreesBody->setName("SpeedTreesBody");

      mySpeedTreesBody->setIsFix(true);
      mySpeedTreesBody->setCollisionDetection(true);

      //      myScene->addRootRigidBody(mySpeedTreesBody);

      VSLibRBDynamX::RBDMaterial* mat = new VSLibRBDynamX::RBDMaterial("SpeedTrees Material", myScene);

      mat->setCoulombDynamicFriction(1.0);
      mat->setCoulombStaticFriction(1.0);

      mySpeedTreesBody->setMaterial(mat);

      //      mapNamesRbs.insert("SpeedTreesBody", mySpeedTreesBody);

      VSD::Connection::connect(
         this,
         SIGNAL(signalFindAreaTrees(VSD::SimState*, const VSG::BoundingBox&, QVector<VSLibTree::TreeBase*>&)),
         VSL::MiscTools::FindChild(p, 0, "VSPluginRenderGLNatureTrees::Project"),
         SLOT(slotFindAreaTrees(VSD::SimState*, const VSG::BoundingBox&, QVector<VSLibTree::TreeBase*>&)));
   }

   if (settings.ReadBool("extOverGroundSupport"))
   {
      VSDForAllElements(ExtensionOverGround*, eog, simState->getDatabase())
      {
         eog->initialize();
      }
   }

   // apply jointValues of RigidBodyChains
   VSD::ElementIndex* rbChainsElementIndex = simState->getDatabase()->getElementIndex<ExtensionSetRigidBodyChain*>();
   VSL_ASSERT_ELSE_MSG_RETURN(rbChainsElementIndex, "ElementIndex for VSPluginRBDynamX::ExtensionSetRigidBodyChain not found");
   VSDForAllElementsInherits(ExtensionSetRigidBodyChain*, pChain, rbChainsElementIndex->getMyElementContainer())
   {
      if (pChain->getEnabled())
         pChain->applyJointValuesToJoints();
   }


   // execute compoundInitializer
   VSD::ElementIndex* compInitElementIndex = simState->getDatabase()->getElementIndex<ExtensionCompoundInitializer*>();
   VSL_ASSERT_ELSE_MSG_RETURN(compInitElementIndex, "ElementIndex for VSPluginRBDynamX::ExtensionCompoundInitializer not found");
   VSDForAllElementsInherits(ExtensionCompoundInitializer*, pCompoundInit, compInitElementIndex->getMyElementContainer())
   {
      pCompoundInit->initializeCompound();
   }

   myScene->postAssembly();

   setDoConfigUpdate(simState);

   myExtensionDynamXScene->setInitialized(true);
   emit(signalPostDynamXInitialized(simState));

   return;
}

void VSPluginRBDynamX::MainSimStateExtension::slotResetDynamXFromStoredInitialState()
{
   if (!myScene)
      return;

   if (!getIsActive())
      return;

   //Ersetzt druch SimState Reset
   myScene->restoreInitialSimulationState();

   for (ExtensionRigidBody* extRb : myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->restoreInitialState();
   }

   timeToBeDone = 0;

   // internal state is not consistent before new initialization
   getExtensionDynamXScene()->setInitialized(false);

   VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
   if (dynamXProject)
      dynamXProject->emitSignalPostDynamXReset();
}

void VSPluginRBDynamX::MainSimStateExtension::slotResetDynamXFromStoredInitialStateDuringRuntime()
{
   VSPluginRBDynamX::MainSimStateExtension* dynmaXSimStateExtension = VSPluginRBDynamX::MainSimStateExtension::findOrCreate(getMySimState());


   if (!dynmaXSimStateExtension->myScene)
      return;

   if (!dynmaXSimStateExtension->getIsActive())
      return;

   //Ersetzt druch SimState Reset
   dynmaXSimStateExtension->myScene->restoreInitialSimulationStateDuringRuntime();

   for (ExtensionRigidBody* extRb : dynmaXSimStateExtension->myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->restoreIntialStateDuringRuntime();
      rb->updateVisu(false);
   }

   dynmaXSimStateExtension->timeToBeDone = 0;

   // internal state is not consistent before new initialization
   dynmaXSimStateExtension->myExtensionDynamXScene->setInitialized(false);

   VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
   if (dynamXProject)
      dynamXProject->emitSignalPostDynamXReset();
}

void VSPluginRBDynamX::MainSimStateExtension::slotResetDynamXFromStoredIntermediateState()
{
   if (!myScene)
      return;

   if (!getIsActive())
      return;

   //Ersetzt druch SimState Reset
   myScene->restoreIntermediateSimulationState();

   for (ExtensionRigidBody* extRb : myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->restoreIntermediateState();
   }

   timeToBeDone = 0;

   // internal state is not consistent before new initialization
   getExtensionDynamXScene()->setInitialized(false);

   VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
   if (dynamXProject)
      dynamXProject->emitSignalPostDynamXReset();
}

void VSPluginRBDynamX::MainSimStateExtension::slotResetDynamXFromStoredIntermediateStateDuringRuntime()
{
   VSPluginRBDynamX::MainSimStateExtension* dynmaXSimStateExtension = VSPluginRBDynamX::MainSimStateExtension::findOrCreate(getMySimState());


   if (!dynmaXSimStateExtension->myScene)
      return;

   if (!dynmaXSimStateExtension->getIsActive())
      return;

   //Ersetzt druch SimState Reset
   dynmaXSimStateExtension->myScene->restoreIntermediateSimulationStateDuringRuntime();

   for (ExtensionRigidBody* extRb : dynmaXSimStateExtension->myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->restoreIntermediateStateDuringRuntime();
      rb->updateVisu(false);
   }

   dynmaXSimStateExtension->timeToBeDone = 0;

   // internal state is not consistent before new initialization
   dynmaXSimStateExtension->myExtensionDynamXScene->setInitialized(false);

   VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
   if (dynamXProject)
      dynamXProject->emitSignalPostDynamXReset();
}



void VSPluginRBDynamX::MainSimStateExtension::slotResetDynamX()
{
   this->runStateChanged(VSS2::RunState::Reset, VSS2::RunState::Run);
   this->runStateChanged(VSS2::RunState::Run, VSS2::RunState::Reset);
   /*if (!myScene)
      return;

   if (!getIsActive())
      return;

   myScene->restoreInitialSimulationStateDuringRuntime();

   for(ExtensionRigidBody* extRb: myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->restoreIntialStateDuringRuntime();
   }*/
}

void VSPluginRBDynamX::MainSimStateExtension::slotFreeRBDScene()
{
   VSLibRBDynamX::RBDScene::removeInstanceFor(getMySimState());
}

VSLibRBDynamX::RBDScene* VSPluginRBDynamX::MainSimStateExtension::getRBDScene() const
{
   return myScene;
}


void VSPluginRBDynamX::MainSimStateExtension::slotHandleCollision(
   VSLibRBDynamX::RBDRigidBody* b0,
   VSLibRBDynamX::RBDRigidBody* b1,
   const VSM::Vector3& pos,
   double velocity)
{
   RigidBodyPair p(b0, b1);

   QList<CollisionEvent*> ceList;
   ceList << myMapBodyPairCollisionEvent.value(p); //QT6??? Das kann eigentlich bislang nicht richtig gewesen sein. Eine QMap liefert immer genau einen Wert zurück. War hier eine QMultiMap gemeint?

   for (CollisionEvent* ce : ceList)
   {
      ce->slotTriggerCollision(pos, velocity);
   }

   if (b0->adapter() && b1->adapter())
   {
      ExtensionRigidBody* extB0 = dynamic_cast<ExtensionRigidBody*>(b0->adapter());
      if (!extB0)
         return;

      ExtensionRigidBody* extB1 = dynamic_cast<ExtensionRigidBody*>(b1->adapter());
      if (!extB1)
         return;

      VSDForAllElements(ExtensionDynamXScene*, ext, getMySimState()->getDatabase())
      {
         VSM::Vector3 vsmP = pos;
         ext->emitSignalCollision(extB0, extB1, vsmP, velocity);
      }
   }
}


// 把数值世界的最终状态压给可视世界，然后广播“保存完成”，最后在需要时结束本帧计时。
void VSPluginRBDynamX::MainSimStateExtension::saveStep()
{
   if (!myScene)
      return;

   if (!getIsActive())
      return;

   for (ExtensionRigidBody* extRb : myExtRBsOrdered.getConstList())
   {
      if (!extRb)
         continue;

      VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
      if (!rb)
         continue;

      rb->updateVisu(false);
   }

   //VSD3D::SpatialView::updateWorldFrames(getMySimState());

   // I'm done
   emit signalDynamXPostSaveStep();
   myExtensionDynamXScene->emitSignalDynamXPostSaveStep();

#ifdef DOTIMING
   myScene->signalEndCycle();
#endif
}

void VSPluginRBDynamX::MainSimStateExtension::createPhysicalBoundaries(VSLibRBDynamX::RBDScene* scene)
{
   if (myRigidBodyBoundary)
      delete myRigidBodyBoundary;

   // Umrandung um die Szene, so dass nichts ins unendliche herunterfällt:
   VSG::BoundingBox aabb = scene->compileSceneAABB();
   myRigidBodyBoundary = new VSLibRBDynamX::RBDRigidBody(scene, 0);
   myRigidBodyBoundary->setName("Body for physical scene boundaries");
   myRigidBodyBoundary->setCollisionDetection(true);
   myRigidBodyBoundary->setIsFix(true);

   VSLibRBDynamX::RBDShapePlane* plane = 0;

   VSM::PoseVector3Quaternion pq;
   pq.setPosition(VSM::Vector3(0, aabb.getMinPos()[1], 0));
   pq.setOrientation(VSM::Matrix3x3(1, 0, 0, 0, 0, 1, 0, -1, 0));
   plane = new VSLibRBDynamX::RBDShapePlane(myRigidBodyBoundary, 0);
   plane->setAbsPose(pq);
   plane->setCollisionDetection(true);
   plane->setMass(1.0);
   myRigidBodyBoundary->addChild(plane);

   pq.setPosition(VSM::Vector3(0, aabb.getMaxPos()[1], 0));
   pq.setOrientation(VSM::Matrix3x3(1, 0, 0, 0, 0, -1, 0, 1, 0));
   plane = new VSLibRBDynamX::RBDShapePlane(myRigidBodyBoundary, 0);
   plane->setAbsPose(pq);
   plane->setCollisionDetection(true);
   plane->setMass(1.0);
   myRigidBodyBoundary->addChild(plane);

   pq.setPosition(VSM::Vector3(aabb.getMinPos()[0], 0, 0));
   pq.setOrientation(VSM::Matrix3x3(0, 0, 1, 0, 1, 0, -1, 0, 0));
   plane = new VSLibRBDynamX::RBDShapePlane(myRigidBodyBoundary, 0);
   plane->setAbsPose(pq);
   plane->setCollisionDetection(true);
   plane->setMass(1.0);
   myRigidBodyBoundary->addChild(plane);

   pq.setPosition(VSM::Vector3(aabb.getMaxPos()[0], 0, 0));
   pq.setOrientation(VSM::Matrix3x3(0, 0, -1, 0, 1, 0, 1, 0, 0));
   plane = new VSLibRBDynamX::RBDShapePlane(myRigidBodyBoundary, 0);
   plane->setAbsPose(pq);
   plane->setCollisionDetection(true);
   plane->setMass(1.0);
   myRigidBodyBoundary->addChild(plane);

   pq.setPosition(VSM::Vector3(0, 0, aabb.getMinPos()[2]));
   pq.setOrientation(VSM::Matrix3x3(1, 0, 0, 0, 1, 0, 0, 0, 1));
   plane = new VSLibRBDynamX::RBDShapePlane(myRigidBodyBoundary, 0);
   plane->setAbsPose(pq);
   plane->setCollisionDetection(true);
   plane->setMass(1.0);
   myRigidBodyBoundary->addChild(plane);

   VSLibRBDynamX::RBDMaterial* mat = new VSLibRBDynamX::RBDMaterial("PhysicalBoundaryMaterial", scene);
   mat->setCoulombDynamicFriction(1.0);
   mat->setCoulombStaticFriction(1.0);
   myRigidBodyBoundary->setMaterial(mat);
   myRigidBodyBoundary->updateCogAndMassAndInertiaAndTwist();
}

void VSPluginRBDynamX::MainSimStateExtension::updateSpeedTreesBody()
{
   if (!mySpeedTreesBody)
      return;

   VSG::BoundingBox aabb;
   aabb.invalidate();

   for (VSLibRBDynamX::RBDRigidBody* b : mySpeedTreeColliderBodies)
   {
      if (b->getParentRigidBody(true) && b->getParentRigidBody(true)->isFix())
         continue;
      aabb.expandBy(*b->getAABB());
   }

   // Clean up current state:
   for (VSLibRBDynamX::SceneElement* shape : mySpeedTreesBody->getChildren())
   {
      mySpeedTreesBody->removeChild(shape);
      //      mySpeedTreesBody->updateCogAndMassAndInertiaAndTwist();

      delete (shape);
   }

   QVector<VSLibTree::TreeBase*> relevantTrees;

   VSM::Vector3 rbmMin(aabb.getMinPos()[0], aabb.getMinPos()[1], aabb.getMinPos()[2]);
   VSM::Vector3 rbmMax(aabb.getMaxPos()[0], aabb.getMaxPos()[1], aabb.getMaxPos()[2]);

   VSM::Vector3 min = rbmMin;
   // OLD: VSM::Vector3 min = vsmVector3FromRBMVector3(rbmMin, 0);
   VSM::Vector3 max = rbmMax;
   // OLD: VSM::Vector3 max = vsmVector3FromRBMVector3(rbmMax, 0);


   VSG::BoundingBox vsgBb(min, max);

   emit signalFindAreaTrees(getMySimState(), vsgBb, relevantTrees);

   // Mappen auf eigenen Typ:
   QVector<VSLibTree::TreeBase*>::const_iterator itTrees = relevantTrees.constBegin();
   while (itTrees != relevantTrees.constEnd())
   {
      VSLibTree::TreeBase* tree = (*itTrees);

      VSLibRBDynamX::RBDShapeCylsphere* cylsphere = new VSLibRBDynamX::RBDShapeCylsphere(
         mySpeedTreesBody, 0);
      cylsphere->setHeight(tree->getTreeHeight());

      if (tree->getDiameterBreastHeight() > 0)
         cylsphere->setRadius(tree->getDiameterBreastHeight() / 2 / 100); // given in cm, tbd
      else
         cylsphere->setRadius(.3);

      cylsphere->setCollisionDetection(true);
      cylsphere->setMass(1000.0);

      VSM::PoseVector3Quaternion pq = PoseVector3QuaternionFromVSMFrame(tree->getWorldFrame(), 0);
      pq.setOrientation(VSM::Quaternion());

      //pq.setPosition(VSM::Vector3(pq.getPosition()[0], pq.getPosition()[1], pq.getPosition()[2] + cylsphere->getHeight() / 2));

      // Wenn die Bäume auf z=0 gezogen sind:
      VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
      if (settings.ReadBool("speedTreesToZero", false))
         pq.setPosition(VSM::Vector3(pq.getPosition()[0], pq.getPosition()[1], cylsphere->getHeight() / 2));
      else
         pq.setPosition(VSM::Vector3(pq.getPosition()[0], pq.getPosition()[1], pq.getPosition()[2] + cylsphere->getHeight() / 2));

      cylsphere->setAbsPose(pq);

      mySpeedTreesBody->addChild(cylsphere);

      ++itTrees;
   }

   mySpeedTreesBody->updateCogAndMassAndInertiaAndTwist(false, false);
   mySpeedTreesBody->setAabbDirty(true);

   if (mySpeedTreesBody->getMass() <= VSM::epsilon)
   {
      delete mySpeedTreesBody;
   }
}

VSPluginRBDynamX::ExtensionDynamXScene* VSPluginRBDynamX::MainSimStateExtension::getExtensionDynamXScene() const
{
   return myExtensionDynamXScene;
}

void VSPluginRBDynamX::MainSimStateExtension::addGluedModel(VSPluginRBDynamX::TTGluableDynamicModel* model)
{
   myGluedDynamicModels.insert(model);
}

void VSPluginRBDynamX::MainSimStateExtension::runStateChanged(VSS2::RunState runState, VSS2::RunState oldRunState)
{
   VSS2::TaskStep::runStateChanged(runState, oldRunState);

   switch (runState)
   {
   case(VSS2::RunState::Reset):
   {
      if (!myScene)
         break;

      if (!getIsActive())
         break;

      //Ersetzt druch SimState Reset 
      myScene->restoreInitialSimulationState();

      for (ExtensionRigidBody* extRb : myExtRBsOrdered.getConstList())
      {
         if (!extRb)
            continue;

         VSLibRBDynamX::RBDRigidBody* rb = extRb->getRBDRigidBody();
         if (!rb)
            continue;

         rb->restoreInitialState();
         rb->resetContacts();
      }

      timeToBeDone = 0;

      // internal state is not consistent before new initialization
      getExtensionDynamXScene()->setInitialized(false);

      VSPluginRBDynamX::Project* dynamXProject = getMyEnvironment()->getMyProject()->findChild<VSPluginRBDynamX::Project*>();
      if (dynamXProject)
         dynamXProject->emitSignalPostDynamXReset();

      break;
   }
   case(VSS2::RunState::Pause):
   {
      break;
   }
   case(VSS2::RunState::Run):
   {
      VSL::SettingsGroup mySettings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
      bool dynamXOn = mySettings.ReadBool("dynamX");

      if ((oldRunState == VSS2::RunState::Reset) && dynamXOn)
      {
         checkModelStructure();
         VSL::SettingsGroup mySettings = getMyEnvironment()->getMyProject()->GetSettings()->Group(settingsGroupName);
         bool dynamXOn = mySettings.ReadBool("dynamX");
         slotInitDynamX();

         // Für den "Spatially save" SaveStep
         createSpatiallySortedListOfExtensionRigidBodies();
      }
      break;
   }
   }
   oldRunState = runState;
}

VSPluginRBDynamX::ExtensionRigidBody* VSPluginRBDynamX::MainSimStateExtension::findRigidBodyByName(VSD::SimState* simState, const QString& name)
{
   if (!simState)
      return 0;

   if (!simState->getDatabase())
      return 0;

   VSDForAllElements(VSPluginRBDynamX::ExtensionRigidBody*, extBody, simState->getDatabase())
   {
      if (extBody->getName() == name)
         return extBody;
   }

   return 0;
}

VSPluginRBDynamX::ExtensionRigidBody* VSPluginRBDynamX::MainSimStateExtension::findBodyByNodeName(VSD::SimState* simState, const QString& name)
{
   if (!simState)
      return 0;

   if (!simState->getDatabase())
      return 0;

   VSDForAllElements(VSPluginRBDynamX::ExtensionRigidBody*, extBody, simState->getDatabase())
   {
      VSD::Node* node = extBody->getMyModelInstance()->instanceCast<VSD::Node*>();
      if (!node)
         continue;

      if (node->getName() == name)
         return extBody;
   }

   return 0;
}

VSD3D::Node* VSPluginRBDynamX::MainSimStateExtension::findNodeByName(VSD::SimState* simState, const QString& name)
{
   if (!simState)
      return 0;

   if (!simState->getDatabase())
      return 0;

   VSDForAllElementsInherits(VSD3D::Node*, node, simState->getDatabase()/*ei->getMyElementContainer()*/)
   {
      if (node->getName() == name)
         return node;
   }
   return 0;
}

// TODO: Sollte nicht Teil der DynamX sein. Gibt es eine Funktion in der VSM, die dies macht?
VSM::PoseVector3Quaternion VSPluginRBDynamX::MainSimStateExtension::PoseVector3QuaternionFromVSMFrame(
   const VSM::Frame& f,
   VSD::Model* model)
{
   return VSM::PoseVector3Quaternion(
      f.getPosition(),
      // OLD: rbmVector3FromVSMVector3(f.getPosition(), model),
      rbmQuaternionFromVSMFrame(f, model));
}

VSM::Frame VSPluginRBDynamX::MainSimStateExtension::vsmFrameFromPoseVector3Quaternion(
   const VSM::PoseVector3Quaternion& f,
   VSD::Model* model)
{
   // TODO: nach VSM?

   VSM::Frame result;

   VSM::Vector3 x(1.0, 0, 0);
   VSM::Vector3 y(0, 1.0, 0);
   VSM::Vector3 z(0, 0, 1.0);

   x = f.getOrientation() * x;
   y = f.getOrientation() * y;
   z = f.getOrientation() * z;

   result.element(0, 0) = x[0];
   result.element(1, 0) = x[1];
   result.element(2, 0) = x[2];

   result.element(0, 1) = y[0];
   result.element(1, 1) = y[1];
   result.element(2, 1) = y[2];

   result.element(0, 2) = z[0];
   result.element(1, 2) = z[1];
   result.element(2, 2) = z[2];

   result.element(0, 3) = f.getPosition()[0];
   result.element(1, 3) = f.getPosition()[1];
   result.element(2, 3) = f.getPosition()[2];

   return result;
}

VSM::Quaternion VSPluginRBDynamX::MainSimStateExtension::rbmQuaternionFromVSMFrame(
   const VSM::Frame& frame,
   VSD::Model* model)
{
   Q_UNUSED(model);

   return frame.getOrientation().getQuaternion();
}


// LEGACY: Sollte nicht länger benutzt werden! (san: ich habe alle Verweise aus dem Quellcode entfernt)
VSM::Vector3 VSPluginRBDynamX::MainSimStateExtension::vsmVector3FromRBMVector3(
   const VSM::Vector3& f,
   VSD::Model* model)
{
   // TODO: Unnötig nach Einheitenumstellung -> entfernen
   return VSM::Vector3(
      f[0],
      f[1],
      f[2]);
}

// LEGACY: Sollte nicht länger benutzt werden!
VSM::Vector3 VSPluginRBDynamX::MainSimStateExtension::rbmVector3FromVSMVector3(
   const VSM::Vector3& v,
   VSD::Model* model)
{
   // TODO: Unnötig nach Einheitenumstellung -> entfernen (san: ich habe alle Verweise aus dem Quellcode entfernt)

   return VSM::Vector3(
      v[0],
      v[1],
      v[2]);
}
// LEGACY: Sollte nicht länger benutzt werden! (san: ich habe alle Verweise aus dem Quellcode entfernt)
VSM::Vector6 VSPluginRBDynamX::MainSimStateExtension::rbmVector6FromVSMVector6(
   const VSM::Vector6& f)
{
   return VSM::Vector6(f[0], f[1], f[2], f[3], f[4], f[5]);
}

void VSPluginRBDynamX::MainSimStateExtension::setDoSimulation(bool doSimulation)
{
   setActive(doSimulation);
}

void VSPluginRBDynamX::MainSimStateExtension::activateDynamX(bool storeInitialFrames /*= true*/)
{
   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group("/Extensions/VSPluginRBDynamX");
   settings.WriteEntry("dynamX", 1);
   setDoSimulation(true);
   slotInitDynamX(storeInitialFrames);
}

void VSPluginRBDynamX::MainSimStateExtension::deactivateDynamX()
{
   VSL::SettingsGroup settings = getMyEnvironment()->getMyProject()->GetSettings()->Group("/Extensions/VSPluginRBDynamX");
   settings.WriteEntry("dynamX", 0);
   setDoSimulation(false);
}

void VSPluginRBDynamX::MainSimStateExtension::slotUpdateIsCollidingFlag()
{
   VSD::Database* database = getMySimState()->getDatabase();

   VSLibRBDynamX::RBDContact** contacts = getRBDScene()->getCurrentContacts();
   int numberContacts = getRBDScene()->getNumberCurrentContacts();

   VSDForAllElements(VSPluginRBDynamX::ExtensionRigidBody*, rb, database)
   {
      VSLibRBDynamX::RBDRigidBody* curBody = rb->getRBDRigidBody();
      bool isColliding = false;
      for (int i = 0; i < numberContacts; i++)
      {
         VSLibRBDynamX::RBDContact* curContact = contacts[i];
         if (curContact->body(0, true) == curBody || curContact->body(1, true) == curBody)
         {
            isColliding = true;
            break;
         }
      }
      if (rb->getIsColliding() != isColliding)
         rb->setIsColliding(isColliding);
   }
}


void VSPluginRBDynamX::MainSimStateExtension::slotResetIsCollidingFlag()
{
   VSD::Database* database = getMySimState()->getDatabase();

   VSDForAllElements(VSPluginRBDynamX::ExtensionRigidBody*, rb, database)
   {
      rb->setIsColliding(false);
   }
}


bool VSPluginRBDynamX::MainSimStateExtension::checkModelStructure() const
{
   // get VSD database
   VSD::Database* database = getMySimState()->getDatabase();

   // searches entire VSD for elements of a given type
   VSDForAllElements(VSPluginRBDynamX::ExtensionRigidBody*, rb, database)
   {
      // get parent of specific type
      VSD3D::Node* parent = rb->getParent<VSD3D::Node*>();

      // get all child node of a given node
      VSD::PropertyRefListWrapperTemplate<VSD::Node> childNodes = parent->getChildNodes();

      // iterate over all nodes in RefList
      VSDForAllRefListInstances(VSD::Node*, child, childNodes)
      {
         VSD::PropertyRefListWrapperTemplate<VSD::ModelInstanceExtension> extensions = child->getExtensions();
         VSDForAllRefListInstances(VSPluginRBDynamX::ExtensionRigidBody*, rb, extensions)
         {
            if (rb)
            {
               qDebug() << "VSPluginRBDynamX: Invalid model structure. The model has an invalid tree like structure.";
               //#ifdef VS_WIDGETS
               //                    QMessageBox message;
               //                    message.critical(getMyEnvironment()->getMyProject()->getMainWindow()->getQMainWindowWidget(), "VSPluginRBDynamX: Invalide model structure", "The model has an invalide tree like structure. The simulation will not start.");
               //                    message.show();
               //#else
               //                   vslError() << "VSPluginRBDynamX: Invalid model structure. The model has an invalid tree like structure. The simulation will not start.";
               //#endif
               //                    VSS2::resetSimulation(*getMyEnvironment());
            }
            //getMyEnvironment()->showError("You did a mistake!");
         }

         //VSDForAllRecursiveChildrenInherits(VSD3D::Node*, childInherits, child)
         ////{
         //VSDForAllChildrenOfAllTypes(VSPluginRBDynamX::ExtensionRigidBody*, rbChild, child)
         //{

         // }

         //}
      }

      // iterate over all nodes in RefList
      //VSDForAllRefListInstances(VSD::Node*, child, childNodes)

      // iterate over all nodes recursively
      //VSDForAllRecursiveChildren(VSD::Node*, node, parent)
      //{
      //}

      // iterate over all nodes recursively that inherite from a given type
      //VSDForAllRecursiveChildrenInherits(VSD3D::Node*, node, parent)
      //{

      //}

      // get parent of element
      //VSD::SimStateInstance* instance = rb->getParent();
      //VSD3D::Node* parent2 = instance->instanceCast<VSD3D::Node*>();

      //qDebug() << rb->getIsFix();

      //getMyEnvironment()->showError("You did a mistake!");

   }

   // iterate over all elements that inherit from ExtensionTwoBodiesJoint
   VSDForAllElementsInherits(VSPluginRBDynamX::ExtensionTwoBodiesJoint*, joint, database)
   {
   }

   return true;
}

