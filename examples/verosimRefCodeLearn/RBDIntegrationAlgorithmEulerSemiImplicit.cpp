#include "../RBDIntegrationAlgorithmEulerSemiImplicit.h"

VSLibRBDynamX::IntegrationAlgorithmEulerSemiImplicit::IntegrationAlgorithmEulerSemiImplicit(RBDScene* scene)
   : IntegrationAlgorithmBaseSemiImplicit(scene)
{
}

VSLibRBDynamX::IntegrationAlgorithmEulerSemiImplicit::~IntegrationAlgorithmEulerSemiImplicit()
{
}

VSLibRBDynamX::DynamXState* VSLibRBDynamX::IntegrationAlgorithmEulerSemiImplicit::performeIntegrationStep(VSLibRBDynamX::DynamXState* currentState, double dt, double newTime)
{
   // compute derivative at current state
   DynamXStateDerivative* derivative = calculateDerivative(currentState, dt, newTime);
   // add derivative to current state to get new state
   DynamXState* newState = this->addDerivativeToStateSemiImplicitPositionUpdate(currentState, derivative, dt);
   // normalize orientations
   normalizeOrientationDynamXState(newState);
   // clean up
   delete derivative;
   delete currentState;
   
   return newState;
}