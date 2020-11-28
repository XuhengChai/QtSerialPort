#pragma once
#include <QtSerialPort/QSerialPort>
#include <QtCore/QTimer>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFrame>
#include <QtCore/QPropertyAnimation>

/*	
读写串口子线程类
对QSerialPort *rw_serialPort的所有操作均应放在子线程里，和主线程信号槽连接。IO Device跨线程的操作极易出现奇怪的bug。

//hint: QList和QLinkedList是链表，QQueue继承自QList；QVector.first()内存地址怎么增删都不变
*/
class RWserial : public QObject
{
	Q_OBJECT
public:
	explicit RWserial(QObject *parent = nullptr);
	~RWserial();
	// 当槽函数被信号触发而被调用时，公有槽和私有槽没有区别
	// handle open signal
	Q_SLOT void openSerialPort(QString com, int baud, int dateBits, int parity, int stopBits, int flowControl);
	Q_SIGNAL void openSerResult(bool);
	Q_SLOT void handleErrorOccurred(QSerialPort::SerialPortError error);
	Q_SIGNAL void serialPortErrorSignal(QString errorString); //QSerialPort::SerialPortError error 无法作为信号槽参数
	// handle close signal
	Q_SLOT void closeSerialPort();
	// handle write signal
	Q_SLOT void readSerialData(const QByteArray &sendData, bool synFlag);			  //receive WriteData and synFlag
	Q_SIGNAL void responseDataReady(const QByteArray &send, const QByteArray &read);  //send sendData and readData
	bool writeSerialSyn(const QByteArray &sendData);
	void readSerialSyn(const QByteArray &sendData);
	Q_SIGNAL void timeout(const QString &s);    //超时信号
	Q_SIGNAL void error(const QString &s);
	// read data Asyn
	Q_SLOT void readSerialAsyn();
	Q_SLOT void handleTimeoutSig();
	Q_SLOT void handleReadyReadSig();
	// handle clear signal
	Q_SLOT void clearBuffer();
	// handle loop signal example
	Q_SLOT void loopPosStart();
	Q_SLOT void loopPosStop();

private:
	QSerialPort *rw_serialPort = nullptr;		    //串口通信名称
	QMutex mutex;
	bool synchronizationFlag; //是否同步
	
	//QList<QByteArray> rw_sendQueue;
	QByteArray responseData;
	QByteArray rw_readData;
	QByteArrayList rw_readDataList;
	QByteArray rw_ListAt;
	QTimer *loopTimer = nullptr;
	QTimer *m_pReadTimer = nullptr;
	char m_swap;

//public slots:
//signals:

};

//如果inline成员函数会在多个源文件中被用到，必须定义在头文件中
//这里定义的是外部函数
/**
* @brief			stop Qtimer; disconnect Qtimer; deleteLater;
* @param			QTimer Pointer
* @return			void
*/
static void safeStopTimer(QTimer * &m_Timer) {
	m_Timer->stop();
	QObject::disconnect(m_Timer, 0, 0, 0);
	m_Timer->deleteLater();
	m_Timer = nullptr;
}

static void safeDeleteSerial(QSerialPort * &serial) {
	QObject::disconnect(serial, 0, 0, 0);
	serial->clear();
	serial->close();
	serial->deleteLater();
	serial = nullptr;

}

inline const char *QString2str(QString str) {
	//return str.toStdString().c_str();
	return str.toStdString().data();
}

static void showHintLabel( QString strText, QWidget *parent = nullptr, QString strFontSize = "16px",
				   QString strColor = "#ffffff", QString strBgColor = "#666666")
{
	if (nullptr == parent) {

		parent = QApplication::desktop()->screen();
	}

	//QFrame *pFrmBg = new QFrame(this); //为了兼容parent为nullptr时的圆角边框  方法是背景透明 上边叠加圆角控件
	QFrame *pFrmBg = new QFrame(parent);
	QLabel *pHintLabel = new QLabel(pFrmBg);
	pHintLabel->setStyleSheet(QString("QLabel{background:%1;color:%2;font:%3 SimHei;border-radius:5px;}")
		.arg(strBgColor).arg(strColor).arg(strFontSize));
	pHintLabel->setText(strText);
	pHintLabel->setAlignment(Qt::AlignCenter);
	pHintLabel->adjustSize();
	pHintLabel->resize(pHintLabel->size() + QSize(60, 30));

	pFrmBg->resize(pHintLabel->size());
	pFrmBg->setWindowFlags(Qt::FramelessWindowHint);
	pFrmBg->setAttribute(Qt::WA_TranslucentBackground);

	QSize parentSize = (nullptr == parent) ? QApplication::desktop()->screenGeometry().size() : parent->size(); //双屏情况下在主屏幕上提示
	//QSize parentSize = this->size();
	pFrmBg->move(parentSize.width() / 2, parentSize.height() / 2);

	pFrmBg->show();

	QPropertyAnimation *pAnimation = new QPropertyAnimation(pFrmBg, "windowOpacity");
	pAnimation->setDuration(1500);
	pAnimation->setEasingCurve(QEasingCurve::InCirc);
	pAnimation->setStartValue(1.0f);
	pAnimation->setEndValue(0.0f);
	pAnimation->start();
	QObject::connect(pAnimation, &QPropertyAnimation::finished, [=] {
		delete pAnimation;
		delete pFrmBg;
	});
}