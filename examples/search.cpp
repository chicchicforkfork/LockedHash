#include "lockedhash.hpp"
#include "person.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;
using namespace chkchk;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100);

  for (int i = 1; i <= 10000; i++) {
    hash(Person("P" + to_string(i), i));
  }

  cout << "total size: " << hash.size() << endl;

  cout << "search P500 (lvalue)" << endl;
  PersonKey P500_key = PersonKey("P500", 500);
  auto P500 = hash(P500_key);
  if (P500.has_value()) {
    cout << P500->to_string() << endl;
  } else {
    cout << "not found P500" << endl;
  }

  cout << "search P600 (lvalue)" << endl;
  PersonKey P600_key = PersonKey("P600", 600);
  auto P600 = hash[P600_key];
  if (P600.has_value()) {
    cout << P600->to_string() << endl;
  } else {
    cout << "not found P600" << endl;
  }
  cout << P600_key.first << ", " << P600_key.second << endl;

  cout << "search P700 (rvalue)" << endl;
  auto P700 = hash[PersonKey("P700", 700)];
  if (P700.has_value()) {
    cout << P700->to_string() << endl;
  } else {
    cout << "not found P700" << endl;
  }
}
