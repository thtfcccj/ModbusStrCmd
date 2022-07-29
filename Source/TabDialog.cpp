/****************************************************************************

                              �Ի���ʵ��

****************************************************************************/

#include <QtGui>
#include <QLabel>
#include "TabDialog.h"


TabDialog::TabDialog(QWidget *parent)
  : QDialog(parent), eCommState(eCommIdie),RetryCount(0), serial(NULL),
  waitStop(false),file(NULL),tStream(NULL)
{
  this->setMinimumWidth(600);
  this->setMinimumHeight(400);
  setWindowTitle(tr("Modbus�ַ�ָ��� V1.00"));

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
  CommCurAdr = LastAdr.toInt(); //���µ�ַ
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

  paraLayout->addStretch(1); //�����Ҳ�
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
  gbEdit->setFixedHeight(70);
  gbEdit->setLayout(editLayout);
  gLayout->addWidget(gbEdit);

  //-----------------------------���������------------------------------
  //װ���������ļ�
  bnLoadBatFile = new QPushButton(tr("װ��..."));
  bnLoadBatFile->setFixedWidth(60);
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
  bnSendBatStr->setFixedWidth(50);
  connect(bnSendBatStr, SIGNAL(clicked()), this, SLOT(SendBatStr()));
  //����
  QHBoxLayout *batLayout = new QHBoxLayout;
  batLayout->addWidget(bnLoadBatFile);
  batLayout->addWidget(leBatFile);
  batLayout->addWidget(bnEditBatFile);
  batLayout->addWidget(bnSendBatStr);

  QGroupBox *gbBat = new QGroupBox(tr("������ָ���"));
  gbBat->setFixedHeight(70);
  gbBat->setLayout(batLayout);
  gLayout->addWidget(gbBat);

  //-----------------------------�������--------------------------
  QHBoxLayout *rcvParaLayout = new QHBoxLayout;
  //��乤��״̬
  cbLogState = new QCheckBox(tr("��乤��״̬"));
  cbLogState->setFixedWidth(100);
  if(settings->value("LogState").toInt())
    cbLogState->setChecked(true); //Ĭ�Ͽ���
  rcvParaLayout->addWidget(cbLogState);
  connect(cbLogState, SIGNAL(stateChanged(int)), this, SLOT(LogStateChanged(int)));
  //���ʱ��ѡ��
  cbLogTime = new QCheckBox(tr("���ʱ��"));
  cbLogTime->setFixedWidth(80);
  if(settings->value("LogTime").toInt())
    cbLogTime->setChecked(true); //Ĭ�Ͽ���
  rcvParaLayout->addWidget(cbLogTime);
  connect(cbLogTime, SIGNAL(stateChanged(int)), this, SLOT(LogTimeChanged(int)));
  //��䷢��ѡ��
  cbLogRead = new QCheckBox(tr("��䷢��"));
  cbLogRead->setFixedWidth(80);
  if(settings->value("LogRead").toInt())
    cbLogRead->setChecked(true); //Ĭ�Ͽ���
  rcvParaLayout->addWidget(cbLogRead);
  connect(cbLogRead, SIGNAL(stateChanged(int)), this, SLOT(LogReadChanged(int)));
  //�������ѡ��
  cbLogEnRewrite = new QCheckBox(tr("�������"));
  cbLogEnRewrite->setFixedWidth(80);
  if(settings->value("LogEnRewrite").toInt())
    cbLogEnRewrite->setChecked(true); //Ĭ�Ͽ���
  rcvParaLayout->addWidget(cbLogEnRewrite);
  connect(cbLogEnRewrite, SIGNAL(stateChanged(int)), this, SLOT(LogEnRewriteChanged(int)));
  //�����ť
  rcvParaLayout->addStretch(1); //�����Ҳ�
  bnClrRcv = new QPushButton(tr("�����ʾ"));
  bnClrRcv->setFixedWidth(60);
  rcvParaLayout->addWidget(bnClrRcv);
  //���水ť
  bnSaveRcv = new QPushButton(tr("�������ļ�..."));
  bnSaveRcv->setFixedWidth(100);
  rcvParaLayout->addWidget(bnSaveRcv);
  connect(bnSaveRcv, SIGNAL(clicked()), this, SLOT(SaveRcv()));
  //��ʾ��
  pRcvWin = new QPlainTextEdit();
  if(!cbLogEnRewrite->isChecked()) pRcvWin->setReadOnly(true); //ֻ��
  connect(bnClrRcv, SIGNAL(clicked()), pRcvWin, SLOT(clear()));//���ʵ��
  //������
  QVBoxLayout *rcvLayout = new QVBoxLayout;
  rcvLayout->addLayout(rcvParaLayout);
  rcvLayout->addWidget(pRcvWin);

  QGroupBox *gbRcv = new QGroupBox(tr("������־"));
  gbRcv->setLayout(rcvLayout);
  gLayout->addWidget(gbRcv);

  //���
  setLayout(gLayout);//���岼��
  connect(cbComId,SIGNAL(currentIndexChanged(int)),this, SLOT(ComSelChanged()));
  ComSelChanged();//��ʼ������

  //��ʼ����ʱ��
  timer = new QTimer();
  timer->setInterval(500);
  connect(timer,SIGNAL(timeout()),this,SLOT(SerialOV()));
}

//-------------------------������ʾ�Ի�����---------------------------------
//AppendState��0�ޣ�1��ֹ,2�ɹ�,3δ����, ��ֵǿ�������log
void TabDialog::MsgNote(QString &Note, int AppendState)
{
  if(AppendState == 1) Note.append(tr("\n��������ֹ��"));
  if(AppendState == 2) Note.append(tr("\n������ɣ�"));
  if(AppendState == 3) Note.append(tr("\n���ս��δ��������"));
  //log����ϵͳ״̬ʱ,�ڴ�ʵ��:
  if(cbLogState->isChecked() || AppendState < 0){
    if(cbLogTime->isChecked()){//��ʱ��ʱ
      QString Str = QDateTime::currentDateTime().toString("yy-MM-dd hh:mm:ss");
      pRcvWin->appendPlainText(Str + ": " + Note);
    }
    else pRcvWin->appendPlainText(Note);
    return;
  }
  //�öԻ���ʵ�֣�
  QMessageBox *msgBox = new QMessageBox();
  msgBox->setText(Note);
  msgBox->exec();
}

//-----------------------------��ַ�޸����---------------------------
void TabDialog::AdrChanged(const QString &text)
{
  CommCurAdr = leAdr->text().toInt(); //���µ�ַ
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
                      settings->value("LastBatFile").toString(),
                      tr("txt�ļ�(*.txt);;csv�ļ�(*.csv)"));//;; *.*�ļ�(*.*)
  if (!fileName.isEmpty()){
    leBatFile->setText(fileName);
    bnEditBatFile->setEnabled(true);   //����༭��
    if(this->serial != NULL)
      bnSendBatStr->setEnabled(true);   //��������
   settings->setValue("LastBatFile",fileName); 
  }
}
//--------------------------�༭�������ļ�--------------------------
void TabDialog::EditBatFile()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(leBatFile->text()));
}

//--------------------------log�ı����ʵ��--------------------------
void TabDialog::LogStateChanged(int state)//����״̬ѡ��ı�
{
  settings->setValue("LogState", state); //��ס�Է����´δ�
}
void TabDialog::LogTimeChanged(int state)//����ʱ��ѡ��ı�
{
  settings->setValue("LogTime", state); //��ס�Է����´δ�
}
void TabDialog::LogReadChanged(int state)//�������Ϣѡ��ı�
{
  settings->setValue("LogRead", state); //��ס�Է����´δ�
}
void TabDialog::LogEnRewriteChanged(int state)//�������ѡ��ı�
{
  if(!cbLogEnRewrite->isChecked()) pRcvWin->setReadOnly(true); //ֻ��
  else pRcvWin->setReadOnly(false); //��д
  settings->setValue("LogEnRewrite", state); //��ס�Է����´δ�
}  

//--------------------------���������Ϣ--------------------------
void TabDialog::SaveRcv()
{
  //Ĭ��Ŀ¼���ļ���;
  QString fileName = QFileDialog::getSaveFileName(0, tr("���������־..."),
                                                  settings->value("LastLogFile").toString(),  
                                                  tr("txt�ļ� (*.txt)"));
  QFile *file = new QFile(fileName);
  if(file->open(QIODevice::WriteOnly) == false){//�ļ���ʧ��
    MsgNote(tr("������־��ʧ��!"));
    delete file;
    return;
  }
  QTextStream t(file);
  t << pRcvWin->toPlainText();

  settings->setValue("LastLogFile",fileName); //���λ��
  MsgNote(tr("������־����ɹ�!"));

  file->close();
  delete file;
} 

