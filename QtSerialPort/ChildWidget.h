#pragma once
#include "Global.h"
#include "rwSerial.h"
#include <iostream>
#include <windows.h>
#include <QtWidgets/QtWidgets>

//串口子窗口类
class QLabel;
class QPushButton;
class QComboBox;
class QRadioButton;
class QCheckBox;
class QSpinBox;
class QTextEdit;
class QLineEdit;
class QLabel;
class QThread;

enum EnmEncodeOpt
{
	ASCII = 0, //分别为0，1，...
	Hex
};
enum EnmReadWayOpt
{
	Async = 0, //分别为0，1，...
	Sync
};
/*设置的信息*/
struct SettingCollect
{
	QString serialPortName;	//端口名
	int baudRate;			//波特率
	int dataBits;			//数据位
	QString parity;			//校验位
	float stopBits;			//停止位
	QString flowControl;	//流控
	EnmEncodeOpt receOpt;	//接收方式
	EnmEncodeOpt sendOpt;	//发送方式
	EnmReadWayOpt readOpt;	//读写方式
	bool stopShowMsg;		//是否停止消息显示
	bool wordwrap;			//发送自动换行
	bool sendRepeat;		//是否重复发送
	int repeatTimes;		//重复发送的间隔
	QString sendText;		//发送文本
	bool sptNote;			//是否支持注释
	bool saveLog;			//是否保存日志
};

class ChildWidget : public  QMainWindow
{
	Q_OBJECT

public:
	ChildWidget(QWidget *parent = nullptr);
	~ChildWidget();
	//串口读写相关
	RWserial *m_rwserial;				//串口读写子线程类
	EnmCurSendOption m_curSendOpt;		//当前连接的状态
	void doDisconnect();
	// 同步异步读写，修改synFlag，这里默认false异步读写
	void serialWrite(const QByteArray &writeData, bool synFlag = false);		//发送信号 sendWriteData
	//清空界面
	Q_SLOT void doClearEdit();
	//配置文件相关
	bool writeSetToJson();							//保存设置
	bool initSetFromJson();							//初始化设置
	//日志相关
	bool m_bSaveLog;							//是否打开日志
	QString m_childLogPath = "";				//日志文件路径
	SettingCollect StructsetInfo;

private:
	//串口子线程读写相关，发送信号，和rwSerial.h里面的槽函数相对应
	QThread m_rwSerThread;
	// open signal
	Q_SIGNAL void openSerPortSignal(QString com, int baud, int dateBits, int parity, int stopBits, int flowControl);
	Q_SLOT void handleOpenResult(bool);
	Q_SLOT void handleSerialPortError(QString errorString);
	// close signal
	Q_SIGNAL void closeSerPortSignal();
	// write signal						
	Q_SIGNAL QString sendDataSignal(const QByteArray &sendData, bool synFlag);			//send WriteData and synFlag
	Q_SLOT void handleResponse(const QByteArray &send, const QByteArray &read);	//receive sendData and readData; And handle
	Q_SLOT void handleTimeout(const QString &s);
	Q_SLOT void handleError(const QString &s);
	// clear signal	
	Q_SIGNAL void clearDataSignal();
	//配置文件相关
	void serStructToJson(SettingCollect &p, QJsonObject &serialSet);
	void jsonToSerStruct(QJsonObject &serialSet, SettingCollect &p);
	void getSerSettings(SettingCollect &setInfo);
	void setSerialPortInfo(const SettingCollect &collect);

private:
	void initViewer();    //初始化函数
	void initCenWgt();	  //初始化主窗口
	void initExapndWgt(); //初始化扩展命令窗口
	QScrollArea *m_pExpandScroll;
	Q_SLOT void on_expandBtnClicked();			//扩展命令按钮点击
	QList<QCheckBox *> m_exCheckHex;
	QList<QCheckBox *> m_exCheckWrap;
	QList<QLineEdit *> m_exLineEdit;
	QList<QPushButton *> m_exBtnSend;
	QSignalMapper *signalMapper;
	QSignalMapper *signalMapperHex;				//扩展命令里发送点击
	Q_SLOT void handleSignalMapper(int i);
	Q_SLOT void handleSignalMapperHex(int i);	//扩展命令里Hex选中
	//初始化状态栏
	void initStatusBar();
	//初始化串口设置
	void initPortSettingList();
	//定时扫描和更新串口
	void updatePortList();
	QTimer *m_pScanTimer = nullptr;
	// 16进制转换函数
	template<typename T> QByteArray hexToString(T data);
	template<typename T> QByteArray stringToHex(T strData);
	//初始化信号槽连接
	void initConnect();
	Q_SLOT void visibleBtnClicked();        //展开折叠
	Q_SLOT void RadbtnSendChanged();        //ascii or hex send
	Q_SLOT void openCloseBtnClicked();      //打开串口 关闭串口
	Q_SLOT void sendBtnClicked();			//发送按钮
	bool m_bSptNote;						//发送支持注释
	Q_SLOT void repeatSend(int iSend);		//选择重复发送
	QTimer *m_pRepeatSendTimer = nullptr;	//重复发送方式1：重复发送定时器
	Q_SIGNAL void loopstart();				//重复发送方式2：开始信号
	Q_SIGNAL void loopstop();				//重复发送方式2：停止信号
	Q_SLOT void sendHistorySelcted();		//选择右下角历史发送
	void appendToComboBox(QComboBox *pCmbBox, const QString &strMsg);//添加到历史发送
	void updateRxTxInfo(EnmUpdateRxTx state, int count = 0);		//更新状态栏的RxTx信息
	void setLabelTextColor(QLabel *pLabel, const QColor &color);	//设置label字体颜色

private:
#pragma region 界面相关  //可折叠
	QWidget *m_pCenWidget;
	QWidget *m_pLeftSetWgt;		//左边的布局窗体

	//串口通信
	QLabel *m_pLabCommList;
	QLabel *m_pLabBaudRate;
	QLabel *m_pLabDataBits;
	QLabel *m_pLabParity;
	QLabel *m_pLabStopBits;
	QLabel *m_pLabFlowControl;
	QComboBox *m_pCmbCommList;	    //可用串口列表
	QComboBox *m_pCmbBaudRate;      //波特率
	QPushButton *m_pBtnVisible;	    //展开 折叠
	QComboBox *m_pCmbDataBits;		//数据位
	QComboBox *m_pCmbParity;		//校验位
	QComboBox *m_pCmbStopBits;		//停止位
	QComboBox *m_pCmbFlowControl;	//流控制
	QRadioButton *m_pRadbtnAsync;
	QRadioButton *m_pRadbtnSync;
	QPushButton *m_pBtnOpenClose;	//打开关闭

	//接收
	QRadioButton *m_pRadbtnRecASCII;
	QRadioButton *m_pRadbtnRecHex;
	QCheckBox *m_pChkShowMsg;       //接收框是否显示消息
	QPushButton *m_pBtnClear;	    //清除数据

	//发送
	QRadioButton *m_pRadbtnSendASCII;
	QRadioButton *m_pRadbtnSendHex;
	QCheckBox *m_pChkWordwrap;      //发送自动换行
	QPushButton *m_pBtnExpand;	    //扩展
	QCheckBox *m_pChkRepeatSend;    //重复发送
	QSpinBox *m_pSpboxTimes;

	//状态栏
	QLabel *m_pLabConnInfo;
	QLabel *m_pLabTx;
	QLabel *m_pLabRx;
	QPushButton *m_pLabClearZero;

	//右边窗口
	QTextEdit *m_pEditMsg;		//显示消息列表
	QStringList editMsg;
	int maxMsgRow;				//显示消息最大行数
	QTextEdit *m_pEditSend;		//发送消息列表
	QPushButton *m_pBtnSend;	//发送按钮
	QComboBox *m_pCmbHistory;	//记录曾经发送的记录
#pragma  endregion
	int m_iRxCount = 0;		//Rx 计数
	int m_iTxCount = 0;		//Tx 计数
protected:
	void closeEvent(QCloseEvent *event);

};

//函数模板的实现和申明要写在同一个文件里面
//Hex(QString/QByteArray)十六进制转字符串(QByteArray)类型
/**
* @brief			16进制编码字符(QByteArray)转字符串(QByteArray)类型；
* @param			QByteArray/QString 16进制编码字符
* @return			QByteArray		   字符串
*/
template<typename T>
QByteArray ChildWidget::hexToString(T data) {
	QString strData = data;
	strData = strData.simplified().replace(" ", ""); //去除多余的空格
	int n = strData.length();
	while (n - 2 > 0)
	{
		n = n - 2;
		strData.insert(n, " ");
	} //每两个字符之间插入空格
	QStringList strDataList = strData.split(" ");
	QByteArray result_str = "";
	for (int i = 0; i< strDataList.size(); ++i)
	{
		QString tmp_str = strDataList.at(i);
		QString st = tmp_str.toInt((bool *)nullptr, 16);
		result_str.append(st);
		//qDebug("read==%s==", qPrintable(result_str));
	}
	return result_str;
}

/**
* @brief			字符串(QByteArray)转Hex(QByteArray)16进制类型；
* @param			QByteArray 字符串
* @return			QByteArray 16进制编码字符
*/
template<typename T>
QByteArray ChildWidget::stringToHex(T strData)
{
	QString data = strData;
	QByteArray temp_str = data.toLatin1().toHex().toUpper();
	int n = temp_str.length();
	while (n - 2 > 0)
	{
		n = n - 2;
		temp_str.insert(n, " ");
	}

	return temp_str;
}