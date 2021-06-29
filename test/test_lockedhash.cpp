#include "lockedhash.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

class TestClass {
public:
  string name = "";
  int value = 0;
  TestClass() {}
  TestClass(const string &n) : TestClass() { name = n; }
};

struct TestClassHash {
  size_t operator()(TestClass const &t) const noexcept {
    return std::hash<string>{}(t.name);
  }
};
struct TestClassMakeKey {
  string operator()(TestClass const &t) const noexcept { return t.name; }
};

void test_insert(
    LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> *hash,
    size_t start, size_t amount) {

  for (size_t i = start; i < start + amount; i++) {
    TestClass t;
    t.name = "k_" + to_string(i);
    (*hash)(t);
  }
}

TEST(LockedHash, insertRvalue) {
  size_t tot;
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(3);
  for (int i = 0; i < 10; i++) {
    hash(TestClass("A" + to_string(i)));
  }

  ASSERT_EQ(hash("A5").has_value(), true);
  ASSERT_EQ(hash("A5")->name, "A5");
  ASSERT_EQ(10, hash.size());

  tot = 0;
  for (auto &v : hash.bucket_elements()) {
    tot += v;
  }
  ASSERT_EQ(tot, hash.size());

  tot = 0;
  hash.loop([&tot](size_t bucket, TestClass &t) {
    (void)bucket;
    (void)t;
    tot += 1;
  });
  ASSERT_EQ(tot, hash.size());
}

TEST(LockedHash, insertLvalue) {
  size_t tot;
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);
  for (int i = 0; i < 10; i++) {
    TestClass t("A" + to_string(i));
    hash(t);
  }

  ASSERT_EQ(hash("A5").has_value(), true);
  ASSERT_EQ(hash("A5")->name, "A5");
  ASSERT_EQ(10, hash.size());

  tot = 0;
  for (auto &v : hash.bucket_elements()) {
    tot += v;
  }
  ASSERT_EQ(tot, hash.size());

  tot = 0;
  hash.loop([&tot](size_t bucket, TestClass &t) {
    (void)bucket;
    (void)t;
    tot += 1;
  });
  ASSERT_EQ(tot, hash.size());
}

TEST(LockedHash, insert) {
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);

  auto k1 = hash("A1");
  ASSERT_EQ(k1.has_value(), false);
  ASSERT_EQ(0, hash.size());

  // equal key and class
  auto k2 = hash("KKK", TestClass("KKK"));
  ASSERT_EQ(k2->name, "KKK");
  ASSERT_EQ(hash("KKK")->name, "KKK");
  ASSERT_EQ(1, hash.size());

  // different key and class
  auto k3 = hash("V1", TestClass("B1"));
  ASSERT_EQ(k3->name, "B1");
  ASSERT_EQ(hash("V1")->name, "B1");
  ASSERT_EQ(2, hash.size());
}

TEST(LockedHash, delete_with_key) {
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);

  for (int i = 0; i < 20; i++) {
    hash(TestClass("A" + to_string(i)));
  }
  ASSERT_EQ(20, hash.size());

  auto k1 = hash.rm("A10");
  ASSERT_EQ(k1.has_value(), true);
  ASSERT_EQ(k1->name, "A10");
  ASSERT_EQ(19, hash.size());

  size_t tot = 0;
  for (auto &v : hash.bucket_elements()) {
    tot += v;
  }
  ASSERT_EQ(tot, hash.size());
  ASSERT_EQ(tot, 19);
}

TEST(LockedHash, delete_with_Lvalue) {
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);

  for (int i = 0; i < 20; i++) {
    hash(TestClass("A" + to_string(i)));
  }
  ASSERT_EQ(20, hash.size());

  TestClass t("A2");
  auto k1 = hash.rm(t);
  ASSERT_EQ(k1.has_value(), true);
  ASSERT_EQ(k1->name, "A2");
  ASSERT_EQ(19, hash.size());

  size_t tot = 0;
  for (auto &v : hash.bucket_elements()) {
    tot += v;
  }
  ASSERT_EQ(tot, hash.size());
  ASSERT_EQ(tot, 19);
}

TEST(LockedHash, delete_with_Rvalue) {
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);

  for (int i = 0; i < 20; i++) {
    hash(TestClass("A" + to_string(i)));
  }
  ASSERT_EQ(20, hash.size());

  auto k1 = hash.rm(TestClass("A2"));
  ASSERT_EQ(k1.has_value(), true);
  ASSERT_EQ(k1->name, "A2");
  ASSERT_EQ(19, hash.size());

  size_t tot = 0;
  for (auto &v : hash.bucket_elements()) {
    tot += v;
  }
  ASSERT_EQ(tot, hash.size());
  ASSERT_EQ(tot, 19);
}

TEST(LockedHash, threadSafe) {
  LockedHash<string, TestClass, TestClassHash, TestClassMakeKey> hash(11);
  size_t amout = 1000;
  vector<thread> vs;
  size_t thread_count = 16;

  for (size_t i = 1; i <= thread_count; i++) {
    vs.push_back(thread(test_insert, &hash, i * amout, amout));
  }

  for (auto &t : vs) {
    t.join();
  }
  ASSERT_EQ(16000, hash.size());

  auto k1 = hash("k_11843");
  ASSERT_EQ(k1.has_value(), true);
  ASSERT_EQ(k1->name, "k_11843");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}