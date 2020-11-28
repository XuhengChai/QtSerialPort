# QtSerialPort

#### Description:
This is a separated independent Serial Port Assitant based on QT and VS using QT muti thread. The modules were optimized compared to ordinary Serial Port write and read. 

#### Features:
 - muti serial widgets create and read concurrently
 - serial auto detect
 - port baudrate bytesize parity stopbits settings
 - basic receive data synchronously or asynchronously (ascii and hex)
 - basic send data (ascii and hex)
 - auto send word wrap; Support comments, do not send data after //
 - repeat send data
 - clear received data area
 - expand widget to auto save customized send data
 - send history and select send again
 - send and receive data count and clearing;
 - auto save and load configs
 - Record All Data Log with different level
 - handle error

#### If QT version mismatch
This project is programmed under VS2015 and QT5.12.9. If there is a QT version mismatch, should include your own QT directory
- Solution 1. Right click the project property to find qtproject setting, change QT installation to yourself version
	> If Solution 1 doesnot work, try the following:  
	> * Solution 2. Right click the project solution, change solution's QT version        
	> * Solution 3. Find file with Suffix vcxproj.user : then modify QTDIR macro  
	*Modification example: \<QTDIR>D:\Programs\Qt\5.12.9\msvc2015_64\</QTDIR>

#### Specifications:
- Class RWserial  (rwSerial.h):
	>  The write and read classes are in worker thread, including receiving data synchronously and asynchronously, and sending data asynchronously over the selected serial port. It commucates with the main-GUI thread through Singals and Slots. Thus it doesnot block the main-GUI Interface. 
- Class ChildWidget (ChildWidget.h):
	>  Each child Serial widget are independent to each other. The gui handling fuctions are in the same main-GUI thread, while the write and read fuctions of different serial ports are in different worker threads. Thus muti serial widgets can be created and read concurrently.
- Class Log(Global.h):
	>  Simple log written class with different log levels using singleton pattern. 
- Class QtSerialPort(QtSerialPort.h):
	>  The main widget.

#### Design idea: 
Serial sub widgets and other sub widgets can communicate through the bridge, the main widget. In a sub widget, interface class and work class handling are separated. 

---
**说明：**   
一个独立串口助手，基于QT5.12.9和VS2015，使用QT多线程。

**功能：** 
1. 同时创建读写多个串口      
2. 串口热插拔自动检测（无法检测串口断电）      
3. 端口/波特率/奇偶校验各种设置              
4. 同步或异步接收数据（ascii和hex）              
5. 同步发送数据（ascii和十六进制）              
6. 发送数据自动换行，支持注释，//后面的内容不发送              
7. 重复发送数据             
8. 清除接收数据区              
9. 扩展窗口，自动保存自定义发送数据              
10. 发送历史记录，然后选择再次发送              
11. 发送接收计数，清0；              
12. 自动保存和加载配置              
13. 记录不同level数据日志              
14. 处理错误

**如果qt版本不匹配**   
右键项目属性，找到QTproject Setting，Change Qt installation to yourself version
> 如果上述解决方案无效，尝试以下：
> * 解决方案2. 打开项目，右键项目解决方案，change solution's qt version
> * 解决方案3. 找到.vcxproj.user：修改QTDIR宏
	*修改示例：\<QTDIR>D:\Programs\Qt\5.12.9\msvc2015_64\</QTDIR>
	

#### Demo:	
![QT SerialPort Assistant GIF](https://user-images.githubusercontent.com/70003795/100515716-166ec100-31b9-11eb-922d-9b6790b5b4f8.gif)
