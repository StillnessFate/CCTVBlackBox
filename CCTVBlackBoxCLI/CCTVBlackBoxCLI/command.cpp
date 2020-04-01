#include "command.h"
#include "setting.h"

using namespace std;

extern CSetting *setting;
bool commandIOAct = true;

unsigned int WINAPI commandIO(void *args) {
	char input[256];
	while (true) {
		cin.getline(input, sizeof(input));
		switch (input[0]) {
			case CMD_SETTIMERINTERVAL: {
				int timerInterval;
				sscanf(&input[1], "%d", &timerInterval);
				printf("%d\n", timerInterval);
				setting->setTimerInterval(timerInterval);
			}break;
			case CMD_SETALARMINTERVAL: {
				int alarmInterval;
				sscanf(&input[1], "%d", &alarmInterval);
				setting->setAlarmInterval(alarmInterval);
			}break;
			case CMD_SETSAVEINTERVAL: {
				pair<int, int> saveInterval;
				sscanf(&input[1], "%d %d", &saveInterval.first, &saveInterval.second);
				setting->setSaveInterval(saveInterval);
			}break;
			case CMD_SETCORNERSIZE: {
				int cornerSize;
				sscanf(&input[1], "%d", &cornerSize);
				setting->setCornerSize(cornerSize);
			}break;
			case CMD_SETTHRESHOLD: {
				float threshold;
				sscanf(&input[1], "%f", &threshold);
				setting->setThreshold(threshold);
			}break;
			case CMD_SETMATCHING: {
				float matching;
				sscanf(&input[1], "%f", &matching);
				setting->setMatching(matching);
			}break;
			case CMD_SETDEBUG: {
				int debug;
				sscanf(&input[1], "%d", &debug);
				setting->setDebug(debug);
			}break;
			case CMD_SETALARM: {
				int alarm;
				sscanf(&input[1], "%d", &alarm);
				setting->setAlarm(alarm);
			}break;
			case CMD_SETDELETEINTERVAL: {
				int deleteInterval;
				sscanf(&input[1], "%d", &deleteInterval);
				if (deleteInterval < setting->getDeleteInterval()) {
					time_t currTime;
					time(&currTime);
					char deleteFolderName[256];
					char deleteFolderPath[256];
					struct tm* deleteTm;
					for (int i = setting->getDeleteInterval(); i >= deleteInterval; i--) {
						time_t deleteTime = currTime - (time_t)86400 * i;
						deleteTm = localtime(&deleteTime);
						strftime(deleteFolderName, sizeof(deleteFolderName), "%y%m%d", deleteTm);
						sprintf(deleteFolderPath, "%s\\%s", setting->getImagePath(), deleteFolderName);
						deleteDirectory(deleteFolderPath);
					}
				}
				setting->setDeleteInterval(deleteInterval);
			}break;
			case CMD_SETSETTINGS: {
				int timerInterval;
				int alarmInterval;
				pair<int, int> saveInterval;
				int cornerSize;
				float threshold;
				float matching;
				int debug;
				int alarm;
				int deleteInterval;

				sscanf(&input[1], "%d,%d,%d %d,%d,%f,%f,%d,%d,%d",
					&timerInterval,
					&alarmInterval,
					&saveInterval.first,
					&saveInterval.second,
					&cornerSize,
					&threshold,
					&matching,
					&debug,
					&alarm,
					&deleteInterval);

				setting->setTimerInterval(timerInterval);
				setting->setAlarmInterval(alarmInterval);
				setting->setSaveInterval(saveInterval);
				setting->setCornerSize(cornerSize);
				setting->setThreshold(threshold);
				setting->setMatching(matching);
				setting->setDebug(debug ? 1 : 0);
				setting->setAlarm(alarm ? 1 : 0);
				setting->setDeleteInterval(deleteInterval);

				if (deleteInterval < setting->getDeleteInterval()) {
					time_t currTime;
					time(&currTime);
					char deleteFolderName[256];
					char deleteFolderPath[256];
					struct tm* deleteTm;
					for (int i = setting->getDeleteInterval(); i >= deleteInterval; i--) {
						time_t deleteTime = currTime - (time_t)86400 * i;
						deleteTm = localtime(&deleteTime);
						strftime(deleteFolderName, sizeof(deleteFolderName), "%y%m%d", deleteTm);
						sprintf(deleteFolderPath, "%s\\%s", setting->getImagePath(), deleteFolderName);
						deleteDirectory(deleteFolderPath);
					}
				}
			}break;
			case CMD_QUIT:
				commandIOAct = false;
				return 0;

			case CMD_GETTIMERINTERVAL: {
				printf("%d\n", setting->getTimerInterval());
			}break;
			case CMD_GETALARMINTERVAL: {
				printf("%d\n", setting->getAlarmInterval());
			}break;
			case CMD_GETSAVEINTERVAL: {
				printf("%d %d\n", setting->getSaveInterval().first, setting->getSaveInterval().second);
			}break;
			case CMD_GETCORNERSIZE: {
				printf("%d\n", setting->getCornerSize());
			}break;
			case CMD_GETTHRESHOLD: {
				printf("%f\n", setting->getThreshold());
			}break;
			case CMD_GETMATCHING: {
				printf("%f\n", setting->getMatching());
			}break;
			case CMD_GETDEBUG: {
				printf("%d\n", setting->getDebug() ? 1 : 0);
			}break;
			case CMD_GETALARM: {
				printf("%d\n", setting->getAlarm() ? 1 : 0);
			}break;
			case CMD_GETDELETEINTERVAL: {
				printf("%d\n", setting->getDeleteInterval());
			}break;
			case CMD_GETSETTINGS: {
				printf("INFO|%d,%d,%d %d,%d,%f,%f,%d,%d,%d\n",
					setting->getTimerInterval(),
					setting->getAlarmInterval(),
					setting->getSaveInterval().first,
					setting->getSaveInterval().second,
					setting->getCornerSize(),
					setting->getThreshold(),
					setting->getMatching(),
					setting->getDebug() ? 1 : 0,
					setting->getAlarm() ? 1 : 0,
					setting->getDeleteInterval());
			}break;
		}
		fflush(stdout);
	}
	return -1;
}