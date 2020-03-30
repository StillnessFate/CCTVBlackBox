#pragma once
#pragma warning(disable:4996)
#include <Windows.h>
#include <thread>
#include <iostream>

#define CMD_SETTIMERINTERVAL 0x41 //A %d
#define CMD_SETALARMINTERVAL 0x42 //B %d
#define CMD_SETSAVEINTERVAL 0x43 //C %d %d
#define CMD_SETCORNERSIZE 0x44 //D %d
#define CMD_SETTHRESHOLD 0x45 //E %f
#define CMD_SETMATCHING 0x46 //F %f
#define CMD_SETDEBUG 0x47 //G %d(bool)
#define CMD_SETALARM 0x48 //H %d(bool)
#define CMD_SETDELETEINTERVAL 0x49 //I %d
#define CMD_QUIT 0x5A//Z

#define CMD_GETTIMERINTERVAL 0x61 //a	%d
#define CMD_GETALARMINTERVAL 0x62 //b	%d
#define CMD_GETSAVEINTERVAL 0x63 //c	%d %d
#define CMD_GETCORNERSIZE 0x64 //d		%d
#define CMD_GETTHRESHOLD 0x65 //e		%f
#define CMD_GETMATCHING 0x66 //f		%f
#define CMD_GETDEBUG 0x67 //g			true:1 false:0
#define CMD_GETALARM 0x68 //h			true:1 false:0
#define CMD_GETDELETEINTERVAL 0x69 //i	%d

unsigned int WINAPI commandIO(void* args);