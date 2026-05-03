#include <gtest/gtest.h>

#include <unordered_set>

#include "DPLL.hpp"
#include "Useful.hpp"

static bool CorrectVerify(const Formula& formula,
                          const Assignment& assignment) {
  for (const Clause& clause : formula) {
    bool satisfied = false;
    for (Literal lit : clause) {
      auto it = assignment.find(std::abs(lit));
      if (it == assignment.end()) {
        continue;
      }
      if ((lit > 0) ? it->second : !it->second) {
        satisfied = true;
        break;
      }
    }
    if (!satisfied) {
      return false;
    }
  }
  return true;
}

static UniversalSearch MakeSearcher(bool detailed_output = false) {
  SolverFunction fallback = [](const Formula& f, long long budget) {
    DPLL dpll(f, BranchStrategy::MinIndex, std::mt19937{0});
    return dpll.ProcessAlgorithm(budget);
  };
  return UniversalSearch(DPLL::BuildPrograms(), DPLL::Verify, fallback, 50000,
                         detailed_output);
}

TEST(StressVerifyTest, RandomSatFormulasVerifyCorrectly) {
  for (uint32_t seed = 0; seed < 100; ++seed) {
    Formula phi = Random3CNF(10, 30, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status == Status::SAT) {
      ASSERT_TRUE(asgn.has_value()) << "seed=" << seed;
      EXPECT_TRUE(DPLL::Verify(phi, *asgn))
          << "Verify вернул false для SAT-присваивания, seed=" << seed;
      EXPECT_TRUE(CorrectVerify(phi, *asgn))
          << "CorrectVerify вернул false для SAT-присваивания, seed=" << seed;
    }
  }
}

TEST(StressVerifyTest, VerifyAndCorrectVerifyAgree) {
  for (uint32_t seed = 0; seed < 100; ++seed) {
    Formula phi = Random3CNF(10, 30, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status == Status::SAT) {
      ASSERT_TRUE(asgn.has_value());
      EXPECT_EQ(DPLL::Verify(phi, *asgn), CorrectVerify(phi, *asgn))
          << "Verify и CorrectVerify расходятся, seed=" << seed;
    }
  }
}

TEST(StressVerifyTest, WrongAssignmentRejected) {
  for (uint32_t seed = 0; seed < 50; ++seed) {
    Formula phi = Random3CNF(10, 30, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status != Status::SAT) {
      continue;
    }
    ASSERT_TRUE(asgn.has_value());

    Assignment wrong_asgn = *asgn;
    for (auto& [var, val] : wrong_asgn) {
      val = !val;
    }

    if (!CorrectVerify(phi, wrong_asgn)) {
      EXPECT_FALSE(DPLL::Verify(phi, wrong_asgn))
          << "Verify принял заведомо неверное присваивание, seed=" << seed;
    }
  }
}

TEST(StressVerifyTest, NearPhaseTransitionSat) {
  const int n_vars = 20;
  const int n_clauses = 85;

  int sat_count = 0;
  int verified_count = 0;

  for (uint32_t seed = 0; seed < 30; ++seed) {
    Formula phi = Random3CNF(n_vars, n_clauses, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status == Status::SAT) {
      ++sat_count;
      ASSERT_TRUE(asgn.has_value());
      if (DPLL::Verify(phi, *asgn) && CorrectVerify(phi, *asgn)) {
        ++verified_count;
      }
    }
  }

  EXPECT_GT(sat_count, 0) << "Ни одна формула не оказалась выполнимой";
  EXPECT_EQ(sat_count, verified_count)
      << "Не все SAT-присваивания прошли верификацию";
}

TEST(StressVerifyTest, NegativeLiteralsVerifiedCorrectly) {
  for (uint32_t seed = 0; seed < 100; ++seed) {
    Formula phi = Random3CNF(10, 30, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status != Status::SAT) {
      continue;
    }
    ASSERT_TRUE(asgn.has_value());

    for (const Clause& clause : phi) {
      for (Literal lit : clause) {
        if (lit < 0) {
          auto it = asgn->find(std::abs(lit));
          if (it != asgn->end()) {
            bool expected_neg_val = !it->second;
            bool verify_neg_val = DPLL::Verify(
                Formula{{lit}}, Assignment{{std::abs(lit), it->second}});
            EXPECT_EQ(verify_neg_val, expected_neg_val)
                << "Неверная обработка отрицательного литерала " << lit
                << ", seed=" << seed;
          }
        }
      }
    }
  }
}

TEST(StressVerifyTest, EmptyAssignmentRejectsNonEmptyFormula) {
  for (uint32_t seed = 0; seed < 20; ++seed) {
    Formula phi = Random3CNF(5, 10, seed);
    Assignment empty_asgn;
    EXPECT_FALSE(DPLL::Verify(phi, empty_asgn))
        << "Verify принял пустое присваивание для непустой формулы, seed="
        << seed;
  }
}

TEST(StressVerifyTest, LargeFormulaSatVerified) {
  const int n_vars = 50;
  const int n_clauses = 150;

  for (uint32_t seed = 0; seed < 10; ++seed) {
    Formula phi = Random3CNF(n_vars, n_clauses, seed);
    auto searcher = MakeSearcher();
    auto [asgn, stats] = searcher.Solve(phi);

    if (stats.status == Status::SAT) {
      ASSERT_TRUE(asgn.has_value());
      EXPECT_TRUE(DPLL::Verify(phi, *asgn))
          << "Верификация большой формулы провалилась, seed=" << seed;
      EXPECT_TRUE(CorrectVerify(phi, *asgn))
          << "CorrectVerify большой формулы провалилась, seed=" << seed;
    }
  }
}

TEST(StressVerifyTest, UnsatFormulaNeverVerifies) {
  Formula unsat_phi = {
      {1, 2, 3},  {1, 2, -3},  {1, -2, 3},  {1, -2, -3},
      {-1, 2, 3}, {-1, 2, -3}, {-1, -2, 3}, {-1, -2, -3},
  };

  std::mt19937 rng(42);
  for (int attempt = 0; attempt < 100; ++attempt) {
    Assignment random_asgn;
    std::bernoulli_distribution dist(0.5);
    for (int var = 1; var <= 3; ++var) {
      random_asgn[var] = dist(rng);
    }
    EXPECT_FALSE(DPLL::Verify(unsat_phi, random_asgn))
        << "Verify принял присваивание для невыполнимой формулы, attempt="
        << attempt;
    EXPECT_FALSE(CorrectVerify(unsat_phi, random_asgn))
        << "CorrectVerify принял присваивание для невыполнимой формулы, "
           "attempt="
        << attempt;
  }
}