#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "Types.hpp"

inline void Logger(const std::string& message, const bool new_line = true) {
  std::cout << message + (new_line ? "\n" : "");
}

inline double ElapsedTime(const TimePoint& time_point) {
  return TimeDuration(Clock::now() - time_point).count();
}

inline int GetVariableCount(const Formula& formula) {
  int result = 1;

  for (const Clause& clause : formula) {
    for (const Literal literal : clause) {
      result = std::max(result, std::abs(literal));
    }
  }

  return result;
}

// Надо будет ещё эти функции дописать, но в 3 часа ночи - не хочу
//
// static void PrintStatistics(const SearchStatistics& stats);
//
// static void PrintAssignment(const Assignment& assignment);
//
// static Formula Random3CNF(int clauses_count, uint32_t seed = 42);
//
// static Formula ParseDimacs(const std::string& text);
//
// static UniversalSearch MakeSearch(int max_rounds = 50000, bool
// detailed_output = true);
