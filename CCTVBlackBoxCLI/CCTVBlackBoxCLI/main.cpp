#include "imageProcessing.h"
#include "setting.h"
#include "command.h"
#include <mmSystem.h>

#pragma comment (lib, "Winmm.lib")

using namespace std;

MMRESULT m_idEvent;

int main(int argc, char* argv[]) {
//	ShowWindow(GetConsoleWindow(), SW_HIDE);
	extern CSetting* setting;
	extern vector<CMonitor> monitor;
	setting = new CSetting;

	char name[128];
	while(true){
		cin.getline(name, sizeof(name));
		if (!name[0])break;
		monitor.push_back(name);
	}

	HANDLE hThread;
	DWORD threadID;
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, L"CCTVBlackBox_CLI");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"�̹� �ٸ� �ν��Ͻ��� �������Դϴ�.", L"���� ����", MB_OK);
		return 0;
	}

	hThread = (HANDLE)_beginthreadex(NULL, 0, commandIO, NULL, 0, (unsigned*)&threadID);

	m_idEvent = timeSetEvent(setting->getTimerInterval(), 10, (LPTIMECALLBACK)&screenCapture, 0, TIME_PERIODIC); //��Ƽ�̵�� Ÿ�̸�
	if (m_idEvent != 0) {
		setting->setTimerId(m_idEvent);
	}

	hMutex = OpenMutex(SYNCHRONIZE, TRUE, L"CCTVBlackBox");
	if (hMutex != NULL)
		WaitForSingleObject(hMutex, INFINITE);

	exit(0);
	
	return 0;
}