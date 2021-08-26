#include "lockedhash.hpp"
#include "person.hpp"
#include <iostream>
#include <kairos.h>
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
  int datasize = 10000000;

  KairosStack kstack("Kairos", 10);

  Kairos k1("hash build");
  k1.begin();
  LockedHash<PersonKey, Person, PersonHash, PersonMakeKey> hash1(datasize, 60);
  for (int i = 1; i <= datasize; i++) {
    hash1(Person("P" + to_string(i), i));
  }
  k1.end();
  kstack.addKairos(k1);

  cout << "total size: " << hash1.size() << endl;

  Kairos k2("hash clear");
  k2.begin();
  hash1.clear();
  k2.end();
  kstack.addKairos(k2);

  cout << "total size: " << hash1.size() << endl;
  cout << kstack << "\n";
}
/*
benchmark
CPU: Intel(R) Core(TM) i5-6400 CPU @ 2.70GHz (4core)

~/git/LockedHash$ ./build/examples/clear
total size: 10000000
total size: 0
[Kairos] total:12.0454375, avg:6.0227187
(1) hash build: 10.0724958
(2) hash clear: 1.0729417
*/
