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
  for (int i = 1; i <= 100; i++) {
    hash1(Person("P" + to_string(i), i));
  }
  size_t tot = 0;
  hash1.loop([&](size_t bucket, time_t timestamp, Person &p) { //
    (void)timestamp;
    (void)bucket;
    (void)p;
    tot++;
    return false;
  });
  cout << "total size: " << hash1.size() << endl;

  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash2(100, 60);
  for (int i = 1; i <= 100; i++) {
    hash2(Person("P" + to_string(i), i));
  }
  hash2.loop_with_delete([&](size_t bucket, time_t timestamp, Person &p) { //
    (void)bucket;
    (void)p;
    (void)timestamp;
    return true;
  });
  cout << "total size: " << hash2.size() << endl;

  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash3(100, 60);
  for (int i = 1; i <= 100; i++) {
    hash3(Person("P" + to_string(i), i));
  }
  size_t odd = 0;
  hash3.loop_with_delete([&](size_t bucket, time_t timestamp, Person &p) { //
    (void)bucket;
    (void)p;
    (void)timestamp;
    if (odd++ % 2 == 0) {
      return true;
    }
    return false;
  });
  cout << "total size: " << hash3.size() << endl;
}
