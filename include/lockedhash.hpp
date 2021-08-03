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

namespace chkchk {

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
  /**
   * @brief LockedHashNode
   *
   * @tparam _Tp Type of Object
   */
  class LockedHashNode {
  public:
    LockedHashNode *prev, *next;
    _Tp _tp;

    LockedHashNode() { prev = next = NULL; }
    LockedHashNode(_Tp &tp) : LockedHashNode() { _tp = tp; }
    LockedHashNode(_Tp &&tp) : LockedHashNode(tp) {}
  };

private:
  /// bucket locks
  std::recursive_mutex *_bucket_locks;
  /// element number for each bucket
  std::atomic<size_t> *_bucket_elements;
  /// bucket array
  LockedHashNode **_buckets;
  /// total elements
  std::atomic<std::size_t> _size;
  /// fixed bucket size
  size_t _bucket_size;
  /// hash function
  _Hash _hash;
  /// make key function
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

  size_t _get_bucket_index(_Tp &tp) { //
    return (_hash(tp) % _bucket_size);
  }

  size_t _get_bucket_index(_Key key) { //
    return (_hash(key) % _bucket_size);
  }

public:
  /**
   * @brief Construct a new LockedHash<_Key, _Tp, _Hash, _MakeKey> object
   *
   * @param bucket_size  fixed bucket size
   */
  LockedHash<_Key, _Tp, _Hash, _MakeKey>(size_t bucket_size) {
    _bucket_size = bucket_size;
    _bucket_locks = new std::recursive_mutex[_bucket_size];
    _bucket_elements = new std::atomic<size_t>[_bucket_size] { (size_t)0, };
    _buckets = new LockedHashNode *[_bucket_size] { nullptr, };
    _size = 0; /// atomic
  }

  /**
   * @brief Destroy the Locked Hash object
   *
   */
  ~LockedHash() {
    delete[] _bucket_locks;
    delete[] _bucket_elements;

    LockedHashNode *c, *n;
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
  size_t size() { //
    return _size.load();
  }

  /**
   * @brief search data (lvalue)
   *
   * @param key
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Key key) {
    return operator()(key, std::nullopt);
  }

  /**
   * @brief search data (lvalue)
   *
   * @param key
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator[](_Key &key) { //
    return operator[](std::move(key));
  }

  /**
   * @brief search data (rvalue)
   *
   * @param key
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator[](_Key &&key) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));

    LockedHashNode *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        return c->_tp;
      }
      c = c->next;
    }

    return std::nullopt;
  }

  /**
   * @brief update data
   *
   * @param key
   * @param interceptor
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Key key, //
                                std::function<void(_Tp &)> interceptor) {
    return operator()(key, operator[](key), interceptor);
  }

  /**
   * @brief insert data
   *
   * @param key
   * @param tp
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> operator()(_Key key, _Tp &tp) {
    return operator()(key, std::make_optional<_Tp>(tp));
  }

  /**
   * @brief insert or update data (Lvalue)
   *
   * @param tp
   * @param interceptor
   * @return std::optional<_Tp>
   */
  std::optional<_Tp>
  operator()(_Tp &tp, //
             std::function<void(_Tp &)> interceptor = nullptr) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return operator()(_makekey(tp), std::make_optional<_Tp>(tp), interceptor);
  }

  /**
   * @brief insert or update data (Rvalue)
   *
   * @param tp
   * @param interceptor
   * @return std::optional<_Tp>
   */
  std::optional<_Tp>
  operator()(_Tp &&tp, //
             std::function<void(_Tp &)> interceptor = nullptr) {
    size_t bucket = _get_bucket_index(tp);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    return operator()(_makekey(tp), std::make_optional<_Tp>(tp), interceptor);
  }

  /**
   * @brief insert or update or search
   *
   * @param key
   * @param tp
   * @param interceptor
   * @return std::optional<_Tp>
   */
  std::optional<_Tp>
  operator()(_Key key,              //
             std::optional<_Tp> tp, //
             std::function<void(_Tp &)> interceptor = nullptr) {
    bool is_insert = tp.has_value();
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));

    LockedHashNode *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        if (!is_insert) {
          // only search
          return std::make_optional<_Tp>(c->_tp);
        }
        if (interceptor) {
          // update data
          interceptor(c->_tp);
        }
        // return std::nullopt;
        return std::make_optional<_Tp>(c->_tp);
      }
      c = c->next;
    }
    if (!is_insert) {
      return std::nullopt;
    }

    c = new LockedHashNode(*tp);
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
   * @brief remove data for Rvalue
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
   * @brief remove data for
   *
   * @param key
   * @return std::optional<_Tp>
   */
  std::optional<_Tp> rm(_Key key) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    std::optional<_Tp> opt = std::nullopt;

    LockedHashNode *c = _buckets[bucket];
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
      LockedHashNode *c = _buckets[i];
      while (c) {
        loopf(i, c->_tp);
        c = c->next;
      }
    }
  }

  /**
   * @brief loop_with_delete(lambda loop function)
   * loopf 결과가 true인 경우 Node를 제거한다.
   *
   * @param loopf
   */
  void loop_with_delete(std::function<bool(size_t bucket, _Tp &tp)> loopf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      LockedHashNode *tmp;
      while (c) {
        if (loopf(i, c->_tp)) {
          tmp = c->next;
          if (c == _buckets[i]) {
            _buckets[i] = c->next;
          } else {
            c->prev->next = c->next;
          }
          if (c->next) {
            c->next->prev = c->prev;
          }
          _size--;
          _bucket_elements[i]--;
          delete c;

          c = tmp;
        } else {
          c = c->next;
        }
      }
    }
  }
};
}; // namespace chkchk

#endif