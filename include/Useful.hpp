#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include "Levin.hpp"
#include "Types.hpp"

void Logger(const std::string& message, const bool new_line = true);

double ElapsedTime(const TimePoint& time_point);

int GetVariableCount(const Formula& formula);

void PrintStatistics(const SearchStatistics& stats);

void PrintAssignment(const Assignment& assignment);

Formula Random3CNF(int variable_count, int clauses_count, uint32_t seed = 42);

Formula ParseDimacs(const std::string& text);

UniversalSearch MakeSearch(int max_rounds = 50000, bool detailed_output = true);
