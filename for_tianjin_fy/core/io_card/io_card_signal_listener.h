#ifndef V2_IOCARD_SIGNAL_LISTENER_H_
#define V2_IOCARD_SIGNAL_LISTENER_H_

class IOCardSignalListener
{
public:
  //来了一个glass信号
  virtual void glassComeSignal() = 0;

  //心跳信号
  virtual void heartBeatSignal() = 0;
};

#endif //V2_IOCARD_SIGNAL_LISTENER_H_ 
