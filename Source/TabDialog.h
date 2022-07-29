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

// ���������ļ���ͷ�ļ�
#include <QSettings>
#include "Win_QextSerialPort.h"

#define RCV_BUF_SIZE    255 //ȡ��ֵ���ڷ�ֹ������ȫ����
#define SEND_BUF_SIZE   255 //ȡ255������չָ��

class TabDialog : public QDialog
{
    Q_OBJECT

public:
    TabDialog(QWidget *parent = 0);

  QSettings *settings; // �����ļ�����

  //LRC8У��
  static unsigned char GetLRC8(unsigned char *pBuf, unsigned short Len);

  //������ʾ�Ի�����
  void MsgNote(QString &Note, int AppendState = 0);//0�ޣ�1��ֹ,2�ɹ�,3δ����

private slots:
    //���ڣ����ݼ�UI:
    void SerialRcv();         //����д���ش���
    void SerialOV();          //д���ݳ�ʱ����
    void ComSelChanged();     //���ڱ�����ѡ��
    void ComRefresh();        //ˢ�´��ڵ�COM��

    void AdrChanged(const QString & text);   //��ַ�޸����
    void FunIdChanged();     //����ID������

    //��ǰ�༭���:
    void EditChanged(const QString & text);  //�༭���
    void SendEditStr();         //���Ͱ�ť      
    //���������:
    void LoadBatFile();       //װ���ļ���ť
    void EditBatFile();       //�����ⲿ�༭����ť
    void SendBatStr();        //�����������ļ���ť 
    //�������:
    void LogStateChanged(int);    //����״̬ѡ��ı�
    void LogTimeChanged(int);    //����ʱ��ѡ��ı�
    void LogReadChanged(int);    //�������Ϣѡ��ı�
    void LogEnRewriteChanged(int);//�������ѡ��ı�
    void SaveRcv();           //���������Ϣ

private:
  //����ѡ��:
  QPushButton *bnComRefresh;    //ˢ��
  QComboBox *cbComId;           //ѡ���Com
  QLineEdit *leAdr;            //ͨѶ��ַ
  QComboBox *cbFunId;           //ѡ��Ĺ���ǰ׺

  //��ǰ�༭���:
  QLineEdit *leEdit;          //���ڱ༭���ַ�
  QPushButton *bnSendEditStr; //���ͱ༭���е��ַ�

  //���������:
  QPushButton *bnLoadBatFile; //���������ļ�
  QLineEdit *leBatFile;       //���Bat�ļ�·��
  QPushButton *bnEditBatFile; //�༭�������ļ�
  QPushButton *bnSendBatStr;  //����

  //�������:
  QCheckBox *cbLogState;      //��乤��״̬
  QCheckBox *cbLogTime;       //��ʱ��
  QCheckBox *cbLogRead;       //������
  QCheckBox *cbLogEnRewrite;  //�������ѡ��
  QPushButton *bnClrRcv;      //�����ť
  QPushButton *bnSaveRcv;     //���水ť

  QPlainTextEdit *pRcvWin;    //���մ���

  //ͨѶ�������:
  Win_QextSerialPort *serial;
  int FindSerial(int FindStart, int FindEnd); //���Ҵ��ں���
  bool InitSerial(bool IsNote);//��ʼ�����ں���
  bool IsCommNote;  //ͨѶ��ʾ

  int LastCom; //���һ��ѡ����ȷ��COM��
  QTimer *timer;              //��ʱ��
  
  //-------------------------����״̬���---------------------------------
  unsigned char CommCurAdr;      //��ǰͨѶ��ַ
  unsigned char RcvStrStartPos;  //���������е���ʼ�ַ���λ��

  unsigned char SendDataBuf[SEND_BUF_SIZE]; //�������ݻ�����
  unsigned char SendCount;       //���η������ݸ���
  unsigned char RcvDataBuf[RCV_BUF_SIZE]; //�������ݻ�����
  unsigned char RcvCount;       //���ν������ݸ���

  volatile int RdDataDelay;   //������ʱ��ʱ

  //------------------------ͨѶ״̬���---------------------------------
  int ComCount; //Com������
  volatile enum eCommState_t{eCommIdie, eRdData, eRdDataWait} eCommState;
  int RetryCount; //��ǰͨѶ���Դ���
  //int CommCount;  //��ǰͨѶ�ܴ���
  bool waitStop;                 //ֹͣͨѶ 

  //�������:
  QFile *file;
  QTextStream *tStream;

  QString LastSendStr;    //���һ�α����͵��ַ���
  int SendLineCount;       //��������
  int UnAskCount;         //����ʱ����Ӧ�����
  int CrcErrCount;        //����ʱ��У��������
  int DataShortCount;     //����ʱ�����ݹ��̴���
  int ValidCount;         //��Ч���ݴ���

  int FullSendBufHeader();//��䷢�ͻ���������ͷ
  int SendString(QString &Str);//�����ַ���
  void FullSendDataBuf(unsigned short Len);//��仺��������
  void StopBatSend(bool IsForceStop);//��ֹ�������ļ��ķ���

  void RcvEndPro(int);     //���ս�������
  void StartCommPro(enum eWorkState_t eNextState);//����ͨѶ����

};

#endif
