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
	
	hThread = (HANDLE)_beginthreadex(NULL, 0, commandIO, NULL, 0, (unsigned*)&threadID);

	m_idEvent = timeSetEvent(setting->getTimerInterval(), 0, (LPTIMECALLBACK)screenCapture, NULL, TIME_PERIODIC);

	extern bool commandIOAct;
	MSG msg;
	while (commandIOAct);

	timeKillEvent(m_idEvent);

	return 0;
}