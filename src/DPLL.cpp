#include "DPLL.hpp"

#include <unordered_set>

#include "Useful.hpp"

DPLL::DPLL(Formula formula, const BranchStrategy strategy,
           const std::mt19937& rng)
    : formula_(std::move(formula)), strategy_(strategy), rng_(rng) {}

std::optional<Formula> DPLL::Simplify(const Formula& formula,
                                      const Literal variable,
                                      const bool value) {
  Formula simplified;
  simplified.reserve(formula.size());

  for (const Clause& clause : formula) {
    Clause new_clause;
    new_clause.reserve(clause.size());
    bool satisfied = false;

    for (Literal literal : clause) {
      if (std::abs(literal) == variable) {
        if (literal > 0 ? value : !value) {
          satisfied = true;
          break;
        }
      } else {
        new_clause.push_back(literal);
      }
    }

    if (satisfied) {
      continue;
    }

    if (new_clause.empty()) {
      return std::nullopt;
    }

    simplified.push_back(std::move(new_clause));
  }

  return simplified;
}

bool DPLL::UnitPropagate(Formula& formula, Assignment& assignment) {
  bool changed = true;

  while (changed) {
    changed = false;

    for (const Clause& clause : formula) {
      if (clause.empty()) {
        return false;
      }

      if (clause.size() != 1) {
        continue;
      }

      Literal literal = clause[0];
      bool value = (literal > 0);

      auto iter = assignment.find(std::abs(literal));
      if (iter != assignment.end()) {
        if (iter->second != value) {
          return false;
        }
      } else {
        assignment[std::abs(literal)] = value;
        auto optimized = Simplify(formula, std::abs(literal), value);

        if (!optimized) {
          return false;
        }

        formula = std::move(*optimized);
        changed = true;
        break;
      }
    }
  }

  return true;
}

void DPLL::PureLiteralAssign(Formula& formula, Assignment& assignment) {
  bool changed = true;

  while (changed) {
    changed = false;
    std::unordered_set<Literal> positive_variables;
    std::unordered_set<Literal> negative_variables;

    for (const Clause& clause : formula) {
      for (Literal literal : clause) {
        (literal > 0 ? positive_variables : negative_variables)
            .insert(std::abs(literal));
      }
    }

    auto optimizer_func = [&](const std::unordered_set<Literal>& goal_vars,
                              const std::unordered_set<Literal>& ungoal_vars,
                              bool use_value) {
      for (Literal variable : goal_vars) {
        if (!ungoal_vars.contains(variable) && !assignment.contains(variable)) {
          assignment[variable] = use_value;
          auto optimized = Simplify(formula, variable, use_value);

          if (!optimized) {
            return false;
          }

          formula = std::move(*optimized);
          changed = true;
        }
      }

      return true;
    };

    if (!optimizer_func(positive_variables, negative_variables, true)) {
      return;
    }

    if (!optimizer_func(negative_variables, positive_variables, false)) {
      return;
    }
  }
}

Literal DPLL::ChooseLiteral(const Formula& formula,
                            const Assignment& assignment) {
  std::vector<Literal> candidates;

  for (const Clause& clause : formula) {
    for (Literal literal : clause) {
      if (!assignment.contains(std::abs(literal))) {
        candidates.push_back(std::abs(literal));
      }
    }
  }

  if (candidates.empty()) {
    return 0;
  }

  auto update_frequencies = [&](std::unordered_map<Literal, int>& frequencies) {
    for (Literal variable : candidates) {
      frequencies[variable]++;
    }
  };

  switch (strategy_) {
    case BranchStrategy::MinIndex: {
      return *std::ranges::min_element(candidates);
    }

    case BranchStrategy::MaxIndex: {
      return *std::ranges::max_element(candidates);
    }

    case BranchStrategy::MostFrequent: {
      std::unordered_map<Literal, int> frequencies;
      update_frequencies(frequencies);

      return std::ranges::max_element(frequencies,
                                      [](auto& first, auto& second) {
                                        return first.second < second.second;
                                      })
          ->first;
    }

    case BranchStrategy::LeastFrequent: {
      std::unordered_map<Literal, int> frequencies;
      update_frequencies(frequencies);

      return std::ranges::min_element(frequencies,
                                      [](auto& first, auto& second) {
                                        return first.second < second.second;
                                      })
          ->first;
    }

    case BranchStrategy::Random: {
      std::uniform_int_distribution<int> dist(
          0, static_cast<int>(candidates.size() - 1));
      return candidates[dist(rng_)];
    }
  }

  return candidates[0];
}

SolverResult DPLL::ProcessAlgorithm(long long max_steps) {
  struct AlgoFrame {
    Formula formula;
    Assignment assignment;
  };

  std::stack<AlgoFrame> algo_stack;
  algo_stack.push({formula_, {}});

  long long steps = 0;

  while (!algo_stack.empty()) {
    if (steps >= max_steps) {
      return {Status::TIMEOUT, {}, steps};
    }

    ++steps;

    auto [form, assign] = std::move(algo_stack.top());
    algo_stack.pop();

    if (!UnitPropagate(form, assign)) {
      continue;
    }

    PureLiteralAssign(form, assign);

    if (form.empty()) {
      return {Status::SAT, std::move(assign), steps};
    }

    Literal variable = ChooseLiteral(form, assign);

    if (variable == 0) {
      continue;
    }

    for (bool value : {false, true}) {
      if (auto optimized = Simplify(form, variable, value)) {
        Assignment new_assignment = assign;
        new_assignment[variable] = value;
        algo_stack.push({std::move(*optimized), std::move(new_assignment)});
      }
    }
  }

  return {Status::NOT_SAT, {}, steps};
}

bool DPLL::Verify(const Formula& formula, const Assignment& assignment) {
  for (const Clause& clause : formula) {
    bool clause_satisfiable = false;

    for (Literal literal : clause) {
      auto iter = assignment.find(std::abs(literal));
      if (iter == assignment.end()) {
        continue;
      }

      if (literal > 0 ? iter->second : !iter->second) {
        clause_satisfiable = true;
        break;
      }
    }

    if (!clause_satisfiable) {
      return false;
    }
  }

  return true;
}

std::vector<Program> DPLL::BuildPrograms() {
  std::vector<Program> programs;

  struct Entry {
    std::string name;
    BranchStrategy strategy;
  };

  uint32_t seed = 0;

  for (const auto& [name, strategy] : std::initializer_list<Entry>{
           {"dpll_min", BranchStrategy::MinIndex},
           {"dpll_max", BranchStrategy::MaxIndex},
           {"dpll_most_frequent", BranchStrategy::MostFrequent},
           {"dpll_least_frequent", BranchStrategy::LeastFrequent},
       }) {
    programs.push_back(
        {name, [strategy, seed](const Formula& formula, const long long limit) {
           DPLL dpll(formula, strategy, std::mt19937{seed});
           return dpll.ProcessAlgorithm(limit);
         }});
  }

  for (; seed < 20; ++seed) {
    programs.push_back({"dpll_random_seed" + std::to_string(seed),
                        [seed](const Formula& formula, const long long limit) {
                          DPLL dpll(formula, BranchStrategy::Random,
                                    std::mt19937{seed});
                          return dpll.ProcessAlgorithm(limit);
                        }});
  }

  return programs;
}