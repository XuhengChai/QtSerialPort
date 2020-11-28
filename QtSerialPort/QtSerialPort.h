#pragma once

#include "ChildWidget.h"
#include <Windows.h>
#include <QtWidgets/QtWidgets>

class QtSerialPort : public QMainWindow
{
    Q_OBJECT

public:
    QtSerialPort(QWidget *parent = Q_NULLPTR);

private:
	void initViewer();
	void createMenu();
	void createToolBtns();
	void initConnect();
	void createCenWgt();
	//void showHintLabel(QString strText, QString strFontSize = "16px",
	//	QString strColor = "#ffffff", QString strBgColor = "#666666"); //一个提示便签  会淡化消失

	Q_SLOT void insertSubTab();			    //新建子页面
	Q_SLOT void removeSubTab(int index);    //移除子页面
	Q_SLOT void clearAllMsg();				//清除所有接收数据
	Q_SLOT void setLogRecordState();		//设置是否记录日志
	Q_SLOT void viewCurDataLog();			//查看当前日志TXT
	Q_SLOT void about();					//关于作者
protected:
	void closeEvent(QCloseEvent *event) override;
#pragma region 菜单相关
	QMenu *m_pFileMenu;
	QMenu *m_pEditMenu;
	QMenu *m_pHelpMenu;
	
	QAction *m_pNewAction;
	QAction *m_pRecordLogAction;
	QAction *m_pViewCurLogActin;
	QAction *m_pExitAction;
	QAction *m_pClearAction;
	QAction *m_pAboutAction;

#pragma endregion

	QToolBar *m_pToolsBar;
	QMenuBar *m_pMenuBar;
	QScrollArea *m_pCenScroll;      // 滚动条
	QList<ChildWidget *> m_listChildWidget;
	QTabWidget *m_pTabWidget;
	ChildWidget *childWgt;
	QList<QString> m_strLogPath;			//记录下日志路径
};