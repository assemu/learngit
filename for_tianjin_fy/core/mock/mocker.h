#ifndef V2_MOCKER_H_
#define V2_MOCKER_H_

#include <windows.h>
#include <atomic>
#include <memory>
#include <QList>
#include <QMutex>
#include <QThread>
#include <QStringList>
#include <QSemaphore>
#include <map>
#include <vector>
#include <list>
#include <core/part_img_source.h>
#include <core/io_card/io_card.h>

typedef std::map<CameraPosition, std::vector<cv::Mat>>  GlassData;

//模拟线扫图像采集器
class Mocker : public PartImgSource
{
public:
  static Mocker* get();

  //提供一个模拟的IOCard接口实例
  std::shared_ptr<IOCard> getIOCard();

  //开始
  virtual bool startWork(GlassConfig glass_config) override;

  //停止
  virtual bool stopWork() override;

  //注册图像处理
  virtual void registerPartImgConsumer(PartImgConsumer* consumer) override;

  //注册图像处理对象
  virtual void unregisterPartImgConsumer(PartImgConsumer* consumer) override;

protected:
  Mocker();
  std::shared_ptr<IOCard> _io_card;

  QList<GlassData> _glasses;
  QMutex _glass_mutex;
  int _glass_index;
  const GlassData& nextGlassData();
  void sendFaceImgs(const GlassData& gd);
  void sendEdgeImgs(const GlassData& gd);

  QList<PartImgConsumer*> _part_img_consumers;
  QMutex _consumers_mutex;

  void signalIO();
  QMutex _signal_mutex;
  std::atomic_int _bitoutput;

  friend class MockIOCard;
  friend class MockGlassThread;
  QSemaphore _semaphore; //信号量
  MockGlassThread* _glass_thread1;
};

#endif //V2_MOCKER_H_
