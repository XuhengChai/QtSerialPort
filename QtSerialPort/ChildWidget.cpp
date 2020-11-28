#pragma once
#include "ChildWidget.h"
#include <QtSerialPort/QSerialPortInfo>

ChildWidget::ChildWidget(QWidget *parent)
	: QMainWindow(parent), m_curSendOpt(EnmCurSendOption::NoneSend), m_bSptNote(true),
	m_bSaveLog(false)
	
{
	initViewer();
	//m_bSaveLog = false;
	//m_bSptNote = true,
	setWindowFlags(Qt::FramelessWindowHint);
}

ChildWidget::~ChildWidget()
{
	if (m_rwSerThread.isRunning())
	{
		m_rwSerThread.quit();
		m_rwSerThread.wait();
	}
	if (m_rwserial) {
		m_rwserial->deleteLater();
	}

	if (m_pScanTimer) {
		safeStopTimer(m_pScanTimer);
	}

}

/**
* @brief		初始化各种信息
* @return		void
*/
void ChildWidget::initViewer()
{
	initExapndWgt();
	initCenWgt();
	initStatusBar();
	initPortSettingList();
	initConnect();

	//设置接收框字体
	QFont font;
	font.setPointSize(11);
	font.setBold(true);
	font.setFamily(QStringLiteral("SimSun"));
	this->m_pEditMsg->setFont(font);

	//定时扫描和更新串口
	
	m_pScanTimer = new QTimer;
	connect(m_pScanTimer, &QTimer::timeout, this, &ChildWidget::updatePortList);//实时更新端口号
	m_pScanTimer->start(1000); //每1000ms定时检测串口状态
	
}

/**
* @brief			初始化窗体设置
* @return			void
*/
void ChildWidget::initCenWgt()
{
	m_pCenWidget = new QWidget;

	QGroupBox *gboxPortSet = new QGroupBox(QStringLiteral("串口设置"));
	QGroupBox *gboxReceiveSet = new QGroupBox(QStringLiteral("接收设置"));
	QGroupBox *gboxSendSet = new QGroupBox(QStringLiteral("发送设置"));

	//串口设置框
	m_pLabCommList = new QLabel(QStringLiteral("串  口:"));
	m_pLabBaudRate = new QLabel(QStringLiteral("波特率:"));
	m_pCmbCommList = new QComboBox();
	m_pCmbBaudRate = new QComboBox();
	m_pBtnVisible = new QPushButton(QStringLiteral("展开设置"));
	m_pBtnVisible->setCursor(Qt::PointingHandCursor);
	m_pBtnVisible->setStyleSheet("QPushButton{color: rgb(0, 170, 255);border: 0px;\
								text-decoration:underline;}:hover{color: Gray;}:pressed{color: darkGray;}");
	m_pLabDataBits = new QLabel(QStringLiteral("数据位:"));
	m_pLabParity = new QLabel(QStringLiteral("校验位:"));
	m_pLabStopBits = new QLabel(QStringLiteral("停止位:"));
	m_pLabFlowControl = new QLabel(QStringLiteral("流  控:"));
	m_pCmbDataBits = new QComboBox();
	m_pCmbParity = new QComboBox();
	m_pCmbStopBits = new QComboBox();
	m_pCmbFlowControl = new QComboBox();
	m_pRadbtnAsync = new QRadioButton(QStringLiteral("Read Async"));
	m_pRadbtnSync = new QRadioButton(QStringLiteral("Read Sync"));
	m_pRadbtnSync->setChecked(true);
	QHBoxLayout *syncLayout = new QHBoxLayout;
	syncLayout->addWidget(m_pRadbtnAsync);
	syncLayout->addWidget(m_pRadbtnSync);

	m_pBtnOpenClose = new QPushButton(QStringLiteral("打开串口"));

	QGridLayout *portSetLayout = new QGridLayout();
	portSetLayout->addWidget(m_pLabCommList, 0, 0);
	portSetLayout->addWidget(m_pCmbCommList, 0, 1);
	portSetLayout->addWidget(m_pLabBaudRate, 1, 0);
	portSetLayout->addWidget(m_pCmbBaudRate, 1, 1);

	portSetLayout->addWidget(m_pBtnVisible, 2, 0);
	int itemp = 3;
	portSetLayout->addWidget(m_pLabDataBits, itemp, 0);
	portSetLayout->addWidget(m_pCmbDataBits, itemp, 1);
	portSetLayout->addWidget(m_pLabParity, itemp + 1, 0);
	portSetLayout->addWidget(m_pCmbParity, itemp + 1, 1);
	portSetLayout->addWidget(m_pLabStopBits, itemp + 2, 0);
	portSetLayout->addWidget(m_pCmbStopBits, itemp + 2, 1);
	portSetLayout->addWidget(m_pLabFlowControl, itemp + 3, 0);
	portSetLayout->addWidget(m_pCmbFlowControl, itemp + 3, 1);
	portSetLayout->addLayout(syncLayout, itemp + 4, 0, 1, 2);
	portSetLayout->addWidget(m_pBtnOpenClose, itemp + 5, 0, 1, 2);

	bool tempVisible = false;
	m_pLabDataBits->setVisible(tempVisible);
	m_pLabParity->setVisible(tempVisible);
	m_pLabStopBits->setVisible(tempVisible);
	m_pLabFlowControl->setVisible(tempVisible);
	m_pCmbDataBits->setVisible(tempVisible);
	m_pCmbParity->setVisible(tempVisible);
	m_pCmbStopBits->setVisible(tempVisible);
	m_pCmbFlowControl->setVisible(tempVisible);
	m_pRadbtnAsync->setVisible(tempVisible);
	m_pRadbtnSync->setVisible(tempVisible);

	gboxPortSet->setLayout(portSetLayout);

	//接收设置框
	m_pRadbtnRecASCII = new QRadioButton(QStringLiteral("ASCII"));
	m_pRadbtnRecASCII->setChecked(true);
	m_pRadbtnRecHex = new QRadioButton(QStringLiteral("Hex"));
	m_pChkShowMsg = new QCheckBox(QStringLiteral("停止消息显示"));
	m_pBtnClear = new QPushButton(QStringLiteral("清除接收数据"));

	QGridLayout *recSetLayout = new QGridLayout;
	recSetLayout->addWidget(m_pRadbtnRecASCII, 0, 0);
	recSetLayout->addWidget(m_pRadbtnRecHex, 0, 1);
	recSetLayout->addWidget(m_pChkShowMsg, 1, 0);
	recSetLayout->addWidget(m_pBtnClear, 2, 0);
	gboxReceiveSet->setLayout(recSetLayout);

	//发送设置
	m_pRadbtnSendASCII = new QRadioButton(QStringLiteral("ASCII"));
	m_pRadbtnSendASCII->setChecked(true);
	m_pRadbtnSendHex = new QRadioButton(QStringLiteral("Hex"));
	m_pChkWordwrap = new QCheckBox(QStringLiteral("自动换行\\r\\n"));
	m_pChkWordwrap->setChecked(true);

	m_pBtnExpand = new QPushButton(QStringLiteral("扩展命令"));
	m_pBtnExpand->setCursor(Qt::PointingHandCursor);
	m_pBtnExpand->setStyleSheet("QPushButton{color: rgb(0, 170, 255);border: 0px;\
								text-decoration:underline;}:hover{color: Gray;}:pressed{color: darkGray;}");
	m_pChkRepeatSend = new QCheckBox(QStringLiteral("重复发送 ms"));
	m_pSpboxTimes = new QSpinBox;
	m_pSpboxTimes->setMaximum(INT_MAX);
	m_pSpboxTimes->setValue(1000);
	QGridLayout *sendSetLayout = new QGridLayout;
	sendSetLayout->addWidget(m_pRadbtnSendASCII, 0, 0);
	sendSetLayout->addWidget(m_pRadbtnSendHex, 0, 1);
	sendSetLayout->addWidget(m_pChkWordwrap, 1, 0);
	sendSetLayout->addWidget(m_pBtnExpand, 1, 1);
	sendSetLayout->addWidget(m_pChkRepeatSend, 2, 0);
	sendSetLayout->addWidget(m_pSpboxTimes, 2, 1);
	gboxSendSet->setLayout(sendSetLayout);

	//左边的布局
	QVBoxLayout *leftLayout = new QVBoxLayout();
	leftLayout->addWidget(gboxPortSet);
	//leftLayout->addWidget(m_pTreeWgtClient);
	leftLayout->addWidget(gboxReceiveSet);
	leftLayout->addWidget(gboxSendSet);

	leftLayout->addStretch(1);

	//用一个Widget 包含进去，方便进行隐藏显示
	m_pLeftSetWgt = new QWidget(this);
	m_pLeftSetWgt->setLayout(leftLayout);
	m_pLeftSetWgt->setMaximumWidth(220);

	//右边的布局
	m_pEditMsg = new QTextEdit;
	//m_pEditMsg->document()->setMaximumBlockCount(20);
	maxMsgRow = 300;
	m_pBtnSend = new QPushButton(QStringLiteral("发送"));
	//m_pBtnSend->setEnabled(false);
	m_pEditSend = new QTextEdit;
	m_pCmbHistory = new QComboBox;

	QGridLayout *rightLayout = new QGridLayout();
	rightLayout->addWidget(m_pEditMsg, 0, 0, 1, 2);
	rightLayout->addWidget(m_pEditSend, 1, 0);
	rightLayout->addWidget(m_pBtnSend, 1, 1);
	rightLayout->addWidget(m_pExpandScroll,2,0,1,2);
	m_pExpandScroll->setVisible(false);
	rightLayout->addWidget(m_pCmbHistory, 3, 0, 1, 2);

	//size设置
	//m_pCmbCommList->setMinimumWidth(130);
	m_pEditMsg->setMinimumWidth(350);
	m_pChkWordwrap->setMinimumWidth(100);
	m_pBtnVisible->setMaximumWidth(50);
	m_pEditSend->setMaximumHeight(70);
	//m_pEditSend->setText("1POS");


	//总布局
	QHBoxLayout *mainLayout = new QHBoxLayout();
	mainLayout->addWidget(m_pLeftSetWgt);
	mainLayout->addLayout(rightLayout);
	m_pCenWidget->setLayout(mainLayout);
	this->setCentralWidget(m_pCenWidget);
}

/**
* @brief			扩展命令窗口
*/
void ChildWidget::initExapndWgt() {
	QWidget *m_pExpandWidget = new QWidget;
	QGridLayout *layout = new QGridLayout();
	//初始化 signalMapper
	signalMapper = new QSignalMapper(this);
	signalMapperHex = new QSignalMapper(this);
	layout->addWidget(new QLabel("Hex"), 0, 0, Qt::AlignHCenter);
	layout->addWidget(new QLabel(QStringLiteral("字符串")), 0, 1, Qt::AlignHCenter);
	layout->addWidget(new QLabel("\\r\\n"), 0, 2, Qt::AlignHCenter);
	layout->addWidget(new QLabel(QStringLiteral("发送")), 0, 3, Qt::AlignHCenter);
	for (int i = 0; i<60; i++)
	{
		m_exCheckHex.append(new QCheckBox);
		m_exLineEdit.append(new QLineEdit);
		m_exCheckWrap.append(new QCheckBox);
		m_exBtnSend.append(new QPushButton(QString::number(i + 1)));
		layout->addWidget(m_exCheckHex.at(i), i + 1, 0, Qt::AlignHCenter);
		layout->addWidget(m_exLineEdit.at(i), i + 1, 1);
		layout->addWidget(m_exCheckWrap.at(i), i + 1, 2, Qt::AlignHCenter);
		layout->addWidget(m_exBtnSend.at(i), i + 1, 3);
		//原始信号传递给signalmapper
		connect(m_exBtnSend.at(i), &QPushButton::clicked, signalMapper, static_cast<void(QSignalMapper::*)()>(&QSignalMapper::map));
		signalMapper->setMapping(m_exBtnSend.at(i), i);//设置signalmapper的转发规则, 转发为参数为int类型的信号
		connect(m_exCheckHex.at(i), SIGNAL(stateChanged(int)), signalMapperHex, SLOT(map()));
		signalMapperHex->setMapping(m_exCheckHex.at(i), i);
	}
	layout->setHorizontalSpacing(10);
	layout->setVerticalSpacing(10);
	layout->setContentsMargins(10, 10, 10, 10);
	//Connect signalmapper Slot
	connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(handleSignalMapper(int)));
	connect(signalMapperHex, SIGNAL(mapped(int)), this, SLOT(handleSignalMapperHex(int)));

	m_pExpandWidget->setLayout(layout);
	m_pExpandScroll = new QScrollArea;
	m_pExpandScroll->setWidget(m_pExpandWidget);
	m_pExpandScroll->setWidgetResizable(true);
}

/**
* @brief			扩展命令槽函数
*/
Q_SLOT void ChildWidget::handleSignalMapper(int i)
{
	if (!int(m_curSendOpt))
	{
		showHintLabel(QStringLiteral("请打开串口！"), this);
		return;
	}
	if (this->m_exLineEdit.at(i)->text().isEmpty())  //QTextEdit获取文本的函数
	{
		this->m_exLineEdit.at(i)->setFocus();
		return;
	}
	QByteArray strData = m_exLineEdit.at(i)->text().toLatin1(); //toUtf8() = toLatin1()
	QByteArray hexData;
	//发送方式是否为16进制发送
	if (m_exCheckHex.at(i)->isChecked())
	{
		hexData = strData;
		strData = hexToString(hexData);
	}
	//windows回车换行
	if (m_exCheckWrap.at(i)->isChecked())
	{
		int len = strData.length();//取字符长度
		strData[len++] = 0x0d;// \r
		strData[len++] = 0x0a;// \n
	}
	QTextCodec *tc = QTextCodec::codecForName("GBK");	//处理中文乱码问题
	const QByteArray send_text = tc->fromUnicode(strData);
	serialWrite(send_text);
}

/**
* @brief			扩展命令槽函数
*/
void ChildWidget::handleSignalMapperHex(int i)
{
	QString strData = m_exLineEdit.at(i)->text();
	if (strData.isEmpty())  //QTextEdit获取文本的函数
	{
		return;
	}
	if (m_exCheckHex.at(i)->isChecked())
	{
		QByteArray byteData = stringToHex(strData.toLatin1());
		m_exLineEdit.at(i)->setText(byteData);
	}
	else
	{
		QByteArray byteData = hexToString(strData);
		m_exLineEdit.at(i)->setText(byteData);
	}
}

/**
* @brief			初始化状态栏
* @return			void
*/
void ChildWidget::initStatusBar()
{
	m_pLabConnInfo = new QLabel(QStringLiteral("串口未连接"));
	m_pLabRx = new QLabel(QStringLiteral("Rx:0 Bytes"));
	m_pLabTx = new QLabel(QStringLiteral("Tx:0 Bytes"));
	//m_pLabClearZero = new QPushButton(QString::fromLocal8Bit("<span style=\" text-decoration: underline;\">计数清0</span>"));
	//m_pLabClearZero->setOpenExternalLinks(true);
	m_pLabClearZero = new QPushButton(QStringLiteral("计数清0"));
	m_pLabClearZero->setCursor(Qt::PointingHandCursor);
	m_pLabClearZero->setStyleSheet("QPushButton{color: rgb(0, 170, 255);border: 0px;\
								text-decoration:underline;}:hover{color: Gray;}:pressed{color: darkGray;}");
	connect(m_pLabClearZero, &QPushButton::clicked, this, [=] { updateRxTxInfo(EnmUpdateRxTx::ClearRxTx); });

	QStatusBar *statusBar = this->statusBar();
	statusBar->addWidget(m_pLabConnInfo, 1);
	statusBar->addWidget(m_pLabRx, 1);
	statusBar->addWidget(m_pLabTx, 1);
	statusBar->addWidget(m_pLabClearZero, 1);

	//文本设置为红色
	//m_pLabConnInfo->setStyleSheet("color:red");
	this->setLabelTextColor(m_pLabConnInfo, Qt::red);
}

/**
* @brief			初始化串口设置组合框的值
* @return			void
*/
void ChildWidget::initPortSettingList()
{
	//初始化串口的相关选项设置
	// baudRate>115200 需要修改qserialport.h: enum BaudRate
	QStringList baudRateList = { "9600", "19200", "38400", "57600", "115200", "128000", "256000" };
	m_pCmbBaudRate->addItems(baudRateList);
	//m_pCmbBaudRate->setCurrentIndex(4);

	QStringList dataBitList = { "5", "6", "7", "8" };
	m_pCmbDataBits->addItems(dataBitList);
	m_pCmbDataBits->setCurrentIndex(3);

	QStringList parityList = { "None", "Even", "Odd",  "Space", "Mark" };
	m_pCmbParity->addItems(parityList);

	QStringList stopBitList = { "1", "1.5", "2" };
	m_pCmbStopBits->addItems(stopBitList);

	QStringList flowCtrlList = { "None", "Hardware", "Software" };
	m_pCmbFlowControl->addItems(flowCtrlList);

	const auto serialList = QSerialPortInfo::availablePorts();
	if (serialList.isEmpty()) {
		return;
	}
	for (const auto &portInfo : serialList)
	{
		m_pCmbCommList->addItem(portInfo.portName());
	}
}

/**
* @brief			每1000ms定时检测串口状态
* @return			void
*/
void ChildWidget::updatePortList()
{
	QStringList newPortStringList;
	//QList<QSerialPortInfo> serialList = QSerialPortInfo::availablePorts();
	//由于...，doSomethingWith() 可能要修改 serialList，因此加上const
	const auto serialList = QSerialPortInfo::availablePorts();
	int portDiff = serialList.size() - m_pCmbCommList->count();
	if (portDiff)
	{
		/*for (const QSerialPortInfo &portInfo : serialList)
		{
			if (m_pCmbCommList->findText(portInfo.portName()) == -1) {
				m_pCmbCommList->addItem(portInfo.portName());// 如果大于，寻找具体该添加哪个元素
			}
		}*/
		m_pCmbCommList->clear();
		for (const QSerialPortInfo &portInfo : serialList)
		{
		m_pCmbCommList->addItem(portInfo.portName());
		}
		if (portDiff > 0) {
			m_pCmbCommList->showPopup();
		}
	}

	//if (msgLines > msgMaxLines) {
	//	doClearEdit();
	//}
	// 如果小于，寻找具体该删除哪个元素
	//if (portLength < m_pCmbCommList->count())
	//{
	//	std::vector<int> indexIgnore;
	//	Q_FOREACH(QSerialPortInfo portInfo, serialList)
	//	{
	//		int temp_index = m_pCmbCommList->findText(portInfo.portName());
	//		if (temp_index != -1) {
	//			indexIgnore.push_back(temp_index);//添加依旧存在的portName的index
	//		}
	//	}
	//	int temp_index = 0;
	//	for (int i = 0; i < m_pCmbCommList->count(); i++)
	//	{
	//		if (std::find(indexIgnore.begin(), indexIgnore.end(), i) == indexIgnore.end())
	//		{
	//			m_pCmbCommList->removeItem(temp_index);//哪个index在indexIgnore里找不到，就删除
	//		}
	//		else
	//			temp_index += 1;//删除后不加1，只有不删除才加1
	//	}
	//}

}

/**
* @brief			发送串口写入信号
* @param			const char *\QByteArray writeData
//QByteArray can transfer to const char *writeData directly
* @return			void
*/
void ChildWidget::serialWrite(const QByteArray &writeData, bool synFlag)
{
	// false异步读写
	m_pRadbtnAsync->isChecked()? emit sendDataSignal(writeData, false): emit sendDataSignal(writeData, true);
}

void ChildWidget::handleResponse(const QByteArray &send, const QByteArray &read) {
	QByteArray sendtext = "";
	QByteArray readtext = "";
	QString tmpqstring ="";
	bool sendEmpty = send.isEmpty();
	bool readEmpty = read.isEmpty();
	//std::cout << "handleResponse--" << qPrintable(send) << qPrintable(read)  << std::endl;
	if (sendEmpty&&readEmpty) {
		return;
	}
	if (!m_pChkShowMsg->isChecked()) {
		//是否16进制
		m_pRadbtnSendHex->isChecked() ? sendtext = stringToHex(send) : sendtext = send; //16进制发送?
		if (!readEmpty) {
			m_pRadbtnRecHex->isChecked() ? readtext = stringToHex(read) : readtext = read; //16进制接收?
		}
		// fromLocal8Bit 解决中文乱码; 
		//打印到界面，文本 
		//if (readEmpty) {
		//	tmpqstring = QString("Send>> %1\n").arg(QString::fromLocal8Bit(sendtext).simplified());
		//}
		//else if (sendEmpty) {
		//	tmpqstring = QString("Recv<< %2\n").arg(QString::fromLocal8Bit(readtext));
		//}
		//else {
		//	tmpqstring = QString("Send>> %1\nRecv<< %2\n").arg(QString::fromLocal8Bit(sendtext).simplified()).arg(QString::fromLocal8Bit(readtext));
		//}
		//this->m_pEditMsg->insertPlainText(tmpqstring);
		//this->m_pEditMsg->moveCursor(QTextCursor::End);
		//this->m_pEditMsg->append(tmpqstring);//添加一个新行, 有时候会莫名的换行

		//打印到界面，多信息彩色文本 insert Html 
		if (readtext.isEmpty()) {
			tmpqstring = QString("<span style='color:#FF4500;'>Send&gt;&gt;</span>&nbsp;%1<br/>")
				.arg(QString::fromLocal8Bit(sendtext).simplified());
		}
		else if (sendtext.isEmpty()) {
			tmpqstring = QString("<span style='color:orange;'>Recv&lt;&lt;</span>&nbsp;%2<br/><br/>")
				.arg(QString::fromLocal8Bit(readtext).simplified());
		}
		else {
			tmpqstring = QString("<span style='color:#FF4500;'>Send&gt;&gt;</span>&nbsp;%1<br/>\
									   <span style='color:orange;'>Recv&lt;&lt;</span>&nbsp;%2<br/><br/>")
				.arg(QString::fromLocal8Bit(sendtext).simplified()).arg(QString::fromLocal8Bit(readtext).simplified());
		}
		editMsg.append(tmpqstring);
		if (editMsg.size() > maxMsgRow) {
			editMsg.removeFirst();
		}
		this->m_pEditMsg->setUpdatesEnabled(false);
		this->m_pEditMsg->setHtml(editMsg.join(""));//insertHtml极小概率会莫名的换行？
		this->m_pEditMsg->moveCursor(QTextCursor::End);
		this->m_pEditMsg->setUpdatesEnabled(true);
	}
	//打印到日志
	if (!sendEmpty) {
		updateRxTxInfo(EnmUpdateRxTx::AddTx, send.length());	//更新状态栏Tx信息  strData.toLocal8Bit().length()
		appendToComboBox(m_pCmbHistory, send.trimmed());
		if (m_bSaveLog)
		{
			tmpqstring = QString("Send-- %1").arg(QString(sendtext).simplified());
			LogInfo(tmpqstring, m_childLogPath);
		}
		
	}
	if (!readEmpty) {
		updateRxTxInfo(EnmUpdateRxTx::AddRx, read.length());
		//if (read.length() != 6) {
		//	tmpqstring = QString("Recv-- %2").arg(QString(readtext).simplified());
		//	LogInfo(tmpqstring, m_childLogPath);
		//}
		if (m_bSaveLog)
		{
			tmpqstring = QString("Recv-- %2").arg(QString(readtext).simplified());
			LogInfo(tmpqstring, m_childLogPath);
		}
		
	}
}

Q_SLOT void ChildWidget::handleTimeout(const QString &s)
{
	LogWarning(s, m_childLogPath);
	//std::cout << "LogWarning" << qPrintable(s) <<"---" << qPrintable(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd"))<<std::endl;
	if (m_pChkRepeatSend->checkState() == Qt::Checked) {
		m_pChkRepeatSend->setCheckState(Qt::Unchecked);
		emit clearDataSignal();
		QTimer::singleShot(30, this, [&] {
			m_pChkRepeatSend->setCheckState(Qt::Checked);
		});
	}
	else {
		Sleep(10);
		emit clearDataSignal();
		Sleep(20);
	}
}

Q_SLOT void ChildWidget::handleError(const QString & s)
{
	LogError(s, m_childLogPath);
	if (m_pChkRepeatSend->checkState() == Qt::Checked) {
		m_pChkRepeatSend->setCheckState(Qt::Unchecked);
		Sleep(10);
		emit clearDataSignal();
		QTimer::singleShot(30, this, [&] {
			m_pChkRepeatSend->setCheckState(Qt::Checked);
		});
	}
	else {
		Sleep(10);
		emit clearDataSignal();
		Sleep(20);
	}
}

/**
* @brief			初始化信号槽的连接
* @return			void
*/
void ChildWidget::initConnect()
{
	// connect(m_pBtn,SIGNAL(sigClicked()),this,SLOT(onClicked())); 连接信号和槽 模板1
	connect(this->m_pBtnVisible, &QPushButton::clicked, this, &ChildWidget::visibleBtnClicked);
	connect(this->m_pRadbtnSendHex, &QRadioButton::toggled, this, &ChildWidget::RadbtnSendChanged);
	connect(this->m_pBtnClear, &QPushButton::clicked, this, &ChildWidget::doClearEdit);
	connect(this->m_pBtnOpenClose, &QPushButton::clicked, this, &ChildWidget::openCloseBtnClicked);
	connect(this->m_pBtnExpand, &QPushButton::clicked, this, &ChildWidget::on_expandBtnClicked);
	// If want to capture mouse clicked event of combobox, you have to rewrite showPopup() or mousePressEvent; then emit yourself singal;
	//connect(this->m_pCmbCommList, &QComboBox::customContextMenuRequested, this, &ChildWidget::updatePortList);
	connect(this->m_pBtnSend, &QPushButton::clicked, this, &ChildWidget::sendBtnClicked);		//发送按钮
	connect(this->m_pCmbHistory, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &ChildWidget::sendHistorySelcted);//选择历史发送消息
	connect(this->m_pChkRepeatSend, &QCheckBox::stateChanged, this, &ChildWidget::repeatSend);	//重复发送复选框

	// 子线程信号初始化  //多线程默认Queued连接
	m_rwserial = new RWserial;//不加括号调用默认构造函数
	m_rwserial->moveToThread(&m_rwSerThread);
	connect(this, &ChildWidget::openSerPortSignal, m_rwserial, &RWserial::openSerialPort, Qt::QueuedConnection);//发送打开串口
	connect(m_rwserial, &RWserial::openSerResult, this, &ChildWidget::handleOpenResult);						//接收打开结果
	connect(m_rwserial, &RWserial::serialPortErrorSignal, this, &ChildWidget::handleSerialPortError);			//接收串口错误
	connect(this, &ChildWidget::closeSerPortSignal, m_rwserial, &RWserial::closeSerialPort);					//发送关闭串口
	connect(this, &ChildWidget::sendDataSignal, m_rwserial, &RWserial::readSerialData);							//发送写串口
	connect(m_rwserial, &RWserial::responseDataReady, this, &ChildWidget::handleResponse);						//接收读串口
	connect(m_rwserial, &RWserial::timeout, this, &ChildWidget::handleTimeout);									//接收超时
	connect(m_rwserial, &RWserial::error, this, &ChildWidget::handleError);										//接收读写错误
	connect(this, &ChildWidget::clearDataSignal, m_rwserial, &RWserial::clearBuffer);							//发送清空串口
	//connect(&m_rwSerThread, &QThread::finished, m_rwserial, &QObject::deleteLater);
	// serialWrite信号可以有返回值，如果是Qt::DirectConnection，即信号和槽运行在同一线程
	//connect(this, &ChildWidget::serialWrite, m_rwserial, &RWserial::readSyn, static_cast<Qt::ConnectionType>(Qt::UniqueConnection | Qt::DirectConnection));
	//子线程里开启循环的示例
	connect(this, &ChildWidget::loopstart, m_rwserial, &RWserial::loopPosStart);
	connect(this, &ChildWidget::loopstop, m_rwserial, &RWserial::loopPosStop);
	m_rwSerThread.start();
	//std::cout << "Controller is running in main thread: " << QThread::currentThreadId() << std::endl;
}

/**
* @brief			展开 折叠
* @return			void
*/
void ChildWidget::visibleBtnClicked()
{
	bool bVisible;
	if (m_pBtnVisible->text() == QStringLiteral("展开设置"))
	{
		bVisible = true; //可见
		m_pBtnVisible->setText(QStringLiteral("折叠设置"));
	}
	else
	{
		bVisible = false;
		m_pBtnVisible->setText(QStringLiteral("展开设置"));
	}

	m_pLabDataBits->setVisible(bVisible);
	m_pLabParity->setVisible(bVisible);
	m_pLabStopBits->setVisible(bVisible);
	m_pLabFlowControl->setVisible(bVisible);

	m_pCmbDataBits->setVisible(bVisible);
	m_pCmbParity->setVisible(bVisible);
	m_pCmbStopBits->setVisible(bVisible);
	m_pCmbFlowControl->setVisible(bVisible);
	m_pRadbtnAsync->setVisible(bVisible);
	m_pRadbtnSync->setVisible(bVisible);
}

/**
* @brief			Send way: ASCII Or Hex，字符串还是16进制
* @return			void
*/
void ChildWidget::RadbtnSendChanged()
{
	QString strData = this->m_pEditSend->toPlainText();
	if (strData.isEmpty())  //QTextEdit获取文本的函数
	{
		return;
	}
	if (m_pRadbtnSendHex->isChecked())
	{
		QByteArray byteData = stringToHex(strData.toLatin1());
		this->m_pEditSend->setText(byteData);
	}
	else
	{
		QByteArray byteData = hexToString(strData);
		this->m_pEditSend->setText(byteData);
	}
}

/**
* @brief		接收到主界面的清除按钮
* @return		void
*/
void ChildWidget::doClearEdit()
{
	m_pEditMsg->clear();
	editMsg.clear();
}

/**
* @brief		接收到主界面的扩展命令按钮
* @return		void
*/
Q_SLOT void ChildWidget::on_expandBtnClicked()
{
	bool bVisible;
	if (m_pBtnExpand->text() == QStringLiteral("扩展命令"))
	{
		bVisible = true; //可见
		m_pBtnExpand->setText(QStringLiteral("折叠扩展命令"));
	}
	else
	{
		bVisible = false;
		m_pBtnExpand->setText(QStringLiteral("扩展命令"));
	}
	m_pEditSend->setVisible(!bVisible);
	m_pBtnSend->setVisible(!bVisible);
	m_pExpandScroll->setVisible(bVisible);
}

/**
* @brief			打开串口 关闭串口
* @return			void
*/
void ChildWidget::openCloseBtnClicked()
{
	if (m_pBtnOpenClose->text() == QStringLiteral("打开串口"))
	{
		QString com = m_pCmbCommList->currentText();			//设置串口号
		int baudRate = m_pCmbBaudRate->currentText().toInt();	//设置波特率
		int dataBits = m_pCmbDataBits->currentText().toInt();	//数据位 直接转(不严谨)
		int stopBits = m_pCmbStopBits->currentIndex();			//校验位 直接转
		int parity = m_pCmbParity->currentIndex();				//停止位
		int flowControl = m_pCmbFlowControl->currentIndex();		//流控制
		emit openSerPortSignal(com, baudRate, dataBits, parity, stopBits, flowControl);
	}
	else
	{
		doDisconnect();
	}
}

void ChildWidget::doDisconnect() {
	if (m_pChkRepeatSend->checkState() == Qt::Checked) {
		m_pChkRepeatSend->setCheckState(Qt::Unchecked);
	}
	Sleep(100);
	emit closeSerPortSignal();
	setLabelTextColor(m_pLabConnInfo, Qt::red);
	m_pLabConnInfo->setText(QString("%1 Disconnect...").arg(m_pCmbCommList->currentText()));
	m_curSendOpt = EnmCurSendOption::NoneSend;	//断开连接后记得把当前状态改变
	m_pBtnSend->setEnabled(false);
	if (m_pBtnOpenClose->text() == QStringLiteral("关闭串口"))
	{
		m_pBtnOpenClose->setText(QStringLiteral("打开串口"));
		m_pBtnOpenClose->setStyleSheet("color: black");
	}
}

void ChildWidget::handleOpenResult(bool result)
{
	if (result) {
		//改变状态和显示信息
		m_pBtnOpenClose->setText(QStringLiteral("关闭串口"));
		m_pBtnOpenClose->setStyleSheet("color: red");
		m_curSendOpt = EnmCurSendOption::SerialPortSend;
		QString strConn = QString("%1 OPENED, %2").arg(m_pCmbCommList->currentText()).arg(m_pCmbBaudRate->currentText());
		this->setLabelTextColor(m_pLabConnInfo, Qt::darkGreen);
		this->m_pLabConnInfo->setText(strConn);
		m_pBtnSend->setEnabled(true);
	}
	else {
		setLabelTextColor(m_pLabConnInfo, Qt::red);
		m_pLabConnInfo->setText(QStringLiteral("%1 打开失败").arg(m_pCmbCommList->currentText()));
		showHintLabel(QStringLiteral("串口正在使用！"), this);
	}
}

/**
* @brief			串口发生错误 (Qt5.8 以后可用)
* @return			void
*/
void ChildWidget::handleSerialPortError(QString errorString)
{
	std::cout << "handleSerialPortError: --" << qPrintable(errorString) << std::endl;
	setLabelTextColor(m_pLabConnInfo, Qt::red);
	m_pLabConnInfo->setText(errorString);
	if (m_curSendOpt == EnmCurSendOption::SerialPortSend) {
		doDisconnect();
	}
	LogError(errorString, m_childLogPath);
	QMessageBox::information(this, tr("Error"), QStringLiteral("串口连接中断，请检查是否正确连接！\nError Detials: ")+errorString);
}

/**
* @brief			发送按钮点击
* @return			void
*/
void ChildWidget::sendBtnClicked()
{
	if (!int(m_curSendOpt))
	{
		showHintLabel(QStringLiteral("请打开串口！"), this);
		return;
	}
	if (this->m_pEditSend->toPlainText().isEmpty())  //QTextEdit获取文本的函数
	{
		this->m_pEditSend->setFocus();
		return;
	}

	QByteArray strData = this->m_pEditSend->toPlainText().toLatin1(); //toUtf8() = toLatin1()
	QByteArray hexData;
	//支持注释,不发送//以后的内容
	if (m_bSptNote)
	{
		int index = strData.indexOf("//");
		if (index)
		{
			strData = strData.remove(index, strData.length() - index);
		}
	}
	//发送方式是否为16进制发送
	if (this->m_pRadbtnSendHex->isChecked())
	{
		hexData = strData;
		strData = hexToString(hexData);
	}
	//windows回车换行
	if (m_pChkWordwrap->isChecked())
	{
		int len = strData.length();//取字符长度
		strData[len++] = 0x0d;// \r
		strData[len++] = 0x0a;// \n
	}
	QTextCodec *tc = QTextCodec::codecForName("GBK");	//处理中文乱码问题
	const QByteArray send_text = tc->fromUnicode(strData);
	serialWrite(send_text);
}


/**
* @brief			 选择重复发送
* @param iSend		 是否选中 2:选中
* @return			 void
*/
void ChildWidget::repeatSend(int iSend)
{
	//勾选
	if (iSend == Qt::Checked)
	{
		if (!int(m_curSendOpt))
		{
			//QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("请打开串口！"));
			showHintLabel(QStringLiteral("请打开串口！"), this);
			m_pChkRepeatSend->setCheckState(Qt::Unchecked);
			return;
		}
		//异步interval可以设置更小， 同步因为有waitFor延时所以interval大一些
		//int interval = this->m_pSpboxTimes->value() < 20 ? 20 : this->m_pSpboxTimes->value();
		int interval = this->m_pSpboxTimes->value();
		if (interval < 35) {
			showHintLabel(QStringLiteral("最小设置35ms"), this);
			this->m_pSpboxTimes->setValue(35);
			m_pChkRepeatSend->setCheckState(Qt::Unchecked);
			return;
		}
		m_pSpboxTimes->setEnabled(false);
		//方式1
		//emit loopstart();
		//方式2
		if (m_pRepeatSendTimer)
		{
			safeStopTimer(m_pRepeatSendTimer);
		}
		m_pRepeatSendTimer = new QTimer(this);
		connect(m_pRepeatSendTimer, &QTimer::timeout, this, &ChildWidget::sendBtnClicked);	//绑定到发送槽
		m_pRepeatSendTimer->setInterval(interval);
		m_pRepeatSendTimer->start();
	}
	else
	{
		if (!int(m_curSendOpt))
		{
			return;
		}
		m_pSpboxTimes->setEnabled(true);
		//方式1
		//emit loopstop();
		//方式2
		if (m_pRepeatSendTimer)
		{
			safeStopTimer(m_pRepeatSendTimer);
		}
	}
}

/**
* @brief			 选择历史发送消息
* @return			 void
*/
void ChildWidget::sendHistorySelcted()
{
	this->m_pEditSend->setText(m_pCmbHistory->currentText());
}

/**
* @brief			把数据添加到指定组合框
* @param pCmbBox	QComboBox指针
* @param strMsg		数据
* @return			void
*/
void ChildWidget::appendToComboBox(QComboBox *pCmbBox, const QString &strMsg)
{
	for (int i = 0; i < pCmbBox->count(); ++i)
	{
		if (pCmbBox->itemText(i) == strMsg)
		{
			return;
		}
	}
	pCmbBox->addItem(strMsg);
}

/**
* @brief			 更新状态栏的RxTx信息
* @param state		 Rx or Tx
* @param count		 更新的字节数
* @return			 void
*/
void ChildWidget::updateRxTxInfo(EnmUpdateRxTx state, int count)
{
	if (state == EnmUpdateRxTx::ClearRxTx)
	{
		//清空计数
		m_pLabRx->setText("Rx:0 Bytes");
		m_pLabTx->setText("Tx:0 Bytes");
		m_iTxCount = 0;
		m_iRxCount = 0;
		return;
	}
	else if (state == EnmUpdateRxTx::AddRx)
	{
		m_iRxCount += count;
		QString strInfo = QString("Rx/Rev:  %0 Bytes").arg(m_iRxCount);
		m_pLabRx->setText(strInfo);
	}
	else if (state == EnmUpdateRxTx::AddTx)
	{
		m_iTxCount += count;
		QString strInfo = QString("Tx/Send:  %0 Bytes").arg(m_iTxCount);
		m_pLabTx->setText(strInfo);
	}
	//else unknown
}

/**
* @brief			设置对应QLabel的字体颜色
* @param pLabel		label指针
* @param color		颜色
* @return			void
*/
void ChildWidget::setLabelTextColor(QLabel *pLabel, const QColor &color)
{
	QPalette palette;
	//palette.setColor(QPalette::Window, Qt::lightGray);  //改变控件背景色
	palette.setColor(QPalette::WindowText, color); //改变控件字体颜色
	pLabel->setPalette(palette);
}

/**
* @brief			窗体关闭事件
* @param event		关闭事件类
*
* @return			void
*/
void ChildWidget::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
}

//**
//* @brief			从配置文件初始化设置
//* @return			bool
//*/
bool ChildWidget::initSetFromJson()
{
	QFile file(QDir::currentPath() + "/config.json");
	if (!file.open(QIODevice::ReadOnly)) {
		if (writeSetToJson()) {
			if (m_bSaveLog) {
				LogInfo("Init: Successfully create config file!", m_childLogPath);
			}
			return true;
		}
		LogWarning("Init: Config file open failed!", m_childLogPath);
		return false;
	}
	else {
		if (m_bSaveLog) {
			LogInfo("Init: Config file open successfully!", m_childLogPath);
		}
	}

	QJsonParseError *error = new QJsonParseError;
	QJsonDocument jdc = QJsonDocument::fromJson(file.readAll(), error);

	//判断文件是否完整
	if (error->error != QJsonParseError::NoError)
	{
		//qDebug() << "parseJson:" << error->errorString();
		return false;
	}

	QJsonObject obj = jdc.object();        //获取对象
	if (obj.contains(u8"串口信息")) {
		QJsonObject objSerInfo = obj[u8"串口信息"].toObject();
		jsonToSerStruct(objSerInfo, StructsetInfo);
		setSerialPortInfo(StructsetInfo);
	}
	else {
		return false;
	}
	
	if (obj.contains(u8"自定义字符串")) {
		QJsonArray arr = obj[u8"自定义字符串"].toArray();
		if (!arr.count()) {
			//if (m_bSaveLog) {
			//	LogInfo("The count of custom string is 0", m_childLogPath);
			//}
			return true;
		}
		for (int i = 0; i<arr.count(); i++)
		{
			QJsonObject arrObj = arr[i].toObject();
			int index = arrObj["index"].toInt();
			m_exCheckHex.at(index)->setChecked(arrObj["hexFlag"].toBool());
			m_exCheckWrap.at(index)->setChecked(arrObj["wrapFlag"].toBool());
			m_exLineEdit.at(index)->setText(arrObj["stringData"].toString());
		}
	}
	
	return true;

}

//**
//* @brief			设置写入配置文件 public
//* @return			bool
//*/
bool ChildWidget::writeSetToJson()
{
	//打开文件
	//QString a = QApplication::applicationDirPath();
	QFile file(QDir::currentPath() + "/config.json");
	if (!file.open(QIODevice::WriteOnly)) {
		LogError("Config file open and write failed!", m_childLogPath);
		return false;
	}
	//else {
	//	qDebug() << "Config file open successfully, waiting for write";
	//}
	QJsonObject serialSet;//定义数组成员
	getSerSettings(StructsetInfo);//获得设置信息
	serStructToJson(StructsetInfo, serialSet);//设置信息转换为QJsonObject

	QJsonDocument jdoc;
	QJsonObject obj;
	QJsonArray arrayStr;

	for (int i = 0; i<m_exCheckHex.count(); i++)
	{
		QString strData = m_exLineEdit.at(i)->text();
		if (strData.isEmpty())  //QTextEdit获取文本的函数
		{
			continue;
		}
		QJsonObject Member;     //定义数组成员
		Member["hexFlag"] = m_exCheckHex.at(i)->isChecked();	//bool
		Member["wrapFlag"] = m_exCheckWrap.at(i)->isChecked();	//bool
		Member["stringData"] = strData;							//string
		Member["index"] = i;									//int
		arrayStr.append(Member);
	}
	obj[u8"串口信息"] = serialSet;
	obj[u8"自定义字符串"] = arrayStr;
	jdoc.setObject(obj);
	file.write(jdoc.toJson(QJsonDocument::Indented)); //Indented:表示自动添加/n回车符
	file.close();
	return true;
}

/**
* @brief			获得配置信息 public
* @return
*/
void ChildWidget::getSerSettings(SettingCollect &setInfo)
{
	//SettingCollect setInfo;
	setInfo.serialPortName		= this->m_pCmbCommList->currentText();
	setInfo.baudRate			= this->m_pCmbBaudRate->currentText().toInt();
	setInfo.dataBits			= this->m_pCmbDataBits->currentText().toInt();
	setInfo.parity				= this->m_pCmbParity->currentText();
	setInfo.stopBits			= this->m_pCmbStopBits->currentText().toFloat();
	setInfo.flowControl			= this->m_pCmbFlowControl->currentText();
	setInfo.receOpt				= this->m_pRadbtnRecASCII->isChecked() ? EnmEncodeOpt::ASCII : EnmEncodeOpt::Hex;
	setInfo.sendOpt				= this->m_pRadbtnSendASCII->isChecked() ? EnmEncodeOpt::ASCII : EnmEncodeOpt::Hex;
	setInfo.readOpt				= this->m_pRadbtnAsync->isChecked() ? EnmReadWayOpt::Async : EnmReadWayOpt::Sync;
	setInfo.stopShowMsg			= this->m_pChkShowMsg->isChecked();
	setInfo.wordwrap			= this->m_pChkWordwrap->isChecked();
	setInfo.sendRepeat			= this->m_pChkRepeatSend->isChecked();
	setInfo.repeatTimes			= this->m_pSpboxTimes->value();
	setInfo.sendText			= this->m_pEditSend->toPlainText();
	setInfo.sptNote				= this->m_bSptNote;
	setInfo.saveLog				= this->m_bSaveLog;
	//return setInfo;
}

void ChildWidget::serStructToJson(SettingCollect &p, QJsonObject &serialSet)
{
	serialSet[u8"端口名"] = p.serialPortName;		serialSet[u8"波特率"] = p.baudRate;
	serialSet[u8"数据位"] = p.dataBits;				serialSet[u8"校验位"] = p.parity;
	serialSet[u8"停止位"] = p.stopBits;				serialSet[u8"流控"] = p.flowControl;
	serialSet[u8"接收方式"] = p.receOpt;			serialSet[u8"发送自动换行"] = p.wordwrap;
	serialSet[u8"停止消息显示"] = p.stopShowMsg;	serialSet[u8"发送方式"] = p.sendOpt;			
	serialSet[u8"是否重复发送"] = p.sendRepeat;		serialSet[u8"重复发送间隔"] = p.repeatTimes;	
	serialSet[u8"发送文本"] = p.sendText;			serialSet[u8"是否支持注释"] = p.sptNote;		
	serialSet[u8"是否保存日志"] = p.saveLog;		serialSet[u8"读写方式"] = p.readOpt;
}

void ChildWidget::jsonToSerStruct(QJsonObject &serialSet, SettingCollect &p)
{
	p.serialPortName = serialSet[u8"端口名"].toString();
	p.baudRate = serialSet[u8"波特率"].toInt();
	p.dataBits = serialSet[u8"数据位"].toInt();
	p.parity = serialSet[u8"校验位"].toString();
	p.stopBits = serialSet[u8"停止位"].toInt();
	p.flowControl = serialSet[u8"流控"].toString();
	p.receOpt = serialSet[u8"接收方式"].toInt() ? EnmEncodeOpt::Hex : EnmEncodeOpt::ASCII;
	p.sendOpt = serialSet[u8"发送方式"].toInt() ? EnmEncodeOpt::Hex : EnmEncodeOpt::ASCII;
	p.readOpt = serialSet[u8"读写方式"].toInt() ? EnmReadWayOpt::Sync : EnmReadWayOpt::Async;
	p.stopShowMsg = serialSet[u8"停止消息显示"].toBool();
	p.wordwrap = serialSet[u8"发送自动换行"].toBool();
	p.sendRepeat = serialSet[u8"是否重复发送"].toBool();
	p.repeatTimes = serialSet[u8"重复发送间隔"].toInt();
	p.sendText = serialSet[u8"发送文本"].toString();
	p.sptNote = serialSet[u8"是否支持注释"].toBool();
	p.saveLog = serialSet[u8"是否保存日志"].toBool();
}

//* @brief			设置串口的信息
//* @param collect	设置的信息
//* @return			void
//*/
void ChildWidget::setSerialPortInfo(const SettingCollect &setInfo)
{
	m_pRadbtnRecASCII->setChecked(setInfo.receOpt == EnmEncodeOpt::ASCII);
	m_pRadbtnRecHex->setChecked(setInfo.receOpt == EnmEncodeOpt::Hex);
	m_pRadbtnSendASCII->setChecked(setInfo.sendOpt == EnmEncodeOpt::ASCII);
	m_pRadbtnSendHex->setChecked(setInfo.sendOpt == EnmEncodeOpt::Hex);
	m_pRadbtnAsync->setChecked(setInfo.readOpt == EnmReadWayOpt::Async);
	m_pRadbtnSync->setChecked(setInfo.readOpt == EnmReadWayOpt::Sync);
	m_pSpboxTimes->setValue(setInfo.repeatTimes);
	m_pChkRepeatSend->setChecked(setInfo.sendRepeat);
	m_pChkShowMsg->setChecked(setInfo.stopShowMsg);
	m_pChkWordwrap->setChecked(setInfo.wordwrap);
	m_pEditSend->setText(setInfo.sendText);
	m_bSptNote = setInfo.sptNote;
	m_bSaveLog = setInfo.saveLog;
	//设置端口名
	for (int i = 0; i < m_pCmbCommList->count(); i++)
	{
		if (m_pCmbCommList->itemText(i) == setInfo.serialPortName)
		{
			m_pCmbCommList->setCurrentIndex(i);
			break;
		}
	}
	// baudRate>115200 需要修改qserialport.h / enum BaudRate{}
	//"9600", "19200", "38400", "57600", "115200"
	switch (setInfo.baudRate)
	{
	case 9600:
		m_pCmbBaudRate->setCurrentIndex(0);
		break;
	case 19200:
		m_pCmbBaudRate->setCurrentIndex(1);
		break;
	case 38400:
		m_pCmbBaudRate->setCurrentIndex(2);
		break;
	case 57600:
		m_pCmbBaudRate->setCurrentIndex(3);
		break;
	case 115200:
		m_pCmbBaudRate->setCurrentIndex(4);
		break;
	case 128000:
		m_pCmbBaudRate->setCurrentIndex(5);
		break;
	case 256000:
		m_pCmbBaudRate->setCurrentIndex(6);
		break;
	default:
		m_pCmbBaudRate->setCurrentIndex(0);
		break;
	}

	//"5", "6", "7", "8"
	switch (setInfo.dataBits)
	{
	case 5:
		m_pCmbDataBits->setCurrentIndex(0);
		break;
	case 6:
		m_pCmbDataBits->setCurrentIndex(1);
		break;
	case 7:
		m_pCmbDataBits->setCurrentIndex(2);
		break;
	case 8:
		m_pCmbDataBits->setCurrentIndex(3);
		break;
	default:
		m_pCmbDataBits->setCurrentIndex(0);
		break;
	}

	//"None", "Even", "Odd",  "Space", "Mark"
	if (setInfo.parity == "None")
	{
		m_pCmbParity->setCurrentIndex(0);
	}
	else if (setInfo.parity == "Even")
	{
		m_pCmbParity->setCurrentIndex(1);
	}
	else if (setInfo.parity == "Odd")
	{
		m_pCmbParity->setCurrentIndex(2);
	}
	else if (setInfo.parity == "Space")
	{
		m_pCmbParity->setCurrentIndex(3);
	}
	else if (setInfo.parity == "Mark")
	{
		m_pCmbParity->setCurrentIndex(4);
	}
	else
	{
		m_pCmbParity->setCurrentIndex(0);
	}

	//"1", "1.5", "2"
	if (setInfo.stopBits == 1)
	{
		m_pCmbStopBits->setCurrentIndex(0);
	}
	else if (setInfo.stopBits == 1.5)
	{
		m_pCmbStopBits->setCurrentIndex(1);
	}
	else if (setInfo.stopBits == 2)
	{
		m_pCmbStopBits->setCurrentIndex(2);
	}
	else
	{
		m_pCmbStopBits->setCurrentIndex(0);
	}

	//"None", "Hardware", "Software"
	if (setInfo.flowControl == "None")
	{
		m_pCmbFlowControl->setCurrentIndex(0);
	}
	else if (setInfo.flowControl == "Hardware")
	{
		m_pCmbFlowControl->setCurrentIndex(1);
	}
	else if (setInfo.flowControl == "Software")
	{
		m_pCmbFlowControl->setCurrentIndex(2);
	}
	else
	{
		m_pCmbFlowControl->setCurrentIndex(0);
	}
}