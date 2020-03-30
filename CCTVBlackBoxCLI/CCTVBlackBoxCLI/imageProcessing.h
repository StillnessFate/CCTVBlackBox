#pragma once
#pragma warning(disable:4996)
#include <iostream>
#include <ctime>
#include <string>
#include <windows.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <direct.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus")

#define BYTE_CHK(n) (BYTE)((n)<0?0:((n)>=256?255:(n)))
#define RATE_CHK(n) ((n)<0?0:((n)>1?1:(n)))
#define SQRT(n) ((n)?sqrt(n):0)

using namespace std;

#pragma pack(push, 1)
typedef struct PIXEL_ARGB {
	BYTE B;
	BYTE G;
	BYTE R;
	BYTE A;
}pixelARGB;
#pragma pack(pop)

const int gaussianFilter[5][5] = {
{1, 4, 6, 4, 1},
{4, 16, 24, 16, 4},
{6, 24, 36, 24, 6},
{4, 16, 24, 16, 4},
{1, 4, 6, 4, 1} };

const int sobelMask[2][3][3] = {
{{-1, 0, 1},
{-2, 0, 2},
{-1, 0, 1}},
{{1, 2, 1},
{0, 0, 0,},
{-1, -2, -1}} };

class CMonitor {
private:
	pair<int, int> point;//X, Y
	pair<int, int> size;//Width, Height
	pair<int, int> resize;//Width, Height
public:
	CMonitor();
	pair<int, int> getPoint();
	pair<int, int> getSize();
	pair<int, int> getResize();
};

void CALLBACK screenCapture(HWND hwnd, UINT uMsg, UINT timerId, DWORD dwTime);
bool imageProcessing(const WCHAR* fileNameWC, bool maximum, bool minimum);
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
void ConvertCtoWC(const char* str, WCHAR* wstr);