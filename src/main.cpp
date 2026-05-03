#include <iostream>

#include "DPLL.hpp"
#include "Useful.hpp"

int main(const int argc, char* argv[]) {
  int variable_count = 20;
  int clauses_count = 85;
  uint32_t seed = 42;
  int max_rounds = 50000;
  bool detailed_output = true;

  try {
    if (argc >= 2) {
      variable_count = std::stoi(argv[1]);
    }

    if (argc >= 3) {
      clauses_count = std::stoi(argv[2]);
    }

    if (argc >= 4) {
      seed = static_cast<uint32_t>(std::stoul(argv[3]));
    }

    if (argc >= 5) {
      max_rounds = std::stoi(argv[4]);
    }

    if (argc >= 6) {
      detailed_output = static_cast<bool>(std::stoi(argv[5]));
    }
  } catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
    return 1;
  }

  if (variable_count < 3) {
    std::cerr << "Ошибка: число переменных должно быть равно хотя бы 3\n";
    return 1;
  }

  if (clauses_count < 1) {
    std::cerr << "Ошибка: число дизъюнкций должно быть равно хотя бы 1\n";
  }

  Formula formula = Random3CNF(variable_count, clauses_count, seed);
  UniversalSearch search = MakeSearch(max_rounds, detailed_output);
  auto [assignment, stats] = search.Solve(formula);

  PrintStatistics(stats);

  if (assignment.has_value()) {
    PrintAssignment(*assignment);

    if (DPLL::Verify(formula, *assignment)) {
      Logger("Верификация: SUCCESS");
      return 0;
    }

    Logger("Верификация: FAILED");
    return 1;
  }

  if (stats.status == Status::NOT_SAT) {
    Logger("Формула невыполнима");
    return 0;
  }

  Logger("Timeout: ответ не найден");
  return 1;
}