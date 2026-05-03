#include <gtest/gtest.h>

#include <sstream>
#include <unordered_set>

#include "DPLL.hpp"
#include "Useful.hpp"

TEST(GetVariableCountTest, SingleClause) {
  Formula phi = {{1, -3, 2}};
  EXPECT_EQ(GetVariableCount(phi), 3);
}

TEST(GetVariableCountTest, MultipleClausesMaxIsCorrect) {
  Formula phi = {{1, 2, 3}, {-4, 5, -6}, {7, -8, 9}};
  EXPECT_EQ(GetVariableCount(phi), 9);
}

TEST(GetVariableCountTest, NegativeLiteralsCountedByAbsValue) {
  Formula phi = {{-5, -3, -1}};
  EXPECT_EQ(GetVariableCount(phi), 5);
}

TEST(GetVariableCountTest, EmptyFormulaReturnsOne) {
  Formula phi = {};
  EXPECT_EQ(GetVariableCount(phi), 1);
}

TEST(ElapsedTimeTest, ReturnsNonNegative) {
  TimePoint start = Clock::now();
  double elapsed = ElapsedTime(start);
  EXPECT_GE(elapsed, 0.0);
}

TEST(ElapsedTimeTest, IncreasesOverTime) {
  TimePoint start = Clock::now();
  volatile int sum = 0;
  for (int i = 0; i < 1000000; ++i) {
    sum += i;
  }
  double elapsed = ElapsedTime(start);
  EXPECT_GT(elapsed, 0.0);
}

TEST(Random3CNFTest, CorrectClauseCount) {
  Formula phi = Random3CNF(10, 30, 42);
  EXPECT_EQ(static_cast<int>(phi.size()), 30);
}

TEST(Random3CNFTest, EachClauseHasThreeLiterals) {
  Formula phi = Random3CNF(10, 50, 42);
  for (const Clause& clause : phi) {
    EXPECT_EQ(clause.size(), 3u);
  }
}

TEST(Random3CNFTest, AllLiteralsInRange) {
  const int n_vars = 10;
  Formula phi = Random3CNF(n_vars, 50, 42);
  for (const Clause& clause : phi) {
    for (Literal lit : clause) {
      EXPECT_GE(std::abs(lit), 1);
      EXPECT_LE(std::abs(lit), n_vars);
    }
  }
}

TEST(Random3CNFTest, NoRepeatedVariablesInClause) {
  Formula phi = Random3CNF(10, 100, 42);
  for (const Clause& clause : phi) {
    std::unordered_set<int> seen;
    for (Literal lit : clause) {
      EXPECT_TRUE(seen.insert(std::abs(lit)).second)
          << "Переменная " << std::abs(lit)
          << " встречается дважды в дизъюнкте";
    }
  }
}

TEST(Random3CNFTest, DifferentSeedsProduceDifferentFormulas) {
  Formula phi1 = Random3CNF(10, 20, 1);
  Formula phi2 = Random3CNF(10, 20, 2);
  EXPECT_NE(phi1, phi2);
}

TEST(Random3CNFTest, SameSeedProducesSameFormula) {
  Formula phi1 = Random3CNF(10, 20, 42);
  Formula phi2 = Random3CNF(10, 20, 42);
  EXPECT_EQ(phi1, phi2);
}

TEST(ParseDimacsTest, SimpleFormula) {
  std::string dimacs =
      "c comment\n"
      "p cnf 3 2\n"
      "1 -2 3 0\n"
      "-1 2 -3 0\n";
  Formula phi = ParseDimacs(dimacs);
  ASSERT_EQ(phi.size(), 2u);
  EXPECT_EQ(phi[0], Clause({1, -2, 3}));
  EXPECT_EQ(phi[1], Clause({-1, 2, -3}));
}

TEST(ParseDimacsTest, CommentsIgnored) {
  std::string dimacs =
      "c this is a comment\n"
      "c another comment\n"
      "p cnf 2 1\n"
      "1 2 -1 0\n";
  Formula phi = ParseDimacs(dimacs);
  ASSERT_EQ(phi.size(), 1u);
}

TEST(ParseDimacsTest, EmptyInputReturnsEmptyFormula) {
  Formula phi = ParseDimacs("");
  EXPECT_TRUE(phi.empty());
}

TEST(ParseDimacsTest, HeaderLineIgnored) {
  std::string dimacs =
      "p cnf 3 1\n"
      "1 2 3 0\n";
  Formula phi = ParseDimacs(dimacs);
  ASSERT_EQ(phi.size(), 1u);
  EXPECT_EQ(phi[0], Clause({1, 2, 3}));
}

TEST(MakeSearchTest, ReturnsCorrectProgramCount) {
  UniversalSearch searcher = MakeSearch();
  EXPECT_EQ(searcher.ProgramCount(), 24);
}

TEST(MakeSearchTest, SolvesSimpleFormula) {
  Formula phi = {{1, 2, 3}};
  UniversalSearch searcher = MakeSearch(1000, false);
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_EQ(stats.status, Status::SAT);
  ASSERT_TRUE(asgn.has_value());
  EXPECT_TRUE(DPLL::Verify(phi, *asgn));
}

TEST(MakeSearchTest, DetectsUnsatFormula) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  UniversalSearch searcher = MakeSearch(1000, false);
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_EQ(stats.status, Status::NOT_SAT);
  EXPECT_FALSE(asgn.has_value());
}

TEST(MakeSearchTest, MaxRoundsParameterRespected) {
  UniversalSearch searcher = MakeSearch(1, false);
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  auto [asgn, stats] = searcher.Solve(phi);
  EXPECT_LE(stats.rounds_completed, 1);
}