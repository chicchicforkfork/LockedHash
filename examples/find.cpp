#include "lockedhash.hpp"
#include "person.hpp"
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace chkchk;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash1(100, 60);
  for (int i = 1; i <= 10; i++) {
    hash1(Person("P" + to_string(i), i));
  }
  hash1.loop([&](size_t bucket, time_t timestamp, Person &p) { //
    (void)bucket;
    (void)timestamp;
    cout << p.to_string() << endl;
    return false;
  });
  PersonKey P1_key = PersonKey("P1", 1);
  hash1.find(P1_key, [=](Person &p) {
    if (p.empno() != 100) {
      p.setEmpno(100);
    }
  });

  Person P2 = Person("P2", 2);
  hash1.find(P2, [=](Person &p) {
    if (p.empno() != 100) {
      p.setEmpno(100);
    }
  });

  hash1.find(Person("P3", 3), [=](Person &p) {
    if (p.empno() != 100) {
      p.setEmpno(100);
    }
  });

  cout << "============================\n";
  hash1.loop([&](size_t bucket, time_t timestamp, Person &p) { //
    (void)bucket;
    (void)timestamp;
    cout << p.to_string() << endl;
    return false;
  });
  cout << "total size: " << hash1.size() << endl;
}
