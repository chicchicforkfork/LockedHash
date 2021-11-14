#include "lockedhash.hpp"
#include "person.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using namespace std;

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100, 5);

  for (int i = 1; i <= 100; i++) {
    hash(Person("P" + to_string(i), i));
  }

  cout << "total size: " << hash.size() << endl;

  std::this_thread::sleep_for(3s);

  auto p50alive = hash.alive(PersonKey("P50", 50));
  cout << "P50 alive? " << p50alive->to_string() << endl;

  auto p20key = PersonKey("P20", 20);
  auto p20alive = hash.alive(p20key);
  cout << "P20 alive? " << p20alive->to_string() << endl;

  std::this_thread::sleep_for(3s);
  {
    auto expired = hash.expire();
    if (expired)
      cout << "Expire nodes : " << (*expired).size() << '\n';
  }

  cout << "total size: " << hash.size() << endl;

  return 0;
}
