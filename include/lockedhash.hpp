#ifndef __LOCKED_HASH_HPP__
#define __LOCKED_HASH_HPP__

#include <assert.h>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief LockedHashNode
 *
 * @tparam _Tp Type of Object
 */
template <typename _Tp> class LockedHashNode {
public:
  LockedHashNode *prev, *next;
  _Tp _tp;

  LockedHashNode() { prev = next = NULL; }
  LockedHashNode(_Tp &tp) : LockedHashNode() { _tp = tp; }
};

/**
 * @brief LockedHash
 *
 * @tparam _Key      Type of key objects.
 * @tparam _Tp       Type of mapped objects.
 * @tparam _Hash     Hashing function object type
 * @tparam _MakeKey  Make Key function object type
 */
template <typename _Key, typename _Tp, typename _Hash, typename _MakeKey> //
class LockedHash {
private:
  std::recursive_mutex *_bucket_locks;
  std::atomic<size_t> *_bucket_elements;
  LockedHashNode<_Tp> **_buckets;
  std::atomic<std::size_t> _size;
  size_t _bucket_size;
  _Hash _hash;
  _MakeKey _makekey;

private:
  std::recursive_mutex &_get_bucket_lock(_Key key) {
    return _bucket_locks[_hash(key) % _bucket_size];
  }

  std::recursive_mutex &_get_bucket_lock(_Tp &tp) {
    return _bucket_locks[_hash(tp) % _bucket_size];
  }

  std::recursive_mutex &_get_bucket_lock(size_t bucket) {
    return _bucket_locks[bucket % _bucket_size];
  }

  size_t _get_bucket_index(_Tp &tp) { return (_hash(tp) % _bucket_size); }

  size_t _get_bucket_index(_Key key) { return (_hash(key) % _bucket_size); }

public:
  /**
   * @brief Construct a new LockedHash<_Key, _Tp, _Hash, _MakeKey> object
   *
   * @param bucket_size  max bucket size
   */
  LockedHash<_Key, _Tp, _Hash, _MakeKey>(size_t bucket_size) {
    _bucket_size = bucket_size;
    _bucket_locks = new std::recursive_mutex[_bucket_size];
    _bucket_elements = new std::atomic<size_t>[_bucket_size] { (size_t)0, };
    _buckets = new LockedHashNode<_Tp> *[_bucket_size] { nullptr, };
    _size = 0;
  }

  /**
   * @brief Destroy the Locked Hash object
   *
   */
  ~LockedHash() {
    delete[] _bucket_locks;
    delete[] _bucket_elements;

    LockedHashNode<_Tp> *c, *n;
    for (size_t i = 0; i < _bucket_size; i++) {
      c = _buckets[i];
      while (c) {
        n = c->next;
        delete c;
        c = n;
      }
    }
    delete[] _buckets;
  }

  /**
   * @brief total element size
   *
   * @return size_t
   */
  size_t size() { return _size.load(); }

  std::optional<_Tp> operator()(_Key key) {
    return operator()(key, std::nullopt);
  }

  /**
   * @brief LockedHash(key, tp)
   *
   * @param key
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Key key, _Tp &tp) {
    return operator()(key, std::make_optional<_Tp>(tp));
  }

  /**
   * @brief LockedHash(key, optional tp)
   *
   * @param key
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Key key, std::optional<_Tp> tp) {
    bool _insert = tp.has_value();
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));

    LockedHashNode<_Tp> *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        if (!_insert) {
          return std::make_optional<_Tp>(c->_tp);
        }
        return std::nullopt;
      }
      c = c->next;
    }
    if (!_insert) {
      return std::nullopt;
    }

    c = new LockedHashNode<_Tp>(*tp);
    c->next = _buckets[bucket];
    _buckets[bucket] = c;
    if (c->next) {
      c->next->prev = c;
    }

    _bucket_elements[bucket]++;
    _size++;

    return std::make_optional<_Tp>(c->_tp);
  }

  /**
   * @brief LockedHash(tp) Lvalue
   *
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Tp &tp) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return operator()(_makekey(tp), std::make_optional<_Tp>(tp));
  }

  /**
   * @brief LockedHash(tp) : Rvalue
   *
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Tp &&tp) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return operator()(_makekey(tp), std::make_optional<_Tp>(tp));
  }

  /**
   * @brief element number of each bucket
   *
   * @return std::vector<size_t>
   */
  std::vector<size_t> bucket_elements() {
    std::vector<size_t> v;

    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      v.push_back(_bucket_elements[i].load());
    }
    return v;
  }

  /**
   * @brief rm(tp) - remove Lvalue
   *
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> rm(_Tp &tp) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return rm(_makekey(tp));
  }

  /**
   * @brief rm(tp) - remove Rvalue
   *
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> rm(_Tp &&tp) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return rm(_makekey(tp));
  }

  /**
   * @brief rm(key) - remove
   *
   * @param key
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> rm(_Key key) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    std::optional<_Tp> opt = std::nullopt;

    LockedHashNode<_Tp> *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        if (c == _buckets[bucket]) {
          _buckets[bucket] = c->next;
        } else {
          c->prev->next = c->next;
        }
        if (c->next) {
          c->next->prev = c->prev;
        }
        _size--;
        _bucket_elements[bucket]--;
        opt = std::make_optional<_Tp>(c->_tp);
        delete c;
        return opt;
      }
      c = c->next;
    }
    return opt;
  }

  /**
   * @brief loop(lambda loop function)
   *
   * @param loopf
   */
  void loop(std::function<void(size_t bucket, _Tp &tp)> loopf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode<_Tp> *c = _buckets[i];
      while (c) {
        loopf(i, c->_tp);
        c = c->next;
      }
    }
  }
};

#endif