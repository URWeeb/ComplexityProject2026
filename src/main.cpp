#include <iostream>

#include "DPLL.hpp"
#include "Useful.hpp"

int main(const int argc, char* argv[]) {
  /* Здесь вообще расписаны эталонные
   * аргументы по умолчанию
   * (надо будет продублировать в README */
  int variable_count = 20;
  int clauses_count = 85;
  uint32_t seed = 42;
  int max_rounds = 50000;
  bool detailed_output = true;

  std::string aggreement = "n";
  std::cout << "Хотите проверить свою формулу? [y|N]: ";
  std::getline(std::cin, aggreement);

  Formula formula{};

  switch (tolower(aggreement[0])) {
    case 'y':
      size_t clause_count;
      std::cout << "Введите кол-во дизъюнктов: ";
      std::cin >> clause_count;
      formula.resize(clause_count);

      for (size_t i = 0; i < clause_count; ++i) {
        int input_literal;

        while (formula[i].size() < 3) {
          std::cin >> input_literal;
          formula[i].push_back(input_literal);
        }
      }

      break;

    default:
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

      formula = Random3CNF(variable_count, clauses_count, seed);
  }

  const UniversalSearch search = MakeSearch(max_rounds, detailed_output);
  auto [assignment, stats] = search.Solve(formula);

  PrintStatistics(stats);

  if (assignment.has_value()) {
    PrintAssignment(*assignment);

    if (DPLL::Verify(formula, *assignment)) {
      Logger("Статус верификации: SUCCESS");
      return 0;
    }

    Logger("Статус верификации: FAILED");
    return 1;
  }

  if (stats.status == Status::NOT_SAT) {
    Logger("Формула невыполнима");
    return 0;
  }

  Logger("Статус верификации: TIMEOUT");
  return 1;
}