#include "./RBDClusterPenalty.h"

#include "../RBDContact.h"
#include "../RBDRigidBody.h"

VSLibRBDynamX::RBDClusterPenalty::RBDClusterPenalty(RBDScene* scene)
: RBDCluster(scene)
{
}

VSLibRBDynamX::RBDClusterPenalty::~RBDClusterPenalty()
{
}


void VSLibRBDynamX::RBDClusterPenalty::doTimeStep(
   double newTime, 
   double delta_t)
{
   Q_UNUSED(newTime); 

   int numberFoundContacts = 0; 
   myConstraintResources.clear(); 

   RBDRigidBodyPtrSet::const_iterator itRb = getRigidBodies().constBegin(); 
   // 遍历本簇刚体，收集接触（并限定属于本簇）
   for (; itRb != getRigidBodies().constEnd(); ++itRb)
   {
      RBDRigidBody* body = *itRb; 

      //getContacts(true)：拿到该刚体当前帧的接触列表。
      const PtrList<RBDContact>& contacts = body->getContacts(true); 
      QList<RBDContact*>::const_iterator itContact = contacts.constBegin(); 
      for (; itContact != contacts.constEnd(); ++itContact)
      {
         RBDContact* contact = *itContact; 
         
         // handled 标志：避免同一接触被“双方刚体”重复放入资源表
         if (!contact->getHandled() && !contact->getDisabled())
         {

            // Wenn eine Szene einen fixen Körper enthält (Boden), so wird dieser gegebenenfalls 
            // in mehrere Cluster eingefügt. Daher können hier Kontakte auftauchen, von denen einer der
            // beiden beteiligten Körper nich in diesem Cluster liegt. 
            // 如果某一侧是“固定体”（地面），允许跨簇；否则要求参与接触的两侧非固定体都在本簇。
            if (!contact->body(0)->isFix() && contact->body(0)->getCluster() != this)
               continue; 

            if (!contact->body(1)->isFix() && contact->body(1)->getCluster() != this)
               continue; 

            ++numberFoundContacts; 
            myConstraintResources.add(contact); 
            contact->setHandled(true); 

//            contact->calculate(); 
//            numberComplementaryConstraints += contact->getNumberComplementaryConstraints(); 
         }
      }
   }
   // 对每个接触，构造法向“弹簧+阻尼”力并施加
   for(RBDConstraintResource* constraintRes: myConstraintResources )
   {
      if (constraintRes->getDisabled())
         continue; 

      constraintRes->setHandled(false); 

//      constraintRes->setEqualityConstraintsOffset(currentRow); 
      //  // 防御式再次检查簇归属
      if ( !constraintRes->body(0)->isFix() && constraintRes->body(0)->getCluster() != this )
         continue; 

      if ( constraintRes->body(1) && !constraintRes->body(1)->isFix() && constraintRes->body(1)->getCluster() != this )
         continue; 

      RBDContact* contact = constraintRes->dynamicCast<RBDContact*>(); 
      if (contact)
      {
         double d = contact->getDepth();  //  // 穿透深度（通常 d>0 表示穿透；需确认你库的符号约定）
         
         // // 1) 弹簧力（刚度常数=500，并乘以质量以实现“加速度标定”）
         contact->body(0, true)->addForceOffCenter(contact->position(), -500*d*contact->normal()*contact->body(0, true)->getMass()); 
         contact->body(1, true)->addForceOffCenter(contact->position(), 500*d*contact->normal()*contact->body(1, true)->getMass()); 
         // 2) 阻尼力（阻尼系数=150，并乘以质量以实现“加速度标定”）
         if (contact->getRelativeNormalSpeedNorm() < 0)
         {
            contact->body(0, true)->addForceOffCenter(contact->position(), -150*fabs(contact->getRelativeNormalSpeedNorm())*contact->normal()*contact->body(0, true)->getMass()); 
            contact->body(1, true)->addForceOffCenter(contact->position(), 150*fabs(contact->getRelativeNormalSpeedNorm())*contact->normal()*contact->body(1, true)->getMass()); 
         }
      }
   }

   for(RBDRigidBody* rBody: getRigidBodies())
   {
      rBody->evolveStateUnconstrained(delta_t); 
      rBody->resetForceAndTorque(); 
   }
}

