#include "QtSerialPort.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	//AllocConsole();//打开控制台
	//freopen("CON", "w", stdout);//将输出定向到控制台
    QApplication a(argc, argv);
	//Style defult: windowsvista; optional: Windows, Fusion
	QApplication::setStyle(QStyleFactory::create("Fusion"));
    QtSerialPort w;
    w.show();
    return a.exec();
}