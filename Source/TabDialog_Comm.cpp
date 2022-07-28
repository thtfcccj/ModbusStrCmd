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
        if(!IsNote) return false;
        //需提示时
        QMessageBox msgBox;
        msgBox.setText(tr("串口打开失败，请检查串口是否存在或被占用！"));
        msgBox.exec();
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
    
    if(IsNote){
      QMessageBox msgBox;
      msgBox.setText(tr("串口成功打开，请确保已正确连接好设备!"));
      msgBox.exec();
    }
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

//----------------------发送缓冲区数据实现--------------------------
void TabDialog::SendData(unsigned char Len)
{
  SendCount = Len;
  RcvCount = 0;
  eCommState = eRdData;
  RdDataDelay = 0;//暂未延时
  serial->readLine((char*)RcvDataBuf, RCV_BUF_SIZE); //发送前先无条件清空接收
  serial->write((char*)SendDataBuf, Len);
  timer->setInterval(Len * 4 + 100); //写数据与读等待时间
  timer->start();//启动定时器
}

//----------------------------串口返回数据处理-----------------------
void TabDialog::SerialRcv()
{
  //异常
  if(eCommState != eRdData){
    eCommState = eCommIdie;//恢复
    return;
  }

  unsigned char Len = 0;
  
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
  unsigned short CRC16 = GetCRC16(RcvDataBuf, Len - 2);
  //高位在前,低位在后
  unsigned short DataCRC16 = ((unsigned short)RcvDataBuf[Len - 2] << 8) + RcvDataBuf[Len - 1];
  //低位在前,低高位在后
  unsigned short DataCRC16A = ((unsigned short)RcvDataBuf[Len - 1] << 8) + RcvDataBuf[Len - 2];

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
}

//----------------------------发送编辑器字符------------------------
void TabDialog::SendEditStr()
{


}

//------------------------发送批处理文件中的字符--------------------
void TabDialog::SendBatStr()
{


}






