/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig, Dennis Weyland                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/ParameterVariables.h>
#include <analysis/variables/ROEVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/EventShapeContainer.h>

#include <analysis/utility/ReferenceFrame.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/Conversion.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/KLMCluster.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

#include <cmath>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

namespace Belle2 {
  namespace Variable {

    Manager::FunctionPtr foxWolframR(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 1) {
        B2ERROR("foxWolframR cannot be called without providing the moment order");
        return nullptr;
      }

      int order = std::stoi(arguments[0]);

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }

      auto func = [order](const Particle*) -> double{

        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (evtShapeCont->getFWMoment(0) == 0)
        {
          B2ERROR("The 0th-order FoxWolfram moment is zero");
          return std::numeric_limits<float>::quiet_NaN();
        }
        return evtShapeCont->getFWMoment(order) / evtShapeCont->getFWMoment(0);
      };
      return func;
    }



    Manager::FunctionPtr foxWolframH(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 1) {
        B2ERROR("foxWolframH cannot be called without providing the moment order");
        return nullptr;
      }

      int order = std::stoi(arguments[0]);

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }

      auto func = [order](const Particle*) -> double{

        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        return evtShapeCont->getFWMoment(order);
      };
      return func;
    }



    Manager::FunctionPtr multipoleMoment(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("multipoleMoment requires two arguments: the multipole order (0-8) and the reference axis name (thrust or collision)");
        return nullptr;
      }

      int order = std::stoi(arguments[0]);
      std::string axisName =  arguments[1];
      boost::to_lower(axisName);

      if (order < 0 || order > 8) {
        B2ERROR("The Fox-Wolfram moment order must be within 0 and 8.");
        return nullptr;
      }
      if (axisName != "thrust" && axisName != "collision") {
        B2ERROR("Invalid axis name "  << arguments[1] << ". The valid options are thrust and collision");
        return nullptr;
      }

      auto func = [order, axisName](const Particle*) -> double{
        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (axisName == "thrust")
          return evtShapeCont->getMultipoleMomentThrust(order);
        else
          return evtShapeCont->getMultipoleMomentCollision(order);
      };
      return func;
    }


    Manager::FunctionPtr cleoCone(const std::vector<std::string>& arguments)
    {
      if (arguments.size() < 2) {
        B2ERROR("cleoCone requires two arguments: the cone order (0-9) and the reference axis name (thrust or collision)");
        return nullptr;
      }

      int order = std::stoi(arguments[0]);
      std::string axisName =  arguments[1];
      boost::to_lower(axisName);

      if (order < 0 || order > 8) {
        B2ERROR("The CLEO cone order must be within 0 and 9.");
        return nullptr;
      }
      if (axisName != "thrust" && axisName != "collision") {
        B2ERROR("Invalid axis name "  << arguments[1] << ". The valid options are thrust and collision");
        return nullptr;
      }

      auto func = [order, axisName](const Particle*) -> double{
        StoreObjPtr<EventShapeContainer> evtShapeCont;
        if (!evtShapeCont)
        {
          B2ERROR("No EventShapeContainer object has been found in the datastore");
          return std::numeric_limits<float>::quiet_NaN();
        }
        if (axisName == "thrust")
          return evtShapeCont->getCleoConeThrust(order);
        else
          return evtShapeCont->getCleoConeCollision(order);
      };
      return func;
    }



    double foxWolframR1(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(1) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR2(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(2) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR3(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(3) / evtShapeCont->getFWMoment(0);
    }

    double foxWolframR4(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      if (evtShapeCont->getFWMoment(0) == 0) {
        B2ERROR("The 0th-order FoxWolfram moment is zero");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getFWMoment(4) / evtShapeCont->getFWMoment(0);
    }


    double sphericity(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      // (3/2)(lamda_2 + lambda_3)
      return 1.5 * (evtShapeCont->getSphericityEigenvalue(1) + evtShapeCont->getSphericityEigenvalue(2)) ;
    }

    double aplanarity(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      // (3/2)(lambda_3)
      return 1.5 * evtShapeCont->getSphericityEigenvalue(2);
    }

    double forwardEmisphereMass(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardEmisphere4Momentum().Mag();
    }

    double forwardEmisphereX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardEmisphere4Momentum().Vect().X();
    }

    double forwardEmisphereY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardEmisphere4Momentum().Vect().Y();
    }

    double forwardEmisphereZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardEmisphere4Momentum().Vect().Z();
    }

    double forwardEmisphereMomentum(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getForwardEmisphere4Momentum().Vect().Mag();
    }



    double backwardEmisphereMass(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardEmisphere4Momentum().Mag();
    }


    double backwardEmisphereX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardEmisphere4Momentum().Vect().X();
    }

    double backwardEmisphereY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardEmisphere4Momentum().Vect().Y();
    }

    double backwardEmisphereZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardEmisphere4Momentum().Vect().Z();
    }

    double backwardEmisphereMomentum(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getBackwardEmisphere4Momentum().Vect().Mag();
    }






    double thrust(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrust();
    }


    double thrustAxisX(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().X();
    }

    double thrustAxisY(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().Y();
    }

    double thrustAxisZ(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().Z();
    }


    double thrustAxisCosTheta(const Particle*)
    {
      StoreObjPtr<EventShapeContainer> evtShapeCont;
      if (!evtShapeCont) {
        B2ERROR("No EventShapeContainer object has been found in the datastore");
        return std::numeric_limits<float>::quiet_NaN();
      }
      return evtShapeCont->getThrustAxis().CosTheta();
    }


    Manager::FunctionPtr useThrustFrame(const std::vector<std::string>& arguments)
    {
      if (arguments.size() == 1) {
        auto variableName = arguments[0];

        const Variable::Manager::Var* var = Manager::Instance().getVariable(arguments[0]);

        auto func = [var](const Particle * particle) -> double {
          StoreObjPtr<EventShapeContainer> evtShapeCont;
          if (!evtShapeCont)
          {
            B2ERROR("No EventShapeContainer object has been found in the datastore");
            return std::numeric_limits<float>::quiet_NaN();
          }

          TVector3 newZ = evtShapeCont->getThrustAxis();
          TVector3 newY(0, 0, 0);
          if (newZ(2) == 0 and newZ(1) == 0)
            newY(0) = 1;
          else{
            newY(1) = newZ(2);
            newY(2) = -newZ(1);
          }
          TVector3 newX = newY.Cross(newZ);

          UseReferenceFrame<CMSRotationFrame> signalframe(newX, newY, newZ);

          return var->function(particle);
        };
        return func;
      } else {
        B2FATAL("Wrong number of arguments for meta function useThrustFrame. It only takes one argument, the variable name.");
      }
    }


    VARIABLE_GROUP("EventShape");

    REGISTER_VARIABLE("foxWolframR(i)", foxWolframR,
                      "[Eventbased] ratio of the i-th to the 0-th order Fox Wolfram moments. The order can go up to 8th.");
    REGISTER_VARIABLE("foxWolframH(i)", foxWolframH, "[Eventbased] i-th order Fox Wolfram moment. The order can go up to 8th.");
    REGISTER_VARIABLE("multipoleMoment(i, axisName)", multipoleMoment,
                      "[Eventbased] i-th order multipole moment, calculated respect to the axis axisName. The order can go up to 8th., the axisName can be either 'thrust' or 'collision'");
    REGISTER_VARIABLE("cleoCone(i, axisName)", cleoCone,
                      "[Eventbased] i-th order cleoCone, calculated respect to the axis axisName. The order can go up to 8th., the axisName can be either 'thrust' or 'collision'");
    REGISTER_VARIABLE("useThrustFrame(variable)", useThrustFrame, "Evaluates a variable value in the thrust reference frame.");


    REGISTER_VARIABLE("foxWolframR1", foxWolframR1,
                      "[Eventbased]  ratio of the 1-st to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(1) defined for the user's covenience.");
    REGISTER_VARIABLE("foxWolframR2", foxWolframR2,
                      "[Eventbased]  ratio of the 2-nd to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(2) defined for the user's covenience.");
    REGISTER_VARIABLE("foxWolframR3", foxWolframR3,
                      "[Eventbased]  ratio of the 3-rd to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(3) defined for the user's covenience.");
    REGISTER_VARIABLE("foxWolframR4", foxWolframR4,
                      "[Eventbased]  ratio of the 4-th to the 0-th order Fox Wolfram moments. This is just an alias of foxWolframR(4) defined for the user's covenience.");
    REGISTER_VARIABLE("sphericity", sphericity,
                      "[Eventbased] Event sphericity, defined as the linear combination of the sphericity eigenvlaues S = (3/2)(lambda2+lambda3)");
    REGISTER_VARIABLE("aplanarity", aplanarity,
                      "[Eventbased] Event aplanarity, defined as the 3/2 of the third sphericity eigenvalue.");

    REGISTER_VARIABLE("thrust", thrust, "[Eventbased] Event thrust.");
    REGISTER_VARIABLE("thrustAxisX", thrustAxisX, "[Eventbased] X component of the thrust axis.");
    REGISTER_VARIABLE("thrustAxisY", thrustAxisY, "[Eventbased] Y component of the thrust axis.");
    REGISTER_VARIABLE("thrustAxisZ", thrustAxisZ, "[Eventbased] Z component of the thrust axis.");
    REGISTER_VARIABLE("thrustAxisCosTheta", thrustAxisCosTheta, "[Eventbased] Cosine of the polar angle component of the thrust axis.");

    REGISTER_VARIABLE("forwardEmisphereMass", forwardEmisphereMass,
                      "[Eventbased] Invariant mass of the particles flying in the same direction of the thrust axis.");
    REGISTER_VARIABLE("forwardEmisphereX", forwardEmisphereX,
                      "[Eventbased] X component of the total momentum of the particles flying in the same direction of the thrust axis");
    REGISTER_VARIABLE("forwardEmisphereY", forwardEmisphereY,
                      "[Eventbased] Y component of the total momentum of the particles flying in the same direction of the thrust axis");
    REGISTER_VARIABLE("forwardEmisphereZ", forwardEmisphereZ,
                      "[Eventbased] Z component of the total momentum of the particles flying in the same  direction of the thrust axis");
    REGISTER_VARIABLE("forwardEmisphereMomentum", forwardEmisphereMomentum,
                      "[Eventbased] Total momentum the particles flying in the same direction of the thrust axis.");

    REGISTER_VARIABLE("backwardEmisphereMass", backwardEmisphereMass,
                      "[Eventbased] Invariant mass of the particles flying in the direction opposite to the thrust axis.");
    REGISTER_VARIABLE("backwardEmisphereX", backwardEmisphereX,
                      "[Eventbased] X component of the total momentum of the particles flying in the direciton opposite to the thrust axis");
    REGISTER_VARIABLE("backwardEmisphereY", backwardEmisphereY,
                      "[Eventbased] Y component of the total momentum of the particles flying in the direction opposite to the thrust axis");
    REGISTER_VARIABLE("backwardEmisphereZ", backwardEmisphereZ,
                      "[Eventbased] Z component of the total momentum of the particles flying in the direction opposite to the thrust axis");
    REGISTER_VARIABLE("backwardEmisphereMomentum", forwardEmisphereMomentum,
                      "[Eventbased] Total momentum the particles flying in the  direction opposite to the thrust axis.");








  }
}
