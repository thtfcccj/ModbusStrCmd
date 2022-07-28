/****************************************************************************

                               主程序

****************************************************************************/

#include <QApplication>

#include <QTextCodec>
#include <QtSql>
#include <QMessageBox>

#include "TabDialog.h"

int main(int argc, char *argv[])
{
  Q_INIT_RESOURCE(Images);
  
  QApplication app(argc, argv);
  QTextCodec::setCodecForTr(QTextCodec::codecForLocale());//tr的中文支持
  QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());//本地文件系统读写中文支持;
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());//字符常量的中文支持;

  TabDialog *tabDialog = new TabDialog();
  return tabDialog->exec();
}
