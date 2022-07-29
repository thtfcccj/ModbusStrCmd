/****************************************************************************

                        主界面通讯部分

****************************************************************************/

#include <QtGui>
#include <QLabel>
#include "TabDialog.h"

extern "C" {
  #include  "NetData.h" 
}

//----------------------------查找串口函数------------------------------
//正： 找到的COM口，负：未找到
int TabDialog::FindSerial(int FindStart, int FindEnd)
{
  //实例化串口类,参数为常用默认值
  const struct PortSettings SerialSetting = {BAUD9600,DATA_8, PAR_NONE, STOP_1,FLOW_OFF,0};

	for(; FindStart <= FindEnd; FindStart++){
      serial = new Win_QextSerialPort("COM" + QString::number(FindStart), SerialSetting,
                   QextSerialBase::EventDriven);

      bool OK = serial->open(QIODevice::ReadWrite); //打开串口
      delete(serial);
      serial = NULL;
       
      //找到了
      if(OK) return FindStart;
	}
	return -1;
}

//----------------------------初始化串口函数------------------------------
bool TabDialog::InitSerial(bool IsNote)//是否有提示
{
    if(cbComId->currentIndex() == 0) return false; //COM0为关闭
    //实例化串口类
    const struct PortSettings SerialSetting = {BAUD4800,DATA_8,
                                              PAR_EVEN,STOP_1,FLOW_OFF,0};
    QString portName = cbComId->currentText(); //获取串口名
    //创建串口
    serial = new Win_QextSerialPort(portName,SerialSetting,
                                    QextSerialBase::EventDriven);
    //打开串口
    bool OK = serial->open(QIODevice::ReadWrite);
    if(!OK){//未成功时先关闭
        //delete serial;
        //serial = NULL;
        if(IsNote)//需提示时
          MsgNote(tr("串口打开失败，请检查串口是否存在或被占用！"));
        return false;
    }

    //流控
    int Cfg = settings->value("ComFlow").toInt();
    if(Cfg == 1) serial->setFlowControl(FLOW_HARDWARE);
    else if(Cfg == 2) serial->setFlowControl(FLOW_XONXOFF);        
    else serial->setFlowControl(FLOW_OFF);

    //校验方式
    Cfg = settings->value("ComPar").toInt();
    if(Cfg == 1) serial->setParity(PAR_ODD);
    else if(Cfg == 2) serial->setParity(PAR_EVEN); 
    else if(Cfg == 3) serial->setParity(PAR_MARK); 
    else serial->setParity(PAR_NONE);

    //数据位
    Cfg = settings->value("ComBit").toInt();
    if(Cfg == 5) serial->setDataBits(DATA_5);
    else if(Cfg == 6) serial->setDataBits(DATA_6);  
    else if(Cfg == 7) serial->setDataBits(DATA_7);  
    else serial->setDataBits(DATA_8);

    //停止位
    Cfg = settings->value("ComStop").toInt();
    if(Cfg == 2) serial->setStopBits(STOP_2);
    else if(Cfg == 15) serial->setStopBits(STOP_1_5); //=1.5时    
    else serial->setStopBits(STOP_1);

    //波特率
    Cfg = settings->value("ComBuad").toInt();
    if(Cfg == 600) serial->setBaudRate(BAUD600);
    else if(Cfg == 1200) serial->setBaudRate(BAUD1200);
    else if(Cfg == 2400) serial->setBaudRate(BAUD2400);
    else if(Cfg == 4800) serial->setBaudRate(BAUD4800);
    else if(Cfg == 14400) serial->setBaudRate(BAUD14400);
    else if(Cfg == 19200) serial->setBaudRate(BAUD19200);
    else if(Cfg == 38400) serial->setBaudRate(BAUD38400);
    else if(Cfg == 56000) serial->setBaudRate(BAUD56000);
    else if(Cfg == 57600) serial->setBaudRate(BAUD57600);
    else if(Cfg == 76800) serial->setBaudRate(BAUD76800);
    else if(Cfg == 115200) serial->setBaudRate(BAUD115200);
    else if(Cfg == 128000) serial->setBaudRate(BAUD128000);
    else if(Cfg == BAUD256000) serial->setBaudRate(BAUD256000);
    else serial->setBaudRate(BAUD9600);

    serial->setDtr(false);
    serial->setRts(false);

    //接收返回码
    connect(this->serial,SIGNAL(readyRead()),this,SLOT(SerialRcv()));
    
    if(IsNote) //带提示时
      MsgNote(tr("串口成功打开，请确保已正确连接好设备!"));
    return true;
}

//----------------------------刷新存在的COM口------------------------
void TabDialog::ComRefresh()
{
  //关闭相关
  cbComId->setDisabled(true);
  if(bnComRefresh != NULL) bnComRefresh->setDisabled(true);

  int Count = cbComId->count(); //先移除
  for(int Item = 1; Item < Count; Item++) 
    cbComId->removeItem(1);
  //再扫描
  int Pos = 1;
  for(int Port = 1; Port < ComCount; Port++){
    if(FindSerial(Port, Port) >= 0){//找到了
      cbComId->insertItem(Pos,"COM" + QString::number(Port), Port);
      Pos++;
    }
  }
  //开启相关
  cbComId->setEnabled(true);
  if(bnComRefresh != NULL) bnComRefresh->setEnabled(true);
}

//----------------------------串口被重新选择------------------------
void TabDialog::ComSelChanged()
{
  if(!cbComId->isEnabled()) return; //禁用了

  if(this->serial != NULL){//已挂接时
    eCommState = eCommIdie;  //强制关闭通讯(状态机会报超时)

    delete(this->serial);//关闭
    this->serial = NULL;
  }

  //重新初始化
  if(InitSerial(IsCommNote) == true){
    //有效时保存最后的选择
    QString Com = cbComId->currentText().right(cbComId->currentText().count() - 3);
    settings->setValue("LastCom", Com);
    cbComId->setToolTip(tr("上次有效选择：COM") + QString::number(LastCom));
    LastCom = Com.toInt(); //更新本次
    //开启相关按钮
    if(!leEdit->text().isEmpty()) bnSendEditStr->setEnabled(true);
    if(!leBatFile->text().isEmpty()) bnSendBatStr->setEnabled(true);
  }
  else{
    cbComId->setToolTip(tr("上次有效选择：COM") + QString::number(LastCom));

    //关闭相关按钮
    bnSendEditStr->setDisabled(true);
    bnSendBatStr->setDisabled(true);
  }
  //关闭时，允许刷新
  if(bnComRefresh != NULL){
    if(cbComId->currentIndex() == 0) 
      bnComRefresh->setEnabled(true);
    else bnComRefresh->setDisabled(true);
  }
}

//----------------------------串口返回数据处理-----------------------
void TabDialog::SerialRcv()
{
  //异常
  if(eCommState != eRdData){
    eCommState = eCommIdie;//恢复
    return;
  }

  if(!RdDataDelay){//此中断会在未接收完数据时返回,根据接收数据长度,重新延时
    int Len = SendCount + 7;
    RdDataDelay = Len * 4;
    if(RdDataDelay < 40) RdDataDelay *= 8;
    timer->stop();  //停止定时器
    timer->setInterval(RdDataDelay);
    timer->start();//重启动定时器
    return;
  }
  
  if(RdDataDelay != -1){//等待中间中断无效
    return;
  }
  
  RdDataDelay = 0;
  timer->stop();  //停止定时器

  //读回返回的数据
  memset(RcvDataBuf, 0, RCV_BUF_SIZE);//先清空缓冲区防止一个都未读出
  RcvCount = serial->readLine((char*)RcvDataBuf,RCV_BUF_SIZE);

  //校验数据是否正确及错误类型
  unsigned short CRC16 = GetCRC16(RcvDataBuf, RcvCount - 2);
  //高位在前,低位在后
  unsigned short DataCRC16 = ((unsigned short)RcvDataBuf[RcvCount - 2] << 8) + RcvDataBuf[RcvCount - 1];
  //低位在前,低高位在后
  unsigned short DataCRC16A = ((unsigned short)RcvDataBuf[RcvCount - 1] << 8) + RcvDataBuf[RcvCount - 2];

  int DataValid = 0;
  if((CRC16 != DataCRC16) && (CRC16 != DataCRC16A)) //两种校验均认为正确
    DataValid = -1;
  //检查地址是否正确
  if(RcvDataBuf[0] != CommCurAdr)
    DataValid = -2;

  RcvEndPro(DataValid);//接收结束处理
}

//----------------------------读写数据超时处理------------------------
void TabDialog::SerialOV()
{
  timer->stop();  //停止定时器
  //异常
  if(eCommState != eRdData){
    eCommState = eCommIdie;//恢复
    return;
  }
  
  //==============================非数据超时处理=========================
  if(RdDataDelay){ //读数据延时过程中时正式读取
    timer->setInterval(200); //恢复默认
    RdDataDelay = -1;//告之完成了
    SerialRcv();
    return;
  }
  RcvEndPro(10);//超时处理
}

//----------------------------接收结束处理------------------------
void TabDialog::RcvEndPro(int DataInvalid)//数据无效状态,0有效
{
  eCommState = eCommIdie; //本次通讯完成
  QString Note;
  if(cbLogRead->isChecked()){//带发送时
    Note.append(LastSendStr + ", ");
  }
  if(DataInvalid){//无应答或校验错误时
    if(DataInvalid < 0){
      Note.append(tr("接收数据校验错误"));
      CrcErrCount++;
    }
    else{
      Note.append(tr("接收无应答"));
      UnAskCount++;
    }
  }
  else{//按收数据处理
    if(RcvCount <= (RcvStrStartPos + (1+2))){//地址1+用户码+校验码
      Note.append(tr("返回数据过短或异常"));
      DataShortCount++;
    }
    else{
      QTextCodec *codec = QTextCodec::codecForLocale();
      if(codec == NULL){
        Note.append(tr("接收时遇字符编码系统错误"));
      }
      else{
        RcvDataBuf[RcvCount - 2] = '\0';//强制加结束字符
        QString RcvData = codec->toUnicode(QByteArray((char*)&RcvDataBuf[1 + RcvStrStartPos]));
        Note.append(RcvData);
      }
    }
  }
  MsgNote(Note, -1); //行数据强制到log
  ValidCount++;
  if(SendLineCount < 0) return; //单行时完成

  //多行时
  if(waitStop == true){//强制停止
    StopBatSend(true);
    return; 
  }
  //准备下一数据
  QString CurString;
  do{
    if(tStream->atEnd()){//发送正式完成了
      StopBatSend(false);
      return;
    }
    CurString = tStream->readLine();//读取
    if((CurString[0] == '/') && (CurString[0] == '/')) continue;//此行注解掉了
    break;
  }while(1);
  if(SendString(CurString)){//首行数据失败直接退出
    StopBatSend(true);
  }
  SendLineCount++;//发出计数
}

//----------------------------中止批处理文件的发送------------------------
void TabDialog::StopBatSend(bool IsForceStop)
{
  waitStop = false;
  if(file != NULL){
    file->close();
    delete file;
    file = NULL;
  }
  if(tStream != NULL){
    delete tStream;
    tStream = NULL;
  }
  //恢复开启相关按钮
  bnSendEditStr->setEnabled(true);
  cbComId->setEnabled(true);
  bnLoadBatFile->setEnabled(true);
  bnSendBatStr->setText(tr("发送"));

  //结束生成报告
  QString EndNote;
  if(IsForceStop) EndNote.append(tr("批处理指令：通讯已中止！\n"));
  else  EndNote.append(tr("批处理指令：通讯已完成！\n"));
  EndNote.append(tr("   共发出数据 ") + QString::number(SendLineCount) + tr(" 条 \n"));
  EndNote.append(tr("   收到有效数据 ") + QString::number(ValidCount) + tr(" 条 \n"));
  if(DataShortCount) EndNote.append(tr("  收到过短数据 ") + QString::number(DataShortCount) + tr(" 条 \n"));
  if(UnAskCount) EndNote.append(tr("     无应答数据 ") + QString::number(UnAskCount) + tr(" 条 \n"));
  if(CrcErrCount) EndNote.append(tr("     收到错误数据 ") + QString::number(CrcErrCount) + tr(" 条 \n"));
  QMessageBox *msgBox = new QMessageBox();
  msgBox->setText(EndNote);
  msgBox->exec();
}

//----------------------------填充发送缓冲区数据头-------------------------
//由地址+功能码+可能的数据组成
//返回填充数据个数,<=0异常
int TabDialog::FullSendBufHeader()
{
  unsigned char *pWrBuf = SendDataBuf;
  *pWrBuf++ = CommCurAdr; //地址
  QString Data = cbFunId->itemData(cbFunId->currentIndex()).toString();
  if(Data.isNull()) {MsgNote(tr("类型解析错误： 没有定义Modbus功能码，不能被执行！")); return -1; }
  QStringList List = Data.split('|'); //=左边是功能码及后续可能数据，右边是其它信息
  //填充其它信息
  bool Ok;
  if(List.count() >= 2)
    RcvStrStartPos = List[1].trimmed().toInt(&Ok); //指定
  else RcvStrStartPos = 1; //仅含功能码

  //填充数据流
  QStringList ParaList = List[0].split(','); //"\\s+"可忽略多个空格,改为,区分以明显
  int Count = ParaList.count();
  if(Count > 10) {
    QString Note = tr("类型解析错误： 数据量超过") +  QString::number(10) +tr("个数据!" );
    MsgNote(Note); 
    return - 2;
  }
 //依次填入数据
  for(int Pos = 0; Pos < Count; Pos++){
    QString Para = ParaList[Pos].trimmed();//去前后空格
    unsigned char CurData = 0;
    //十六进制方式读取
    if((Para[0] == '0') && ((Para[1] == 'x') || (Para[1] == 'X'))){
      CurData = Para.toInt(&Ok, 16);
    }
    else if((Para[0] >= '0') && (Para[0] <= '9')){//十进制
      CurData = Para.toInt(&Ok);
    }
    else{//只接受直接数据
      QString Note = tr("类型解析错误：第") +  QString::number(Pos) +tr("个数据格式不被支持！" );
      MsgNote(Note); 
      return - 3;
    }
    if(Ok == false){
      QString Note = tr("类型解析错误：第") +  QString::number(Pos) +tr("组数据表达有误！" );
      MsgNote(Note); 
      return -4;
    }
    *pWrBuf++ = CurData;
  } //end for

  return pWrBuf - SendDataBuf;
}

//----------------------------发送字符串----------------------------
//返回0正确填充并启动发出，否则为其它错误
int TabDialog::SendString(QString &Str)
{
  int HeaderLen = FullSendBufHeader();
  if(HeaderLen <= 0) return HeaderLen;
  
  //QString存储的是unicode码，需要转换为GB2312码
  QTextCodec *codec = QTextCodec::codecForLocale();
  if(codec == NULL){
    MsgNote(tr("发送时需字符编码系统错误!")); 
    return -10;
  }
  QByteArray string = codec->fromUnicode(Str);
  int StrLen = string.length();
  if(StrLen >= ((SEND_BUF_SIZE - 2) - HeaderLen)){
    MsgNote(tr("发送时需编码用户字符串过长!")); 
    return - 11;
  }
  qstrcpy((char*)&SendDataBuf[HeaderLen], string);
  LastSendStr = Str; //缓冲最后发送字符串
  FullSendDataBuf(HeaderLen + StrLen);
  return 0;
}

//--------------------------填充缓冲区数据------------------------
void TabDialog::FullSendDataBuf(unsigned short Len)
{
  //准备校验码
  unsigned short CRC16 = GetCRC16(SendDataBuf, Len);
  #ifndef CRC16_ANTI//高位在前,低位在后
    SendDataBuf[Len++] = (unsigned char)(CRC16 >> 8);
    SendDataBuf[Len++] = (unsigned char)(CRC16);
  #else//低位在前,低高位在后
    SendDataBuf[Len++] = (unsigned char)(CRC16);
    SendDataBuf[Len++] = (unsigned char)(CRC16 >> 8);
  #endif

  SendCount = Len;
  RcvCount = 0;
  eCommState = eRdData;
  RdDataDelay = 0;//暂未延时
  serial->readLine((char*)RcvDataBuf,RCV_BUF_SIZE); //发送前先无条件清空接收
  serial->write((char*)SendDataBuf,Len);
  timer->setInterval(Len + 200); //写数据与等待时间
  timer->start();//启动定时器
}

//----------------------------发送编辑器字符------------------------
void TabDialog::SendEditStr()
{
  if(SendString(leEdit->text())) return;
  SendLineCount = -1;//单行状态
}

//------------------------发送批处理文件中的字符--------------------
void TabDialog::SendBatStr()
{
  //发送中时
  if(bnSendBatStr->text() == tr("停止")){
    waitStop = true;
    bnSendBatStr->setText(tr("发送"));
    return;
  }
  else if(waitStop == true) return; //停止未结束

  //启动发送 
  file = new QFile(leBatFile->text());
  if(!file->open(QIODevice::ReadOnly)){
    MsgNote(tr("批处理文件打开失败!"));
    return;
  }
  tStream = new QTextStream(file);
  QString CurString;
  do{
    if(tStream->atEnd()){
      MsgNote(tr("批处理文件内无数据需要发送!"));
      return;
    }
    CurString = tStream->readLine();//读取
    if((CurString[0] == '/') && (CurString[0] == '/')) continue;//此行注解掉了
    break;
  }while(1);
  
  if(SendString(CurString)) return; //首行数据失败直接退出

  //初始化计数器
  SendLineCount = 1;
  UnAskCount = 0;         //多行时，无应答次数
  CrcErrCount = 0;        //多行时，校验错误次数
  DataShortCount = 0;     //多行时，数据过短次数
  ValidCount = 0;         //有效数据次数

  //关闭相关按钮
  bnSendEditStr->setDisabled(true);
  cbComId->setDisabled(true);
  bnLoadBatFile->setDisabled(true);
  bnSendBatStr->setText(tr("停止"));
}






