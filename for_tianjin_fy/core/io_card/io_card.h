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

  //��ȡ�ֽ� wPortNo �˿ںš�dwOutValue ���ֵ
  virtual bool readByte(WORD wPortNo, DWORD& dwOutValue) = 0;

  //��ȡλ wPortNo �˿ںš�wBitNo λ��wOutValue ���ֵ
  virtual bool readBit(WORD wPortNo, WORD wBitNo, WORD& wOutValue) = 0;

  //д���ֽ� wOutputPort д��˿ںš�dwDOVal д��ֵ
  virtual bool writeByte(DWORD wOutputPort, DWORD dwDOVal) = 0;

  //д��λ wOutputPort д��˿ںš�wBitNo λ��wDOVal д��ֵ
  virtual bool writeBit(DWORD wOutputPort, WORD wBitNo, WORD wDOVal) = 0;

protected:
  WORD wTotalBoards;//�忨����
  WORD wRtn;//���Ʊ�־���ã��жϺ���ִ�к��״̬
  WORD wBoardNo;//ʹ�õİ忨���
  WORD wCardType;//������
};

#endif
