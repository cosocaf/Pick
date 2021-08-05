/**
 * @file lazy.h
 * @author cosocaf (cosocaf@gmail.com)
 * @brief 不完全型をメンバに持ちたいときのラッパークラス
 * @version 0.1
 * @date 2021-08-04
 * 
 * @copyright Copyright (c) 2021 cosocaf
 * 
 */
#ifndef PICKC_LAZY_H_
#define PICKC_LAZY_H_

#include <utility>
#include <cassert>

namespace pickc {
  /**
   * @brief 不完全型をメンバに持ちたいときのラッパークラス。
   * このクラスはヒープ領域を使用する。
   * 不完全型でない場合は標準ライブラリのstd::optionalを使用するべき。
   * 
   * @tparam T 不完全型
   */
  template<typename T>
  class Lazy {
    T* value;
  public:
    Lazy() : value(nullptr) {}
    Lazy(const T& value) : value(new T(value)) {}
    Lazy(T&& value) : value(new T(std::move(value))) {}
    Lazy(const Lazy& lazy) : value(nullptr) {
      if(lazy.value != nullptr) {
        value = new T(*lazy.value);
      }
    }
    Lazy(Lazy&& lazy) : value(lazy.value) {
      lazy.value = nullptr;
    }
    Lazy& operator=(const T& value) {
      clear();
      this->value = new T(value);
      return *this;
    }
    Lazy& operator=(T&& value) {
      clear();
      this->value = new T(std::move(value));
      return *this;
    }
    Lazy& operator=(const Lazy& lazy) {
      if(this != &lazy) {
        clear();
        this->value = new T(*lazy.value);
      }
      return *this;
    }
    Lazy& operator=(Lazy&& lazy) {
      assert(this != &lazy);
      clear();
      this->value = lazy.value;
      lazy.value = nullptr;
      return *this;
    }
    operator T() const {
      assert(value != nullptr);
      return *value;
    }
    T* operator->() const {
      return value;
    }
  private:
    void clear() {
      delete value;
      value = nullptr;
    }
  };
}

#endif // PICKC_LAZY_H_