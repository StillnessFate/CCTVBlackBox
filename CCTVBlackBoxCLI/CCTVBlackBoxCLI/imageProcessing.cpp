#include "imageProcessing.h"
#include "setting.h"

using namespace std;

CMonitor::CMonitor(char *name) {
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	hWnd = WindowFromPoint(cursorPos);
	strcpy(screenName, name);
	alarmClock = saveClock = clock();
}

pair<int, int> CMonitor::getPoint() {
	RECT rect[2];
	GetWindowRect(hWnd, &rect[0]);
	GetClientRect(hWnd, &rect[1]);
	pair<int, int> point = { rect[0].right - rect[1].right, rect[0].bottom - rect[1].bottom };
	return point;
}

pair<int, int> CMonitor::getSize() {
	RECT rect[2];
	GetWindowRect(hWnd, &rect[0]);
	GetClientRect(hWnd, &rect[1]);
	size = { rect[1].right, rect[1].bottom };
	resize = { size.first / 2 + size.first % 2, size.second / 2 + size.second % 2};
	return size;
}

char* CMonitor::getScreenName() {
	return screenName;
}

vector<CMonitor> monitor;
CSetting* setting;

void CALLBACK screenCapture(UINT timerId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD w2) {
	char fileName[16];
	char filePath[256];
	WCHAR filePathWC[256];
	char folderName[16];
	char imgFolderPath[256];
	time_t currTime;
	struct tm* currTm;
	clock_t start = clock();
	time(&currTime);
	currTm = localtime(&currTime);
	strftime(folderName, sizeof(folderName), "%y%m%d", currTm);
	sprintf(imgFolderPath, "%s\\%s", setting->getImagePath(), folderName);

	if (!dirExists(imgFolderPath)) {
		mkdir(imgFolderPath);
		char deleteFolderName[256];
		char deleteFolderPath[256];
		time_t deleteTime = currTime - (time_t)86400 * setting->getDeleteInterval();
		struct tm* deleteTm;
		deleteTm = localtime(&deleteTime);
		strftime(deleteFolderName, sizeof(deleteFolderName), "%y%m%d", deleteTm);
		sprintf(deleteFolderPath, "%s\\%s", setting->getImagePath(), deleteFolderName);
		deleteDirectory(deleteFolderPath);
	}

	for (int i = 0; i < monitor.size(); i++) {
		strftime(fileName, sizeof(fileName), "%H-%M-%S", currTm);
		char screenFolderPath[256];
		sprintf(screenFolderPath, "%s\\%s", imgFolderPath, monitor[i].getScreenName());
		if (!dirExists(screenFolderPath))
			mkdir(screenFolderPath);
		sprintf(filePath, "%s\\%s.jpeg", screenFolderPath, fileName);
		ConvertCtoWC(filePath, filePathWC);

		monitor[i].imageProcessing(filePathWC);
	}

	clock_t term = clock() - start;
	if (term > setting->getTimerInterval()) {
		setting->setTimerInterval(term * 1.1);
	}
}

bool CMonitor::imageProcessing(const WCHAR* filePathWC) {
	clock_t nowClock = clock();
	getSize();
	int Width = size.first, Height = size.second;
	int resizeWidth = resize.first, resizeHeight = resize.second;

	BYTE *imgArr, *grayArr;
	pair<float, float> *edgeArr;//X, Y
	float *gdx2, *gdy2, *gdxy, *R;

	grayArr = new BYTE[(size_t)resizeWidth * resizeHeight];
	imgArr = new BYTE[(size_t)resizeWidth * resizeHeight];
	edgeArr = new pair<float, float>[(size_t)resizeWidth * resizeHeight];
	gdx2 = new float[(size_t)resizeWidth * resizeHeight];
	gdy2 = new float[(size_t)resizeWidth * resizeHeight];
	gdxy = new float[(size_t)resizeWidth * resizeHeight];
	R = new float[(size_t)resizeWidth * resizeHeight];

	using namespace Gdiplus;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	MSG msg;
	HDC scrdc, memdc;
	HBITMAP membit;
	scrdc = GetDC(hWnd);
	memdc = CreateCompatibleDC(scrdc);
	membit = CreateCompatibleBitmap(scrdc, Width, Height);
	SelectObject(memdc, membit);
	BitBlt(memdc, 0, 0, Width, Height, scrdc, 0, 0, SRCCOPY);

	Bitmap* origin = new Bitmap(membit, NULL);
	Bitmap* change = new Bitmap(resizeWidth, resizeHeight, PixelFormat32bppARGB);

	BitmapData* bitmapData = new BitmapData;
	Rect rect(0, 0, Width, Height);

	origin->LockBits(&rect, ImageLockModeRead, PixelFormat32bppARGB, bitmapData);

	pixelARGB* pixels = (pixelARGB*)(bitmapData->Scan0);

	for (int j = 0; j < Height; j += 2) {//grayscale
		for (int i = 0; i < Width; i += 2) {
			int idx = j * Width + i;
			int resizeIdx = (j * resizeWidth + i) / 2;
			grayArr[resizeIdx] = BYTE_CHK(0.2126f * pixels[idx].R + 0.7152f * pixels[idx].G + 0.0722f * pixels[idx].B);
		}
	}

	origin->UnlockBits(bitmapData);

	for (int j = 1; j < resizeHeight - 1; j++) {//edge detection
		for (int i = 1; i < resizeWidth - 1; i++) {
			int idx = j * resizeWidth + i;
			edgeArr[idx] = { 0, 0 };
			for (int y = 0; y < 3; y++) {
				for (int x = 0; x < 3; x++) {
					int maskIdx = idx + (y - 1) * resizeWidth + (x - 1);
					edgeArr[idx].first += (float)sobelMask[0][y][x] * grayArr[maskIdx];
					edgeArr[idx].second += (float)sobelMask[1][y][x] * grayArr[maskIdx];
				}
			}
			edgeArr[idx].first /= 1020;
			edgeArr[idx].second /= 1020;
		}
	}

	for (int j = 2; j < resizeHeight - 2; j += 2) {//gaussian filter
		for (int i = 2; i < resizeWidth - 2; i += 2) {
			int idx = j * resizeWidth + i;
			gdx2[idx] = gdy2[idx] = gdxy[idx] = 0;
			for (int y = 0; y < 5; y++) {
				for (int x = 0; x < 5; x++) {
					int filterIdx = idx + (y - 2) * resizeWidth + (x - 2);
					gdx2[idx] += edgeArr[filterIdx].first * edgeArr[filterIdx].first * gaussianFilter[y][x];
					gdy2[idx] += edgeArr[filterIdx].second * edgeArr[filterIdx].second * gaussianFilter[y][x];
					gdxy[idx] += edgeArr[filterIdx].first * edgeArr[filterIdx].second * gaussianFilter[y][x];
				}
			}
			gdx2[idx] /= 256;
			gdy2[idx] /= 256;
			gdxy[idx] /= 256;
		}
	}

	Rect resizeRect(0, 0, resizeWidth, resizeHeight);

	change->LockBits(&resizeRect, ImageLockModeWrite, PixelFormat32bppARGB, bitmapData);
	pixels = (pixelARGB*)(bitmapData->Scan0);

	float K = 0.04f;//corner detection
	float th = setting->getThreshold();
	const int maxSize = setting->getCornerSize();
	int startIdx = maxSize * 2, endIdx = maxSize * 2 + 1;
	vector<int>* nowCorner;
	nowCorner = new vector<int>;

	for (int j = 2; j < resizeHeight - 2; j++) {
		for (int i = 2; i < resizeWidth - 2; i++) {
			int idx = j * resizeWidth + i;
			R[idx] = (gdx2[idx] * gdy2[idx] - gdxy[idx] * gdxy[idx]) - K * (gdx2[idx] + gdy2[idx]) * (gdx2[idx] + gdy2[idx]);
			if (R[idx] > th) {
				bool c = true;
				for (int y = -2; c && y <= 2; y++)
					for (int x = -2; c && x <= 2; x++)
						if (R[idx] < R[idx + y * resizeWidth + x])
							c = false;
				if (c) {
					nowCorner->push_back(idx);
				}
			}
			if (setting->getDebug()) {
				pixels[idx] = { grayArr[idx], grayArr[idx], grayArr[idx], 0 };
			}
		}
	}

	//corner matching
	if (nowCorner->size() > maxSize)
		sort(nowCorner->begin(), nowCorner->end(), [=](int a, int b) {
		if (R[a] > R[b]) return true;
		else return false;
			});

	pair<int, pair<int, int>>* cost;
	cost = new pair<int, pair<int, int>>[maxSize * 2 + 2];//cost, <root, idx>
	bool* inQ;
	inQ = new bool[maxSize * 2 + 2];
	vector<pair<int, int>>* edge;
	edge = new vector<pair<int, int>>[maxSize * 2 + 2] ;//dest, cost
	int nowSize = min(maxSize, nowCorner->size());
	int prevSize = min(maxSize, prevCorner->size());

	if (setting->getDebug()) {
		for (int i = 0; i < nowSize; i++) {
			int idx = nowCorner->at(i);
			for (int y = -2; y <= 2; y++) {
				for (int x = -2; x <= 2; x++) {
					pixels[idx + y * resizeWidth + x] = { 0, 0, grayArr[idx + y * resizeWidth + x], 0 };
				}
			}
		}
	}

	for (int i = 0; i < nowSize; i++)
		edge[startIdx].push_back({ i, 0 });
	for (int i = 0; i < prevSize; i++)
		edge[maxSize + i].push_back({ endIdx, 0 });

	for (int j = 0; j < nowSize; j++) {
		for (int i = 0; i < prevSize; i++) {
			int nowX = nowCorner->at(j) % resizeWidth, nowY = nowCorner->at(j) / resizeWidth;
			int prevX = prevCorner->at(i) % resizeWidth, prevY = prevCorner->at(i) / resizeWidth;
			if (abs(nowX - prevX) + abs(nowY - prevY) < 20)
				edge[j].push_back({ maxSize + i , abs(nowX - prevX) + abs(nowY - prevY) });
		}
	}

	queue<int> Q;

	while (true) {
		for (int i = 0; i < maxSize * 2 + 2; i++) {
			cost[i] = { INT_MAX, {-1, -1} };
			inQ[i] = false;
		}
		cost[startIdx].first = 0;
		Q.push(startIdx);
		for (int queueFront; !Q.empty(); Q.pop()) {
			queueFront = Q.front();
			for (int i = 0; i < edge[queueFront].size(); i++) {
				if (cost[edge[queueFront][i].first].first > cost[queueFront].first + edge[queueFront][i].second) {
					cost[edge[queueFront][i].first] = { cost[queueFront].first + edge[queueFront][i].second, {queueFront, i} };
					if (!inQ[edge[queueFront][i].first]) {
						inQ[edge[queueFront][i].first] = true;
						Q.push(edge[queueFront][i].first);
					}
				}
			}
			inQ[queueFront] = false;
		}
		if (cost[endIdx].second.first == -1)
			break;

		for (int i = endIdx; i != startIdx; i = cost[i].second.first) {
			edge[i].push_back({ cost[i].second.first, -edge[cost[i].second.first][cost[i].second.second].second });
			swap(edge[cost[i].second.first][cost[i].second.second], edge[cost[i].second.first].back());
			edge[cost[i].second.first].pop_back();
		}
	}

	change->UnlockBits(bitmapData);

	CLSID clsid;
	GetEncoderClsid(L"image/jpeg", &clsid);

	if (edge[endIdx].size() / (float)prevSize < setting->getMatching()) {
		if ((nowClock - alarmClock > setting->getAlarmInterval() * 1000) && setting->getAlarm()) {
			printf("%s\n", "CHANGE_DETECTION");
			fflush(stdout);
			setting->setAlarmClock(nowClock);
		}
		if (nowClock - saveClock > setting->getSaveInterval().first * 1000) {
			WCHAR eventFilePathWC[256];
			swprintf(eventFilePathWC, L"%ls(event).jpeg", wstring(filePathWC).substr(0, lstrlenW(filePathWC) - 5).c_str());
			origin->Save(eventFilePathWC, &clsid, NULL);
			saveClock = nowClock;
		}
	}
	else if (nowClock - saveClock > setting->getSaveInterval().second * 1000) {
		origin->Save(filePathWC, &clsid, NULL);
		saveClock = nowClock;
	}

	if (setting->getDebug()) {
		WCHAR debugFilePathWC[256];
		swprintf(debugFilePathWC, L"%ls(debug).jpeg", wstring(filePathWC).substr(0, lstrlenW(filePathWC) - 5).c_str());
		change->Save(debugFilePathWC, &clsid, NULL);
	}

	delete[]cost;
	delete[]inQ;
	delete[]edge;

	delete prevCorner;
	prevCorner = nowCorner;

	delete bitmapData;
	delete origin;
	delete change;
	DeleteObject(memdc);
	DeleteObject(membit);
	ReleaseDC(NULL, scrdc);

	delete[]grayArr;
	delete[]imgArr;
	delete[]edgeArr;
	delete[]gdx2;
	delete[]gdy2;
	delete[]gdxy;
	delete[]R;

	GdiplusShutdown(gdiplusToken);

	return true;
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
	using namespace Gdiplus;
	UINT	num = 0;	// number of image encoders
	UINT	size = 0;	// size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;	// Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;	// Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;	// Success
		}
	}

	free(pImageCodecInfo);
	return 0;
}

void ConvertCtoWC(const char* str, WCHAR* wstr){
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, wstr, MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL));
}