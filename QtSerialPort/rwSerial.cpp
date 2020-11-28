#include "rwSerial.h"
#include <iostream>
#include <QtCore/QTime>
#include <QtCore/QThread>
#include <Windows.h>
#include <QtWidgets/QMessageBox>

RWserial::RWserial(QObject *parent):
	m_swap('\n'), synchronizationFlag(false)
{
	rw_readData.clear();
}

RWserial::~RWserial()
{
	if (rw_serialPort)
	{
		safeDeleteSerial(rw_serialPort);
	}
	if (m_pReadTimer) {
		safeStopTimer(m_pReadTimer);
	}
}

/**
* @brief		主界面单击连接按钮触发
* @return		void
*/
void RWserial::openSerialPort(QString com, int baud, int dateBits, int parity, int stopBits, int flowControl)
{
	if (rw_serialPort)
	{
		safeDeleteSerial(rw_serialPort);
	}
	rw_serialPort = new QSerialPort(this);
	rw_serialPort->setPortName(com);									//设置串口号
	//baudRate>115200 需要修改qserialport.h
		/*enum BaudRate里面添加
		Baud128000 = 128000,
		Baud256000 = 256000,
		Baud460800 = 460800,
		Baud961200 = 961200,
		Baud1382400 = 1382400,*/
	rw_serialPort->setBaudRate(baud);									//设置波特率
	rw_serialPort->setDataBits((QSerialPort::DataBits)dateBits);		//数据位 直接转(不严谨)
	rw_serialPort->setParity((QSerialPort::Parity)parity);				//校验位 直接转
	rw_serialPort->setStopBits((QSerialPort::StopBits)stopBits);		//停止位
	rw_serialPort->setFlowControl((QSerialPort::FlowControl)flowControl); //流控制
	if (!rw_serialPort->open(QIODevice::ReadWrite))
	{
		safeDeleteSerial(rw_serialPort);
		emit openSerResult(false);
		return;
	}
	//	readyRead信号
	//connect(rw_serialPort, &QSerialPort::readyRead, this, &RWserial::readSerialAsyn, Qt::UniqueConnection);
	//	异步读写方式2：写两个槽函数，一次读取50ms数据，解决读取数据不完整，serach QT serial example for detial
	m_pReadTimer = new QTimer();
	connect(m_pReadTimer, &QTimer::timeout, this, &RWserial::handleTimeoutSig, Qt::UniqueConnection);
	connect(rw_serialPort, &QSerialPort::readyRead, this, &RWserial::handleReadyReadSig, Qt::UniqueConnection);
	//	处理错误信息 (Qt5.8 以后可用)
	connect(rw_serialPort, &QSerialPort::errorOccurred, this, &RWserial::handleErrorOccurred, Qt::UniqueConnection);
	emit openSerResult(true);
	return;
}

void RWserial::closeSerialPort()
{
	if (rw_serialPort)
	{
		safeDeleteSerial(rw_serialPort);
	}
	if (m_pReadTimer) {
		safeStopTimer(m_pReadTimer);
	}
	return;
}



Q_SLOT void RWserial::handleErrorOccurred(QSerialPort::SerialPortError error)
{
	if (error == QSerialPort::ResourceError) {
		emit serialPortErrorSignal(rw_serialPort->errorString());
	}
}

void RWserial::clearBuffer() {
	rw_serialPort->clear();
}

void RWserial::loopPosStart()
{
	//std::cout << "loopPosStart: \n";
	if (loopTimer)
	{
		safeStopTimer(loopTimer);
	}
	loopTimer = new QTimer(this);
	// 多条命令异步读写最好加非阻塞延时，否则发送指令太快，有可能读不过来，读取顺序会乱
	connect(loopTimer, &QTimer::timeout, this, [=] {
		readSerialData("1POS\r", true);
		/*QTimer::singleShot(50, this, [&] {
			readSerialData("2POS\r", false);
		});*/
	});
	//connect(loopTimer, &QTimer::timeout, this, [=] {
	//	readSerialData("1POS\r", true);
	//	readSerialData("2POS\r", true);
	//});
	loopTimer->start(40);
}

void RWserial::loopPosStop() {
	//std::cout << "loopPosStop: \n";
	if (loopTimer)
	{
		safeStopTimer(loopTimer);
	}
}

/**
* @brief			 串口写入数据
* @param writeData	 写入的数据
* @return			 true or false
* @emit	error		 写入错误
*/
// Qt 定时器 阻塞触发容易崩溃
//所以在定时器需要在定时器间隔时间处理完事件，否则容易崩溃。
bool RWserial::writeSerialSyn(const QByteArray &writeData) {
	const qint64 bytesWritten = rw_serialPort->write(writeData);
	// 顺序很重要，时间间隔也很重要
	if (!rw_serialPort->waitForBytesWritten(30)) {
		emit timeout(QString("Wait write response timeout"));
		rw_serialPort->clear();
		return false;
	}
	else if (bytesWritten == -1) {
		emit error(QString("Failed to write the data to port, error: %1")
			.arg(rw_serialPort->errorString()));
		return false;
	}
	else if (bytesWritten != strlen(writeData)) {
		emit error(QString("Failed to write all the data to port, error: %1")
			.arg(rw_serialPort->errorString()));
		return false;
	}
	return true;
}

/**
* @brief			 串口读取数据
*/
void RWserial::readSerialData(const QByteArray &sendData, bool synFlag) {
	QMutexLocker locker(&mutex);
	//std::cout << "I'm readDataSyn in slavethread: " << QThread::currentThreadId()
	//	<< "--with send data--" << qPrintable(sendData)  << "\n";
	synchronizationFlag = synFlag;
	switch (synchronizationFlag) {
	case true:
		readSerialSyn(sendData);
		break;
	case false:
		// 针对循环异步发送，如果数据还没读完。
		if (!rw_readData.isEmpty()) {
			if (m_pReadTimer->isActive()) {
				m_pReadTimer->stop();//启动定时器，接收25毫秒数据（根据情况设定）
			}
			emit this->responseDataReady(NULL, rw_readData);
		}
		rw_readData.clear();
		if (writeSerialSyn(sendData)) {
			emit this->responseDataReady(sendData, NULL);
			// 异步信号槽实现同步的效果。
			//QTimer::singleShot(27, this, [=] {
			//	emit this->responseDataReady(sendData, rw_serialPort->readAll());
			//});
		}
		break;

	}
}

/**
* @brief			 同步读写串口
* @param sendData	 写入的数据
* @return			 读取的数据
* @emit	responseDataReady	写入，和读取的数据
		timeout				读写超时
		error				写入错误
*/
void RWserial::readSerialSyn(const QByteArray &sendData) {
	if (!synchronizationFlag) {
		return;
	}
	if (!writeSerialSyn(sendData)) {
		//goto emitNull;
		return;
	}
	if (rw_serialPort->waitForReadyRead(30)) {
		responseData = rw_serialPort->readAll();
		//std::cout << "串口数据个数：" << qPrintable(responseData) << "--" << responseData.size() << std::endl;
		//while (rw_serialPort->waitForReadyRead(5) || (!responseData.endsWith("\n")))
		while (rw_serialPort->waitForReadyRead(17))
		{
			responseData.append(rw_serialPort->readAll());
		}
		emit this->responseDataReady(sendData, responseData);
		responseData.clear();
		return;
	}

	else {
		emit timeout(QString("Wait read response timeout"));
		emit this->responseDataReady(sendData, NULL);
		return;
	}
	//当代码顺序执行到标签处/此处时,也会执行
//emitNull:
	//emit this->responseDataReady(sendData, NULL);
	//return NULL;
}


/**
* @brief			 异步读串口
*/
void RWserial::handleReadyReadSig()
{
	if (synchronizationFlag) {
		return;
	}
	rw_readData.append(rw_serialPort->readAll());
	if (!m_pReadTimer->isActive()) {
		m_pReadTimer->start(30);//启动定时器，接收25毫秒数据（根据情况设定）
	}
}

void RWserial::handleTimeoutSig()
{
	m_pReadTimer->stop();
	if (!rw_readData.isEmpty()) {
		emit this->responseDataReady(NULL, rw_readData);
	}
	rw_readData.clear();
}

// 也可以根据读的终止数据位做处理
void RWserial::readSerialAsyn()
{
	if (synchronizationFlag) {
		return;
	}
	rw_readData.append(rw_serialPort->readAll());
	if (rw_readData.endsWith('\n'))//只有等到\n的时候才能进入
	{
		emit this->responseDataReady(NULL, rw_readData);
		rw_readData.clear();
	}
	//想确保rw_readData怎么都不出错，做一些字符串处理
	//if (!rw_readData.contains('\n'))//只有等到\n的时候才能进入
	//{
	//	return;
	//}
	//if (rw_readData.count('\n') == 1 && rw_readData.endsWith('\n')) {
	//	emit this->responseDataReady(NULL, rw_readData);
	//	rw_readData.clear();
	//	return;
	//}
	//else {
	//	rw_readDataList.clear();
	//	rw_readDataList = rw_readData.split(m_swap);
	//	int list_size = rw_readDataList.size();
	//	for (int i = 0; i < (list_size - 1); ++i)
	//	{
	//		rw_ListAt = rw_readDataList.at(i);
	//		//std::cout << "Send queue QByteArrayList--" << qPrintable(tmp_str) <<std::endl;
	//		int len = rw_ListAt.length();//取字符长度
	//		rw_ListAt[len++] = 0x0a;// \n
	//		emit this->responseDataReady(NULL, rw_ListAt);
	//		rw_ListAt.clear();
	//	}
	//	rw_readData = rw_readDataList.last();
	//	return;
	//}
}
//void RWserial::readSerialAsyn()
//{
//	if (synchronizationFlag) {
//		return;
//	}
//	rw_readData.append(rw_serialPort->readAll());
//	//int nIndex = rw_readData.indexOf("\n");
//	//if (nIndex == -1)//只有等到\n的时候才能进入
//	//{
//	//	return;
//	//}
//	while (rw_readData.indexOf("\n") != -1) {
//		int nIndex = rw_readData.indexOf("\n");
//		const QByteArray str_target = rw_readData.left(nIndex + 1);// 这儿是取前nIndex位
//		rw_readData.remove(0, nIndex + 1);
//		if (rw_sendQueue.isEmpty()) {
//			//std::cout << "Send queue empty--" << std::endl;
//			emit error(QString("Send queue empty, error: %1")
//				.arg(rw_serialPort->errorString()));
//			return;
//		}
//		emit this->responseDataReady(rw_sendQueue.takeFirst(), str_target);
//	}
//}