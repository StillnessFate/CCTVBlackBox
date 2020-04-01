#include "setting.h"
#include "imageProcessing.h" 

CSetting::CSetting() {
	sprintf_s(mainPath, "C:\\Program Files\\CCTVBlackBox");
	if (!dirExists(mainPath)) {
		_mkdir(mainPath);
	}
	sprintf_s(imagePath, "%s\\image", mainPath);
	if (!dirExists(imagePath)) {
		_mkdir(imagePath);
	}
	sprintf_s(settingPath, "%s\\setting.ini", mainPath);

	readSetting("timerInterval", &timerInterval, DEFAULT_TIMER_INTERVAL);
	readSetting("alarmInterval", &alarmInterval, DEFAULT_ALARM_INTERVAL);
	readSetting("deleteInterval", &deleteInterval, DEFAULT_DELETE_INTERVAL);
	readSetting("saveInterval", &saveInterval, DEFAULT_SAVE_INTERVAL);
	readSetting("CornerSize", &cornerSize, DEFAULT_CORNER_SIZE);
	readSetting("threshold", &threshold, DEFAULT_THRESHOLD);
	readSetting("matching", &matching, DEFAULT_MATCHING);

	alarm = TRUE;
	debug = FALSE;
}

void CSetting::setSaveInterval(pair<int, int> saveinterval) {
	saveInterval = saveinterval;
	writeSetting("saveInterval", saveInterval);
}

void CSetting::setTimerInterval(int timerinterval) {
	timerInterval = timerinterval;
	writeSetting("timerInterval", timerInterval);
	extern MMRESULT m_idEvent;
	timeKillEvent(m_idEvent);
	m_idEvent = timeSetEvent(getTimerInterval(), 0, (LPTIMECALLBACK)screenCapture, NULL, TIME_PERIODIC);
}

void CSetting::setAlarmInterval(int alarminterval) {
	alarmInterval = alarminterval;
	writeSetting("alarmInterval", alarmInterval);
}

void CSetting::setDeleteInterval(int deleteinterval) {
	deleteInterval = deleteinterval;
	writeSetting("deleteInterval", deleteinterval);
}

void CSetting::setCornerSize(int size) {
	cornerSize = size;
	writeSetting("cornerSize", cornerSize);
}

void CSetting::setThreshold(float th) {
	threshold = th;
	writeSetting("threshold", threshold);
}

void CSetting::setMatching(float rate) {
	matching = rate;
	writeSetting("matching", matching);
}

void CSetting::setTimerId(UINT_PTR id) {
	timerId = id;
}

void CSetting::setDebug(bool chk) {
	debug = chk;
}

void CSetting::setAlarm(bool chk) {
	alarm = chk;
}

pair<int, int> CSetting::getSaveInterval() {
	return saveInterval;
}

int CSetting::getDeleteInterval() {
	return deleteInterval;
}
int CSetting::getTimerInterval() {
	return timerInterval;
}

int CSetting::getAlarmInterval() {
	return alarmInterval;
}

int CSetting::getCornerSize() {
	return cornerSize;
}

float CSetting::getThreshold() {
	return threshold;
}

float CSetting::getMatching() {
	return matching;
}

UINT_PTR CSetting::getTimerId() {
	return timerId;
}

char* CSetting::getMainPath() {
	return mainPath;
}

char* CSetting::getImagePath() {
	return imagePath;
}

bool CSetting::getDebug() {
	return debug;
}

bool CSetting::getAlarm() {
	return alarm;
}

void CSetting::readSetting(const char* var, int* ret, int def) {
	char buf[32];
	if (!GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath)) {
		writeSetting(var, def);
		GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath);
	}
	*ret = atoi(buf);
}

void CSetting::readSetting(const char* var, float* ret, float def) {
	char buf[32];
	if (!GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath)) {
		writeSetting(var, def);
		GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath);
	}
	*ret = (float)atof(buf);
}

void CSetting::readSetting(const char* var, pair<int, int> * ret, pair<int, int> def) {
	char buf[32];
	if (!GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath)) {
		writeSetting(var, def);
		GetPrivateProfileStringA("variable", var, NULL, buf, sizeof(buf), settingPath);
	}
	sscanf_s(buf, "{%d, %d}", &(ret->first), &(ret->second));
}

void CSetting::writeSetting(const char* var, int data) {
	char buf[32];
	sprintf_s(buf, "%d", data);
	WritePrivateProfileStringA("variable", var, buf, settingPath);
}

void CSetting::writeSetting(const char* var, float data) {
	char buf[32];
	sprintf_s(buf, "%f", data);
	WritePrivateProfileStringA("variable", var, buf, settingPath);

}

void CSetting::writeSetting(const char* var, pair<int, int> data) {
	char buf[32];
	sprintf_s(buf, "{%d, %d}", data.first, data.second);
	WritePrivateProfileStringA("variable", var, buf, settingPath);
}

bool dirExists(const char *dirName) {
	DWORD ftyp = GetFileAttributesA(dirName);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;	//something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;	 // this is a directory!

	return false;		// this is not a directory!
}

void deleteDirectory(char* RootDir) {
	char folderPath[256];
	char filePath[256];
	HANDLE hFind;
	sprintf_s(folderPath, "%s\\*", RootDir);
	WIN32_FIND_DATAA wfd;
	if ((hFind = FindFirstFileA(folderPath, &wfd)) == INVALID_HANDLE_VALUE)return;
	do {
		sprintf_s(filePath, "%s", wfd.cFileName);
		if (filePath[0] != '.') {
			sprintf_s(filePath, "%s\\%s", RootDir, wfd.cFileName);
			if (wfd.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				deleteDirectory(filePath);
			else
				DeleteFileA(filePath);
		}
	} while (FindNextFileA(hFind, &wfd));
	FindClose(hFind);
	rmdir(RootDir);
}