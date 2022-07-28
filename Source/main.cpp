/****************************************************************************

                               ������

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
  QTextCodec::setCodecForTr(QTextCodec::codecForLocale());//tr������֧��
  QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());//�����ļ�ϵͳ��д����֧��;
  QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());//�ַ�����������֧��;

  TabDialog *tabDialog = new TabDialog();
  return tabDialog->exec();
}
