#include <gtest/gtest.h>

#include "DPLL.hpp"
#include "Levin.hpp"

static UniversalSearch MakeSearcher(int max_rounds = 10000,
                                    bool detailed_output = false) {
  SolverFunction fallback = [](const Formula& f, long long budget) {
    DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(budget);
  };
  return UniversalSearch(DPLL::BuildPrograms(), DPLL::Verify, fallback,
                         max_rounds, detailed_output);
}

TEST(LevinSatTest, TrivialOneClause) {
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_TRUE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::SAT);
  EXPECT_TRUE(DPLL::Verify(phi, *asgn));
}

TEST(LevinSatTest, TwoClausesSat) {
  Formula phi = {{1, 2, 3}, {-1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_TRUE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::SAT);
  EXPECT_TRUE(DPLL::Verify(phi, *asgn));
}

TEST(LevinSatTest, FiveVariablesSat) {
  Formula phi = {
      {1, 2, 3}, {-1, 4, 5}, {2, -3, 4}, {-2, 3, 5}, {1, -4, -5},
  };
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_TRUE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::SAT);
  EXPECT_TRUE(DPLL::Verify(phi, *asgn));
}

TEST(LevinSatTest, EmptyFormulaIsSat) {
  Formula phi = {};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_EQ(stats.status, Status::SAT);
}

TEST(LevinSatTest, UnitClauseForcesAssignment) {
  Formula phi = {{1}, {1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_TRUE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::SAT);
  EXPECT_EQ(asgn->at(1), true);
}

TEST(LevinUnsatTest, AllCombinationsThreeVarsUnsat) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_FALSE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::NOT_SAT);
}

TEST(LevinUnsatTest, DirectContradiction) {
  Formula phi = {{1}, {-1}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_FALSE(asgn.has_value());
  EXPECT_EQ(stats.status, Status::NOT_SAT);
}

TEST(LevinCorrectnessTest, VerifierAlwaysConfirmsSat) {
  std::vector<Formula> cases = {
      {{1, 2, 3}},
      {{1, 2, 3}, {-1, 2, 3}},
      {{1, 2, 3}, {-1, 2, 3}, {1, -2, 3}, {1, 2, -3}},
      {{1, -2, 3}, {-1, 2, 3}, {1, 2, -3}},
  };
  for (const auto& phi : cases) {
    auto [asgn, stats] = MakeSearcher().Solve(phi);
    if (stats.status == Status::SAT) {
      ASSERT_TRUE(asgn.has_value());
      EXPECT_TRUE(DPLL::Verify(phi, *asgn))
          << "Верификатор отверг присваивание для выполнимой формулы";
    }
  }
}

TEST(LevinCorrectnessTest, NeverReturnsSatForUnsatFormula) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_NE(stats.status, Status::SAT);
  EXPECT_FALSE(asgn.has_value());
}

TEST(LevinStatsTest, RoundsCompletedAtLeastOne) {
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_GE(stats.rounds_completed, 1);
}

TEST(LevinStatsTest, TotalStepsPositive) {
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_GT(stats.total_steps, 0LL);
}

TEST(LevinStatsTest, ElapsedSecondsNonNegative) {
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  EXPECT_GE(stats.elapsed_seconds, 0.0);
}

TEST(LevinStatsTest, WinnerFieldsSetOnSat) {
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_EQ(stats.status, Status::SAT);
  EXPECT_FALSE(stats.winner_program.empty());
  EXPECT_GE(stats.winner_round, 1);
}

TEST(LevinStatsTest, WinnerFieldsEmptyOnUnsat) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  auto [asgn, stats] = MakeSearcher().Solve(phi);
  ASSERT_EQ(stats.status, Status::NOT_SAT);
  EXPECT_TRUE(stats.winner_program.empty());
  EXPECT_EQ(stats.winner_round, -1);
}

TEST(LevinSettingsTest, AddProgramIncreasesCount) {
  auto searcher = MakeSearcher();
  int before = searcher.ProgramCount();
  searcher.AddProgram({
      "custom_dpll",
      [](const Formula& f, long long budget) {
        DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
        return dpll.ProcessAlgorithm(budget);
      },
  });
  EXPECT_EQ(searcher.ProgramCount(), before + 1);
}

TEST(LevinSettingsTest, SetMaxRoundsLimitsSearch) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  auto searcher = MakeSearcher(1);
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_LE(stats.rounds_completed, 1);
}

TEST(LevinSettingsTest, CustomVerificatorAlwaysFalsePreventsSat) {
  SolverFunction fallback = [](const Formula& f, long long budget) {
    DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(budget);
  };
  VerificatorFunction always_false = [](const Formula&, const Assignment&) {
    return false;
  };

  UniversalSearch searcher(DPLL::BuildPrograms(), always_false, fallback, 5,
                           false);
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_NE(stats.status, Status::SAT);
  EXPECT_FALSE(asgn.has_value());
}

TEST(LevinSettingsTest, CustomFallbackIsUsed) {
  int fallback_calls = 0;
  SolverFunction counting_fallback = [&fallback_calls](const Formula& f,
                                                       long long budget) {
    ++fallback_calls;
    DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(budget);
  };

  UniversalSearch searcher({}, DPLL::Verify, counting_fallback, 3, false);
  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_GT(fallback_calls, 0);
}

TEST(LevinSettingsTest, EmptyProgramListUsesOnlyFallback) {
  SolverFunction fallback = [](const Formula& f, long long budget) {
    DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(budget);
  };
  UniversalSearch searcher({}, DPLL::Verify, fallback, 1000, false);

  Formula phi = {{1, 2, 3}};
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_EQ(stats.status, Status::SAT);
}