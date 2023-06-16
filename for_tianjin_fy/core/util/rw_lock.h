#ifndef BASE_RW_LOCK_H_
#define BASE_RW_LOCK_H_

#include <mutex>

class rwLock
{
public:
  void lockForRead()
  {
    std::unique_lock<std::mutex> lock(m);
    while(writerUsed == true) {
      cv.wait(lock);
    }
    readerCount++;
  }

  void lockForWrite()
  {
    std::unique_lock<std::mutex> lock(m);
    while(readerCount != 0 || writerUsed == true) {
      cv.wait(lock);
    }
    writerUsed = true;
  }

  void unlockRead()
  {
    std::unique_lock<std::mutex> lock(m);
    readerCount--;
    if(readerCount == 0) {
      cv.notify_all();
    }
  }

  void unlockWrite()
  {
    std::unique_lock<std::mutex> lock(m);
    writerUsed = false;
    cv.notify_all();
  }

private:
  int readerCount = 0;
  bool writerUsed = false;
  std::mutex m;
  std::condition_variable cv;
};

#endif //BASE_RW_LOCK_H_
