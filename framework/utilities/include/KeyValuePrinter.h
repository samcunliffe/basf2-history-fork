/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace Belle2 {
  /** create human-readable or JSON output for key value pairs.
   *
   * Usage example:
      \code
      KeyValuePrinter pr(false); //JSON=false
      pr.put("a", 123);
      pr.put("bbbbb", "string");
      pr.put("ccccccc", std::vector<std::string>({"string", "abc"}));
      std::cout << pr.string();
      //which prints
      // a:          123
      // bbbbb:      string
      // ccccccc:    [string, abc]
      \endcode
   *
   * std::string, numeric types and std::vector<SUPPORTED TYPE>
   * are supported for values.
   * Compared to boost::property_tree::json_parser::write, KeyValuePrinter
   * handles non-string types correctly.
   */
  class KeyValuePrinter {
  public:
    /** Ctor.
     * @param use_json true for JSON output, false for human-readable output.
     * @param key_max_length for human-readable output, this should be equal to
     *                       strlen(longest_key) for aligning values in output.
     */
    explicit KeyValuePrinter(bool use_json, unsigned key_max_length = 10):
      m_json(use_json),
      m_maxpad(key_max_length + 2),
      m_delim("")
    { }
    ~KeyValuePrinter() { }

    /** Return completed string. */
    std::string string() const
    {
      if (m_json)
        return "{\n" + m_stream.str() + "\n}\n";
      else
        return m_stream.str();
    }
    /** Add one key-value pair. */
    template <class T> void put(const std::string& key, const T& value)
    {
      if (m_json)
        m_stream << m_delim << "\t" << escape(key) << ": " << escape(value) << "";
      else
        m_stream << std::left << std::setw(m_maxpad) << key + ": " << escape(value) << "\n";
      m_delim = ",\n";
    }
  private:
    bool m_json; /**< create JSON output? */
    unsigned m_maxpad; /**< for human-readable output: how much padding after key? */
    std::string m_delim; /**< for JSON: comma to print after prev entry. */
    std::stringstream m_stream; /**< output being built. */

    /** escape string. */
    std::string escape(const std::string& value) const;
    /** escape string literals. */
    std::string escape(const char* value) const { return escape(std::string(value)); }
    /** escape numeric value. */
    template <class T> std::string escape(const T& value) const { return std::to_string(value); }
    /** escape vector<T>. */
    template <class T> std::string escape(const std::vector<T>& value) const
    {
      std::string delim = "";
      std::string s = "[";
      for (auto el : value) {
        s += delim + escape(el);
        delim = ", ";
      }
      s += "]";
      return s;
    }
  };
}
