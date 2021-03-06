#ifndef __LOCKED_HASH_HPP__
#define __LOCKED_HASH_HPP__

#include <assert.h>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <mutex>
#include <optional.hpp>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>

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
    time_t _timestamp = time(nullptr);

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

  time_t _expire_time = 0;

private:
  std::recursive_mutex &_get_bucket_lock(_Key key) {
    return _bucket_locks[_hash(key) % _bucket_size];
  }

  std::recursive_mutex &_get_bucket_lock(_Tp &tp) {
    return _get_bucket_lock(_makekey(tp));
  }

  std::recursive_mutex &_get_bucket_lock(size_t bucket) {
    return _bucket_locks[bucket % _bucket_size];
  }

  size_t _get_bucket_index(_Tp &tp) { //
    return _get_bucket_index(_makekey(tp));
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
  LockedHash<_Key, _Tp, _Hash, _MakeKey>(size_t bucket_size,
                                         time_t expire_time) {
    _bucket_size = bucket_size;
    _bucket_locks = new std::recursive_mutex[_bucket_size];
    _bucket_elements =
        new std::atomic<size_t>[_bucket_size] { ATOMIC_VAR_INIT(0) };
    _buckets = new LockedHashNode *[_bucket_size] { nullptr, };
    _size = 0; /// atomic
    _expire_time = expire_time;
  }

  /**
   * @brief Destroy the Locked Hash object
   *
   */
  virtual ~LockedHash() {
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
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> operator()(_Key key) {
    return operator()(key, tl::nullopt);
  }

  /**
   * @brief search data (lvalue)
   *
   * @param key
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> operator[](_Key &key) { //
    return operator[](std::move(key));
  }

  /**
   * @brief search data (rvalue)
   *
   * @param key
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> operator[](_Key &&key) { //
    return operator()(key);
  }

  /**
   * @brief update data
   *
   * @param key
   * @param interceptor
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> operator()(_Key key, //
                               std::function<void(_Tp &)> interceptor) {
    return operator()(key, operator[](key), interceptor);
  }

  /**
   * @brief insert data
   *
   * @param key
   * @param tp
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> operator()(_Key key, _Tp &tp) {
    return operator()(key, tl::make_optional<_Tp>(tp));
  }

  /**
   * @brief insert or update data (Lvalue)
   *
   * @param tp
   * @param interceptor
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp>
  operator()(_Tp &tp, //
             std::function<void(_Tp &)> interceptor = nullptr) {
    return operator()(_makekey(tp), tl::make_optional<_Tp>(tp), interceptor);
  }

  /**
   * @brief insert or update data (Rvalue)
   *
   * @param tp
   * @param interceptor
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp>
  operator()(_Tp &&tp, //
             std::function<void(_Tp &)> interceptor = nullptr) {
    return operator()(_makekey(tp), tl::make_optional<_Tp>(tp), interceptor);
  }

  /**
   * @brief insert or update or search
   *
   * @param key
   * @param tp
   * @param interceptor
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp>
  operator()(_Key key,             //
             tl::optional<_Tp> tp, //
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
          return tl::make_optional<_Tp>(c->_tp);
        }
        if (interceptor) {
          // update data
          interceptor(c->_tp);
          c->_timestamp = time(nullptr);
        }
        // insert ??? ???????????? return ?????? ??????
        // update ??? ???????????? return nullopt ??????
        // insert??? update ???????
        return tl::nullopt;
        // return tl::make_optional<_Tp>(c->_tp);
      }
      c = c->next;
    }
    if (!is_insert) {
      return tl::nullopt;
    }

    c = new LockedHashNode(*tp);
    c->next = _buckets[bucket];
    _buckets[bucket] = c;
    if (c->next) {
      c->next->prev = c;
    }

    _bucket_elements[bucket]++;
    _size++;

    return tl::make_optional<_Tp>(c->_tp);
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
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> rm(_Tp &tp) { //
    return rm(_makekey(tp));
  }

  /**
   * @brief remove data for Rvalue
   *
   * @param tp
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> rm(_Tp &&tp) { //
    return rm(_makekey(tp));
  }

  /**
   * @brief remove data for
   *
   * @param key
   * @return tl::optional<_Tp>
   */
  tl::optional<_Tp> rm(_Key key, std::function<bool(_Tp &tp)> rmf = nullptr) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));
    tl::optional<_Tp> opt = tl::nullopt;

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
        bool do_delete = true;
        if (rmf) {
          do_delete = rmf(c->_tp);
        }
        if (do_delete) {
          _size--;
          _bucket_elements[bucket]--;
          opt = tl::make_optional<_Tp>(c->_tp);
          delete c;
        }
        return opt;
      }
      c = c->next;
    }
    return opt;
  }

  void find(_Key key, std::function<void(_Tp &tp)> findf) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));

    LockedHashNode *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        findf(c->_tp);
        return;
      }
      c = c->next;
    }
    return;
  }

  void find(_Tp &&tp, std::function<void(_Tp &tp)> findf) { //
    return find(_makekey(tp), findf);
  }

  void find(_Tp &tp, std::function<void(_Tp &tp)> findf) { //
    return find(_makekey(tp), findf);
  }

  /**
   * @brief loop(lambda loop function)
   * loopf ????????? true??? ?????? Node??? timestamp??? ???????????? ??????.
   *
   * @param loopf
   */
  void
  loop(std::function<bool(size_t bucket, time_t timestamp, _Tp &tp)> loopf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      while (c) {
        if (loopf(i, c->_timestamp, c->_tp)) {
          c->_timestamp = time(nullptr);
        }
        c = c->next;
      }
    }
  }

  /**
   * @brief loop_with_delete(lambda loop function)
   * loopf ????????? true??? ?????? Node??? ????????????.
   *
   * @param loopf
   */
  void loop_with_delete(
      std::function<bool(size_t bucket, time_t timestamp, _Tp &tp)> loopf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      LockedHashNode *tmp;
      while (c) {
        if (loopf(i, c->_timestamp, c->_tp)) {
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

  void clear() {
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      LockedHashNode *tmp;
      while (c) {
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
      }
    }
  }

  /**
   * @brief expire_time ?????? ???????????? ?????? ?????? Node??? ????????????.
   * expire_time??? 0??? ??????, ???????????? ??????.
   *
   * @return tl::optional<std::list<_Tp>> ????????? ????????? ????????? list, ?????????
   * tl::nullopt??? ??????.
   */
  tl::optional<std::list<_Tp>> expire(                                        //
      std::function<bool(_Tp &, time_t timestamp, void *)> expiref = nullptr, //
      void *arg = nullptr) {                                                  //
    return expiref ? _expire(expiref, arg) : _expire();
  }

  tl::optional<std::list<_Tp>> _expire() {
    if (_expire_time == 0) {
      return tl::nullopt;
    }
    std::list<_Tp> expired;

    time_t now = time(nullptr);
    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      LockedHashNode *tmp;

      while (c) {
        if (now - c->_timestamp > _expire_time) {
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

          expired.push_back(c->_tp);
          delete c;

          c = tmp;
        } else {
          c = c->next;
        }
      }
    }

    return expired.empty() ? tl::nullopt : tl::make_optional(expired);
  }

  tl::optional<std::list<_Tp>>
  _expire(std::function<bool(_Tp &, time_t timestamp, void *)> expiref,
          void *arg) {
    std::list<_Tp> expired;

    for (size_t i = 0; i < _bucket_size; i++) {
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      LockedHashNode *tmp;

      while (c) {
        if (expiref(c->_tp, c->_timestamp, arg)) {
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

          expired.push_back(c->_tp);
          delete c;

          c = tmp;
        } else {
          c = c->next;
        }
      }
    }

    return expired.empty() ? tl::nullopt : tl::make_optional(expired);
  }

  void showdata(std::function<void(size_t bucket, _Tp &tp)> showdataf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      if (_bucket_elements[i].load() == 0) {
        continue;
      }
      std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(i));
      LockedHashNode *c = _buckets[i];
      while (c) {
        showdataf(i, c->_tp);
        c = c->next;
      }
    }
  }

  void showbucket(std::function<void(size_t bucket, size_t cnt)> showdataf) {
    for (size_t i = 0; i < _bucket_size; i++) {
      showdataf(i, _bucket_elements[i].load());
    }
  }

  /**
   * @brief (life)timestamp update
   *
   * @param key
   * @return true
   * @return false
   */
  tl::optional<_Tp> //
  alive(_Key &key) {
    size_t bucket = _get_bucket_index(key);
    std::lock_guard<std::recursive_mutex> guard(_get_bucket_lock(bucket));

    LockedHashNode *c = _buckets[bucket];
    while (c) {
      _Key k = _makekey(c->_tp);
      if (k == key) {
        c->_timestamp = time(nullptr);
        return tl::make_optional<_Tp>(c->_tp);
      }
      c = c->next;
    }
    return tl::nullopt;
  }

  tl::optional<_Tp> //
  alive(_Key &&key) {
    return alive(key);
  }
};

#endif
