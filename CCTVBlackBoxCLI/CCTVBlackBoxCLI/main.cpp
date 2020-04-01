#include "imageProcessing.h"
#include "setting.h"
#include "command.h"
#include <windows.h>

#pragma comment(lib, "Winmm.lib")

using namespace std;

extern CSetting* setting;

int main(int argc, char* argv[]) {
	setting = new CSetting;
	HANDLE hThread;
	DWORD threadID;
	HANDLE hMutex;

	hMutex = CreateMutex(NULL, FALSE, L"CCTVBlackBox_CLI");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		MessageBox(NULL, L"이미 다른 인스턴스가 실행중입니다.", L"실행 실패", MB_OK);
		return 0;
	}

	hThread = (HANDLE)_beginthreadex(NULL, 0, commandIO, NULL, 0, (unsigned*)&threadID);

	MMRESULT m_idEvent = timeSetEvent(setting->getTimerInterval(), 10, (LPTIMECALLBACK)&screenCapture, 0, TIME_PERIODIC); //멀티미디어 타이머
	if (m_idEvent != 0) {
		setting->setTimerId(m_idEvent);
	}

	hMutex = OpenMutex(SYNCHRONIZE, TRUE, L"CCTVBlackBox");
	if (hMutex != NULL)
		WaitForSingleObject(hMutex, INFINITE);

	exit(0);
	return 0;
}