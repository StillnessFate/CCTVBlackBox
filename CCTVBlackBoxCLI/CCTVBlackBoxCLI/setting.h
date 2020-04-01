#pragma once
#pragma warning(disable:4996)
#include <Windows.h>
#include <atlstr.h>
#include <utility>
#include <time.h>

#define DEFAULT_TIMER_INTERVAL 500
#define DEFAULT_ALARM_INTERVAL 60
#define DEFAULT_DELETE_INTERVAL 7
#define DEFAULT_SAVE_INTERVAL {10, 180}
#define DEFAULT_CORNER_SIZE 100
#define DEFAULT_THRESHOLD 0.004F
#define DEFAULT_MATCHING 0.8F

using namespace std;

class CSetting {
private:
	pair<int, int> saveInterval;//minimum, maximum
	int deleteInterval;//day
	int timerInterval;//ms
	int alarmInterval;//sec
	int cornerSize;//ea
	float threshold;//corner threshold
	float matching;//matching rate
	char mainPath[256];
	char imagePath[256];
	char settingPath[256];
	UINT_PTR timerId;
	bool debug;
	bool alarm;
public:
	CSetting();
	void setSaveInterval(pair<int, int> savedelay);
	void setTimerInterval(int timerinterval);
	void setAlarmInterval(int alarminterval);
	void setDeleteInterval(int deleteInterval);
	void setTimerId(UINT_PTR id);
	void setCornerSize(int size);
	void setThreshold(float th);
	void setMatching(float rate);
	void setDebug(bool chk);
	void setAlarm(bool chk);
	pair<int, int>getSaveInterval();
	int getDeleteInterval();
	int getTimerInterval();
	int getAlarmInterval();
	int getCornerSize();
	float getThreshold();
	float getMatching();
	UINT_PTR getTimerId();
	char* getMainPath();
	char* getImagePath();
	bool getDebug();
	bool getAlarm();

	void readSetting(const char* var, int* ret, int def);
	void readSetting(const char* var, float* ret, float def);
	void readSetting(const char* var, pair<int, int>* ret, pair<int, int> def);
	void writeSetting(const char* var, int data);
	void writeSetting(const char* var, float data);
	void writeSetting(const char* var, pair<int, int> data);
};

bool dirExists(const char *dirName);
void deleteDirectory(char *RootDir);