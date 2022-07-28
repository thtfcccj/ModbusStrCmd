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
  static void MsgNote(QString &Note, int AppendState = 1); //1��ֹ,2�ɹ�,������

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

  //ͨѶ�������:
  Win_QextSerialPort *serial;
  int FindSerial(int FindStart, int FindEnd); //���Ҵ��ں���
  bool InitSerial(bool IsNote);//��ʼ�����ں���
  bool IsCommNote;  //ͨѶ��ʾ

  int LastCom; //���һ��ѡ����ȷ��COM��
  QTimer *timer;              //��ʱ��
  
  //-------------------------����״̬���---------------------------------
  unsigned char CommCurAdr;    //��ǰͨѶ��ַ
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
  void SendData(unsigned char Len) ;//���ͻ���������

  //����ʱ�Ի�����
  void ErrCommNote(QMessageBox *msgBox,//��new�ĶԻ���
                   int CommState);//������ʱ�����ݣ�����������

  void RcvEndPro(int);     //���ս�������
  void StartCommPro(enum eWorkState_t eNextState);//����ͨѶ����

};

#endif
