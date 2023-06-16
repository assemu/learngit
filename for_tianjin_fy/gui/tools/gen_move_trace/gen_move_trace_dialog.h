#ifndef GUI_GEN_MOVE_TRACE_DIALOG_H_
#define GUI_GEN_MOVE_TRACE_DIALOG_H_

#include <QDialog>

namespace Ui
{
class GenMoveTraceDialog;
}

class GenMoveTraceDialog : public QDialog
{
public:
  GenMoveTraceDialog();
  ~GenMoveTraceDialog();

private:
  Ui::GenMoveTraceDialog* _ui;
};

#endif //GUI_GEN_MOVE_TRACE_DIALOG_H_
