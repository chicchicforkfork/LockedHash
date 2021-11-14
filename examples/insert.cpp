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
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash(100, 60);

  cout << "old lis\n";
  hash(Person("P501", 501));
  hash(Person("P502", 502));

  hash.loop([](size_t bucket, time_t timestamp, Person &p) {
    (void)timestamp;
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

  hash.loop([](size_t bucket, time_t timestamp, Person &p) {
    (void)timestamp;
    (void)bucket;
    cout << p.to_string() << "\n";
    return false;
  });
}
