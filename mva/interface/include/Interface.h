/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#ifndef INCLUDE_GUARD_BELLE2_MVA_INTERFACE_HEADER
#define INCLUDE_GUARD_BELLE2_MVA_INTERFACE_HEADER

#include <mva/interface/Teacher.h>
#include <mva/interface/Expert.h>

#include <map>
#include <vector>
#include <string>
#include <memory>

namespace Belle2 {
  namespace MVA {

    /**
     * Abstract Interface to third-party MVA libraries
     */
    class AbstractInterface {
    public:
      /**
       * Creates a new Interface to a third-party library
       * @param name of the third-party MVA library
       */
      AbstractInterface(std::string name);

      /**
       * Virtual destructor
       */
      virtual ~AbstractInterface();

      /**
       * Returns the name of the third-party MVA library
       * @return name of the third-party MVA library
       */
      std::string getName() const;

      /**
       * Returns interfaces supported by the MVA Interface
       * @return supported interfaces
       */
      static std::map<std::string, AbstractInterface*> getSupportedInterfaces() { return s_supported_interfaces; }

      /**
       * Static function which initliazes all supported interfaces,
       * has to be called once before getSupportedInterfaces can be used.
       * It is save to call it multiple times
       */
      static void initSupportedInterfaces();

      /**
       * Get Options of this MVA library
       */
      virtual std::unique_ptr<Options> getOptions() const = 0;

      /**
       * Get Teacher of this MVA library
       * @param general_options shared options of all libraries
       * @param specific_options specific options for this library
       */
      virtual std::unique_ptr<Teacher> getTeacher(const GeneralOptions& general_options, const Options& specific_options) const = 0;

      /**
       * Get Exoert of this MVA library
       */
      virtual std::unique_ptr<MVA::Expert> getExpert() const = 0;

    private:
      /**
       * Name of the third-party library
       */
      std::string m_name;

      /**
       * Map of supported interfaces
       */
      static std::map<std::string, AbstractInterface*> s_supported_interfaces;

    };


    /**
     * Template class to easily construct a interface for an MVA library using
     * a library-specific Options, Teacher and Expert class
     */
    template<class TemplateOptions, class TemplateTeacher, class TemplateExpert>
    class Interface : public AbstractInterface {
    public:
      /**
       * Constructs a new interface with the given name
       * @param name of the interface
       */
      Interface(std::string name) : AbstractInterface(name) { }

      /**
       * Get Options of this MVA library
       */
      virtual std::unique_ptr<Options> getOptions() const override { return std::unique_ptr<Options>(new TemplateOptions); }

      /**
       * Get Teacher of this MVA library
       * @param general_options shared options of all libraries
       * @param specific_options specific options for this library
       */
      virtual std::unique_ptr<Teacher> getTeacher(const GeneralOptions& general_options, const Options& specific_options) const override
      {
        return std::unique_ptr<Teacher>(new TemplateTeacher(general_options, dynamic_cast<const TemplateOptions&>(specific_options)));
      }

      /**
       * Get Exoert of this MVA library
       */
      virtual std::unique_ptr<MVA::Expert> getExpert() const override { return std::unique_ptr<MVA::Expert>(new TemplateExpert); }

    };

  }
}
#endif
