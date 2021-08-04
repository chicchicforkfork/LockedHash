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
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100, 1);

  for (int i = 1; i <= 10000; i++) {
    hash(Person("P" + to_string(i), i));
  }

  cout << "total size: " << hash.size() << endl;

  std::this_thread::sleep_for(5s);

  cout << "update P500" << endl;
  {
    auto P500 = hash(PersonKey("P500", 500), [](Person &p) { p.hit(); });
    cout << "updated: " << P500->to_string() << endl;
  }

  {
    PersonKey P500_key = PersonKey("P500", 500);
    auto P500 = hash(P500_key);
    if (P500.has_value()) {
      cout << "P500 found: " << P500->to_string() << endl;
    } else {
      cout << "not found P500" << endl;
    }
  }

  {
    auto expired = hash.expire();
    if (expired)
      cout << "Expire nodes : " << (*expired).size() << '\n';
  }
}
