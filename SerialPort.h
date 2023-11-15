#ifndef _SERIAIPORT_H_
#define _SERIAIPORT_H_
#include <windows.h>
#include <map>
using namespace std;
//ͨ�õĴ����࣬ͨ��List�洢����
class  SerialPort
{

public:
	SerialPort();
	~SerialPort();
private:

	//ʹ��CreateFile�򿪴��ڣ�ͬ����ʽ��
	HANDLE OpenPort(UINT PortNo = 1);

public:
	//ͨ����ѯע���ʽ��ȡϵͳ����Ĵ��ڣ����ش���������ָ��ָ�򴮿��б�
	 UINT GetPortNum();

	//��ָ�����ڻ�������ȡ����
	 UINT GetByte(UINT PortNo);

	//�ر�ָ�����ڣ�����mapɾ��
	 void ClosePort(UINT PortNo);

	//�ر����д���
	 void ClearAllPort();

	//��ʼ��ָ���˿ڣ��ɹ������map
	 int InitPort(UINT PortNo, UINT Baud = 9600, byte Parity = 0, byte Data = 8, byte Stop = 0); //��ʼ������

	//��ָ���˿ڶ�ȡ����,����ֵ��ʾ��ȡ��bytes�� 	
	 int ReadPort(UINT PortNo, char* P_recved, int length);

	//��ָ�����ڷ�������,����ʵ��д��
	 int WritePort(UINT PortNo, const char* pData, int length = 0);

	//���ָ������״̬
	int PortState(UINT PortNo);

	//���ָ�����ڻ�����
	void ClearCom(UINT PortNo);

	//ָ���˿�RTS�źż�⣬���ڽ�̤�����źŴ�����
	 int CheckRTS(UINT PortNo);
	
	 //�жϴ����Ƿ��ڴ�״̬
	 bool CheckPort(UINT PortNo);

public:
	//static UINT PortNum;				//���ڶ˿ں�
	static map<int, HANDLE> ComMap;

	//static CRITICAL_SECTION C_com;		//�ؼ���,���ݱ���
};

#endif

