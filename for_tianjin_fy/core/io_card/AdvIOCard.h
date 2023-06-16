#ifndef __ADVIOCARD_H
#define __ADVIOCARD_H

#include <bdaqctrl.h>
#include <core/io_card/io_card.h>

using namespace Automation::BDaq;

class AdvIOCard : public IOCard
{
public:
  AdvIOCard();
  ~AdvIOCard();

  bool readByte(WORD wPortNo, DWORD& dwOutValue);
  bool readBit(WORD wPortNo, WORD wBitNo, WORD& wOutValue);
  bool writeByte(DWORD wOutputPort, DWORD dwDOVal);
  bool writeBit(DWORD wOutputPort, WORD wBitNo, WORD wDOVal);

private:
  //Di
  InstantDiCtrl* instantDiCtrl;
  //Do
  InstantDoCtrl* instantDoCtrl;
};

#endif
