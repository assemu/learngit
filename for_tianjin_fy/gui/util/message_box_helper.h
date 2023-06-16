#ifndef GUI_COMMON_MESSAGEBOX_HELPER_H_
#define GUI_COMMON_MESSAGEBOX_HELPER_H_

#include <QString>
#include <QPushButton>
#include <QMessageBox>

class MessageBoxHelper
{
public:
  static void show_critical(const QString& message)
  {
    QMessageBox box(QMessageBox::Critical, QObject::tr("Critical"), message);
    box.setButtonText(QMessageBox::Ok, QObject::tr("ok"));
    box.setButtonText(QMessageBox::Cancel, QObject::tr("cancel"));
    box.exec();
  }

  static void show_info(const QString& message)
  {
    QMessageBox box(QMessageBox::Information, QObject::tr("Info"), message);
    box.setButtonText(QMessageBox::Ok, QObject::tr("ok"));
    box.setButtonText(QMessageBox::Cancel, QObject::tr("cancel"));
    box.exec();
  }

  static bool show_confirm(const QString& message)
  {
    QMessageBox box(QMessageBox::Question, QObject::tr("Confirm"), message);
    QPushButton* btnYes = box.addButton(QObject::tr("ok"), QMessageBox::YesRole);
    QPushButton* btnNo =  box.addButton(QObject::tr("cancel"), QMessageBox::NoRole);
    box.exec();
    if((QPushButton*)box.clickedButton() == btnYes)
      return true;
    return false;
  }
};

#endif //GUI_COMMON_MESSAGEBOX_HELPER_H_
