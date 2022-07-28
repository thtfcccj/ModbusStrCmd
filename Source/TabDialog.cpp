/****************************************************************************

                              对话框实现

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
  setWindowTitle(tr("Modbus字符指令器 V1.00"));

  settings = new QSettings("ModbusStrCmd.ini",QSettings::IniFormat);
  QVBoxLayout *gLayout = new QVBoxLayout();
  QHBoxLayout *paraLayout = new QHBoxLayout;

	//===============================串口选择====================================
  QHBoxLayout *comLayout = new QHBoxLayout;

  //选择正确的COM口
  LastCom = settings->value("LastCom").toInt();
  IsCommNote = !settings->value("DisCommNote").toInt();

  QLabel *lableCmd = new QLabel(tr("COM选择"));
  lableCmd->setFixedWidth(50);
  comLayout->addWidget(lableCmd);
  //自动刷新按钮
  int EnComAutoFilter = settings->value("EnComAutoFilter").toInt();
  if(EnComAutoFilter){
    bnComRefresh = new QPushButton(tr("刷新"));
    bnComRefresh->setFixedWidth(40);
    bnComRefresh->setDisabled(true); //COM0时才开启
    connect(bnComRefresh, SIGNAL(clicked()), this, SLOT(ComRefresh()));
    comLayout->addWidget(bnComRefresh);
  }
  else bnComRefresh = NULL; //无

  //COM下拉选择
  cbComId = new QComboBox();
  cbComId->setFixedWidth(70);
  cbComId->setToolTip (tr("上次有效选择：COM") + QString::number(LastCom));
  comLayout->addWidget(cbComId);

  //填充下拉框
  cbComId->insertItem(0, "关闭",0); //COM0为关闭
  ComCount = settings->value("ComCount").toInt();
  if(ComCount <= 0) ComCount = 20; //不支持时默认为0
  else if (ComCount > 256) ComCount = 256;//电脑支持的最大个数
  int Pos = 1; //COM0为关闭
  for(int Port = 1; Port <= ComCount; Port++){
    if(EnComAutoFilter){//自动过滤开启的COM口时
      if(FindSerial(Port, Port) >= 0){//找到了
        cbComId->insertItem(Pos,"COM" + QString::number(Port), Port);
        Pos++;
      }
    }
    else cbComId->insertItem(Port,"COM" + QString::number(Port), Port);//全部加载
  }

  //禁止自动连接时，为关闭
  if(settings->value("DisComAutoConnect").toInt())
    cbComId->setCurrentIndex(0);
  //用户最后选择无效,或选择的没法打开时
  else if((LastCom <= 0) || (LastCom > 256) || (FindSerial(LastCom, LastCom) < 0)){
    if(!settings->value("DisComAutoSel").toInt()){//没有禁止自动选择首个时
      for(int Id = 1; Id < cbComId->count(); Id++){
        int Port = cbComId->itemData(Id).toInt();
        if(FindSerial(Port, Port) >= 0){//找到了
          cbComId->setCurrentIndex(Id);
          break;
        }
      }//end for
    }
   else cbComId->setCurrentIndex(0); //默认为关闭
  }
  else if(EnComAutoFilter)//自动过滤开启的COM口时,需查找位置
    cbComId->setCurrentIndex(cbComId->findText("COM" + QString::number(LastCom)));
  else cbComId->setCurrentIndex(LastCom);//直接就是

  paraLayout->addLayout(comLayout);
  //=============================通讯地址===============================
  paraLayout->addSpacing(10);
  QHBoxLayout *adrLayout = new QHBoxLayout;
  QLabel *lableAdr = new QLabel(tr("设备地址"));
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
  //=============================功能选择===============================
  paraLayout->addSpacing(10);
  QHBoxLayout *funLayout = new QHBoxLayout;
  QLabel *lableFun = new QLabel(tr("指令类型"));
  lableFun->setFixedWidth(50);
  funLayout->addWidget(lableFun);
  cbFunId = new QComboBox();
  QFile File("Fun.ini");
  if(File.open(QIODevice::ReadOnly) == true){//能打开时
    QTextStream t(&File);
    do{
      if(t.atEnd()) break;
      QStringList LineList = t.readLine().split(';');
      QStringList List = LineList[0].split('='); //=左边是功能提示，右边是信息
      if(List.count() != 2) continue;
      QString Header = List[0].trimmed();//去前后空格
      if((Header[0] == '/') && (Header[1] == '/'))  continue; //注解掉了
      cbFunId->addItem(Header, List[1].trimmed());//去前后空格
    }while(1);
  }
  cbFunId->setFixedWidth(150);
  int LastFun = settings->value("LastFun").toInt(); //自动装载最后
  if((LastFun >= 0) && (LastFun < cbFunId->maxCount()))
    cbFunId->setCurrentIndex(LastFun);
  connect(cbFunId,SIGNAL(currentIndexChanged(int)),this, SLOT(FunIdChanged()));
  funLayout->addWidget(cbFunId);

  paraLayout->addLayout(funLayout);
  paraLayout->addStretch(1);

  QGroupBox *gbComm = new QGroupBox(tr("参数设置"));
  gbComm->setToolTip(tr("波特率等配置，请直接编辑“ModbusStrCmd.ini”文件"));
  gbComm->setFixedHeight(55);
  gbComm->setLayout(paraLayout);

  gLayout->addWidget(gbComm);
  //-----------------------------当前编辑相关----------------------------
  //指令标签与编辑器
  QLabel *editLable =  new QLabel(tr("指令:"));
  leEdit = new QLineEdit(); //编辑器
  connect(leEdit, SIGNAL(textChanged(const QString &)), 
            this, SLOT(EditChanged(const QString &)));

  //发送按钮
  bnSendEditStr = new QPushButton(tr("发送"));
  bnSendEditStr->setFixedWidth(50);
  bnSendEditStr->setDisabled(true);   //通讯中开启
  connect(bnSendEditStr, SIGNAL(clicked()), this, SLOT(SendEditStr()));

  //此行
  QHBoxLayout *editLayout = new QHBoxLayout;
  editLayout->addWidget(editLable);
  editLayout->addWidget(leEdit);
  editLayout->addWidget(bnSendEditStr);

  QGroupBox *gbEdit = new QGroupBox(tr("单指令功能"));
  gbEdit->setToolTip(tr("返回结果将自动保存至“eLog.txt”文件"));
  gbEdit->setFixedHeight(70);
  gbEdit->setLayout(editLayout);
  gLayout->addWidget(gbEdit);

  //-----------------------------批处理相关------------------------------
  //装载批处理文件
  bnLoadBatFile = new QPushButton(tr("装载"));
  bnLoadBatFile->setFixedWidth(50);
  connect(bnLoadBatFile, SIGNAL(clicked()), this, SLOT(LoadBatFile()));  
  leBatFile = new QLineEdit(); //文件位置
  leBatFile->setDisabled(true);   //只允许查看
  //编辑按钮
  bnEditBatFile = new QPushButton(tr("编辑")); //发送过程中时，显示为"中止"
  bnEditBatFile->setDisabled(true);   //装载文件后开启
  bnEditBatFile->setFixedWidth(50);
  connect(bnEditBatFile, SIGNAL(clicked()), this, SLOT(EditBatFile()));

  //发送按钮
  bnSendBatStr = new QPushButton(tr("发送")); //发送过程中时，显示为"中止"
  bnSendBatStr->setDisabled(true);   //装载文件后开启
  bnSendEditStr->setFixedWidth(50);
  connect(bnSendEditStr, SIGNAL(clicked()), this, SLOT(SendBatStr()));
  //此行
  QHBoxLayout *batLayout = new QHBoxLayout;
  batLayout->addWidget(bnLoadBatFile);
  batLayout->addWidget(leBatFile);
  batLayout->addWidget(bnEditBatFile);
  batLayout->addWidget(bnSendBatStr);

  QGroupBox *gbBat = new QGroupBox(tr("批处理指令功能"));
  gbEdit->setToolTip(tr("返回结果将自动保存至“bakLog.txt”文件"));
  gbBat->setFixedHeight(70);
  gbBat->setLayout(batLayout);
  gLayout->addWidget(gbBat);

  setLayout(gLayout);

  //初始化串口
  connect(cbComId,SIGNAL(currentIndexChanged(int)),this, SLOT(ComSelChanged()));
  ComSelChanged();

  //初始化定时器
  timer = new QTimer();
  timer->setInterval(500);
  connect(timer,SIGNAL(timeout()),this,SLOT(SerialOV()));
}

//-----------------------------地址修改完成---------------------------
void TabDialog::AdrChanged(const QString &text)
{
  settings->setValue("LastAdr", text); //记住以方便下次打开
}

//-----------------------------功能ID被更改---------------------------
void TabDialog::FunIdChanged()
{
  settings->setValue("LastFun", cbFunId->currentIndex()); //记住以方便下次打开
}

//-------------------------------编辑完成---------------------------
void TabDialog::EditChanged(const QString &text)
{
  //条件满足时允许点发送
  if(!text.isEmpty() && (this->serial != NULL)) 
    bnSendEditStr->setEnabled(true);
  else bnSendEditStr->setDisabled(true);
}

//---------------------------装载文件------------------------------
void TabDialog::LoadBatFile()
{
  QString fileName = QFileDialog::getOpenFileName(0, 
                      tr("打开批处理文本文件..."),
                      QDir::currentPath(),
                      tr("txt文件(*.txt);;csv文件(*.csv)"));//;; *.*文件(*.*)
  if (!fileName.isEmpty()){
    leBatFile->setText(fileName);
    bnEditBatFile->setEnabled(true);   //允许编辑了
    if(this->serial != NULL)
      bnSendBatStr->setEnabled(true);   //允许发送了
  }
}
//--------------------------编辑批处理文件--------------------------
void TabDialog::EditBatFile()
{
  QDesktopServices::openUrl(QUrl::fromLocalFile(leBatFile->text()));
}
