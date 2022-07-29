/****************************************************************************

                        ������ͨѶ����

****************************************************************************/

#include <QtGui>
#include <QLabel>
#include "TabDialog.h"

extern "C" {
  #include  "NetData.h" 
}

//----------------------------���Ҵ��ں���------------------------------
//���� �ҵ���COM�ڣ�����δ�ҵ�
int TabDialog::FindSerial(int FindStart, int FindEnd)
{
  //ʵ����������,����Ϊ����Ĭ��ֵ
  const struct PortSettings SerialSetting = {BAUD9600,DATA_8, PAR_NONE, STOP_1,FLOW_OFF,0};

	for(; FindStart <= FindEnd; FindStart++){
      serial = new Win_QextSerialPort("COM" + QString::number(FindStart), SerialSetting,
                   QextSerialBase::EventDriven);

      bool OK = serial->open(QIODevice::ReadWrite); //�򿪴���
      delete(serial);
      serial = NULL;
       
      //�ҵ���
      if(OK) return FindStart;
	}
	return -1;
}

//----------------------------��ʼ�����ں���------------------------------
bool TabDialog::InitSerial(bool IsNote)//�Ƿ�����ʾ
{
    if(cbComId->currentIndex() == 0) return false; //COM0Ϊ�ر�
    //ʵ����������
    const struct PortSettings SerialSetting = {BAUD4800,DATA_8,
                                              PAR_EVEN,STOP_1,FLOW_OFF,0};
    QString portName = cbComId->currentText(); //��ȡ������
    //��������
    serial = new Win_QextSerialPort(portName,SerialSetting,
                                    QextSerialBase::EventDriven);
    //�򿪴���
    bool OK = serial->open(QIODevice::ReadWrite);
    if(!OK){//δ�ɹ�ʱ�ȹر�
        //delete serial;
        //serial = NULL;
        if(IsNote)//����ʾʱ
          MsgNote(tr("���ڴ�ʧ�ܣ����鴮���Ƿ���ڻ�ռ�ã�"));
        return false;
    }

    //����
    int Cfg = settings->value("ComFlow").toInt();
    if(Cfg == 1) serial->setFlowControl(FLOW_HARDWARE);
    else if(Cfg == 2) serial->setFlowControl(FLOW_XONXOFF);        
    else serial->setFlowControl(FLOW_OFF);

    //У�鷽ʽ
    Cfg = settings->value("ComPar").toInt();
    if(Cfg == 1) serial->setParity(PAR_ODD);
    else if(Cfg == 2) serial->setParity(PAR_EVEN); 
    else if(Cfg == 3) serial->setParity(PAR_MARK); 
    else serial->setParity(PAR_NONE);

    //����λ
    Cfg = settings->value("ComBit").toInt();
    if(Cfg == 5) serial->setDataBits(DATA_5);
    else if(Cfg == 6) serial->setDataBits(DATA_6);  
    else if(Cfg == 7) serial->setDataBits(DATA_7);  
    else serial->setDataBits(DATA_8);

    //ֹͣλ
    Cfg = settings->value("ComStop").toInt();
    if(Cfg == 2) serial->setStopBits(STOP_2);
    else if(Cfg == 15) serial->setStopBits(STOP_1_5); //=1.5ʱ    
    else serial->setStopBits(STOP_1);

    //������
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

    //���շ�����
    connect(this->serial,SIGNAL(readyRead()),this,SLOT(SerialRcv()));
    
    if(IsNote) //����ʾʱ
      MsgNote(tr("���ڳɹ��򿪣���ȷ������ȷ���Ӻ��豸!"));
    return true;
}

//----------------------------ˢ�´��ڵ�COM��------------------------
void TabDialog::ComRefresh()
{
  //�ر����
  cbComId->setDisabled(true);
  if(bnComRefresh != NULL) bnComRefresh->setDisabled(true);

  int Count = cbComId->count(); //���Ƴ�
  for(int Item = 1; Item < Count; Item++) 
    cbComId->removeItem(1);
  //��ɨ��
  int Pos = 1;
  for(int Port = 1; Port < ComCount; Port++){
    if(FindSerial(Port, Port) >= 0){//�ҵ���
      cbComId->insertItem(Pos,"COM" + QString::number(Port), Port);
      Pos++;
    }
  }
  //�������
  cbComId->setEnabled(true);
  if(bnComRefresh != NULL) bnComRefresh->setEnabled(true);
}

//----------------------------���ڱ�����ѡ��------------------------
void TabDialog::ComSelChanged()
{
  if(!cbComId->isEnabled()) return; //������

  if(this->serial != NULL){//�ѹҽ�ʱ
    eCommState = eCommIdie;  //ǿ�ƹر�ͨѶ(״̬���ᱨ��ʱ)

    delete(this->serial);//�ر�
    this->serial = NULL;
  }

  //���³�ʼ��
  if(InitSerial(IsCommNote) == true){
    //��Чʱ��������ѡ��
    QString Com = cbComId->currentText().right(cbComId->currentText().count() - 3);
    settings->setValue("LastCom", Com);
    cbComId->setToolTip(tr("�ϴ���Чѡ��COM") + QString::number(LastCom));
    LastCom = Com.toInt(); //���±���
    //������ذ�ť
    if(!leEdit->text().isEmpty()) bnSendEditStr->setEnabled(true);
    if(!leBatFile->text().isEmpty()) bnSendBatStr->setEnabled(true);
  }
  else{
    cbComId->setToolTip(tr("�ϴ���Чѡ��COM") + QString::number(LastCom));

    //�ر���ذ�ť
    bnSendEditStr->setDisabled(true);
    bnSendBatStr->setDisabled(true);
  }
  //�ر�ʱ������ˢ��
  if(bnComRefresh != NULL){
    if(cbComId->currentIndex() == 0) 
      bnComRefresh->setEnabled(true);
    else bnComRefresh->setDisabled(true);
  }
}

//----------------------------���ڷ������ݴ���-----------------------
void TabDialog::SerialRcv()
{
  //�쳣
  if(eCommState != eRdData){
    eCommState = eCommIdie;//�ָ�
    return;
  }

  if(!RdDataDelay){//���жϻ���δ����������ʱ����,���ݽ������ݳ���,������ʱ
    int Len = SendCount + 7;
    RdDataDelay = Len * 4;
    if(RdDataDelay < 40) RdDataDelay *= 8;
    timer->stop();  //ֹͣ��ʱ��
    timer->setInterval(RdDataDelay);
    timer->start();//��������ʱ��
    return;
  }
  
  if(RdDataDelay != -1){//�ȴ��м��ж���Ч
    return;
  }
  
  RdDataDelay = 0;
  timer->stop();  //ֹͣ��ʱ��

  //���ط��ص�����
  memset(RcvDataBuf, 0, RCV_BUF_SIZE);//����ջ�������ֹһ����δ����
  RcvCount = serial->readLine((char*)RcvDataBuf,RCV_BUF_SIZE);

  //У�������Ƿ���ȷ����������
  unsigned short CRC16 = GetCRC16(RcvDataBuf, RcvCount - 2);
  //��λ��ǰ,��λ�ں�
  unsigned short DataCRC16 = ((unsigned short)RcvDataBuf[RcvCount - 2] << 8) + RcvDataBuf[RcvCount - 1];
  //��λ��ǰ,�͸�λ�ں�
  unsigned short DataCRC16A = ((unsigned short)RcvDataBuf[RcvCount - 1] << 8) + RcvDataBuf[RcvCount - 2];

  int DataValid = 0;
  if((CRC16 != DataCRC16) && (CRC16 != DataCRC16A)) //����У�����Ϊ��ȷ
    DataValid = -1;
  //����ַ�Ƿ���ȷ
  if(RcvDataBuf[0] != CommCurAdr)
    DataValid = -2;

  RcvEndPro(DataValid);//���ս�������
}

//----------------------------��д���ݳ�ʱ����------------------------
void TabDialog::SerialOV()
{
  timer->stop();  //ֹͣ��ʱ��
  //�쳣
  if(eCommState != eRdData){
    eCommState = eCommIdie;//�ָ�
    return;
  }
  
  //==============================�����ݳ�ʱ����=========================
  if(RdDataDelay){ //��������ʱ������ʱ��ʽ��ȡ
    timer->setInterval(200); //�ָ�Ĭ��
    RdDataDelay = -1;//��֮�����
    SerialRcv();
    return;
  }
  RcvEndPro(10);//��ʱ����
}

//----------------------------���ս�������------------------------
void TabDialog::RcvEndPro(int DataInvalid)//������Ч״̬,0��Ч
{
  eCommState = eCommIdie; //����ͨѶ���
  QString Note;
  if(cbLogRead->isChecked()){//������ʱ
    Note.append(LastSendStr + ", ");
  }
  if(DataInvalid){//��Ӧ���У�����ʱ
    if(DataInvalid < 0){
      Note.append(tr("��������У�����"));
      CrcErrCount++;
    }
    else{
      Note.append(tr("������Ӧ��"));
      UnAskCount++;
    }
  }
  else{//�������ݴ���
    if(RcvCount <= (RcvStrStartPos + (1+2))){//��ַ1+�û���+У����
      Note.append(tr("�������ݹ��̻��쳣"));
      DataShortCount++;
    }
    else{
      QTextCodec *codec = QTextCodec::codecForLocale();
      if(codec == NULL){
        Note.append(tr("����ʱ���ַ�����ϵͳ����"));
      }
      else{
        RcvDataBuf[RcvCount - 2] = '\0';//ǿ�Ƽӽ����ַ�
        QString RcvData = codec->toUnicode(QByteArray((char*)&RcvDataBuf[1 + RcvStrStartPos]));
        Note.append(RcvData);
      }
    }
  }
  MsgNote(Note, -1); //������ǿ�Ƶ�log
  ValidCount++;
  if(SendLineCount < 0) return; //����ʱ���

  //����ʱ
  if(waitStop == true){//ǿ��ֹͣ
    StopBatSend(true);
    return; 
  }
  //׼����һ����
  QString CurString;
  do{
    if(tStream->atEnd()){//������ʽ�����
      StopBatSend(false);
      return;
    }
    CurString = tStream->readLine();//��ȡ
    if((CurString[0] == '/') && (CurString[0] == '/')) continue;//����ע�����
    break;
  }while(1);
  if(SendString(CurString)){//��������ʧ��ֱ���˳�
    StopBatSend(true);
  }
  SendLineCount++;//��������
}

//----------------------------��ֹ�������ļ��ķ���------------------------
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
  //�ָ�������ذ�ť
  bnSendEditStr->setEnabled(true);
  cbComId->setEnabled(true);
  bnLoadBatFile->setEnabled(true);
  bnSendBatStr->setText(tr("����"));

  //�������ɱ���
  QString EndNote;
  if(IsForceStop) EndNote.append(tr("������ָ�ͨѶ����ֹ��\n"));
  else  EndNote.append(tr("������ָ�ͨѶ����ɣ�\n"));
  EndNote.append(tr("   ���������� ") + QString::number(SendLineCount) + tr(" �� \n"));
  EndNote.append(tr("   �յ���Ч���� ") + QString::number(ValidCount) + tr(" �� \n"));
  if(DataShortCount) EndNote.append(tr("  �յ��������� ") + QString::number(DataShortCount) + tr(" �� \n"));
  if(UnAskCount) EndNote.append(tr("     ��Ӧ������ ") + QString::number(UnAskCount) + tr(" �� \n"));
  if(CrcErrCount) EndNote.append(tr("     �յ��������� ") + QString::number(CrcErrCount) + tr(" �� \n"));
  QMessageBox *msgBox = new QMessageBox();
  msgBox->setText(EndNote);
  msgBox->exec();
}

//----------------------------��䷢�ͻ���������ͷ-------------------------
//�ɵ�ַ+������+���ܵ��������
//����������ݸ���,<=0�쳣
int TabDialog::FullSendBufHeader()
{
  unsigned char *pWrBuf = SendDataBuf;
  *pWrBuf++ = CommCurAdr; //��ַ
  QString Data = cbFunId->itemData(cbFunId->currentIndex()).toString();
  if(Data.isNull()) {MsgNote(tr("���ͽ������� û�ж���Modbus�����룬���ܱ�ִ�У�")); return -1; }
  QStringList List = Data.split('|'); //=����ǹ����뼰�����������ݣ��ұ���������Ϣ
  //���������Ϣ
  bool Ok;
  if(List.count() >= 2)
    RcvStrStartPos = List[1].trimmed().toInt(&Ok); //ָ��
  else RcvStrStartPos = 1; //����������

  //���������
  QStringList ParaList = List[0].split(','); //"\\s+"�ɺ��Զ���ո�,��Ϊ,����������
  int Count = ParaList.count();
  if(Count > 10) {
    QString Note = tr("���ͽ������� ����������") +  QString::number(10) +tr("������!" );
    MsgNote(Note); 
    return - 2;
  }
 //������������
  for(int Pos = 0; Pos < Count; Pos++){
    QString Para = ParaList[Pos].trimmed();//ȥǰ��ո�
    unsigned char CurData = 0;
    //ʮ�����Ʒ�ʽ��ȡ
    if((Para[0] == '0') && ((Para[1] == 'x') || (Para[1] == 'X'))){
      CurData = Para.toInt(&Ok, 16);
    }
    else if((Para[0] >= '0') && (Para[0] <= '9')){//ʮ����
      CurData = Para.toInt(&Ok);
    }
    else{//ֻ����ֱ������
      QString Note = tr("���ͽ������󣺵�") +  QString::number(Pos) +tr("�����ݸ�ʽ����֧�֣�" );
      MsgNote(Note); 
      return - 3;
    }
    if(Ok == false){
      QString Note = tr("���ͽ������󣺵�") +  QString::number(Pos) +tr("�����ݱ������" );
      MsgNote(Note); 
      return -4;
    }
    *pWrBuf++ = CurData;
  } //end for

  return pWrBuf - SendDataBuf;
}

//----------------------------�����ַ���----------------------------
//����0��ȷ��䲢��������������Ϊ��������
int TabDialog::SendString(QString &Str)
{
  int HeaderLen = FullSendBufHeader();
  if(HeaderLen <= 0) return HeaderLen;
  
  //QString�洢����unicode�룬��Ҫת��ΪGB2312��
  QTextCodec *codec = QTextCodec::codecForLocale();
  if(codec == NULL){
    MsgNote(tr("����ʱ���ַ�����ϵͳ����!")); 
    return -10;
  }
  QByteArray string = codec->fromUnicode(Str);
  int StrLen = string.length();
  if(StrLen >= ((SEND_BUF_SIZE - 2) - HeaderLen)){
    MsgNote(tr("����ʱ������û��ַ�������!")); 
    return - 11;
  }
  qstrcpy((char*)&SendDataBuf[HeaderLen], string);
  LastSendStr = Str; //����������ַ���
  FullSendDataBuf(HeaderLen + StrLen);
  return 0;
}

//--------------------------��仺��������------------------------
void TabDialog::FullSendDataBuf(unsigned short Len)
{
  //׼��У����
  unsigned short CRC16 = GetCRC16(SendDataBuf, Len);
  #ifndef CRC16_ANTI//��λ��ǰ,��λ�ں�
    SendDataBuf[Len++] = (unsigned char)(CRC16 >> 8);
    SendDataBuf[Len++] = (unsigned char)(CRC16);
  #else//��λ��ǰ,�͸�λ�ں�
    SendDataBuf[Len++] = (unsigned char)(CRC16);
    SendDataBuf[Len++] = (unsigned char)(CRC16 >> 8);
  #endif

  SendCount = Len;
  RcvCount = 0;
  eCommState = eRdData;
  RdDataDelay = 0;//��δ��ʱ
  serial->readLine((char*)RcvDataBuf,RCV_BUF_SIZE); //����ǰ����������ս���
  serial->write((char*)SendDataBuf,Len);
  timer->setInterval(Len + 200); //д������ȴ�ʱ��
  timer->start();//������ʱ��
}

//----------------------------���ͱ༭���ַ�------------------------
void TabDialog::SendEditStr()
{
  if(SendString(leEdit->text())) return;
  SendLineCount = -1;//����״̬
}

//------------------------�����������ļ��е��ַ�--------------------
void TabDialog::SendBatStr()
{
  //������ʱ
  if(bnSendBatStr->text() == tr("ֹͣ")){
    waitStop = true;
    bnSendBatStr->setText(tr("����"));
    return;
  }
  else if(waitStop == true) return; //ֹͣδ����

  //�������� 
  file = new QFile(leBatFile->text());
  if(!file->open(QIODevice::ReadOnly)){
    MsgNote(tr("�������ļ���ʧ��!"));
    return;
  }
  tStream = new QTextStream(file);
  QString CurString;
  do{
    if(tStream->atEnd()){
      MsgNote(tr("�������ļ�����������Ҫ����!"));
      return;
    }
    CurString = tStream->readLine();//��ȡ
    if((CurString[0] == '/') && (CurString[0] == '/')) continue;//����ע�����
    break;
  }while(1);
  
  if(SendString(CurString)) return; //��������ʧ��ֱ���˳�

  //��ʼ��������
  SendLineCount = 1;
  UnAskCount = 0;         //����ʱ����Ӧ�����
  CrcErrCount = 0;        //����ʱ��У��������
  DataShortCount = 0;     //����ʱ�����ݹ��̴���
  ValidCount = 0;         //��Ч���ݴ���

  //�ر���ذ�ť
  bnSendEditStr->setDisabled(true);
  cbComId->setDisabled(true);
  bnLoadBatFile->setDisabled(true);
  bnSendBatStr->setText(tr("ֹͣ"));
}






