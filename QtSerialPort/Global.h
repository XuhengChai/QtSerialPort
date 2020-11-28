#pragma once
#include <QtCore/QFlag>
#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QTextStream>

#define MAGICNUMBER (quint32)0xA0B0C0D0
/*文件MagicNumber*/

//Log 日志读写类，单例模式
//本示例中每个字串口页面，根据其名字配一个日志文件
#ifndef _Serport_LOG_
#define LogInfo(...)     Log::GetInstance()->writeLogger("INFO", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
#define LogWarning(...)  Log::GetInstance()->writeLogger("WARNING", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
#define LogError(...)    Log::GetInstance()->writeLogger("ERROR", __FILE__, __LINE__, __FUNCSIG__, __VA_ARGS__)
#endif

class Log
{
public:
	static Log* GetInstance() {
		static Log logInstance;
		return &logInstance;
	}

	void writeLogger(const char* pszLevel, const char* pszFile, int lineNo,
		const char* pszFuncSig, QString logMsg, QString logPath = "") {
		if (logPath.isEmpty()) {
			logPath = m_LogPath;
		}
		QFile file(logPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
		{
			//QMessageBox::critical(NULL, "Warning", "Can not create log file!");
			return;
		}
		QTextStream text_stream(&file);
		QString timeNow = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
		if (!strcmp(pszLevel, "INFO")) {
			text_stream << QString("[%0] [%1] : %2").arg(timeNow).arg(pszLevel).arg(logMsg) << "\r\n";
		}
		else {
			text_stream << QString("[%0] [%1] : %2  --[%3]-- line %4 - Function: %5").arg(timeNow).arg(pszLevel).arg(logMsg)
				.arg(pszFile).arg(lineNo).arg(pszFuncSig) << "\r\n";
		}
		file.flush();
		file.close();
	}
private:
	Log() {
		QString strDate = QDateTime::currentDateTime().toString("yyyyMMdd");
		m_LogPath = QDir::currentPath() + "/log_" + strDate + ".txt";
	}
	~Log() {};
	QString m_LogPath;
	Log(const Log&);//拷贝构造
	//Log& operator=(const Log&);
};

/*当前的运行状态*/
enum EnmCurSendOption
{
	NoneSend = 0,
	/*串口发送*/
	SerialPortSend
};

/*输出消息的方式*/
//enum EnmLogOption
//{
//	NoneLog		= 0x0000,
//	/*控制台输出*/
//	Console		= 0x0010,
//	/*日志文件*/
//	LogFile = 0x0020,
//	/*界面上*/
//	Interface	= 0x0040
//};
//
//Q_DECLARE_FLAGS(EnmLogOptions, EnmLogOption) // typedef QFlags<Enum> Flags
//Q_DECLARE_OPERATORS_FOR_FLAGS(EnmLogOptions) // 给 Flags 定义了运算符 |

/*更新状态栏RX TX的信息*/
enum EnmUpdateRxTx
{
	/*清空RxTx数量*/
	ClearRxTx = 0,
	/*增加Rx的计数*/
	AddRx,
	/*增加Tx的计数*/
	AddTx,
};