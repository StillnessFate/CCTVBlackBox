#include "imageProcessing.h"
#include "setting.h"
#include "command.h"

using namespace std;

extern CSetting* setting;

int main(int argc, char* argv[]) {
//	ShowWindow(GetConsoleWindow(), SW_HIDE);
	MSG msg;
	setting = new CSetting;

	HANDLE hThread;
	DWORD threadID;
	
	hThread = (HANDLE)_beginthreadex(NULL, 0, commandIO, NULL, 0, (unsigned*)&threadID);

	setting->setTimerId(SetTimer(NULL, 0, setting->getTimerInterval(), (TIMERPROC)&screenCapture));

	extern bool commandIOAct;
	while (GetMessage(&msg, NULL, 0, 0) && commandIOAct) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(NULL, setting->getTimerId());
	return 0;
}