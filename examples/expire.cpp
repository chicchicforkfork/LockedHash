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
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100, 2);

  for (int i = 0; i < 10; i++) {
    hash(Person("P" + to_string(i), i));
  }
  cout << "[old list]\n";
  hash.loop([](size_t bucket, Person &p) {
    cout << p.to_string() << endl;
    return false;
  });

  sleep(1);
  hash(PersonKey("P5", 5), [](Person &p) {
    p.setData(1);
    p.hit();
  });

  sleep(2);
  auto expired = hash.expire();
  if (expired)
    cout << "Expire nodes : " << (*expired).size() << '\n';

  cout << "[after expire]\n";
  hash.loop([](size_t bucket, Person &p) {
    cout << p.to_string() << endl;
    return false;
  });
}
