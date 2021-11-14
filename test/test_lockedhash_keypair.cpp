#include "lockedhash.hpp"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

using PersonKey = pair<string, int>;

class Person {
private:
  string _name = "";
  int _empno = 1;

public:
  Person() {}
  Person(const PersonKey &k) : Person() {
    _name = k.first;
    _empno = k.second;
  }
  Person(const string &name, int empno) : Person() {
    _name = name;
    _empno = empno;
  }
  const string name() const { return _name; }
  int empno() const { return _empno; }
};

struct PersonHash {
  size_t operator()(Person const &p) const noexcept {
    return std::hash<string>{}(p.name()) + p.empno();
  }
};
struct PersonMakeKey {
  PersonKey operator()(Person const &p) const noexcept {
    return PersonKey(p.name(), p.empno());
  }
};

TEST(LockedHash_pairkey, insertRvalue_pairkey) {
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(3);
  for (int i = 1; i <= 10; i++) {
    hash(Person("P#" + to_string(i), 100 + i));
  }

  ASSERT_EQ(hash(PersonKey("P#3", 103)).has_value(), true);
  ASSERT_EQ(hash(PersonKey("P#3", 103))->name(), "P#3");
  ASSERT_EQ(10, hash.size());
}

TEST(LockedHash_pairkey, insertLvalue_pairkey) {
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(3);
  for (int i = 1; i <= 10; i++) {
    Person p("P#" + to_string(i), 100 + i);
    hash(p);
  }

  ASSERT_EQ(hash(PersonKey("P#3", 103)).has_value(), true);
  ASSERT_EQ(hash(PersonKey("P#3", 103))->name(), "P#3");
  ASSERT_EQ(10, hash.size());
}
