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
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100, 2);

  for (int i = 0; i < 10; i++) {
    hash(Person("P" + to_string(i), i));
  }
  cout << "[old list]\n";
  hash.loop([](size_t bucket, time_t timestamp, Person &p) {
    (void)bucket;
    (void)timestamp;
    cout << p.to_string() << endl;
    return false;
  });

  auto expired = hash.expire([](Person &p, time_t timestamp, void *arg) {
    (void)arg;
    (void)timestamp;
    if (p.empno() > 1) {
      return true;
    }
    return false;
  });
  if (expired)
    cout << "Expire nodes : " << (*expired).size() << '\n';

  cout << "[after expire]\n";
  hash.loop([](size_t bucket, time_t timestamp, Person &p) {
    (void)bucket;
    (void)timestamp;
    cout << p.to_string() << endl;
    return false;
  });
}
