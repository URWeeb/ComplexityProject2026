#pragma once

#include <functional>
#include <optional>
#include <vector>

#include "Types.hpp"

using VerificatorFunction =
    std::function<bool(const Formula&, const Assignment&)>;

class UniversalSearch {
 public:
  UniversalSearch(std::vector<Program> programs,
                  VerificatorFunction verificator_function,
                  SolverFunction solver_function, int max_rounds = 50000,
                  bool detailed_output = true);

  std::pair<std::optional<Assignment>, SearchStatistics> Solve(
      const Formula& formula) const;

  void SetMaxRounds(int rounds) { max_rounds_ = rounds; }

  void SetDetailedOutput(bool detailed_output) {
    detailed_output_ = detailed_output;
  }

  void AddProgram(Program program) { programs_.push_back(std::move(program)); }

  [[nodiscard]] int ProgramCount() const {
    return static_cast<int>(programs_.size());
  }

 private:
  std::vector<Program> programs_;
  VerificatorFunction verificator_;
  SolverFunction solver_;
  int max_rounds_;
  bool detailed_output_;
};