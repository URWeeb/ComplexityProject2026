#include <gtest/gtest.h>

#include <optional>

#include "DPLL.hpp"

TEST(SimplifyTest, TrueLiteralRemovesClause) {
  Formula phi = {{1, 2, 3}};
  auto result = DPLL::Simplify(phi, 1, true);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(SimplifyTest, FalseLiteralRemovedFromClause) {
  Formula phi = {{1, 2, 3}};
  auto result = DPLL::Simplify(phi, 1, false);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1u);
  EXPECT_EQ((*result)[0], Clause({2, 3}));
}

TEST(SimplifyTest, NegatedLiteralTrueRemovesClause) {
  Formula phi = {{-1, 2}};
  auto result = DPLL::Simplify(phi, 1, false);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(SimplifyTest, NegatedLiteralFalseRemovedFromClause) {
  Formula phi = {{-1, 2}};
  auto result = DPLL::Simplify(phi, 1, true);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1u);
  EXPECT_EQ((*result)[0], Clause({2}));
}

TEST(SimplifyTest, EmptyClauseReturnsNullopt) {
  Formula phi = {{1}};
  auto result = DPLL::Simplify(phi, 1, false);
  EXPECT_FALSE(result.has_value());
}

TEST(SimplifyTest, UnrelatedClauseUnchanged) {
  Formula phi = {{2, 3}};
  auto result = DPLL::Simplify(phi, 1, true);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1u);
  EXPECT_EQ((*result)[0], Clause({2, 3}));
}

TEST(SimplifyTest, MultipleClausesMixed) {
  Formula phi = {{1, 2}, {-1, 3}};
  auto result = DPLL::Simplify(phi, 1, true);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result->size(), 1u);
  EXPECT_EQ((*result)[0], Clause({3}));
}

TEST(SimplifyTest, EmptyFormulaStaysEmpty) {
  Formula phi = {};
  auto result = DPLL::Simplify(phi, 1, true);
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());
}

TEST(UnitPropagateTest, PositiveUnitClausePropagated) {
  Formula phi = {{1}};
  Assignment asgn;
  EXPECT_TRUE(DPLL::UnitPropagate(phi, asgn));
  EXPECT_TRUE(phi.empty());
  EXPECT_EQ(asgn.at(1), true);
}

TEST(UnitPropagateTest, NegativeUnitClausePropagated) {
  Formula phi = {{-2}};
  Assignment asgn;
  EXPECT_TRUE(DPLL::UnitPropagate(phi, asgn));
  EXPECT_TRUE(phi.empty());
  EXPECT_EQ(asgn.at(2), false);
}

TEST(UnitPropagateTest, ChainPropagation) {
  Formula phi = {{1}, {-1, 2}};
  Assignment asgn;
  EXPECT_TRUE(DPLL::UnitPropagate(phi, asgn));
  EXPECT_TRUE(phi.empty());
  EXPECT_EQ(asgn.at(1), true);
  EXPECT_EQ(asgn.at(2), true);
}

TEST(UnitPropagateTest, ConflictingUnitClausesReturnFalse) {
  Formula phi = {{1}, {-1}};
  Assignment asgn;
  EXPECT_FALSE(DPLL::UnitPropagate(phi, asgn));
}

TEST(UnitPropagateTest, ExistingConsistentAssignmentKept) {
  Formula phi = {{1}};
  Assignment asgn = {{1, true}};
  EXPECT_TRUE(DPLL::UnitPropagate(phi, asgn));
}

TEST(UnitPropagateTest, ExistingContradictoryAssignmentReturnsFalse) {
  Formula phi = {{1}};
  Assignment asgn = {{1, false}};
  EXPECT_FALSE(DPLL::UnitPropagate(phi, asgn));
}

TEST(UnitPropagateTest, NoUnitClausesNoChange) {
  Formula phi = {{1, 2}};
  Assignment asgn;
  EXPECT_TRUE(DPLL::UnitPropagate(phi, asgn));
  EXPECT_EQ(phi.size(), 1u);
  EXPECT_TRUE(asgn.empty());
}

TEST(UnitPropagateTest, EmptyClauseInFormulaReturnsFalse) {
  Formula phi = {{}};
  Assignment asgn;
  EXPECT_FALSE(DPLL::UnitPropagate(phi, asgn));
}

TEST(PureLiteralAssignTest, PositivePureLiteralFixed) {
  Formula phi = {{1, 2}, {1, 3}};
  Assignment asgn;
  DPLL::PureLiteralAssign(phi, asgn);
  EXPECT_EQ(asgn.at(1), true);
  EXPECT_TRUE(phi.empty());
}

TEST(PureLiteralAssignTest, NegativePureLiteralFixed) {
  Formula phi = {{-1, 2}, {-1, 3}};
  Assignment asgn;
  DPLL::PureLiteralAssign(phi, asgn);
  EXPECT_EQ(asgn.at(1), false);
  EXPECT_TRUE(phi.empty());
}

TEST(PureLiteralAssignTest, MixedSignNotPure) {
  Formula phi = {{1, 2}, {-1, 3}};
  Assignment asgn;
  DPLL::PureLiteralAssign(phi, asgn);
  EXPECT_FALSE(asgn.count(1));
}

TEST(PureLiteralAssignTest, AlreadyAssignedVariableSkipped) {
  Formula phi = {{2, 3}};
  Assignment asgn = {{2, false}};
  DPLL::PureLiteralAssign(phi, asgn);
  EXPECT_EQ(asgn.at(2), false);
}

TEST(PureLiteralAssignTest, EmptyFormulaNoChange) {
  Formula phi = {};
  Assignment asgn;
  DPLL::PureLiteralAssign(phi, asgn);
  EXPECT_TRUE(asgn.empty());
}

TEST(VerifyTest, AllPositiveLiteralsCorrect) {
  Formula phi = {{1, 2, 3}};
  Assignment asgn = {{1, true}, {2, false}, {3, false}};
  EXPECT_TRUE(DPLL::Verify(phi, asgn));
}

TEST(VerifyTest, AllPositiveLiteralsAllFalseNotSat) {
  Formula phi = {{1, 2, 3}};
  Assignment asgn = {{1, false}, {2, false}, {3, false}};
  EXPECT_FALSE(DPLL::Verify(phi, asgn));
}

TEST(VerifyTest, EmptyFormulaAlwaysTrue) {
  Formula phi = {};
  Assignment asgn = {};
  EXPECT_TRUE(DPLL::Verify(phi, asgn));
}

TEST(VerifyTest, MultipleClausesAllSatisfied) {
  Formula phi = {{1, 2}, {2, 3}};
  Assignment asgn = {{1, false}, {2, true}, {3, false}};
  EXPECT_TRUE(DPLL::Verify(phi, asgn));
}

TEST(VerifyTest, OneClauseUnsatisfiedReturnsFalse) {
  Formula phi = {{1, 2}, {3}};
  Assignment asgn = {{1, true}, {2, true}, {3, false}};
  EXPECT_FALSE(DPLL::Verify(phi, asgn));
}

TEST(VerifyTest, NegativeLiteralSatisfied) {
  Formula phi = {{-1}};
  Assignment asgn = {{1, false}};
  EXPECT_TRUE(DPLL::Verify(phi, asgn));
}

TEST(ProcessAlgorithmTest, TrivialOneClauseSat) {
  Formula phi = {{1, 2, 3}};
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
  EXPECT_GT(result.steps_used, 0);
}

TEST(ProcessAlgorithmTest, UnitPropagationSolvesDirectly) {
  Formula phi = {{1}, {1, 2, 3}};
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
  EXPECT_EQ(result.assignment.at(1), true);
}

TEST(ProcessAlgorithmTest, ThreeVariablesSat) {
  Formula phi = {{1, 2, 3}, {-1, 2, 3}, {1, -2, 3}};
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
}

TEST(ProcessAlgorithmTest, SatWithMaxIndexStrategy) {
  Formula phi = {{1, 2, 3}, {1, -2, 3}};
  DPLL dpll(phi, BranchStrategy::MaxIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
}

TEST(ProcessAlgorithmTest, SatWithMostFrequentStrategy) {
  Formula phi = {{1, 2, 3}, {1, -2, 3}, {1, 2, -3}};
  DPLL dpll(phi, BranchStrategy::MostFrequent, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
}

TEST(ProcessAlgorithmTest, SatWithLeastFrequentStrategy) {
  Formula phi = {{1, 2, 3}, {1, -2, 3}};
  DPLL dpll(phi, BranchStrategy::LeastFrequent, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
}

TEST(ProcessAlgorithmTest, SatWithRandomStrategy) {
  Formula phi = {{1, 2, 3}, {-1, 2, 3}};
  DPLL dpll(phi, BranchStrategy::Random, std::mt19937{42});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::SAT);
}

TEST(ProcessAlgorithmTest, DirectContradictionUnsat) {
  Formula phi = {{1}, {-1}};
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(10000);
  EXPECT_EQ(result.status, Status::NOT_SAT);
}

TEST(ProcessAlgorithmTest, AllCombinationsUnsat) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(100000);
  EXPECT_EQ(result.status, Status::NOT_SAT);
  EXPECT_TRUE(result.assignment.empty());
}

TEST(ProcessAlgorithmTest, ZeroBudgetGivesTimeout) {
  Formula phi = {{1, 2, 3}};
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(0);
  EXPECT_EQ(result.status, Status::TIMEOUT);
}

TEST(ProcessAlgorithmTest, StepsUsedDoesNotExceedBudget) {
  Formula phi = {{1, 2, 3}, {-1, 2, 3}, {1, -2, 3}};
  long long budget = 2;
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(budget);
  EXPECT_LE(result.steps_used, budget);
}

TEST(ProcessAlgorithmTest, TimeoutReturnsNoAssignment) {
  Formula phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };
  DPLL dpll(phi, BranchStrategy::MinIndex, std::mt19937{0});
  auto result = dpll.ProcessAlgorithm(1);
  EXPECT_EQ(result.status, Status::TIMEOUT);
}

TEST(BuildProgramsTest, ReturnsTwentyFourPrograms) {
  auto programs = DPLL::BuildPrograms();
  EXPECT_EQ(programs.size(), 24u);
}

TEST(BuildProgramsTest, AllProgramsHaveNonEmptyName) {
  auto programs = DPLL::BuildPrograms();
  for (const auto& prog : programs) {
    EXPECT_FALSE(prog.name.empty()) << "Пустое имя у программы";
  }
}

TEST(BuildProgramsTest, DeterministicProgramNamesCorrect) {
  auto programs = DPLL::BuildPrograms();
  EXPECT_EQ(programs[0].name, "dpll_min");
  EXPECT_EQ(programs[1].name, "dpll_max");
  EXPECT_EQ(programs[2].name, "dpll_most_frequent");
  EXPECT_EQ(programs[3].name, "dpll_least_frequent");
}

TEST(BuildProgramsTest, RandomSeedProgramNamesCorrect) {
  auto programs = DPLL::BuildPrograms();
  for (int i = 0; i < 20; ++i) {
    EXPECT_EQ(programs[4 + i].name, "dpll_random_seed" + std::to_string(i));
  }
}