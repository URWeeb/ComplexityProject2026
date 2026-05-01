#include <gtest/gtest.h>

#include "Types.hpp"

TEST(SolverResultTest, StatusSatStored) {
  SolverResult result{Status::SAT, {{1, true}, {2, false}}, 10};
  EXPECT_EQ(result.status, Status::SAT);
  EXPECT_EQ(result.steps_used, 10);
  EXPECT_EQ(result.assignment.at(1), true);
  EXPECT_EQ(result.assignment.at(2), false);
}

TEST(SolverResultTest, StatusNotSatHasEmptyAssignment) {
  SolverResult result{Status::NOT_SAT, {}, 42};
  EXPECT_EQ(result.status, Status::NOT_SAT);
  EXPECT_TRUE(result.assignment.empty());
}

TEST(SolverResultTest, StatusTimeoutHasStepsUsed) {
  SolverResult result{Status::TIMEOUT, {}, 99};
  EXPECT_EQ(result.status, Status::TIMEOUT);
  EXPECT_EQ(result.steps_used, 99);
}

TEST(SearchStatisticsTest, DefaultValues) {
  SearchStatistics stats;
  EXPECT_EQ(stats.rounds_completed, 0);
  EXPECT_EQ(stats.total_steps, 0LL);
  EXPECT_EQ(stats.winner_round, -1);
  EXPECT_DOUBLE_EQ(stats.elapsed_seconds, 0.0);
  EXPECT_EQ(stats.status, Status::TIMEOUT);
  EXPECT_TRUE(stats.winner_program.empty());
}

TEST(SearchStatisticsTest, FieldsAreAssignable) {
  SearchStatistics stats;
  stats.rounds_completed = 5;
  stats.total_steps = 1000;
  stats.winner_program = "dpll_min";
  stats.winner_round = 3;
  stats.elapsed_seconds = 0.42;
  stats.status = Status::SAT;

  EXPECT_EQ(stats.rounds_completed, 5);
  EXPECT_EQ(stats.total_steps, 1000LL);
  EXPECT_EQ(stats.winner_program, "dpll_min");
  EXPECT_EQ(stats.winner_round, 3);
  EXPECT_DOUBLE_EQ(stats.elapsed_seconds, 0.42);
  EXPECT_EQ(stats.status, Status::SAT);
}

TEST(ProgramTest, NameIsStored) {
  Program prog{"my_solver", [](const Formula&, long long) {
                 return SolverResult{Status::TIMEOUT, {}, 0};
               }};
  EXPECT_EQ(prog.name, "my_solver");
}

TEST(ProgramTest, SolverIsCallable) {
  int call_count = 0;
  Program prog{
      "counter",
      [&call_count](const Formula&, long long budget) {
        ++call_count;
        return SolverResult{Status::TIMEOUT, {}, budget};
      },
  };
  Formula phi = {{1, 2, 3}};
  auto result = prog.solver(phi, 7);
  EXPECT_EQ(call_count, 1);
  EXPECT_EQ(result.steps_used, 7);
}

TEST(ProgramTest, SolverReceivesCorrectFormula) {
  Formula captured;
  Program prog{
      "capture",
      [&captured](const Formula& f, long long) {
        captured = f;
        return SolverResult{Status::TIMEOUT, {}, 0};
      },
  };
  Formula phi = {{1, -2, 3}, {-1, 2}};
  prog.solver(phi, 10);
  EXPECT_EQ(captured, phi);
}

TEST(TypeAliasesTest, LiteralIsInt) {
  Literal lit = -3;
  EXPECT_EQ(std::abs(lit), 3);
  EXPECT_LT(lit, 0);
}

TEST(TypeAliasesTest, ClauseIsVectorOfLiterals) {
  Clause clause = {1, -2, 3};
  EXPECT_EQ(clause.size(), 3u);
  EXPECT_EQ(clause[0], 1);
  EXPECT_EQ(clause[1], -2);
}

TEST(TypeAliasesTest, FormulaIsVectorOfClauses) {
  Formula phi = {{1, 2}, {-1, 3}};
  EXPECT_EQ(phi.size(), 2u);
  EXPECT_EQ(phi[0], Clause({1, 2}));
}

TEST(TypeAliasesTest, AssignmentIsMap) {
  Assignment asgn = {{1, true}, {2, false}, {3, true}};
  EXPECT_TRUE(asgn.at(1));
  EXPECT_FALSE(asgn.at(2));
  EXPECT_TRUE(asgn.at(3));
}