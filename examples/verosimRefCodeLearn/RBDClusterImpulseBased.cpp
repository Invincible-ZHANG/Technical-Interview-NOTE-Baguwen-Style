// 碰撞反弹被关了、接触/关节混用一套位置修正规则、
// 缺少跨簇过滤、没有有效质量/投影/热启动/迭代上限
// 没有右端项，没有雅可比？？
// 没有反弹
// 我觉得这个代码应该是，仅供学习参考，而没有任何实际工程价值的。
#include "./RBDClusterImpulseBased.h"

#include "../RBDScene.h"
#include "../RBDContact.h"
#include "../RBDRigidBody.h"
#include "../../VSLibRBDynMath/VSLibRBDynMathMatrix.h"

VSLibRBDynamX::RBDClusterImpulseBased::RBDClusterImpulseBased(RBDScene* scene)
: RBDCluster(scene)
{
}

VSLibRBDynamX::RBDClusterImpulseBased::~RBDClusterImpulseBased()
{
}

void VSLibRBDynamX::RBDClusterImpulseBased::doTimeStep(
	double newTime, 
	double dt)
{
   Q_UNUSED(newTime); 

   // 1. Schritt: Nur Auflösung der ersten Kollision! 

   int numberFoundContacts = 0; 
   QList<RBDContact*> handledContacts; 
   RBDConstraintResourcePtrSet constraintResources; 
   QList<VSLibRBDynMath::RBMMatrix> KMatrices; 


   // 先做一次“无约束积分”（预测态）
   // 不考虑任何约束/接触，只用外力把刚体状态推进到“预测态”（速度/位置被更新）。
   for(RBDRigidBody* b: getRigidBodies())
   {
      b->evolveStateUnconstrained(dt); 
   }

   // 扫描并收集所有约束资源（接触 + 关节）
   // Durchlauf aller Bodies
   RBDRigidBodyPtrSet::const_iterator itRb = getRigidBodies().constBegin(); 
   for (; itRb != getRigidBodies().constEnd(); ++itRb)
   {
      RBDRigidBody* body = *itRb; 

      // 先收集接触（Contact）
      const PtrList<RBDContact>& contacts = body->getContacts(true); 
      QList<RBDContact*>::const_iterator itContact = contacts.constBegin(); 
      for (; itContact != contacts.constEnd(); ++itContact)
      {
         // Erster Durchlauf: Auflösen aller Kollisionen, also nicht bleibender Kontakte. 
         // Kollisionen werden in unendlich kurzer Zeit aufgelöst! 
         RBDContact* contact = *itContact; 

         if (contact->getHandled())  //  // 去重
            continue; 

         if (!contact->getDisabled())  // 跳过禁用
         {
            handledContacts.append(contact); 
            contact->setHandled(true); 

            ++numberFoundContacts; 

            // Handelt es sich tatsächlich um eine Kollision ( oder einen bleibenden Kontakt )? 
            contact->compileKMatrix(dt);  // // 预计算接触核（未在本函数里直接使用）

            if (true)//!contact->doCollisionResponse())
            {
               // // 把“持续接触/约束”留给后面做位置修正
               // Bleibende Kontakte werden später behandelt
               constraintResources.add(contact); 

               // Die Körper bewegen sich auseinander
               continue; 
            }
/*

            contact->doCollisionResponse(); 
*/

         }
      }

      // aufsammeln aller weiteren resources
      // // 再收集该刚体的其它约束资源（关节等）
      for(RBDConstraintResource* joint: body->getConstraintResources())
      {
         if (joint->getDisabled())
            continue; 

         constraintResources.add(joint); 
      }
   }

   // 清理现场，方便下一时间步重新扫描。
   for(RBDContact* contact: handledContacts)
   {
      contact->setHandled(false); 
   }

   bool allConstraintsSolved = false; 

   qDebug() << "#contacts: " << handledContacts.size(); 

   int iteration = 0; 
   // 位置修正主循环（Gauss–Seidel 风格，逐约束迭代）
   while (!constraintResources.isEmpty() && !allConstraintsSolved)
   {
      allConstraintsSolved = true; 

      for(RBDConstraintResource* contact: constraintResources)
      {
         // Die bleibenden (resting) contacts auflösen, die keine Kollision waren: 
         // Zunächst den Fehler berechnen der auftritt, wenn der Kontakt nicht behandelt wird: 
         //VSM::Vector3 errorInNextStep = contact->getPositionalErrorInNextStep(dt);
         // // 评估：如果不处理，这个约束在下个时间步的“绝对位置误差”是多少
         contact->compileAbsPositionalErrorInNextStep(dt); 
         double d1 = contact->getAbsPositionalErrorInNextStep(); 

         // // 判定：阈值是 -1e-6（通常，负号表示“穿透/违约”）
         if (d1 < -1e-6)
         {
            allConstraintsSolved = false; 
         }
         else
         {
            continue;  // // 这个约束已满足，跳过
         }

         //  // 关键动作：对该约束施加一次“位置修正冲量”
         contact->applyPositionalCorrectionImpulse(dt, iteration); 
      }

      ++iteration; 
   }

   qDebug() << "Anzahl Iterationen: " << iteration; 
}

