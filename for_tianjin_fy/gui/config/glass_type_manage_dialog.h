#ifndef GUI_GLASS_TYPE_MANAGE_DIALOG_H_
#define GUI_GLASS_TYPE_MANAGE_DIALOG_H_

#include <QDialog>

namespace Ui
{
class GlassTypeManageDialog;
}

class GlassTypeManageDialog : public QDialog
{
  Q_OBJECT;
public:
  GlassTypeManageDialog();

  void reload();

private:
  std::string getSelectedConfig();
  Ui::GlassTypeManageDialog* _ui;
};

#endif //GUI_GLASS_TYPE_MANAGE_DIALOG_H_
