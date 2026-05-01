#pragma once

#include <algorithm>
#include <optional>
#include <random>
#include <stack>

#include "Types.hpp"

enum class BranchStrategy {
  MinIndex,
  MaxIndex,
  MostFrequent,
  LeastFrequent,
  Random,
};

class DPLL {
 public:
  explicit DPLL(Formula formula, BranchStrategy strategy,
                const std::mt19937& rng);

  SolverResult ProcessAlgorithm(long long max_steps);

  static std::optional<Formula> Simplify(const Formula& formula,
                                         Literal variable, bool value);

  static bool UnitPropagate(Formula& formula, Assignment& assignment);

  static void PureLiteralAssign(Formula& formula, Assignment& assignment);

  Literal ChooseLiteral(const Formula& formula, const Assignment& assignment);

  static bool Verify(const Formula& formula, const Assignment& assignment);

  static std::vector<Program> BuildPrograms();

 private:
  Formula formula_{};
  BranchStrategy strategy_{};
  std::mt19937 rng_;
};