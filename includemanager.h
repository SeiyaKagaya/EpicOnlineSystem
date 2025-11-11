//------------------------------------------------------------
// @file        includemanager.h
// @brief       必要なヘッダをここに置いて一括管理
//------------------------------------------------------------
#ifndef _INCLUDE_MANAGE_H_
#define _INCLUDE_MANAGE_H_


#define NOMINMAX   // Windows.h の max/min マクロを無効化
#include <windows.h>
#include <cctype>   // toupper 用

#include <string>
#include <iostream>
#include <thread>


//#include "eos_sdk.h"
#include <eos_auth.h>
#include <eos_sdk.h>




#include <DirectXMath.h>
using namespace DirectX;


struct GameContext {
    std::string username;
    bool isHost = false;
};


// 随時
struct AnyTime
{
    int playerId;
    unsigned int inputFlags;
    DWORD timeStamp;
};

// 定期
struct Regular
{
    unsigned int objectID;
    XMFLOAT3 position;
    XMFLOAT4 rotation;
    XMFLOAT3 linerVelocity;
    XMFLOAT3 angularVelocity;
};



enum ConsoleColor {
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7,
    GRAY = 8,
    LIGHT_BLUE = 9,
    LIGHT_GREEN = 10,
    LIGHT_CYAN = 11,
    LIGHT_RED = 12,
    LIGHT_MAGENTA = 13,
    LIGHT_YELLOW = 14,
    BRIGHT_WHITE = 15
};


//Consoleの文字色変更
inline void SetConsoleColor(ConsoleColor color) 
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, static_cast<WORD>(color));
}



#endif