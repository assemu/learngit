#ifndef CORE_THREAD_SAFE_QUEUE_H_
#define CORE_THREAD_SAFE_QUEUE_H_

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <initializer_list>


//线程安全的队列
template<typename T>
class ThreadSafeQueue
{
private:
  // data_queue访问信号量
  mutable std::mutex mutex;
  mutable std::condition_variable cond;
  using QueueType = std::queue<T>;
  QueueType data_queue;

public:
  using ValueType = typename QueueType::value_type;
  using ContainerType = typename QueueType::container_type;

  ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

  /*
   * 使用迭代器为参数的构造函数,适用所有容器对象
   * */
  template<typename _InputIterator>
  ThreadSafeQueue(_InputIterator first, _InputIterator last)
  {
    for(auto itor = first; itor != last; ++itor) {
      data_queue.push(*itor);
    }
  }

  explicit ThreadSafeQueue(const ContainerType& c)
    : data_queue(c)
  {}

  /*
   * 使用初始化列表为参数的构造函数
   * */
  ThreadSafeQueue(std::initializer_list<ValueType> list)
    : ThreadSafeQueue(list.begin(), list.end())
  {}

  /*
   * 将元素加入队列
   * */
  void push_back(const ValueType& new_value)
  {
    std::lock_guard<std::mutex> lock(mutex);
    data_queue.push(std::move(new_value));
    cond.notify_one();
  }

  /*
   * 从队列中弹出一个元素,如果队列为空就阻塞
   * */
  ValueType wait_and_pop_front()
  {
    std::unique_lock<std::mutex> lock(mutex);
    cond.wait(lock, [this] {return !this->data_queue.empty();});
    auto value = std::move(data_queue.front());
    data_queue.pop();
    return value;
  }

  /*
   * 从队列中弹出一个元素,如果队列为空返回false
   * */
  bool try_pop_front(ValueType& value)
  {
    std::lock_guard<std::mutex> lock(mutex);
    if(data_queue.empty())
      return false;
    value = std::move(data_queue.front());
    data_queue.pop();
    return true;
  }

  /*
   * 返回队列是否为空
   * */
  auto empty() const->decltype(data_queue.empty())
  {
    std::lock_guard<std::mutex>lock(mutex);
    return data_queue.empty();
  }

  /*
   * 返回队列中元素数个
   * */
  auto size() const->decltype(data_queue.size())
  {
    std::lock_guard<std::mutex>lock(mutex);
    return data_queue.size();
  }

  //清空
  void clear()
  {
    ValueType v;
    while(try_pop(v))
      ;
  }
};

#endif //CORE_THREAD_SAFE_QUEUE_H_
