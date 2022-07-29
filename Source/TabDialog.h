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
#include <QPlainTextEdit>

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
  void MsgNote(QString &Note, int AppendState = 0);//0无，1中止,2成功,3未解析

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
    //接收相关:
    void LogStateChanged(int);    //允许状态选择改变
    void LogTimeChanged(int);    //允许时间选择改变
    void LogReadChanged(int);    //允许读信息选择改变
    void LogEnRewriteChanged(int);//允许更改选择改变
    void SaveRcv();           //保存接收信息

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

  //接收相关:
  QCheckBox *cbLogState;      //填充工作状态
  QCheckBox *cbLogTime;       //带时间
  QCheckBox *cbLogRead;       //带发送
  QCheckBox *cbLogEnRewrite;  //允许更改选择
  QPushButton *bnClrRcv;      //清除按钮
  QPushButton *bnSaveRcv;     //保存按钮

  QPlainTextEdit *pRcvWin;    //接收窗口

  //通讯过程相关:
  Win_QextSerialPort *serial;
  int FindSerial(int FindStart, int FindEnd); //查找串口函数
  bool InitSerial(bool IsNote);//初始化串口函数
  bool IsCommNote;  //通讯提示

  int LastCom; //最后一次选择正确的COM口
  QTimer *timer;              //定时器
  
  //-------------------------工作状态相关---------------------------------
  unsigned char CommCurAdr;      //当前通讯地址
  unsigned char RcvStrStartPos;  //接收数据中的起始字符串位置

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

  //发送相关:
  QFile *file;
  QTextStream *tStream;

  QString LastSendStr;    //最后一次被发送的字符串
  int SendLineCount;       //发送总数
  int UnAskCount;         //多行时，无应答次数
  int CrcErrCount;        //多行时，校验错误次数
  int DataShortCount;     //多行时，数据过短次数
  int ValidCount;         //有效数据次数

  int FullSendBufHeader();//填充发送缓冲区数据头
  int SendString(QString &Str);//发送字符串
  void FullSendDataBuf(unsigned short Len);//填充缓冲区数据
  void StopBatSend(bool IsForceStop);//中止批处理文件的发送

  void RcvEndPro(int);     //接收结束处理
  void StartCommPro(enum eWorkState_t eNextState);//启动通讯处理

};

#endif
