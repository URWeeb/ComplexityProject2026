#include "Useful.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <unordered_set>

#include "DPLL.hpp"
#include "Levin.hpp"
#include "Types.hpp"

void Logger(const std::string& message, const bool new_line) {
  std::cout << message + (new_line ? "\n" : "");
}

double ElapsedTime(const TimePoint& time_point) {
  return TimeDuration(Clock::now() - time_point).count();
}

int GetVariableCount(const Formula& formula) {
  int result = 1;

  for (const Clause& clause : formula) {
    for (const Literal literal : clause) {
      result = std::max(result, std::abs(literal));
    }
  }

  return result;
}

void PrintStatistics(const SearchStatistics& stats) {
  std::string stringed_status;

  switch (stats.status) {
    case Status::SAT:
      stringed_status = "SAT";
      break;
    case Status::NOT_SAT:
      stringed_status = "NOT_SAT";
      break;
    case Status::TIMEOUT:
      stringed_status = "TIMEOUT";
  }

  std::cout << "  Результат      : " << stringed_status << "\n"
            << "  Раундов        : " << stats.rounds_completed << "\n"
            << "  Шагов суммарно : " << stats.total_steps << "\n"
            << "  Время          : " << stats.elapsed_seconds << "s\n";

  if (!stats.winner_program.empty()) {
    std::cout << " Победившая M_i : " << stats.winner_program << " (раунд "
              << stats.winner_round << ")\n";
  }
}

void PrintAssignment(const Assignment& assignment) {
  std::map<Literal, bool> sorted_assignment(assignment.begin(),
                                            assignment.end());

  for (auto& [variable, value] : sorted_assignment) {
    std::cout << "  x" << variable << " = " << (value ? "1\n" : "0\n");
  }
}

Formula Random3CNF(int variable_count, int clauses_count, uint32_t seed) {
  std::mt19937 generator(seed);
  std::uniform_int_distribution<int> variable_distribution(1, variable_count);
  std::bernoulli_distribution sign_distribution(0.5);

  Formula formula;
  formula.reserve(clauses_count);

  for (int i = 0; i < clauses_count; ++i) {
    std::unordered_set<int> used;
    Clause clause;

    while (static_cast<int>(clause.size()) < 3) {
      int variable = variable_distribution(generator);

      if (used.insert(variable).second) {
        clause.push_back(sign_distribution(generator) ? variable : -variable);
      }
    }

    formula.push_back(std::move(clause));
  }

  return formula;
}

Formula ParseDimacs(const std::string& text) {
  Formula result;
  std::istringstream iss(text);
  std::string line;

  while (std::getline(iss, line)) {
    if (line.empty() || line[0] == 'c' || line[0] == 'p' || line[0] == '%') {
      continue;
    }

    std::istringstream linestream(line);
    Clause clause;
    Literal literal;

    while (linestream >> literal && literal != 0) {
      clause.push_back(literal);
    }

    if (!clause.empty()) {
      result.push_back(std::move(clause));
    }
  }

  return result;
}

UniversalSearch MakeSearch(int max_rounds, bool detailed_output) {
  SolverFunction fallback = [](const Formula& formula, const long long limit) {
    DPLL dpll(formula, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(limit);
  };

  return {DPLL::BuildPrograms(), DPLL::Verify, fallback, max_rounds,
          detailed_output};
}