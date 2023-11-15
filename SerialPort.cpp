#include<tchar.h>
#include<iostream>
//#include<map>
#include <windows.h>
//using namespace  std;
#include"SerialPort.h"

map<int, HANDLE>  SerialPort::ComMap;  

//构造函数
SerialPort::SerialPort()
{

} //构造函数
//析构函数
SerialPort::~SerialPort()
{

}

//获取串口列表
UINT SerialPort::GetPortNum()
{
	LPCTSTR Reg_Path = _T("HARDWARE\\DEVICEMAP\\SERIALCOMM");  //串口注册表路径
	//TCHAR pnum[8];
	HKEY H_Key;
	static UINT portlist[255];									//一般情况下255个数组够用了。
	long Status;
	UINT PortNum;
	DWORD count = 0;//遍历键值计数。
	DWORD Com_Length, Type_Length, Type;
	TCHAR Com_Name[256];//存储串口名字
	TCHAR Type_Name[256];//存储串口类型	
	Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, Reg_Path, 0, KEY_READ, &H_Key); //打开注册表表键，返回long
	memset(portlist, 0, sizeof(portlist)); // 清0

	if (Status) //返回值为0表示打开成功，不为0则表示打开失败
	{
		return 0;
	}

	while (true)
	{
		Com_Length = 256;						//存储两个长度，每次都要赋一个大一点的值，不然下次可能会失败,Com_length值翻倍
		Type_Length = 256;
		PortNum = 0;
		Status = RegEnumValue(H_Key, count++, Type_Name, &Type_Length
			, 0, &Type, PUCHAR(Com_Name), &Com_Length);
		if (Status)	//查询失败说明遍历结束，跳出循环。
		{
			RegCloseKey(H_Key);//关闭注册表
			break;
		}

		for (int i = 3; Com_Name[i]; i++)   //转换为数字
		{
			PortNum = PortNum * 10 + (Com_Name[i] - '0');
		}
		  
		portlist[count - 1] = PortNum;
		/*if (CheckPort(PortNum) == 0) //串口已打开,获取句柄，并将句柄放到Map中
		{
			volatile HANDLE H_Com = 0;
			TCHAR PortName[20];
			//将串口号打印到字符数组里，该方式可以打开10以上串口
			_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNum);
			H_Com = CreateFile(PortName,                
				GENERIC_READ | GENERIC_WRITE,//读写
				0,
				NULL,
				OPEN_EXISTING,  //已经存在的
				0,
				0); //打开串口，同步方式
			ComMap[PortNum] = H_Com;		// 存入map
		}*/

	}

	//*PortList = portlist;      //重定向指针
	return (count - 1);
}

//_stprintf_s(DcbPara, _T("baud=%d parity=%c data=%d stop=%d"), 9600, 'N', 8, 1);
//打开端口
HANDLE SerialPort::OpenPort(UINT PortNo)   //打开端口
{


	HANDLE H_Com;		//串口句柄

	TCHAR PortName[20];
	//将串口号打印到字符数组里，该方式可以打开10以上串口
	_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNo);

	H_Com = CreateFile(PortName,
		GENERIC_READ | GENERIC_WRITE,//读写
		0,
		NULL,
		OPEN_EXISTING,  //已经存在的
		0,
		0); //打开串口，同步方式
	if (H_Com == INVALID_HANDLE_VALUE)//如果串口打开失败
	{
		return INVALID_HANDLE_VALUE;
	}

	SetupComm(H_Com, 1024, 1024); //设置读写缓冲区为1024;

	ComMap[PortNo] = H_Com;		// 打开成功则，存入map
	return H_Com;
}
//关闭端口
void SerialPort::ClosePort(UINT PortNo)	//关闭串口
{
	if (PortState(PortNo)) //串口是打开的
	{
		HANDLE H_Com = ComMap[PortNo];  //获取句柄
		CloseHandle(H_Com);
		ComMap.erase(PortNo);			//从map中清除

	}

}

//关闭所有端口
void SerialPort::ClearAllPort()	//关闭串口
{
	for (map<int, HANDLE>::iterator iter = ComMap.begin(); iter != ComMap.end();iter++ )
	{
		CloseHandle(iter->second);					//关闭串口
		iter = ComMap.erase(iter);					//必须这样写 否则迭代器失效会引发异常。
	}

}

//初始化串口
int SerialPort::InitPort(UINT PortNo, UINT Baud, byte Parity, byte Data, byte Stop)
{

	if (PortState(PortNo))   //如果端口已经存在
	{
		ClosePort(PortNo);  //关掉端口
	}

	if (Parity < 0 || Parity >4)  //parity 0~4
		return 0;

	if (Stop < 0 || Stop >2) //stop 0~2
		return 0;

	HANDLE H_Com;  //串口句柄
	//TCHAR DcbPara[50];			//串口Dcb结构
	//_stprintf_s(DcbPara, _T("baud=%d parity=%c data=%d stop=%d") , Baud, Parity, Data, Stop);
	H_Com = OpenPort(PortNo);

	if (H_Com == INVALID_HANDLE_VALUE)
	{
		return 0;
	}


	//int count = 0;
	COMMTIMEOUTS ComTimeouts = { 10, 0, 0, 0, 0 };
	/*
	ComTimeouts.ReadIntervalTimeout = 0;
	ComTimeouts.ReadTotalTimeoutConstant = 0;
	ComTimeouts.ReadTotalTimeoutMultiplier = 0;
	ComTimeouts.WriteTotalTimeoutConstant = 0;
	ComTimeouts.WriteTotalTimeoutMultiplier = 0;
	*/
	DCB S_dcb;
	if (!SetCommTimeouts(H_Com, &ComTimeouts))//设置超时
	{

		return 0;
	}
	if (!GetCommState(H_Com, &S_dcb))		//获取现有DCB结构
	{

		return 0;
	}

	S_dcb.BaudRate = Baud;
	S_dcb.ByteSize = Data;
	S_dcb.Parity = Parity;
	S_dcb.StopBits = Stop;
	//if(!BuildCommDCB(DcbPara , &S_dcb))//构建DCB
	//{

	//	return 0;
	//}

	S_dcb.fRtsControl = RTS_CONTROL_ENABLE; //允许Rts信号
	if (!SetCommState(H_Com, &S_dcb))	//设置DCB
	{
		int i = GetLastError();


		return 0;
	}

	PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_TXCLEAR |
		PURGE_RXABORT | PURGE_TXABORT); //清空缓冲区

	return 1;
}
int SerialPort::PortState(UINT PortNo)  //只能用于检测已经打开的串口，而且是程序自身存入的打开与否信息
{
	map<int, HANDLE>::iterator iter;

	iter = ComMap.find(PortNo);

	if (iter != ComMap.end())  //说明查找到了端口
		return 1;
	else
		return 0;
}

// 检查串口是够处于打开状态
bool SerialPort::CheckPort(UINT PortNo)
{
		volatile HANDLE H_Com = 0;
		TCHAR PortName[20];
		//将串口号打印到字符数组里，该方式可以打开10以上串口
		_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNo);
		H_Com=CreateFile(PortName,
			GENERIC_READ | GENERIC_WRITE,//读写
			0,
			NULL,
			OPEN_EXISTING,  //已经存在的
			0,
			0); //打开串口，同步方式

		if (H_Com != NULL && H_Com != INVALID_HANDLE_VALUE)
		{
			CloseHandle(H_Com);
			H_Com = NULL;
			return TRUE;        // 串口未打开
		}
		else
		{ 
			return FALSE;      // 串口已打开

		}
}

//读串口
int SerialPort::ReadPort(UINT PortNo, char* P_recved, int length) //重载函数 一次读取指定长度的数据
{
	
	volatile HANDLE H_Com = 0;
	if (CheckPort(PortNo) == 1) //串口未打开
	{
		H_Com = OpenPort(PortNo);
	}
	else
	{
		if (PortState(PortNo) == 1)
			H_Com = ComMap[PortNo];
		else
			return 0;
		    
	}
	DWORD Recn;  //实际读取长度
	BOOL Result = TRUE;

	Result = ReadFile(H_Com, P_recved, length, &Recn, NULL);
	if (Result == 0)
	{
		int resultValue=GetLastError();  // 998 内存分配无效
		PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_RXABORT);

		return 0;
	}

	return Recn;
}
//清空缓冲区
void SerialPort::ClearCom(UINT PortNo)
{
	if (PortState(PortNo) == 0) //串口未打开
		return;
	HANDLE H_Com = ComMap[PortNo];
	PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_RXABORT);
}
int SerialPort::CheckRTS(UINT PortNo)
{
	return 0;
}

//写串口
int SerialPort::WritePort(UINT PortNo, const char* pData, int length)
{
	volatile HANDLE H_Com = 0;
	if (CheckPort(PortNo) == 1) //串口未打开
	{   
		H_Com =OpenPort(PortNo);
	}
	else
	{   
		if (PortState(PortNo) == 1)
			H_Com = ComMap[PortNo];
		else
			return 0;
	}

	if (length == 0)
		length = strlen(pData);

	DWORD Recn;
	BOOL Result = TRUE;

	if (H_Com == INVALID_HANDLE_VALUE)
		return -1;

	Result = WriteFile(H_Com, pData, length, &Recn, NULL);
	if (Result == 0)
	{
		int resultValue = GetLastError();
		PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_RXABORT);
		return 0;
	}

	return Recn;

}
//获取要读取的字节数
UINT SerialPort::GetByte(UINT PortNo) //该函数获取缓冲区字节长度。
{
	if (PortState(PortNo) == 0) //串口未打开
		return 0;
	HANDLE H_Com = ComMap[PortNo];
	DWORD dwError;
	COMSTAT comstat;
	DWORD BytesInQue = 0;
	DWORD FirstSize = 0;

	memset(&comstat, 0, sizeof(COMSTAT)); //初始化缓冲区；
	while (true)
	{

		BytesInQue = FirstSize;
		Sleep(10);
		if (ClearCommError(H_Com, &dwError, &comstat))
			FirstSize = comstat.cbInQue; //重新赋值

		if (BytesInQue == FirstSize)
			break;
	}

	return BytesInQue;
}