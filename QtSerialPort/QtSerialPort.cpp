#include "QtSerialPort.h"
#define PI acos(-1)

QtSerialPort::QtSerialPort(QWidget *parent)
	: QMainWindow(parent)
{
	initViewer();
}
void QtSerialPort::initViewer()
{
	createMenu();
	createToolBtns();
	createCenWgt();
	initConnect();
	this->resize(720, 665);
	this->setWindowTitle(QStringLiteral("串口调试工具"));
	this->setWindowIcon(QIcon(":/images/Icon.png"));
	childWgt->initSetFromJson();  //配置文件只初始化第一个子窗口
	if (childWgt->m_bSaveLog) {
		m_pRecordLogAction->setChecked(true);
	}
}

/**
* @brief		初始化菜单
* @return		void
*/
void QtSerialPort::createMenu()
{
	m_pFileMenu = new QMenu(QStringLiteral("文件(&F)"));
	m_pEditMenu = new QMenu(QStringLiteral("编辑(&E)"));
	m_pHelpMenu = new QMenu(QStringLiteral("帮助(&H)"));

	m_pNewAction = new QAction(QStringLiteral("新建串口页面"));
	m_pNewAction->setIcon(QIcon(":/images/New.png"));
	m_pRecordLogAction = new QAction(QStringLiteral("Record All Data Log"));
	m_pRecordLogAction->setIcon(QIcon(":/images/Log.png"));
	m_pRecordLogAction->setCheckable(true);
	m_pViewCurLogActin = new QAction(QStringLiteral("View Current Data Log"));
	m_pViewCurLogActin->setIcon(QIcon(":/images/View.png"));
	m_pExitAction = new QAction(QStringLiteral("退出(&X)"));
	m_pClearAction = new QAction(QStringLiteral("Clear All Rev Data"));
	m_pClearAction->setIcon(QIcon(":/images/Clear.png"));
	m_pAboutAction = new QAction(QStringLiteral("关于(&A)"));
	m_pAboutAction->setIcon(QIcon(":/images/About.png"));

	m_pFileMenu->addAction(m_pNewAction);
	m_pFileMenu->addAction(m_pRecordLogAction);
	m_pFileMenu->addAction(m_pViewCurLogActin);
	m_pFileMenu->addSeparator();
	m_pFileMenu->addAction(m_pExitAction);
	m_pEditMenu->addAction(m_pClearAction);
	m_pEditMenu->addSeparator();
	m_pHelpMenu->addAction(m_pAboutAction);

	m_pMenuBar = this->menuBar();
	m_pMenuBar->addMenu(m_pFileMenu);
	m_pMenuBar->addMenu(m_pEditMenu);
	m_pMenuBar->addMenu(m_pHelpMenu);
}

/**
* @brief		初始化工具栏
* @return		void
*/
void QtSerialPort::createToolBtns()
{
	m_pToolsBar = this->addToolBar("Default");
	m_pToolsBar->setIconSize(QSize(28, 28));
	m_pToolsBar->addAction(m_pNewAction);
	m_pToolsBar->addAction(m_pRecordLogAction);
	m_pToolsBar->addAction(m_pClearAction);
	m_pToolsBar->addSeparator();
}

/**
* @brief		初始化控件
* @return		void
*/
void QtSerialPort::createCenWgt()
{
	m_pCenScroll = new QScrollArea;
	m_pTabWidget = new QTabWidget();
	childWgt = new ChildWidget();
	// 第一个子tab窗口不可关闭，之后可关闭
	m_pTabWidget->addTab(childWgt, QString("Port1"));
	m_pTabWidget->setTabsClosable(true);
	((QTabBar*)(m_pTabWidget->tabBar()))->setTabButton(m_pTabWidget->indexOf(childWgt), QTabBar::RightSide, NULL);
	m_pCenScroll->setWidget(m_pTabWidget);
	m_pCenScroll->setWidgetResizable(true);
	this->setCentralWidget(m_pCenScroll);
	m_listChildWidget.push_back(childWgt);
	//添加日志路径list
	QString temp_strDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString temp_tabStr = m_pTabWidget->tabText(m_pTabWidget->count()-1);
	QString temp_LogPath = QDir::currentPath() + "/log_" + temp_tabStr + "_" + temp_strDate + ".txt";
	childWgt->m_childLogPath = temp_LogPath;
	m_strLogPath.push_back(temp_LogPath);
}

/**
* @brief		初始化信号槽的连接信息
* @return		void
*/
void QtSerialPort::initConnect()
{
	connect(this->m_pNewAction, &QAction::triggered, this, &QtSerialPort::insertSubTab);				//新建页面
	connect(this->m_pTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(removeSubTab(int)));			//删除页面
	connect(this->m_pClearAction, &QAction::triggered, this, &QtSerialPort::clearAllMsg);				//清除按钮
	connect(this->m_pRecordLogAction, &QAction::triggered, this, &QtSerialPort::setLogRecordState);		//设置是否打开日志记录
	connect(this->m_pViewCurLogActin, &QAction::triggered, this, &QtSerialPort::viewCurDataLog);		//查看当前日志
	connect(this->m_pExitAction, &QAction::triggered, this, &QtSerialPort::close);						//退出
	connect(this->m_pAboutAction, &QAction::triggered, this, &QtSerialPort::about);						//关于
}

/**
* @brief		窗体关闭事件
* @param event  关闭事件类
* @return		void
*/
void QtSerialPort::closeEvent(QCloseEvent *event)
{
	for (auto iter = m_listChildWidget.begin(); iter != m_listChildWidget.end(); iter++)
	{
		if ((*iter)->m_curSendOpt == EnmCurSendOption::SerialPortSend) {
			(*iter)->doDisconnect();
		}
	}
	//保存配置文件
	childWgt->writeSetToJson();
	//弹出yes no 窗口
	//switch (QMessageBox::information(this, QStringLiteral("确认关闭?"),
	//	QStringLiteral("请确认串口已经关闭~"),
	//	tr("Yes"), tr("No"),
	//	0, 1))
	//{
	//case 0:
	//	event->accept();
	//	break;
	//case 1:
	//	//childWgt->doDisconnect();
	//default:
	//	event->ignore();
	//	break;
	//}
}

void QtSerialPort::about()
{
	//int ret = QMessageBox::warning(this, tr("About"),
	//	tr("Copyright @ 2020\n"
	//		"Email"),
	//	QMessageBox::Save | QMessageBox::Discard
	//	| QMessageBox::Cancel,
	//	QMessageBox::Save);

	QDialog *temp_widget = new QDialog();
	QLabel *m_pLabIcon = new QLabel;
	QPixmap pixmap(":/images/Icon.png");
	pixmap.scaled(QSize(60, 60), Qt::KeepAspectRatio);
	m_pLabIcon->setPixmap(pixmap);

	QLabel *m_pLabInfo = new QLabel;
	QFont font(QStringLiteral("新宋体"), 10);
	m_pLabInfo->setFont(font);
	m_pLabInfo->setText("Copyright @ 2020.10\nBy Chai\nEmail: 1076280466@qq.com");

	QGridLayout *rightLayout = new QGridLayout();
	QPushButton *m_pBtnOk = new QPushButton("OK");

	rightLayout->addWidget(m_pLabIcon, 0, 0);
	rightLayout->addWidget(m_pLabInfo, 0, 1);
	rightLayout->addWidget(m_pBtnOk, 1, 2);
	temp_widget->setWindowTitle(QStringLiteral("关于"));
	temp_widget->setLayout(rightLayout);
	//信号需要用指针的形式，而不能用SIGNAL()的形式  Lambda函数 [=]() {temp_widget->close(); }
	connect(m_pBtnOk, &QPushButton::clicked, this, [=] {temp_widget->close(); });
	temp_widget->exec();
}

/**
* @brief		插入新的tab页面
* @return		void
*/
void QtSerialPort::insertSubTab() {

	ChildWidget *childWidget = new ChildWidget(this);
	m_listChildWidget.push_back(childWidget);
	//m_pTabWidget->addTab(childWidget, QString("Port %1").arg(m_listChildWidget.count() + 1));
	QString tabStr = m_pTabWidget->tabText(m_pTabWidget->count()-1);
	m_pTabWidget->addTab(childWidget, QString("Port%1").arg(tabStr.right(1).toInt() + 1));

	QString temp_strDate = QDateTime::currentDateTime().toString("yyyyMMdd");
	QString temp_tabStr = m_pTabWidget->tabText(m_pTabWidget->count()-1);
	QString temp_LogPath = QDir::currentPath() + "/log_" + temp_tabStr + "_" + temp_strDate + ".txt";
	if (m_pRecordLogAction->isChecked()) {
		childWidget->m_childLogPath = temp_LogPath;
		childWidget->m_bSaveLog = true;
	}
	m_strLogPath.push_back(temp_LogPath);
}

/**
* @brief		移除tab页面
* @return		void
*/
void QtSerialPort::removeSubTab(int index)
{
	//removeTab方法移除指定索引位置的选项卡，但选项卡对应的页面部件对象并没有删除
	m_pTabWidget->removeTab(index);
	//删除ChildWidget
	ChildWidget *cellWidget = m_listChildWidget.at(index);
	cellWidget->setParent(nullptr);			//这里一定要设置为null
	cellWidget->deleteLater();				//调用析构
	m_listChildWidget.removeAt(index);
	m_strLogPath.removeAt(index);
}

/**
* @brief		清除所有窗口的信息
* @return		void
*/
void QtSerialPort::clearAllMsg()
{
	for (auto iter = m_listChildWidget.begin(); iter != m_listChildWidget.end(); iter++)
	{
		(*iter)->doClearEdit();
	}
}

/**
* @brief			 是否打开日志记录
* @return			 void
*/
void QtSerialPort::setLogRecordState()
{
	if (m_pRecordLogAction->isChecked())
	{
		m_pRecordLogAction->setIcon(QIcon(":/images/LogNo.png"));
		m_pRecordLogAction->setIconText(tr("Disable Log"));
		for (int i = 0; i < m_listChildWidget.size(); ++i)
		{	//如果Path空，设置LogPath
			if (m_listChildWidget.at(i)->m_childLogPath.isEmpty()) {
				m_listChildWidget.at(i)->m_childLogPath = m_strLogPath.at(i);
			}
			m_listChildWidget.at(i)->m_bSaveLog=true;
		}
	}
	else
	{
		m_pRecordLogAction->setIcon(QIcon(":/images/Log.png"));
		for (int i = 0; i < m_listChildWidget.size(); ++i)
		{
			m_listChildWidget.at(i)->m_bSaveLog=false;
		}
	}
}

/**
* @brief			 打开日志文本
* @return			 void
*/
void QtSerialPort::viewCurDataLog()
{
	QProcess *process = new QProcess;
	//QStringList list;
	//list << m_strLogPath.at(m_pTabWidget->currentIndex());
	//process->start("Notepad.exe", list);
	QString fileName = m_strLogPath.at(m_pTabWidget->currentIndex());
	QFileInfo file(fileName);
	if (file.exists()){
		process->start(QString("Notepad.exe %1").arg(fileName));
	}
	else {
		showHintLabel(QStringLiteral("未找到日志文件！"), this);;
	}
}
//void QtSerialPort::showHintLabel(QString strText, QString strFontSize, QString strColor, QString strBgColor)
//{
//	//QWidget *parent
//	//if (nullptr == parent) {
//	//	parent = QApplication::desktop()->screen();
//	//}
//
//	QFrame *pFrmBg = new QFrame(this); //为了兼容parent为nullptr时的圆角边框  方法是背景透明 上边叠加圆角控件
//
//	QLabel *pHintLabel = new QLabel(pFrmBg);
//	pHintLabel->setStyleSheet(QString("QLabel{background:%1;color:%2;font:%3 SimHei;border-radius:5px;}")
//		.arg(strBgColor).arg(strColor).arg(strFontSize));
//	pHintLabel->setText(strText);
//	pHintLabel->setAlignment(Qt::AlignCenter);
//	pHintLabel->adjustSize();
//	pHintLabel->resize(pHintLabel->size() + QSize(60, 30));
//
//	pFrmBg->resize(pHintLabel->size());
//	pFrmBg->setWindowFlags(Qt::FramelessWindowHint);
//	pFrmBg->setAttribute(Qt::WA_TranslucentBackground);
//
//	//QSize parentSize = (nullptr == parent) ? QApplication::desktop()->screenGeometry().size() : parent->size(); //双屏情况下在主屏幕上提示
//	QSize subSize = this->size();
//	pFrmBg->move(subSize.width() / 2, subSize.height() / 2);
//
//	pFrmBg->show();
//
//	QPropertyAnimation *pAnimation = new QPropertyAnimation(pFrmBg, "windowOpacity");
//	pAnimation->setDuration(1500);
//	pAnimation->setEasingCurve(QEasingCurve::InCirc);
//	pAnimation->setStartValue(1.0f);
//	pAnimation->setEndValue(0.0f);
//	pAnimation->start();
//	connect(pAnimation, &QPropertyAnimation::finished, [=] {
//		delete pAnimation;
//		delete pFrmBg;
//	});
//}