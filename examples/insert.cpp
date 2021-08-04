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

  cout << "old lis\n";
  hash(Person("P501", 501));
  hash(Person("P502", 502));

  hash.loop([](size_t bucket, Person &p) {
    (void)bucket;
    cout << p.to_string() << "\n";
    return false;
  });

  cout << "\nafter lis\n";
  Person p500 = Person("P500", 500);
  p500.setData(100);
  hash(PersonKey("P500", 500), //
       p500,                   //
       [](Person &p) { p.setData(100); });

  hash.loop([](size_t bucket, Person &p) {
    (void)bucket;
    cout << p.to_string() << "\n";
    return false;
  });
}
