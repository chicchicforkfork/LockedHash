#ifndef __PERSON_SAMPLE_HPP__
#define __PERSON_SAMPLE_HPP__

#include <string>

using PersonKey = std::pair<std::string, int>;

class Person {
private:
  std::string _name;
  int _empno;
  int _cache;

public:
  Person() {}
  Person(const PersonKey &k) : Person() {
    _name = k.first;
    _empno = k.second;
    _cache = 0;
  }
  Person(const std::string &name, int empno) : Person() {
    _name = name;
    _empno = empno;
  }
  const std::string name() const { return _name; }
  void setName(const std::string name) { _name = name; }
  int empno() const { return _empno; }
  void setEmpno(int empno) { _empno = empno; }
  void hit() { _cache++; }

  std::string to_string() {
    return "name: " + _name + ", empno: " + std::to_string(_empno) +
           "cache: " + std::to_string(_cache);
  }
};

struct PersonHash {
  size_t operator()(Person const &p) const noexcept {
    return std::hash<std::string>{}(p.name()) + p.empno();
  }
};
struct PersonMakeKey {
  PersonKey operator()(Person const &p) const noexcept {
    return PersonKey(p.name(), p.empno());
  }
};

#endif