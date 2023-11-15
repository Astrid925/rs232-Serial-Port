#ifndef _SERIAIPORT_H_
#define _SERIAIPORT_H_
#include <windows.h>
#include <map>
using namespace std;
//通用的串口类，通过List存储串口
class  SerialPort
{

public:
	SerialPort();
	~SerialPort();
private:

	//使用CreateFile打开串口，同步方式打开
	HANDLE OpenPort(UINT PortNo = 1);

public:
	//通过查询注册表方式获取系统里面的串口，返回串口数量，指针指向串口列表
	 UINT GetPortNum();

	//从指定串口缓冲区读取数据
	 UINT GetByte(UINT PortNo);

	//关闭指定串口，并从map删除
	 void ClosePort(UINT PortNo);

	//关闭所有串口
	 void ClearAllPort();

	//初始化指定端口，成功后存入map
	 int InitPort(UINT PortNo, UINT Baud = 9600, byte Parity = 0, byte Data = 8, byte Stop = 0); //初始化串口

	//从指定端口读取数据,返回值表示读取的bytes， 	
	 int ReadPort(UINT PortNo, char* P_recved, int length);

	//往指定串口发送数据,返回实际写入
	 int WritePort(UINT PortNo, const char* pData, int length = 0);

	//检查指定串口状态
	int PortState(UINT PortNo);

	//清空指定串口缓冲区
	void ClearCom(UINT PortNo);

	//指定端口RTS信号检测，用于脚踏开关信号触发，
	 int CheckRTS(UINT PortNo);
	
	 //判断串口是否处于打开状态
	 bool CheckPort(UINT PortNo);

public:
	//static UINT PortNum;				//串口端口号
	static map<int, HANDLE> ComMap;

	//static CRITICAL_SECTION C_com;		//关键段,数据保护
};

#endif

