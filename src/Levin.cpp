#include "../include/Levin.hpp"

#include <algorithm>
#include <chrono>
#include <optional>

#include "../include/Useful.hpp"

UniversalSearch::UniversalSearch(std::vector<Program> programs,
                                 VerificatorFunction verificator_function,
                                 SolverFunction solver_function, int max_rounds,
                                 const bool detailed_output)
    : programs_(std::move(programs)),
      verificator_(std::move(verificator_function)),
      solver_(std::move(solver_function)),
      max_rounds_(max_rounds),
      detailed_output_(detailed_output) {}

std::pair<std::optional<Assignment>, SearchStatistics> UniversalSearch::Solve(
    const Formula& formula) const {
  SearchStatistics stats;

  int variable_counter = GetVariableCount(formula);

  int program_counter = static_cast<int>(programs_.size());
  auto start_time = Clock::now();

  for (int round = 1; round <= max_rounds_; ++round) {
    stats.rounds_completed = round;

    for (int i = 0; i < std::min(round, program_counter); ++i) {
      auto [status, assignment, steps_used] =
          programs_[i].solver(formula, round);
      stats.total_steps += steps_used;

      if (status != Status::SAT) {
        continue;
      }

      if (verificator_(formula, assignment)) {
        stats.elapsed_seconds = ElapsedTime(start_time);
        stats.winner_program = programs_[i].name;
        stats.winner_round = round;
        stats.status = Status::SAT;

        if (detailed_output_) {
          Logger("SAT | программа=" + programs_[i].name +
                 " | раунд=" + std::to_string(round) +
                 " | шагов=" + std::to_string(stats.total_steps) +
                 " | время=" + std::to_string(stats.elapsed_seconds) + "s");
        }

        return {assignment, stats};
      }
    }

    long long budget_fallback = round * round * variable_counter;
    auto [status, assignment, steps_used] = solver_(formula, budget_fallback);
    stats.total_steps += steps_used;

    if (status == Status::NOT_SAT) {
      stats.elapsed_seconds = ElapsedTime(start_time);
      stats.status = Status::NOT_SAT;

      if (detailed_output_) {
        Logger("UNSAT доказан fallback" + std::string(" | раунд=") +
               std::to_string(round) +
               " | шагов=" + std::to_string(stats.total_steps) +
               " | время=" + std::to_string(stats.elapsed_seconds) + "s");
      }

      return {std::nullopt, stats};
    }

    if (status == Status::SAT && verificator_(formula, assignment)) {
      stats.elapsed_seconds = ElapsedTime(start_time);
      stats.winner_program = "fallback";
      stats.winner_round = round;
      stats.status = Status::SAT;

      Logger("SAT | программа=fallback" + std::string(" | раунд=") +
             std::to_string(round) +
             " | шагов=" + std::to_string(stats.total_steps) +
             " | время=" + std::to_string(stats.elapsed_seconds) + "s");

      return {assignment, stats};
    }
  }

  stats.elapsed_seconds = ElapsedTime(start_time);
  stats.status = Status::TIMEOUT;

  Logger("TIMEOUT: исчерпано " + std::to_string(max_rounds_) + " раундов" +
         " | шагов=" + std::to_string(stats.total_steps));

  return {std::nullopt, stats};
}