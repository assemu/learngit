#include "flaw_analyzer.h"
#include "flaw_util.h"
#include "detectors/detectors_pool.h"
#include "detectors/face_img_split.h"
#include <core/glass_object.h>
#include <QThread>
#include <thread>
#include <atomic>
#include <sstream>
#include <fstream>
#include <core/util/log.h>
#include <core/util/time_util.h>
#include <core/scheduler.h>

class FlawAnalyzeThread : public QThread
{
public:
  FlawAnalyzeThread(FlawAnalyzer* analyzer)
    : QThread(),
      _analyzer(analyzer),
      _need_stop(false),
      _in_running(false)
  {
    //setPriority(QThread::HighPriority);
  }

  bool startWork()
  {
    if(!inRunning()) {
      start();
      return true;
    }
    return false;
  }

  void preStopWork()
  {
    if(inRunning()) {
      _need_stop = true;
    }
  }

  void stopWork()
  {
    if(inRunning()) {
      _need_stop = true;
      for(int i = 0; i < 100; i++)
        _analyzer->_part_img_events.push_back(NULL);
      wait();
      log_info("FlawAnalyzerThread stoped!");
      return;
    }
  }

  bool inRunning() const
  {
    return _in_running;
  }

protected:
  //@see QThread
  virtual void run() override
  {
    _need_stop = false;
    _in_running = true;
    while(!_need_stop) {
      //等待
      process(_analyzer->_part_img_events.wait_and_pop_front());
      log_trace("analyzer part_img_events left:{}", _analyzer->_part_img_events.size());
      if(_need_stop)
        break;

      //及时处理
      std::shared_ptr<GlassPartImageEvent> e;
      while(_analyzer->_part_img_events.try_pop_front(e)) {
        if(!e)
          break;
        process(e);
        log_trace("analyzer part_img_events left:{}", _analyzer->_part_img_events.size());
        if(_need_stop)
          break;
      }
    }

    //运行结束
    _in_running = false;
  }

  void process(std::shared_ptr<GlassPartImageEvent> e)
  {
    if(e) {
      auto part_img = e->partImg();
      auto glass = e->glass();

      if(glass) {
        auto begin_step = glass->config().camera_configs[part_img.position].analyze_begin;
        auto end_step = glass->config().camera_configs[part_img.position].analyze_end;

        //前几个图片不分析
        if(part_img.part_num < begin_step)
          return;

        //后几个图片不分析
        if(part_img.part_num > end_step)
          return;

        if(glass->detectStatus() == GlassObject::DetectFinished) {
          log_warn("detect glass:{} that detection finished", glass->id());
          return;
        }

        std::vector<Flaw> flaws;
        auto pool = DetectorsPool::get();
        if(isEdgePosition(part_img.position)) { //侧面图片
          cv::Mat img;
          auto roi = glass->config().camera_configs[part_img.position].edge_rois;
          roi = roi & cv::Rect(0, 0, part_img.img.cols, part_img.img.rows); //防止崩溃
          cv::Mat roied_img = part_img.img(roi); //根据Roi裁剪出图片
          if(roied_img.empty())
            return;

          cv::cvtColor(roied_img, img, cv::COLOR_GRAY2RGB);

          {
            TimerClock t;
            //2.识别
            std::vector<BoxInfo> boxes;
            if(pool->detectEdgeImg(img, boxes) && boxes.size() > 0) {
              for(auto& b : boxes) {
                Flaw f;
                f.timestamp = get_timestamp();
                f.part_img = part_img;
                f.rect = cv::Rect(roi.x + b.rect.x, roi.y + b.rect.y, b.rect.width, b.rect.height);
                f.score = b.score;
                f.label = b.label;
                f.realWidth = b.rect.width * 0.075f;      //计算实际长度
                f.realHeight = b.rect.height * 0.06f;    //计算实际长度
                f.area = f.realWidth * f.realHeight;    //计算实际面积
                f.detailed_img = genFlawDetailImg(f, true);
                f.detailed_img_nomark = genFlawDetailImg(f, false);
                flaws.push_back(f);
              }
            }

            log_trace("glass:{} analyze edge part img, cam:{} part_num:{} {}x{} flaws:{} cost:{}ms",
                      glass->id(),
                      cameraNameOf(part_img.position),
                      part_img.part_num,
                      part_img.img.cols,
                      part_img.img.rows,
                      flaws.size(),
                      t.getTimerMilliSec());
          }
        } else {  //正面图片
          log_trace("glass:{} analyze face part img,cam:{} part_num:{} {}x{}",
                    glass->id(),
                    cameraNameOf(part_img.position),
                    part_img.part_num,
                    part_img.img.cols,
                    part_img.img.rows);

          std::vector<cv::Mat> ceil_img;    //切割好要检测的图片
          std::vector<double> ceil_num;     //图片对应的滑动窗口标号
          int win_size = part_img.img.rows; //窗口大小

          {
            TimerClock t;
            float binary_threshold = glass->config().camera_configs[part_img.position].face_binary_split_thresholds;
            //Cut_face_img(part_img.img, win_size, &ceil_img, &ceil_num, binary_threshold);
            Cut_face_img(part_img.img, win_size, &ceil_img, &ceil_num, binary_threshold, 1);
            log_trace("glass:{} cut face img , cam:{} part_num:{} {}x{}, cost: {}ms got:{}",
                      glass->id(),
                      cameraNameOf(part_img.position),
                      part_img.part_num,
                      part_img.img.cols,
                      part_img.img.rows,
                      t.getTimerMilliSec(),
                      ceil_img.size());
          }

          for(int i = 0; i < ceil_num.size(); i++) {
            std::vector<BoxInfo> boxes;
            if(ceil_img[i].empty())
              continue;
            cv::cvtColor(ceil_img[i], ceil_img[i], cv::COLOR_GRAY2RGB);
            if(pool->detectFaceImg(ceil_img[i], boxes) && boxes.size() > 0) {
              for(auto& d : boxes) {     //框图转换坐标
                BoxInfo b = d;
                //Fredrick Save 1000 image
                b.rect = cv::Rect(win_size * ceil_num[i] + d.rect.x,
                                  d.rect.y,
                                  d.rect.width,
                                  d.rect.height);
                /*b.rect = cv::Rect(win_size * ceil_num[i],
                	0,
                	1000,
                	1000);*/

                Flaw f;
                f.timestamp = get_timestamp();
                f.part_img = part_img;
                f.rect = b.rect;
                f.score = b.score;
                f.label = b.label;
                //std::cout << "-------------ffffffffff--------->" << f.score << " " << f.label << std::endl;

                f.realWidth = b.rect.width * 0.075f;      //计算实际长度
                f.realHeight = b.rect.height * 0.06f;    //计算实际长度
                f.area = f.realWidth * f.realHeight;     //计算实际面积
                f.detailed_img = genFlawDetailImg(f, true);
                f.detailed_img_nomark = genFlawDetailImg(f, false);
                flaws.push_back(f);
              }
            }
          }
        }

        log_trace("glass:{} filtering flaws for part img,cam:{} part_num:{} {}x{}",
                  glass->id(),
                  cameraNameOf(part_img.position),
                  part_img.part_num,
                  part_img.img.cols,
                  part_img.img.rows);
        _analyzer->filterFlaws(flaws); //过滤

        //标记缺陷对象,缺陷对象会增加ID属性
        //无论是否有缺陷都需要mark一下
        log_trace("glass:{} mark detect for part img,cam:{} part_num:{} {}x{}",
                  glass->id(),
                  cameraNameOf(part_img.position),
                  part_img.part_num,
                  part_img.img.cols,
                  part_img.img.rows);
        glass->markDetect(part_img, flaws);

        log_trace("glass:{} part img,cam:{} part_num:{} {}x{} analyze done!",
                  glass->id(),
                  cameraNameOf(part_img.position),
                  part_img.part_num,
                  part_img.img.cols,
                  part_img.img.rows);
      }
    }
  }

private:
  FlawAnalyzer* _analyzer;
  std::atomic_bool _need_stop;
  std::atomic_bool _in_running;
};

FlawAnalyzer::FlawAnalyzer()
{
  _in_running = false;
  for(int i = 0; i < 10; i++) { //开10个处理线程
    _work_threads << new FlawAnalyzeThread(this);
  }
}

FlawAnalyzer::~FlawAnalyzer()
{
  stopWork();
}

void FlawAnalyzer::glassEvent(std::shared_ptr<GlassEvent> event)
{
  if(!_in_running)
    return;

  //将事件加入队列立即返回，工作线程会被激活进行分析
  if(event->type() == GlassEvent::GlassPartImageEvent) {
    _part_img_events.push_back(std::dynamic_pointer_cast<GlassPartImageEvent>(event));
  }
}

bool FlawAnalyzer::startWork()
{
  DetectorsPool::get(); //加载模型，消耗时间的

  bool re = true;
  for(auto& th : _work_threads)
    re = re && th->startWork();

  if(re)
    _in_running = true;

  return re;
}

void FlawAnalyzer::stopWork()
{
  for(auto& th : _work_threads)
    th->preStopWork();

  for(auto& th : _work_threads) {
    _part_img_events.push_back(NULL);
    th->stopWork();
  }

  _in_running = false;
}

//只是在当前线扫图的范围内过滤缺陷,整体的过滤策略见GlassObject
void FlawAnalyzer::filterFlaws(std::vector<Flaw>& flaws)
{
  if(flaws.size() <= 0)
    return;

  std::vector<Flaw> re;
  auto glass_config = Scheduler::get()->currentBatch().glass_config;
  for(auto& f : flaws) {
    auto label = f.label;
    auto position = f.part_img.position;
    if(glass_config.camera_configs.count(position) > 0) {
      auto camera_config = glass_config.camera_configs[position];

      //按分类过滤,界面没打勾的不要
      if(camera_config.label_filter_outs.count(label) > 0
         && camera_config.label_filter_outs[label] == true) {
        continue;
      }

      //是需要检测的分类，但是置信概率太小,不要
      if(camera_config.confidence_probabilities.count(label) > 0
         && f.score < camera_config.confidence_probabilities[f.label]) {
        continue;
      }

      //是需要检测的分类，置信概率也足够，长度超过阈值,要了
      if(camera_config.length_thresholds.count(label) > 0) {
        if(f.realWidth >= camera_config.length_thresholds[f.label]
           || f.realHeight >= camera_config.length_thresholds[f.label]) {
          re.push_back(f);
          continue; //要了
        }
      }

      //是需要检测的分类，置信概率也足够， 面积超过阈值,要了
      if(camera_config.area_thresholds.count(label) > 0) {
        if(f.area >= camera_config.area_thresholds[f.label]) {
          re.push_back(f);
          continue; //要了
        }
      }
    }
  }

  flaws.clear();
  flaws = re;
}
