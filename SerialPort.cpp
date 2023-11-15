#include<tchar.h>
#include<iostream>
//#include<map>
#include <windows.h>
//using namespace  std;
#include"SerialPort.h"

map<int, HANDLE>  SerialPort::ComMap;  

//���캯��
SerialPort::SerialPort()
{

} //���캯��
//��������
SerialPort::~SerialPort()
{

}

//��ȡ�����б�
UINT SerialPort::GetPortNum()
{
	LPCTSTR Reg_Path = _T("HARDWARE\\DEVICEMAP\\SERIALCOMM");  //����ע���·��
	//TCHAR pnum[8];
	HKEY H_Key;
	static UINT portlist[255];									//һ�������255�����鹻���ˡ�
	long Status;
	UINT PortNum;
	DWORD count = 0;//������ֵ������
	DWORD Com_Length, Type_Length, Type;
	TCHAR Com_Name[256];//�洢��������
	TCHAR Type_Name[256];//�洢��������	
	Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, Reg_Path, 0, KEY_READ, &H_Key); //��ע�����������long
	memset(portlist, 0, sizeof(portlist)); // ��0

	if (Status) //����ֵΪ0��ʾ�򿪳ɹ�����Ϊ0���ʾ��ʧ��
	{
		return 0;
	}

	while (true)
	{
		Com_Length = 256;						//�洢�������ȣ�ÿ�ζ�Ҫ��һ����һ���ֵ����Ȼ�´ο��ܻ�ʧ��,Com_lengthֵ����
		Type_Length = 256;
		PortNum = 0;
		Status = RegEnumValue(H_Key, count++, Type_Name, &Type_Length
			, 0, &Type, PUCHAR(Com_Name), &Com_Length);
		if (Status)	//��ѯʧ��˵����������������ѭ����
		{
			RegCloseKey(H_Key);//�ر�ע���
			break;
		}

		for (int i = 3; Com_Name[i]; i++)   //ת��Ϊ����
		{
			PortNum = PortNum * 10 + (Com_Name[i] - '0');
		}
		  
		portlist[count - 1] = PortNum;
		/*if (CheckPort(PortNum) == 0) //�����Ѵ�,��ȡ�������������ŵ�Map��
		{
			volatile HANDLE H_Com = 0;
			TCHAR PortName[20];
			//�����ںŴ�ӡ���ַ�������÷�ʽ���Դ�10���ϴ���
			_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNum);
			H_Com = CreateFile(PortName,                
				GENERIC_READ | GENERIC_WRITE,//��д
				0,
				NULL,
				OPEN_EXISTING,  //�Ѿ����ڵ�
				0,
				0); //�򿪴��ڣ�ͬ����ʽ
			ComMap[PortNum] = H_Com;		// ����map
		}*/

	}

	//*PortList = portlist;      //�ض���ָ��
	return (count - 1);
}

//_stprintf_s(DcbPara, _T("baud=%d parity=%c data=%d stop=%d"), 9600, 'N', 8, 1);
//�򿪶˿�
HANDLE SerialPort::OpenPort(UINT PortNo)   //�򿪶˿�
{


	HANDLE H_Com;		//���ھ��

	TCHAR PortName[20];
	//�����ںŴ�ӡ���ַ�������÷�ʽ���Դ�10���ϴ���
	_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNo);

	H_Com = CreateFile(PortName,
		GENERIC_READ | GENERIC_WRITE,//��д
		0,
		NULL,
		OPEN_EXISTING,  //�Ѿ����ڵ�
		0,
		0); //�򿪴��ڣ�ͬ����ʽ
	if (H_Com == INVALID_HANDLE_VALUE)//������ڴ�ʧ��
	{
		return INVALID_HANDLE_VALUE;
	}

	SetupComm(H_Com, 1024, 1024); //���ö�д������Ϊ1024;

	ComMap[PortNo] = H_Com;		// �򿪳ɹ��򣬴���map
	return H_Com;
}
//�رն˿�
void SerialPort::ClosePort(UINT PortNo)	//�رմ���
{
	if (PortState(PortNo)) //�����Ǵ򿪵�
	{
		HANDLE H_Com = ComMap[PortNo];  //��ȡ���
		CloseHandle(H_Com);
		ComMap.erase(PortNo);			//��map�����

	}

}

//�ر����ж˿�
void SerialPort::ClearAllPort()	//�رմ���
{
	for (map<int, HANDLE>::iterator iter = ComMap.begin(); iter != ComMap.end();iter++ )
	{
		CloseHandle(iter->second);					//�رմ���
		iter = ComMap.erase(iter);					//��������д ���������ʧЧ�������쳣��
	}

}

//��ʼ������
int SerialPort::InitPort(UINT PortNo, UINT Baud, byte Parity, byte Data, byte Stop)
{

	if (PortState(PortNo))   //����˿��Ѿ�����
	{
		ClosePort(PortNo);  //�ص��˿�
	}

	if (Parity < 0 || Parity >4)  //parity 0~4
		return 0;

	if (Stop < 0 || Stop >2) //stop 0~2
		return 0;

	HANDLE H_Com;  //���ھ��
	//TCHAR DcbPara[50];			//����Dcb�ṹ
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
	if (!SetCommTimeouts(H_Com, &ComTimeouts))//���ó�ʱ
	{

		return 0;
	}
	if (!GetCommState(H_Com, &S_dcb))		//��ȡ����DCB�ṹ
	{

		return 0;
	}

	S_dcb.BaudRate = Baud;
	S_dcb.ByteSize = Data;
	S_dcb.Parity = Parity;
	S_dcb.StopBits = Stop;
	//if(!BuildCommDCB(DcbPara , &S_dcb))//����DCB
	//{

	//	return 0;
	//}

	S_dcb.fRtsControl = RTS_CONTROL_ENABLE; //����Rts�ź�
	if (!SetCommState(H_Com, &S_dcb))	//����DCB
	{
		int i = GetLastError();


		return 0;
	}

	PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_TXCLEAR |
		PURGE_RXABORT | PURGE_TXABORT); //��ջ�����

	return 1;
}
int SerialPort::PortState(UINT PortNo)  //ֻ�����ڼ���Ѿ��򿪵Ĵ��ڣ������ǳ����������Ĵ������Ϣ
{
	map<int, HANDLE>::iterator iter;

	iter = ComMap.find(PortNo);

	if (iter != ComMap.end())  //˵�����ҵ��˶˿�
		return 1;
	else
		return 0;
}

// ��鴮���ǹ����ڴ�״̬
bool SerialPort::CheckPort(UINT PortNo)
{
		volatile HANDLE H_Com = 0;
		TCHAR PortName[20];
		//�����ںŴ�ӡ���ַ�������÷�ʽ���Դ�10���ϴ���
		_stprintf_s(PortName, _T("\\\\.\\COM%d"), PortNo);
		H_Com=CreateFile(PortName,
			GENERIC_READ | GENERIC_WRITE,//��д
			0,
			NULL,
			OPEN_EXISTING,  //�Ѿ����ڵ�
			0,
			0); //�򿪴��ڣ�ͬ����ʽ

		if (H_Com != NULL && H_Com != INVALID_HANDLE_VALUE)
		{
			CloseHandle(H_Com);
			H_Com = NULL;
			return TRUE;        // ����δ��
		}
		else
		{ 
			return FALSE;      // �����Ѵ�

		}
}

//������
int SerialPort::ReadPort(UINT PortNo, char* P_recved, int length) //���غ��� һ�ζ�ȡָ�����ȵ�����
{
	
	volatile HANDLE H_Com = 0;
	if (CheckPort(PortNo) == 1) //����δ��
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
	DWORD Recn;  //ʵ�ʶ�ȡ����
	BOOL Result = TRUE;

	Result = ReadFile(H_Com, P_recved, length, &Recn, NULL);
	if (Result == 0)
	{
		int resultValue=GetLastError();  // 998 �ڴ������Ч
		PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_RXABORT);

		return 0;
	}

	return Recn;
}
//��ջ�����
void SerialPort::ClearCom(UINT PortNo)
{
	if (PortState(PortNo) == 0) //����δ��
		return;
	HANDLE H_Com = ComMap[PortNo];
	PurgeComm(H_Com, PURGE_RXCLEAR | PURGE_RXABORT);
}
int SerialPort::CheckRTS(UINT PortNo)
{
	return 0;
}

//д����
int SerialPort::WritePort(UINT PortNo, const char* pData, int length)
{
	volatile HANDLE H_Com = 0;
	if (CheckPort(PortNo) == 1) //����δ��
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
//��ȡҪ��ȡ���ֽ���
UINT SerialPort::GetByte(UINT PortNo) //�ú�����ȡ�������ֽڳ��ȡ�
{
	if (PortState(PortNo) == 0) //����δ��
		return 0;
	HANDLE H_Com = ComMap[PortNo];
	DWORD dwError;
	COMSTAT comstat;
	DWORD BytesInQue = 0;
	DWORD FirstSize = 0;

	memset(&comstat, 0, sizeof(COMSTAT)); //��ʼ����������
	while (true)
	{

		BytesInQue = FirstSize;
		Sleep(10);
		if (ClearCommError(H_Com, &dwError, &comstat))
			FirstSize = comstat.cbInQue; //���¸�ֵ

		if (BytesInQue == FirstSize)
			break;
	}

	return BytesInQue;
}