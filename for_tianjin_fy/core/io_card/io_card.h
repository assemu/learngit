#ifndef __IOCARD_H
#define __IOCARD_H

#include <windows.h>

class IOCard
{
public:
  IOCard()
    : wBoardNo(0)
  {
  }

  virtual ~IOCard() {}

  //读取字节 wPortNo 端口号、dwOutValue 输出值
  virtual bool readByte(WORD wPortNo, DWORD& dwOutValue) = 0;

  //读取位 wPortNo 端口号、wBitNo 位、wOutValue 输出值
  virtual bool readBit(WORD wPortNo, WORD wBitNo, WORD& wOutValue) = 0;

  //写入字节 wOutputPort 写入端口号、dwDOVal 写入值
  virtual bool writeByte(DWORD wOutputPort, DWORD dwDOVal) = 0;

  //写入位 wOutputPort 写入端口号、wBitNo 位、wDOVal 写入值
  virtual bool writeBit(DWORD wOutputPort, WORD wBitNo, WORD wDOVal) = 0;

protected:
  WORD wTotalBoards;//板卡数量
  WORD wRtn;//类似标志作用，判断函数执行后的状态
  WORD wBoardNo;//使用的板卡编号
  WORD wCardType;//卡类型
};

#endif
