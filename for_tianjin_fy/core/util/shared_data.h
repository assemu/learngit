#ifndef UTIL_SHARED_DATA_H_
#define UTIL_SHARED_DATA_H_

/**
 * 实现隐式共享机制
 */

#include <atomic>
#include <algorithm>
#include <iostream>

class SharedData
{
public:
  mutable std::atomic_int ref;

  inline SharedData() noexcept : ref(0) { }
  inline SharedData(const SharedData&) noexcept : ref(0) { }

  // using the assignment operator would lead to corruption in the ref-counting
  SharedData& operator=(const SharedData&) = delete;
  ~SharedData() = default;
};

template <class T>
class SharedDataPointer
{
public:
  typedef T Type;
  typedef T* pointer;

  //分离
  inline void detach()
  {
    if(d && d->ref.load() != 1)
      detach_helper();
  }

  //先分离
  inline T& operator*()
  {
    detach();
    return *d;
  }

  //不用分离
  inline const T& operator*() const
  {
    return *d;
  }

  //先分离
  inline T* operator->()
  {
    detach();
    return d;
  }

  //不用分离
  inline const T* operator->() const
  {
    return d;
  }

  //先分离
  inline operator T* ()
  {
    detach();
    return d;
  }

  //不用分离
  inline operator const T* () const
  {
    return d;
  }

  //先分离
  inline T* data()
  {
    detach();
    return d;
  }

  //不用分离
  inline const T* data() const
  {
    return d;
  }

  //不用分离
  inline const T* constData() const
  {
    return d;
  }

  inline bool operator==(const SharedDataPointer<T>& other) const
  {
    return d == other.d;
  }

  inline bool operator!=(const SharedDataPointer<T>& other) const
  {
    return d != other.d;
  }

  //构造函数
  inline SharedDataPointer()
  {
    d = nullptr;
  }

  //析构函数
  inline ~SharedDataPointer()
  {
    if(d && (d->ref--) <= 0)
      delete d;
  }

  explicit SharedDataPointer(T* data) noexcept;

  inline SharedDataPointer(const SharedDataPointer<T>& o)
    : d(o.d)
  {
    if(d)
      d->ref++;
  }

  inline SharedDataPointer<T>& operator=(const SharedDataPointer<T>& o)
  {
    if(o.d != d) {
      if(o.d)
        o.d->ref++;
      T* old = d;
      d = o.d;
      if(old && (old->ref--) <= 0)
        delete old;
    }
    return *this;
  }

  inline SharedDataPointer& operator=(T* o)
  {
    if(o != d) {
      if(o)
        o->ref++;
      T* old = d;
      d = o;
      if(old && (old->ref--) <= 0)
        delete old;
    }
    return *this;
  }

  //右值引用，偷取
  SharedDataPointer(SharedDataPointer&& o) noexcept : d(o.d)
  {
    o.d = nullptr;
  }

  //右值引用
  inline SharedDataPointer<T>& operator=(SharedDataPointer<T>&& other) noexcept
  {
    SharedDataPointer moved(std::move(other));
    swap(moved);
    return *this;
  }

  //指针非空判断
  inline bool operator!() const
  {
    return !d;
  }

  inline void swap(SharedDataPointer& other) noexcept
  {
    std::swap(d, other.d);
  }

protected:
  T* clone();

private:
  void detach_helper();

  T* d;
};

template <class T> inline bool operator==(std::nullptr_t p1, const SharedDataPointer<T>& p2)
{
  return !p2;
}

template <class T> inline bool operator==(const SharedDataPointer<T>& p1, std::nullptr_t p2)
{
  return !p1;
}

template <class T>
SharedDataPointer<T>::SharedDataPointer(T* adata) noexcept
  : d(adata)
{
  if(d)
    d->ref++;
}

template <class T>
T* SharedDataPointer<T>::clone()
{
  return new T(*d);
}

template <class T>
void SharedDataPointer<T>::detach_helper()
{
  T* x = clone();
  x->ref++;

  if(d && (d->ref--) <= 0)
    delete d;
  d = x;
}

#endif //UTIL_SHARED_DATA_H_
