#include "mocker.h"
#include <iostream>
#include <core/part_img_consumer.h>
#include <core/util/time_util.h>
#include <core/util/log.h>
#include <core/util/image_util.h>
#include <core/common.h>
#include <core/app_config.h>
#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>
#include <QMutexLocker>

class MockIOCard : public IOCard
{
public:
  MockIOCard(Mocker* mocker)
    : _mocker(mocker)
  {
  }

  bool readByte(WORD wPortNo, DWORD& dwOutValue)
  {
    return true;
  }

  bool readBit(WORD wPortNo, WORD wBitNo, WORD& wOutValue)
  {
    if(wPortNo == 0 && wBitNo == 0) {
      wOutValue = _mocker->_bitoutput;
      return true;
    }
    return false;
  }

  bool writeByte(DWORD wOutputPort, DWORD dwDOVal)
  {
    return true;
  }

  bool writeBit(DWORD wOutputPort, WORD wBitNo, WORD wDOVal)
  {
    return true;
  }

private:
  Mocker* _mocker;
};

class MockGlassThread : public QThread
{
public:
  MockGlassThread(Mocker* mocker)
    : _mocker(mocker),
      _need_stop(false),
      _in_running(false)
  {}

  bool startWork()
  {
    if(!_in_running) {
      start();
      return true;
    }
    return false;
  }

  bool stopWork()
  {
    if(_in_running) {
      log_info("try to stop MockerThread ... ");
      _need_stop = true;
      _mocker->_semaphore.release();
      wait();
      log_info("MockerThread stopped!");
      return true;
    }
    return false;
  }

protected:
  void run() override
  {
    _need_stop = false;
    _in_running = true;

    while(!_need_stop) {
      const GlassData& glass_data = _mocker->nextGlassData();

      //等信号量
      _mocker->_semaphore.acquire();

      if(_need_stop)
        break;

      //发送IO信号
      _mocker->signalIO();
      thread_spin_ms(50); //50ms之后开始发送正面图片

      _mocker->sendEdgeImgs(glass_data);
      thread_spin_ms(30);
      _mocker->sendFaceImgs(glass_data);

      thread_spin_ms(1000);
      _mocker->_semaphore.release();
    }

    _in_running = false;
  }

  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;

private:
  Mocker* _mocker;
};

Mocker* Mocker::get()
{
  static Mocker* _inst = NULL;
  if(_inst == NULL)
    _inst = new Mocker();
  return _inst;
}

Mocker::Mocker()
  : _glass_index(0)
{
  _bitoutput = 0;
  _io_card = std::make_shared<MockIOCard>(this);

  _glass_thread1 = new MockGlassThread(this);
}

bool Mocker::startWork(GlassConfig glass_config)
{
  _glasses.clear();
  auto data_dir = AppConfig::get()->getMockDataDir() + "/" + glass_config.name;

  //遍历文件夹
  log_debug("mocker load images from {}", data_dir);

  std::list<std::string> glass_dirs;
  {
    boost::filesystem::path data_path(data_dir);
    boost::filesystem::directory_iterator end;
    for(boost::filesystem::directory_iterator it(data_path); it != end; it++) {
      if(boost::filesystem::is_directory(*it)) {
        auto glass_dir = QString::fromStdString(it->path().string()).replace("\\", "/").toStdString();
        glass_dirs.push_back(glass_dir);
        log_debug("found glass : {}", glass_dir);
      }
    }
  }

  for(const auto& glass_dir : glass_dirs) {
    auto glass_name = QString::fromStdString(glass_dir).mid(data_dir.size() + 1);
    log_debug("scan image paths for glass:{}", glass_name.toStdString());
    //列文件
    std::map<CameraPosition, QStringList> glass_image_paths;
    boost::filesystem::path glass_data_path(glass_dir);
    boost::filesystem::directory_iterator end;
    for(boost::filesystem::directory_iterator it(glass_data_path); it != end; it++) {
      if(!boost::filesystem::is_directory(*it)) {
        auto image_path = QString::fromStdString(it->path().string()).replace("\\", "/");
        auto file_name = image_path.mid(glass_dir.size() + 1);
        for(auto& position : allCameraPositions()) {
          if(file_name.startsWith(QString::fromStdString(cameraNameOf(position)))) {
            glass_image_paths[position] << image_path;
            break;
          }
        }
      }
    }

    //排序
    for(auto& kv : glass_image_paths) {
      auto image_paths = kv.second;
      std::sort(image_paths.begin(), image_paths.end(),
      [data_dir](const QString & a, const QString & b) {
        auto a_name = a.mid(data_dir.size() + 1);
        auto b_name = b.mid(data_dir.size() + 1);
        auto as = a_name.split("_");
        auto bs = b_name.split("_");
        return as[1].split(".")[0].toInt() < bs[1].split(".")[0].toInt();
      });
      glass_image_paths[kv.first] = image_paths;
    }

    //加载文件
    std::map<CameraPosition, std::vector<cv::Mat>> glass_images;
    for(auto& kv : glass_image_paths) {
      auto image_paths = kv.second;
      for(int i = 0; i < image_paths.size(); i++) {
        auto image_path = image_paths[i];
        auto file_name = image_path.mid(glass_dir.size() + 1);
        auto img = readImgGRAY(image_path.toStdString());
        glass_images[kv.first].push_back(img);
        log_debug("glass:{} position:{} load [{}] = {}",
                  glass_name.toStdString(),
                  cameraNameOf(kv.first),
                  i,
                  file_name.toStdString());
      }
    }

    _glasses << glass_images;
  }

  bool re = true;
  re = re && _glass_thread1->startWork();
  _semaphore.release();
  return re;
}

bool Mocker::stopWork()
{
  bool re = true;
  _semaphore.release();
  re = re && _glass_thread1->stopWork();
  log_info("Mocker Stopped");
  return re;
}

void Mocker::registerPartImgConsumer(PartImgConsumer* consumer)
{
  QMutexLocker locker(&_consumers_mutex);
  if(consumer)
    _part_img_consumers.append(consumer);
}

void Mocker::unregisterPartImgConsumer(PartImgConsumer* consumer)
{
  QMutexLocker locker(&_consumers_mutex);
  _part_img_consumers.removeAll(consumer);
}

std::shared_ptr<IOCard> Mocker::getIOCard()
{
  return _io_card;
}

const GlassData& Mocker::nextGlassData()
{
  QMutexLocker locker(&_glass_mutex);
  const auto& re = _glasses.at(_glass_index);
  _glass_index++;
  _glass_index = _glass_index < _glasses.size() ? _glass_index : 0;
  return re;
}

void Mocker::sendFaceImgs(const GlassData& glass_data)
{
  int image_index = 0;
  while(true) {

    //每个工位发一张
    for(const auto& kv : glass_data) {
      auto position = kv.first;
      if(isFacePosition(position)) {
        const auto& images = glass_data.at(position);
        if(image_index < images.size()) {
          PartImg img;
          img.timestamp = get_timestamp(); //时间戳
          img.img = images[image_index];
          img.position = position;
          log_trace("mocker send face image [{}] ,position:{}", image_index, cameraNameOf(position));
          for(auto& cs : _part_img_consumers) {
            cs->acceptFacePartImg(img);
          }
        }
      }
    }

    image_index++;

    bool finished = true;
    for(const auto& kv : glass_data) {
      auto position = kv.first;
      const auto& images = glass_data.at(position);
      if(image_index < images.size()) {
        finished = false;
        break;
      }
    }

    if(finished)
      break;

    //Nms 以后再发下一张
    thread_spin_ms(30);
  }
}

void Mocker::sendEdgeImgs(const GlassData& glass_data)
{
  int image_index = 0;
  while(true) {

    //每个工位发一张
    for(const auto& kv : glass_data) {
      auto position = kv.first;
      if(isEdgePosition(position)) {
        const auto& images = glass_data.at(position);
        if(image_index < images.size()) {
          PartImg img;
          img.timestamp = get_timestamp(); //时间戳
          img.img = images[image_index];
          img.position = position;
          log_trace("mocker send edge image [{}] ,position:{}", image_index, cameraNameOf(position));
          for(auto& cs : _part_img_consumers) {
            cs->acceptEdgePartImg(img);
          }
        }
      }
    }

    image_index++;

    bool finished = true;
    for(const auto& kv : glass_data) {
      auto position = kv.first;
      const auto& images = glass_data.at(position);
      if(image_index < images.size()) {
        finished = false;
        break;
      }
    }

    if(finished)
      break;

    //Nms 以后再发下一张
    thread_spin_ms(30);
  }
}

void Mocker::signalIO()
{
  QMutexLocker locker(&_signal_mutex);
  //产生IO信号，保持20ms
  _bitoutput = 1;
  thread_spin_ms(20); //20ms
  _bitoutput = 0;
}
