/****************************************************************************


****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

#include <QDialog>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QStringList>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>

// 引入配置文件的头文件
#include <QSettings>
#include "Win_QextSerialPort.h"

#define RCV_BUF_SIZE    255 //取大值用于防止不能完全读完
#define SEND_BUF_SIZE   255 //取255用于扩展指令

class TabDialog : public QDialog
{
    Q_OBJECT

public:
    TabDialog(QWidget *parent = 0);

  QSettings *settings; // 配置文件对象

  //LRC8校验
  static unsigned char GetLRC8(unsigned char *pBuf, unsigned short Len);

  //错误提示对话框处理
  static void MsgNote(QString &Note, int AppendState = 1); //1中止,2成功,其它无

private slots:
    //串口：数据及UI:
    void SerialRcv();         //串口写返回处理
    void SerialOV();          //写数据超时处理
    void ComSelChanged();     //串口被重新选择
    void ComRefresh();        //刷新存在的COM口

    void AdrChanged(const QString & text);   //地址修改完成
    void FunIdChanged();     //功能ID被更改

    //当前编辑相关:
    void EditChanged(const QString & text);  //编辑完成
    void SendEditStr();         //发送按钮      
    //批处理相关:
    void LoadBatFile();       //装载文件按钮
    void EditBatFile();       //调用外部编辑器按钮
    void SendBatStr();        //发送批处理文件按钮 

private:
  //串口选择:
  QPushButton *bnComRefresh;    //刷新
  QComboBox *cbComId;           //选择的Com
  QLineEdit *leAdr;            //通讯地址
  QComboBox *cbFunId;           //选择的功能前缀

  //当前编辑相关:
  QLineEdit *leEdit;          //正在编辑的字符
  QPushButton *bnSendEditStr; //发送编辑器中的字符

  //批处理相关:
  QPushButton *bnLoadBatFile; //打开批处理文件
  QLineEdit *leBatFile;       //存放Bat文件路径
  QPushButton *bnEditBatFile; //编辑批处理文件
  QPushButton *bnSendBatStr;  //发送

  //通讯过程相关:
  Win_QextSerialPort *serial;
  int FindSerial(int FindStart, int FindEnd); //查找串口函数
  bool InitSerial(bool IsNote);//初始化串口函数
  bool IsCommNote;  //通讯提示

  int LastCom; //最后一次选择正确的COM口
  QTimer *timer;              //定时器
  
  //-------------------------工作状态相关---------------------------------
  unsigned char CommCurAdr;    //当前通讯地址
  unsigned char SendDataBuf[SEND_BUF_SIZE]; //发送数据缓冲区
  unsigned char SendCount;       //本次发送数据个数
  unsigned char RcvDataBuf[RCV_BUF_SIZE]; //接收数据缓冲区
  unsigned char RcvCount;       //本次接收数据个数


  volatile int RdDataDelay;   //读数据时延时

  //------------------------通讯状态相关---------------------------------
  int ComCount; //Com口总数
  volatile enum eCommState_t{eCommIdie, eRdData, eRdDataWait} eCommState;
  int RetryCount; //当前通讯重试次数
  //int CommCount;  //当前通讯总次数
  bool waitStop;                 //停止通讯 
  void SendData(unsigned char Len) ;//发送缓冲区数据

  //错误时对话框处理
  void ErrCommNote(QMessageBox *msgBox,//已new的对话框
                   int CommState);//正：超时无数据，负数据有误

  void RcvEndPro(int);     //接收结束处理
  void StartCommPro(enum eWorkState_t eNextState);//启动通讯处理

};

#endif
