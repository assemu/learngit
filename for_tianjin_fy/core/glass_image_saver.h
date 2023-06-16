#ifndef JIANFENG_CORE_GLASS_IMAGE_SAVER_H_
#define JIANFENG_CORE_GLASS_IMAGE_SAVER_H_

#include <atomic>
#include <string>
#include <vector>
#include <QThread>
#include <memory>
#include <core/common.h>
#include <core/util/thread_safe_queue.h>
#include <core/glass_event.h>
#include <map>
#include <opencv2/opencv.hpp>

class GlassImageSaver : public QThread, public GlassEventListener
{
public:
  GlassImageSaver();

  //开始运行
  bool startWork();

  //停止运行
  void stopWork();

  virtual void glassEvent(std::shared_ptr<GlassEvent> event) override;

protected:
  void run() override;

private:
  struct SaveItem {
    std::string path;
    cv::String format;
    cv::Mat img;
  };
  void _save(const SaveItem& it);

  std::string _glassDir(std::shared_ptr<GlassEvent> event);

  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;

  ThreadSafeQueue<std::shared_ptr<GlassEvent>> _events;
  void process(std::shared_ptr<GlassEvent> e);
};

#endif //JIANFENG_CORE_GLASS_IMAGE_SAVER_H_
