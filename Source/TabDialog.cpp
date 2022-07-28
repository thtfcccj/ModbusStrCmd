/****************************************************************************

                              �Ի���ʵ��

****************************************************************************/

#include <QtGui>
#include <QLabel>
#include "TabDialog.h"


TabDialog::TabDialog(QWidget *parent)
  : QDialog(parent), eCommState(eCommIdie),RetryCount(0), serial(NULL)
{
  this->setMinimumWidth(550);
  this->setMinimumHeight(215);
  this->setMaximumHeight(280);
  setWindowTitle(tr("Modbus�ַ�ָ���� V1.00"));

  settings = new QSettings("ModbusStrCmd.ini",QSettings::IniFormat);
  QVBoxLayout *gLayout = new QVBoxLayout();
  QHBoxLayout *paraLayout = new QHBoxLayout;

	//===============================����ѡ��====================================
  QHBoxLayout *comLayout = new QHBoxLayout;

  //ѡ����ȷ��COM��
  LastCom = settings->value("LastCom").toInt();
  IsCommNote = !settings->value("DisCommNote").toInt();

  QLabel *lableCmd = new QLabel(tr("COMѡ��"));
  lableCmd->setFixedWidth(50);
  comLayout->addWidget(lableCmd);
  //�Զ�ˢ�°�ť
  int EnComAutoFilter = settings->value("EnComAutoFilter").toInt();
  if(EnComAutoFilter){
    bnComRefresh = new QPushButton(tr("ˢ��"));
    bnComRefresh->setFixedWidth(40);
    bnComRefresh->setDisabled(true); //COM0ʱ�ſ���
    connect(bnComRefresh, SIGNAL(clicked()), this, SLOT(ComRefresh()));
    comLayout->addWidget(bnComRefresh);
  }
  else bnComRefresh = NULL; //��

  //COM����ѡ��
  cbComId = new QComboBox();
  cbComId->setFixedWidth(70);
  cbComId->setToolTip (tr("�ϴ���Чѡ��COM") + QString::number(LastCom));
  comLayout->addWidget(cbComId);

  //���������
  cbComId->insertItem(0, "�ر�",0); //COM0Ϊ�ر�
  ComCount = settings->value("ComCount").toInt();
  if(ComCount <= 0) ComCount = 20; //��֧��ʱĬ��Ϊ0
  else if (ComCount > 256) ComCount = 256;//����֧�ֵ�������
  int Pos = 1; //COM0Ϊ�ر�
  for(int Port = 1; Port <= ComCount; Port++){
    if(EnComAutoFilter){//�Զ����˿�����COM��ʱ
      if(FindSerial(Port, Port) >= 0){//�ҵ���
        cbComId->insertItem(Pos,"COM" + QString::number(Port), Port);
        Pos++;
      }
    }
    else cbComId->insertItem(Port,"COM" + QString::number(Port), Port);//ȫ������
  }

  //��ֹ�Զ�����ʱ��Ϊ�ر�
  if(settings->value("DisComAutoConnect").toInt())
    cbComId->setCurrentIndex(0);
  //�û����ѡ����Ч,��ѡ���û����ʱ
  else if((LastCom <= 0) || (LastCom > 256) || (FindSerial(LastCom, LastCom) < 0)){
    if(!settings->value("DisComAutoSel").toInt()){//û�н�ֹ�Զ�ѡ���׸�ʱ
      for(int Id = 1; Id < cbComId->count(); Id++){
        int Port = cbComId->itemData(Id).toInt();
        if(FindSerial(Port, Port) >= 0){//�ҵ���
          cbComId->setCurrentIndex(Id);
          break;
        }
      }//end for
    }
   else cbComId->setCurrentIndex(0); //Ĭ��Ϊ�ر�
  }
  else if(EnComAutoFilter)//�Զ����˿�����COM��ʱ,�����λ��
    cbComId->setCurrentIndex(cbComId->findText("COM" + QString::number(LastCom)));
  else cbComId->setCurrentIndex(LastCom);//ֱ�Ӿ���

  paraLayout->addLayout(comLayout);
  //=============================ͨѶ��ַ===============================
  paraLayout->addSpacing(10);
  QHBoxLayout *adrLayout = new QHBoxLayout;
  QLabel *lableAdr = new QLabel(tr("�豸��ַ"));
  lableAdr->setFixedWidth(50);
  adrLayout->addWidget(lableAdr);
  QString LastAdr = settings->value("LastAdr").toString();
  if(LastAdr.isEmpty()) LastAdr = tr("1");
  leAdr = new QLineEdit(LastAdr);
  leAdr->setFixedWidth(30);
  leAdr->setMaxLength(3);
  leAdr->setInputMask(QString::fromUtf8("000"));
  connect(leAdr, SIGNAL(textChanged(const QString &)), 
            this, SLOT(AdrChanged(const QString &)));

  adrLayout->addWidget(leAdr);
  paraLayout->addLayout(adrLayout);
  //=============================����ѡ��===============================
  paraLayout->addSpacing(10);
  QHBoxLayout *funLayout = new QHBoxLayout;
  QLabel *lableFun = new QLabel(tr("ָ������"));
  lableFun->setFixedWidth(50);
  funLayout->addWidget(lableFun);
  cbFunId = new QComboBox();
  QFile File("Fun.ini");
  if(File.open(QIODevice::ReadOnly) == true){//�ܴ�ʱ
    QTextStream t(&File);
    do{
      if(t.atEnd()) break;
      QStringList LineList = t.readLine().split(';');
      QStringList List = LineList[0].split('='); //=����ǹ�����ʾ���ұ�����Ϣ
      if(List.count() != 2) continue;
      QString Header = List[0].trimmed();//ȥǰ��ո�
      if((Header[0] == '/') && (Header[1] == '/'))  continue; //ע�����
      cbFunId->addItem(Header, List[1].trimmed());//ȥǰ��ո�
    }while(1);
  }
  cbFunId->setFixedWidth(150);
  int LastFun = settings->value("LastFun").toInt(); //�Զ�װ�����
  if((LastFun >= 0) && (LastFun < cbFunId->maxCount()))
    cbFunId->setCurrentIndex(LastFun);
  connect(cbFunId,SIGNAL(currentIndexChanged(int)),this, SLOT(FunIdChanged()));
  funLayout->addWidget(cbFunId);

  paraLayout->addLayout(funLayout);
  paraLayout->addStretch(1);

  QGroupBox *gbComm = new QGroupBox(tr("��������"));
  gbComm->setToolTip(tr("�����ʵ����ã���ֱ�ӱ༭��ModbusStrCmd.ini���ļ�"));
  gbComm->setFixedHeight(55);
  gbComm->setLayout(paraLayout);

  gLayout->addWidget(gbComm);
  //-----------------------------��ǰ�༭���----------------------------
  //ָ���ǩ��༭��
  QLabel *editLable =  new QLabel(tr("ָ��:"));
  leEdit = new QLineEdit(); //�༭��
  connect(leEdit, SIGNAL(textChanged(const QString &)), 
            this, SLOT(EditChanged(const QString &)));

  //���Ͱ�ť
  bnSendEditStr = new QPushButton(tr("����"));
  bnSendEditStr->setFixedWidth(50);
  bnSendEditStr->setDisabled(true);   //ͨѶ�п���
  connect(bnSendEditStr, SIGNAL(clicked()), this, SLOT(SendEditStr()));

  //����
  QHBoxLayout *editLayout = new QHBoxLayout;
  editLayout->addWidget(editLable);
  editLayout->addWidget(leEdit);
  editLayout->addWidget(bnSendEditStr);

  QGroupBox *gbEdit = new QGroupBox(tr("��ָ���"));
  gbEdit->setToolTip(tr("���ؽ�����Զ���������eLog.txt���ļ�"));
  gbEdit->setFixedHeight(70);
  gbEdit->setLayout(editLayout);
  gLayout->addWidget(gbEdit);

  //-----------------------------���������------------------------------
  //װ���������ļ�
  bnLoadBatFile = new QPushButton(tr("װ��"));
  bnLoadBatFile->setFixedWidth(50);
  connect(bnLoadBatFile, SIGNAL(clicked()), this, SLOT(LoadBatFile()));  
  leBatFile = new QLineEdit(); //�ļ�λ��
  leBatFile->setDisabled(true);   //ֻ����鿴
  //�༭��ť
  bnEditBatFile = new QPushButton(tr("�༭")); //���͹�����ʱ����ʾΪ"��ֹ"
  bnEditBatFile->setDisabled(true);   //װ���ļ�����
  bnEditBatFile->setFixedWidth(50);
  connect(bnEditBatFile, SIGNAL(clicked()), this, SLOT(EditBatFile()));

  //���Ͱ�ť
  bnSendBatStr = new QPushButton(tr("����")); //���͹�����ʱ����ʾΪ"��ֹ"
  bnSendBatStr->setDisabled(true);   //װ���ļ�����
  bnSendEditStr->setFixedWidth(50);
  connect(bnSendEditStr, SIGNAL(clicked()), this, SLOT(SendBatStr()));
  //����
  QHBoxLayout *batLayout = new QHBoxLayout;
  batLayout->addWidget(bnLoadBatFile);
  batLayout->addWidget(leBatFile);
  batLayout->addWidget(bnEditBatFile);
  batLayout->addWidget(bnSendBatStr);

  QGroupBox *gbBat = new QGroupBox(tr("������ָ���"));
  gbEdit->setToolTip(tr("���ؽ�����Զ���������bakLog.txt���ļ�"));
  gbBat->setFixedHeight(70);
  gbBat->setLayout(batLayout);
  gLayout->addWidget(gbBat);

  setLayout(gLayout);

  //��ʼ������
  connect(cbComId,SIGNAL(currentIndexChanged(int)),this, SLOT(ComSelChanged()));
  ComSelChanged();

  //��ʼ����ʱ��
  timer = new QTimer();
  timer->setInterval(500);
  connect(timer,SIGNAL(timeout()),this,SLOT(SerialOV()));
}

//-----------------------------��ַ�޸����---------------------------
void TabDialog::AdrChanged(const QString &text)
{
  settings->setValue("LastAdr", text); //��ס�Է����´δ�
}

//-----------------------------����ID������---------------------------
void TabDialog::FunIdChanged()
{
  settings->setValue("LastFun", cbFunId->currentIndex()); //��ס�Է����´δ�
}

//-------------------------------�༭���---------------------------
void TabDialog::EditChanged(const QString &text)
{
  //��������ʱ����㷢��
  if(!text.isEmpty() && (this->serial != NULL)) 
    bnSendEditStr->setEnabled(true);
  else bnSendEditStr->setDisabled(true);
}

//---------------------------װ���ļ�------------------------------
void TabDialog::LoadBatFile()
{
  QString fileName = QFileDialog::getOpenFileName(0, 
                      tr("���������ı��ļ�..."),
                      QDir::currentPath(),
                      tr("txt�ļ�(*.txt);;csv�ļ�(*.csv)"));//;; *.*�ļ�(*.*)
  if (!fileName.isEmpty()){
    leBatFile->setText(fileName);
    bnEditBatFile->setEnabled(true);   //����༭��
    if(this->serial != NULL)
      bnSendBatStr->setEnabled(true);   //��������
  }
}
//--------------------------�༭�������ļ�--------------------------
void TabDialog::EditBatFile()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(leBatFile->text()));
}
