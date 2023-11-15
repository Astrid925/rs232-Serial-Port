#include <iostream>
#include <cstdlib>
#include "SerialPort.h"
#include <windows.h>
#include <tchar.h>

using namespace std;

int main()
{  
 
    SerialPort com;
    int countNum = com.GetPortNum();
    if (com.InitPort(1, 9600, 0, 8, 0))
    {
        cout << "COM1 is open!" << endl;
    }
    else
    {
        cout << "Fail to open COM1 !" << endl;
    }

    //²âÊÔ·¢ËÍ
    /*if (com.WritePort(2, "Hello World!"))
    {
        cout << "send success" << endl;
    }
    else
    {
        cout << "send fail" << endl;
    }*/

    // ²âÊÔ½ÓÊÕ
    char* pData = new char[14];
    if (com.ReadPort(2, pData,12))
    {
        cout << "receive success" << endl;
    }
    else
    {
        cout << "receive fail" << endl;
    }

    delete[] pData;
    
    //com.ClosePort(1);
    //com.ClearAllPort();

    return 0;
}
