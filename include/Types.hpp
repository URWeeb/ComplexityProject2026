#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Поскольку используем std::chrono, лучше добавить алиасы для читаемости
using Clock = std::chrono::steady_clock;
using TimePoint = std::chrono::time_point<Clock>;
using TimeDuration = std::chrono::duration<double>;

// Объявим алиасы для далее используемых классов/типов
using Literal = int;
using Clause = std::vector<Literal>;
using Formula = std::vector<Clause>;
using Assignment = std::unordered_map<int, bool>;

enum class Status { SAT, NOT_SAT, TIMEOUT };

struct SolverResult {
  Status status;
  Assignment assignment;
  long long steps_used;
};

using SolverFunction = std::function<SolverResult(const Formula&, long long)>;

struct Program {
  std::string name;
  SolverFunction solver;
};

struct SearchStatistics {
  int rounds_completed = 0;
  long long total_steps = 0;
  std::string winner_program;
  int winner_round = -1;
  double elapsed_seconds = 0.0;
  Status status = Status::TIMEOUT;
};
