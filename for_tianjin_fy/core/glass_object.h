#ifndef V2_GLASS_OBJECT_H_
#define V2_GLASS_OBJECT_H_

#include <memory>
#include <string>
#include <QReadWriteLock>
#include <QMutex>
#include <QList>
#include <core/common.h>
#include <core/debug_info.h>

class GlassImageCombiner;

//玻璃对象
class GlassObject : public std::enable_shared_from_this<GlassObject>
{
public:
  //玻璃的扫描状态
  enum PositionStatus {
    OnTheWay = 0,           //未到来
    InFront = 1,            //IO信号刚触发
    ScanEdgeStarted = 2,    //开始扫描侧面
    ScanEdgeFinished = 3,   //侧面扫描完成
    ScanFaceStarted = 4,    //开始扫描正面
    ScanFaceFinished = 5   //正面扫描完成
  };

  //识别状态
  enum DetectStatus {
    DetectNotStarted = 0, //识别未开始
    Detecting = 1,        //正在识别
    DetectFinished = 2    //识别已经结束
  };

  //位置状态
  PositionStatus positionStatus() const;

  //识别状态
  DetectStatus detectStatus() const;

  //调试信息
  DebugInfo debug_info;

public:
  GlassObject(const std::string& id, const GlassConfig& config);
  ~GlassObject();

  //玻璃ID
  std::string id() const;

  //玻璃型号
  GlassConfig config() const;

  //玻璃触发IO的时间
  int64_t timestamp() const
  {
    return _timestamp;
  }

public:
  //该玻璃触发IO信号
  void signalIO();

  //PartImg,添加线扫图像,添加成功会修改img的part_num
  bool putEdgePartImg(PartImg& img);
  bool putFacePartImg(PartImg& img);

  //获取工位上的线扫长条图数组,对象轻量直接复制
  //分析出结果后，这些缓存数据会被清除
  std::vector<PartImg> getPartImgs(CameraPosition position);
  int getPartImgsCount(CameraPosition position);

  //获取拼合图,RGB格式
  //在某些情况下可能返回空图
  cv::Mat getFrontImg(int& left_margin, int& right_margin);
  cv::Mat getBackImg(int& left_margin, int& right_margin);
  cv::Mat getTopImg(int& left_margin, int& right_margin);

  //检测完后可调用
  bool isDetectDone();
  bool isOk();

  //所有缺陷,识别过程中，顺序会被改变
  QList<Flaw> allFlaws();

  //标记对某个图片进行了检测
  void markDetect(const PartImg& img, std::vector<Flaw>& flaws);

  //强制分析完成
  void forceDetectDone();

private:
  std::string _id;
  GlassConfig _config;
  int64_t _timestamp;

  bool _isEdgeImgsEnough();
  bool _isFaceImgsEnough();
  std::map<CameraPosition, std::vector<PartImg>> _imgs;
  std::map<CameraPosition, int> _imgs_count;
  QReadWriteLock _imgs_lock;

  std::shared_ptr<GlassImageCombiner> _front_image_combiner;
  std::shared_ptr<GlassImageCombiner> _back_image_combiner;
  std::shared_ptr<GlassImageCombiner> _top_image_combiner;

  QList<Flaw> _flaws;
  QMutex _flaws_mutex;
  std::map<CameraPosition, int> _position_detect_complete;

  void _changePositionStatusTo(PositionStatus position_status);
  QMutex _position_status_mutex;
  PositionStatus _position_status;

  void _changeDetectStatusTo(DetectStatus detect_status);
  QMutex _detect_status_mutex;
  DetectStatus _detect_status;
};

#endif //V2_GLASS_OBJECT_H_
