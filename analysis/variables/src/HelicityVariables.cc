/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Phillip Urquijo, Fernando Abudinen                       *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#include <analysis/variables/HelicityVariables.h>
#include <analysis/variables/EventVariables.h>

#include <analysis/dataobjects/Particle.h>

#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/ReferenceFrame.h>

#include <framework/utilities/Conversion.h>

#include <TLorentzVector.h>
#include <TVector3.h>
#include <cmath>

namespace Belle2 {
  namespace Variable {

    double cosHelicityAngle(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;

      if (daughters.size() == 2) {

        bool isOneConversion = false;

        for (auto& idaughter : daughters) {
          if (idaughter -> getNDaughters() == 2) {
            if (std::abs(idaughter -> getDaughters()[0]-> getPDGCode()) == 11) isOneConversion = true;
          }
        }

        if (isOneConversion) {
          //only for pi0 decay where one gamma converts

          TLorentzVector pGamma;

          for (auto& idaughter : daughters) {
            if (idaughter -> getNDaughters() == 2) continue;
            else pGamma = frame.getMomentum(idaughter);
          }

          pGamma.Boost(motherBoost);

          return std::cos(motherMomentum.Angle(pGamma.Vect()));

        } else {
          TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
          TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);

          pDaughter1.Boost(motherBoost);
          pDaughter2.Boost(motherBoost);

          TVector3 p12 = (pDaughter2 - pDaughter1).Vect();

          return std::cos(motherMomentum.Angle(p12));
        }

      } else if (daughters.size() == 3) {

        TLorentzVector pDaughter1 = frame.getMomentum(daughters[0]);
        TLorentzVector pDaughter2 = frame.getMomentum(daughters[1]);
        TLorentzVector pDaughter3 = frame.getMomentum(daughters[2]);

        pDaughter1.Boost(motherBoost);
        pDaughter2.Boost(motherBoost);
        pDaughter3.Boost(motherBoost);

        TVector3 p12 = (pDaughter2 - pDaughter1).Vect();
        TVector3 p13 = (pDaughter3 - pDaughter1).Vect();

        TVector3 n = p12.Cross(p13);

        return std::cos(motherMomentum.Angle(n));

      }  else return 0;

    }

    double cosHelicityAnglePi0Dalitz(const Particle* part)
    {

      const auto& frame = ReferenceFrame::GetCurrent();
      TVector3 motherBoost = - frame.getMomentum(part).BoostVector();
      TVector3 motherMomentum = frame.getMomentum(part).Vect();
      const auto& daughters = part -> getDaughters() ;


      if (daughters.size() == 3) {

        TLorentzVector pGamma;

        for (auto& idaughter : daughters) {
          if (std::abs(idaughter -> getPDGCode()) == 22) pGamma = frame.getMomentum(idaughter);
        }

        pGamma.Boost(motherBoost);

        return std::cos(motherMomentum.Angle(pGamma.Vect()));

      }  else return 0;

    }


    Manager::FunctionPtr cosHelicityAngleIfCMSIsTheMother(const std::vector<std::string>& arguments)
    {
      int idau = 0;
      if (arguments.size() == 1) {
        try {
          idau = Belle2::convertString<int>(arguments[0]);
        } catch (boost::bad_lexical_cast&) {
          B2FATAL("The argument of cosHelicityAngleWrtCMSFrame must be an integer!");
          return nullptr;
        }
      } else {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfCMSIsTheMother");
      }
      auto func = [idau](const Particle * mother) -> double {
        const Particle* part = mother->getDaughter(idau);
        if (!part)
        {
          B2FATAL("Couldn't find the " << idau << "th daughter");
          return -999.0;
        }

        TLorentzVector beam4Vector(getBeamPx(NULL), getBeamPy(NULL), getBeamPz(NULL), getBeamE(NULL));
        TLorentzVector part4Vector = part->get4Vector();
        TLorentzVector mother4Vector = mother->get4Vector();

        TVector3 motherBoost = -(mother4Vector.BoostVector());

        TLorentzVector beam4Vector_motherFrame, part4Vector_motherFrame;
        beam4Vector_motherFrame = beam4Vector;
        part4Vector_motherFrame = part4Vector;

        beam4Vector_motherFrame.Boost(motherBoost);
        part4Vector_motherFrame.Boost(motherBoost);

        return std::cos(beam4Vector_motherFrame.Angle(part4Vector_motherFrame.Vect()));
      };
      return func;
    }


    Manager::FunctionPtr cosHelicityAngleIfCMSIsTheDaughter(const std::vector<std::string>& arguments)
    {
      // have to tell cppcheck that these lines are fine, because it doesn't
      // support the lambda function syntax and throws a (wrong) variableScope

      // cppcheck-suppress variableScope
      int iDau = 0;
      int iGrandDau = 0;
      if (arguments.size() == 2) {
        try {
          // cppcheck-suppress unreadVariable
          iDau = Belle2::convertString<int>(arguments[0]);
          iGrandDau = Belle2::convertString<int>(arguments[1]);
        } catch (boost::bad_lexical_cast&) {
          B2FATAL("The two arguments of cosHelicityAngleIfCMSIsTheDaughter must be integers!");
          return nullptr;
        }
      } else {
        B2FATAL("Wrong number of arguments for cosHelicityAngleIfCMSIsTheDaughter: two are needed.");
      }
      auto func = [iDau, iGrandDau](const Particle * mother) -> double {

        const Particle* daughter = mother->getDaughter(iDau);
        if (!daughter)
          B2FATAL("Couldn't find the " << iDau << "th daughter.");

        const Particle* grandDaughter = daughter->getDaughter(iGrandDau);
        if (!grandDaughter)
          B2FATAL("Couldn't find the " << iGrandDau << "th daughter of the " << iDau << "th daughter.");


        TLorentzVector mother4Vector = mother->get4Vector();
        TLorentzVector daughter4Vector = daughter->get4Vector();
        TLorentzVector grandDaughter4Vector = grandDaughter->get4Vector();

        TVector3 daughterBoost = -(daughter4Vector.BoostVector());

        // We boost the momentum of the mother and of the granddaughter to the reference frame of the daughter.
        grandDaughter4Vector.Boost(daughterBoost);
        mother4Vector.Boost(daughterBoost);

        return - grandDaughter4Vector.Vect().Dot(mother4Vector.Vect()) / grandDaughter4Vector.Vect().Mag() / mother4Vector.Vect().Mag();

      };
      return func;
    }

    Manager::FunctionPtr cosAcoplanarityAngleIfCMSIsTheMother(const std::vector<std::string>& arguments)
    {
      // have to tell cppcheck that these lines are fine, because it doesn't
      // support the lambda function syntax and throws a (wrong) variableScope

      // cppcheck-suppress variableScope
      int iGrandDau1 = 0;
      int iGrandDau2 = 0;
      if (arguments.size() == 2) {
        try {
          // cppcheck-suppress unreadVariable
          iGrandDau1 = Belle2::convertString<int>(arguments[0]);
          iGrandDau2 = Belle2::convertString<int>(arguments[1]);

        } catch (boost::bad_lexical_cast&) {
          B2FATAL("The two arguments of cosAcoplanarityAngleIfCMSIsTheMother must be integers!");
          return nullptr;
        }
      } else {
        B2FATAL("Wrong number of arguments for cosAcoplanarityAngleIfCMSIsTheMother: two are needed.");
      }
      auto func = [iGrandDau1, iGrandDau2](const Particle * mother) -> double {

        if (mother->getNDaughters() != 2)
          B2FATAL("cosAcoplanarityAngleIfCMSIsTheMother: this variable works only for two-body decays.");

        const Particle* daughter1 = mother-> getDaughter(0);
        const Particle* daughter2 = mother-> getDaughter(1);

        const Particle* grandDaughter1 = daughter1 -> getDaughter(iGrandDau1);
        if (!grandDaughter1)
          B2FATAL("Couldn't find the " << iGrandDau1 << "th daughter of the first daughter.");

        const Particle* grandDaughter2 = daughter2 -> getDaughter(iGrandDau2);
        if (!grandDaughter2)
          B2FATAL("Couldn't find the " << iGrandDau2 << "th daughter of the second daughter.");

        TLorentzVector mother4Vector = mother->get4Vector();
        TLorentzVector daughter4Vector1 = daughter1->get4Vector();
        TLorentzVector daughter4Vector2 = daughter2->get4Vector();
        TLorentzVector grandDaughter4Vector1 = grandDaughter1->get4Vector();
        TLorentzVector grandDaughter4Vector2 = grandDaughter2->get4Vector();

        TVector3 motherBoost = -(mother4Vector.BoostVector());
        TVector3 daughter1Boost = -(daughter4Vector1.BoostVector());
        TVector3 daughter2Boost = -(daughter4Vector2.BoostVector());

        // Boosting daughters to reference frame of the mother
        daughter4Vector1.Boost(motherBoost);
        daughter4Vector2.Boost(motherBoost);

        // Boosting each grand daughter to reference frame of its mother
        grandDaughter4Vector1.Boost(daughter1Boost);
        grandDaughter4Vector2.Boost(daughter2Boost);

        // We calculate the normal vectors of the decay two planes
        TVector3 normalVector1 = daughter4Vector1.Vect().Cross(grandDaughter4Vector1.Vect());
        TVector3 normalVector2 = daughter4Vector2.Vect().Cross(grandDaughter4Vector2.Vect());

        return std::cos(normalVector1.Angle(normalVector2));

      };
      return func;
    }


    VARIABLE_GROUP("Helicity variables");

    REGISTER_VARIABLE("cosHelicityAngle",
                      cosHelicityAngle,
                      "If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference of the two daughters in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame\n"
                      "If the given particle has three daughters: cosine of the angle between the normal vector of the plane defined by the momenta of the three daughters in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame.\n"
                      "Else: 0.");

    REGISTER_VARIABLE("cosHelicityAnglePi0Dalitz",
                      cosHelicityAnglePi0Dalitz,
                      "To be used for the decay pi0 -> e+ e- gamma: cosine of the angle between the momentum of the gamma in the frame of the given particle (mother)"
                      "and the momentum of the given particle in the lab frame.\n"
                      "Else: 0.");

    REGISTER_VARIABLE("cosHelicityAngleIfCMSIsTheMother", cosHelicityAngleIfCMSIsTheMother,
                      "Cosine of the helicity angle of the i-th (where 'i' is the parameter passed to the function) daughter of the particle provided,\n"
                      "assuming that the mother of the provided particle correspond to the Centre of Mass System, whose parameters are\n"
                      "automatically loaded by the function, given the accelerators conditions.");

    REGISTER_VARIABLE("cosHelicityAngleIfCMSIsTheDaughter", cosHelicityAngleIfCMSIsTheDaughter,
                      "Cosine of the helicity angle between the momentum of the provided particle and the momentum of the selected granddaughter\n"
                      "in the reference frame of the selected daughter (theta_1 and theta_2 in the PDG 2018, p. 722).\n"
                      "This variable needs two integer arguments: the first one is the index of the daughter and the second one is the index of the granddaughter.\n"
                      "For example, in the Decay B0 -> [J/psi -> mu+ mu-] [K*0 -> K+ pi-], if the provided particle is B0 and the selected indices are (0, 0),\n"
                      "the variable will return the angle between the momentum of the B0 and the momentum of the mu+,\n"
                      "both momenta in the rest frame of the J/psi.\n"
                      "This variable is needed for angular analyses of B meson decays into two vector particles.");

    REGISTER_VARIABLE("cosAcoplanarityAngleIfCMSIsTheMother", cosAcoplanarityAngleIfCMSIsTheMother,
                      "Cosine of the acoplanarity angle (Phi in the PDG 2018, p. 722). Given a two-body decay, the acoplanarity angle is defined as\n"
                      "the angle between the two decay planes in the reference frame of the mother. We calculate the acoplanarity angle as the angle between the two\n"
                      "normal vectors of the decay planes. Each normal vector is the cross product of the momentum of one daughter (in the frame of the mother) and the\n"
                      "momentum of one of the granddaughters (in the reference frame of the daughter).\n"
                      "This variable needs two integer arguments: the first one is the index of the first grand daughter, and the second one the index of the\n"
                      "second granddaughter. \n"
                      "For example, in the Decay B0 -> [J/psi -> mu+ mu-] [K*0 -> K+ pi-], if the provided particle is B0 and the selected indices are (0, 0),\n"
                      "the variable will return the acoplanarity using the mu+ and the K+ granddaughters.");

  }
}
