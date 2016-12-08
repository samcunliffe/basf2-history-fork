/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/utilities/NumberSequence.h>

#include <utility>
#include <vector>

namespace Belle2 {

  std::set<int64_t> generate_number_sequence(const std::string& str)
  {
    // Tokenize input
    std::vector<std::string> tokens;
    unsigned int lastdelimiter = 0;
    for (unsigned int i = 0; i < str.size(); ++i) {
      if (str[i] == ',') {
        tokens.push_back(str.substr(lastdelimiter, i - lastdelimiter));
        lastdelimiter = i + 1;
      }
    }
    std::string last = str.substr(lastdelimiter);
    if (last.size() != 0) {
      tokens.push_back(last);
    }

    // Convert tokens into intervals
    std::vector<std::pair<int64_t, int64_t>> intervals;
    intervals.reserve(tokens.size());
    for (const auto& token : tokens) {
      size_t interval_marker_position = token.find(':');
      // Single number
      if (interval_marker_position == std::string::npos) {
        int64_t number = std::stol(token);
        intervals.push_back(std::make_pair(number, number));
      } else {
        int64_t interval_begin = std::stol(token.substr(0, interval_marker_position));
        int64_t interval_end = std::stol(token.substr(interval_marker_position + 1));
        intervals.push_back(std::make_pair(interval_begin, interval_end));
      }
    }

    // Convert tokens into number sequence
    std::set<int64_t> sequence;
    for (const auto& interval : intervals) {
      for (int64_t number = interval.first; number <= interval.second; ++number)
        sequence.insert(number);
    }

    return sequence;
  }

}
