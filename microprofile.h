#pragma once
// This is free and unencumbered software released into the public domain.
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// For more information, please refer to <http://unlicense.org/>
//
// ***********************************************************************
//
//
//
//
// Howto:
// Call these functions from your code:
//  MicroProfileOnThreadCreate
//  MicroProfileMouseButton
//  MicroProfileMousePosition 
//  MicroProfileModKey
//  MicroProfileFlip  				<-- Call this once per frame
//  MicroProfileDraw  				<-- Call this once per frame
//  MicroProfileToggleDisplayMode 	<-- Bind to a key to toggle profiling
//  MicroProfileTogglePause			<-- Bind to a key to toggle pause
//
// Use these macros in your code in blocks you want to time:
//
// 	MICROPROFILE_DECLARE
// 	MICROPROFILE_DEFINE
// 	MICROPROFILE_DECLARE_GPU
// 	MICROPROFILE_DEFINE_GPU
// 	MICROPROFILE_SCOPE
// 	MICROPROFILE_SCOPEI
// 	MICROPROFILE_SCOPEGPU
// 	MICROPROFILE_SCOPEGPUI
//
//	Usage:
//
//	{
//		MICROPROFILE_SCOPEI("GroupName", "TimerName", nColorRgb):
// 		..Code to be timed..
//  }
//
//	MICROPROFILE_DECLARE / MICROPROFILE_DEFINE allows defining groups in a shared place, to ensure sorting of the timers
//
//  (in global scope)
//  MICROPROFILE_DEFINE(g_ProfileFisk, "Fisk", "Skalle", nSomeColorRgb);
//
//  (in some other file)
//  MICROPROFILE_DECLARE(g_ProfileFisk);
//
//  void foo(){
//  	MICROPROFILE_SCOPE(g_ProfileFisk);
//  }
//
//  Once code is instrumented the gui is activeted by calling MicroProfileToggleDisplayMode or by clicking in the upper left corner of
//  the screen
//
// The following functions must be implemented before the profiler is usable
//  debug render:
// 		void MicroProfileDrawText(int nX, int nY, uint32_t nColor, const char* pText);
// 		void MicroProfileDrawBox(int nX, int nY, int nX1, int nY1, uint32_t nColor, MicroProfileBoxType = MicroProfileBoxTypeFlat);
// 		void MicroProfileDrawLine2D(uint32_t nVertices, float* pVertices, uint32_t nColor);
//  Gpu time stamps:
// 		uint32_t MicroProfileGpuInsertTimeStamp();
// 		uint64_t MicroProfileGpuGetTimeStamp(uint32_t nKey);
// 		uint64_t MicroProfileTicksPerSecondGpu();
//  threading:
//      const char* MicroProfileGetThreadName(); Threadnames in detailed view


#ifndef MICROPROFILE_ENABLED
#define MICROPROFILE_ENABLED 1
#endif

#if 0 == MICROPROFILE_ENABLED

#define MICROPROFILE_DECLARE(var)
#define MICROPROFILE_DEFINE(var, group, name, color)
#define MICROPROFILE_DECLARE_GPU(var)
#define MICROPROFILE_DEFINE_GPU(var, group, name, color)
#define MICROPROFILE_SCOPE(var) do{}while(0)
#define MICROPROFILE_SCOPEI(group, name, color) do{}while(0)
#define MICROPROFILE_SCOPEGPU(var) do{}while(0)
#define MICROPROFILE_SCOPEGPUI(group, name, color) do{}while(0)
#define MICROPROFILE_FORCEENABLECPUGROUP(s) do{} while(0)
#define MICROPROFILE_FORCEDISABLECPUGROUP(s) do{} while(0)
#define MICROPROFILE_FORCEENABLEGPUGROUP(s) do{} while(0)
#define MICROPROFILE_FORCEDISABLEGPUGROUP(s) do{} while(0)

#define MicroProfileGetTime(group, name) 0.f
#define MicroProfileOnThreadCreate(foo) do{}while(0)
#define MicroProfileMouseButton(foo, bar) do{}while(0)
#define MicroProfileMousePosition(foo, bar) do{}while(0)
#define MicroProfileModKey(key) do{}while(0)
#define MicroProfileFlip() do{}while(0)
#define MicroProfileDraw(foo, bar) do{}while(0)
#define MicroProfileIsDrawing() 0
#define MicroProfileToggleDisplayMode() do{}while(0)
#define MicroProfileSetDisplayMode() do{}while(0)
#define MicroProfileTogglePause() do{}while(0)

#else

#include <stdint.h>
#include <string.h>

#if defined(__APPLE__)
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>
#include <libkern/OSAtomic.h>
#define MP_TICK() mach_absolute_time()
inline int64_t MicroProfileTicksPerSecondCpu()
{
	static int64_t nTicksPerSecond = 0;	
	if(nTicksPerSecond == 0) 
	{
		mach_timebase_info_data_t sTimebaseInfo;	
		mach_timebase_info(&sTimebaseInfo);
		nTicksPerSecond = 1000000000ll * sTimebaseInfo.denom / sTimebaseInfo.numer;
	}
	return nTicksPerSecond;
}

#define MP_BREAK() __builtin_trap()
#define MP_THREAD_LOCAL __thread
#define MP_STRCASECMP strcasecmp
#elif defined(_WIN32)
int64_t MicroProfileTicksPerSecondCpu();
int64_t MicroProfileGetTick();
#define MP_TICK() MicroProfileGetTick()
#define MP_BREAK() __debugbreak()
#define MP_THREAD_LOCAL __declspec(thread)
#define MP_STRCASECMP _stricmp
#endif

#ifndef MICROPROFILE_API
#define MICROPROFILE_API
#endif

#define MP_ASSERT(a) do{if(!(a)){MP_BREAK();} }while(0)
#define MICROPROFILE_DECLARE(var) extern MicroProfileToken g_mp_##var
#define MICROPROFILE_DEFINE(var, group, name, color) MicroProfileToken g_mp_##var = MicroProfileGetToken(group, name, color, MicroProfileTokenTypeCpu)
#define MICROPROFILE_DECLARE_GPU(var) extern MicroProfileToken g_mp_##var
#define MICROPROFILE_DEFINE_GPU(var, group, name, color) MicroProfileToken g_mp_##var = MicroProfileGetToken(group, name, color, MicroProfileTokenTypeGpu)
#define MICROPROFILE_TOKEN_PASTE0(a, b) a ## b
#define MICROPROFILE_TOKEN_PASTE(a, b)  MICROPROFILE_TOKEN_PASTE0(a,b)
#define MICROPROFILE_SCOPE(var) MicroProfileScopeHandler MICROPROFILE_TOKEN_PASTE(foo, __LINE__)(g_mp_##var)
#define MICROPROFILE_SCOPEI(group, name, color) static MicroProfileToken MICROPROFILE_TOKEN_PASTE(g_mp,__LINE__) = MicroProfileGetToken(group, name, color, MicroProfileTokenTypeCpu); MicroProfileScopeHandler MICROPROFILE_TOKEN_PASTE(foo,__LINE__)( MICROPROFILE_TOKEN_PASTE(g_mp,__LINE__))
#define MICROPROFILE_SCOPEGPU(var) MicroProfileScopeGpuHandler MICROPROFILE_TOKEN_PASTE(foo, __LINE__)(g_mp_##var)
#define MICROPROFILE_SCOPEGPUI(group, name, color) static MicroProfileToken MICROPROFILE_TOKEN_PASTE(g_mp,__LINE__) = MicroProfileGetToken(group, name, color,  MicroProfileTokenTypeGpu); MicroProfileScopeGpuHandler MICROPROFILE_TOKEN_PASTE(foo,__LINE__)( MICROPROFILE_TOKEN_PASTE(g_mp,__LINE__))

///configuration
#ifndef MICROPROFILE_TEXT_WIDTH
#define MICROPROFILE_TEXT_WIDTH 5
#endif
#ifndef MICROPROFILE_TEXT_HEIGHT
#define MICROPROFILE_TEXT_HEIGHT 8
#endif
#ifndef MICROPROFILE_DETAILED_BAR_HEIGHT
#define MICROPROFILE_DETAILED_BAR_HEIGHT 12
#endif
#ifndef MICROPROFILE_GRAPH_WIDTH
#define MICROPROFILE_GRAPH_WIDTH 256
#endif
#ifndef MICROPROFILE_GRAPH_HEIGHT
#define MICROPROFILE_GRAPH_HEIGHT 256
#endif
#ifndef MICROPROFILE_BORDER_SIZE 
#define MICROPROFILE_BORDER_SIZE 1
#endif
#ifndef MICROPROFILE_USE_THREAD_NAME_CALLBACK
#define MICROPROFILE_USE_THREAD_NAME_CALLBACK 0
#endif
#ifndef MICROPROFILE_DRAWCURSOR
#define MICROPROFILE_DRAWCURSOR 0
#endif

#ifndef MICROPROFILE_GPU_FRAME_DELAY
#define MICROPROFILE_GPU_FRAME_DELAY 3 //must be > 0
#endif

#ifndef MICROPROFILE_HELP_LEFT
#define MICROPROFILE_HELP_LEFT "Left-Click"
#endif

#ifndef MICROPROFILE_HELP_ALT
#define MICROPROFILE_HELP_ALT "Alt-Click"
#endif

#ifndef MICROPROFILE_HELP_MOD
#define MICROPROFILE_HELP_MOD "Mod"
#endif




#define MICROPROFILE_FORCEENABLECPUGROUP(s) MicroProfileForceEnableGroup(s, MicroProfileTokenTypeCpu)
#define MICROPROFILE_FORCEDISABLECPUGROUP(s) MicroProfileForceDisableGroup(s, MicroProfileTokenTypeCpu)
#define MICROPROFILE_FORCEENABLEGPUGROUP(s) MicroProfileForceEnableGroup(s, MicroProfileTokenTypeGpu)
#define MICROPROFILE_FORCEDISABLEGPUGROUP(s) MicroProfileForceDisableGroup(s, MicroProfileTokenTypeGpu)

#define MICROPROFILE_INVALID_TICK ((uint64_t)-1)
#define MICROPROFILE_GROUP_MASK_ALL 0xffffffffffff


typedef uint64_t MicroProfileToken;
typedef uint16_t MicroProfileGroupId;

#define MICROPROFILE_INVALID_TOKEN (uint64_t)-1

enum MicroProfileTokenType
{
	MicroProfileTokenTypeCpu,
	MicroProfileTokenTypeGpu,
};
enum MicroProfileBoxType
{
	MicroProfileBoxTypeBar,
	MicroProfileBoxTypeFlat,
};

struct MicroProfileState
{
	uint32_t nDisplay;
	uint32_t nMenuAllGroups;
	uint64_t nMenuActiveGroup;
	uint32_t nMenuAllThreads;
	uint32_t nAggregateFlip;
	uint32_t nBars;
	float fReferenceTime;
};


MICROPROFILE_API void MicroProfileInit();
MICROPROFILE_API MicroProfileToken MicroProfileFindToken(const char* sGroup, const char* sName);
MICROPROFILE_API MicroProfileToken MicroProfileGetToken(const char* sGroup, const char* sName, uint32_t nColor, MicroProfileTokenType Token = MicroProfileTokenTypeCpu);
MICROPROFILE_API uint64_t MicroProfileEnter(MicroProfileToken nToken);
MICROPROFILE_API void MicroProfileLeave(MicroProfileToken nToken, uint64_t nTick);
MICROPROFILE_API uint64_t MicroProfileGpuEnter(MicroProfileToken nToken);
MICROPROFILE_API void MicroProfileGpuLeave(MicroProfileToken nToken, uint64_t nTick);
inline uint16_t MicroProfileGetTimerIndex(MicroProfileToken t){ return (t&0xffff); }
inline uint64_t MicroProfileGetGroupMask(MicroProfileToken t){ return ((t>>16)&MICROPROFILE_GROUP_MASK_ALL);}
inline MicroProfileToken MicroProfileMakeToken(uint64_t nGroupMask, uint16_t nTimer){ return (nGroupMask<<16) | nTimer;}

MICROPROFILE_API void MicroProfileFlip(); //! called once per frame.
MICROPROFILE_API void MicroProfileDraw(uint32_t nWidth, uint32_t nHeight); //! call if drawing microprofilers
MICROPROFILE_API bool MicroProfileIsDrawing();
MICROPROFILE_API void MicroProfileToggleGraph(MicroProfileToken nToken);
MICROPROFILE_API bool MicroProfileDrawGraph(uint32_t nScreenWidth, uint32_t nScreenHeight);
MICROPROFILE_API void MicroProfileSetAggregateCount(uint32_t nCount); //!Set no. of frames to aggregate over. 0 for infinite
MICROPROFILE_API void MicroProfileToggleDisplayMode(); //switch between off, bars, detailed
MICROPROFILE_API void MicroProfileSetDisplayMode(int); //switch between off, bars, detailed
MICROPROFILE_API void MicroProfileClearGraph();
MICROPROFILE_API void MicroProfileTogglePause();
MICROPROFILE_API void MicroProfileGetState(MicroProfileState* pStateOut);
MICROPROFILE_API void MicroProfileSetState(MicroProfileState* pStateIn);
MICROPROFILE_API void MicroProfileForceEnableGroup(const char* pGroup, MicroProfileTokenType Type);
MICROPROFILE_API void MicroProfileForceDisableGroup(const char* pGroup, MicroProfileTokenType Type);
MICROPROFILE_API float MicroProfileGetTime(const char* pGroup, const char* pName);
MICROPROFILE_API void MicroProfileMousePosition(uint32_t nX, uint32_t nY, int nWheelDelta);
MICROPROFILE_API void MicroProfileModKey(uint32_t nKeyState);
MICROPROFILE_API void MicroProfileMouseButton(uint32_t nLeft, uint32_t nRight);
MICROPROFILE_API void MicroProfileOnThreadCreate(const char* pThreadName); //should be called from newly created threads
MICROPROFILE_API void MicroProfileInitThreadLog();
MICROPROFILE_API void MicroProfileDrawLineVertical(int nX, int nTop, int nBottom, uint32_t nColor);
MICROPROFILE_API void MicroProfileDrawLineHorizontal(int nLeft, int nRight, int nY, uint32_t nColor);




//UNDEFINED: MUST BE IMPLEMENTED ELSEWHERE
MICROPROFILE_API void MicroProfileDrawText(int nX, int nY, uint32_t nColor, const char* pText);
MICROPROFILE_API void MicroProfileDrawBox(int nX, int nY, int nX1, int nY1, uint32_t nColor, MicroProfileBoxType = MicroProfileBoxTypeFlat);
MICROPROFILE_API void MicroProfileDrawLine2D(uint32_t nVertices, float* pVertices, uint32_t nColor);
MICROPROFILE_API uint32_t MicroProfileGpuInsertTimeStamp();
MICROPROFILE_API uint64_t MicroProfileGpuGetTimeStamp(uint32_t nKey);
MICROPROFILE_API uint64_t MicroProfileTicksPerSecondGpu();
#if MICROPROFILE_USE_THREAD_NAME_CALLBACK
MICROPROFILE_API const char* MicroProfileGetThreadName();
#else
#define MicroProfileGetThreadName() "<implement MicroProfileGetThreadName to get threadnames>"
#endif

struct MicroProfileScopeHandler
{
	MicroProfileToken nToken;
	uint64_t nTick;
	MicroProfileScopeHandler(MicroProfileToken Token):nToken(Token)
	{
		nTick = MicroProfileEnter(nToken);
	}
	~MicroProfileScopeHandler()
	{
		MicroProfileLeave(nToken, nTick);
	}
};

struct MicroProfileScopeGpuHandler
{
	MicroProfileToken nToken;
	uint64_t nTick;
	MicroProfileScopeGpuHandler(MicroProfileToken Token):nToken(Token)
	{
		nTick = MicroProfileGpuEnter(nToken);
	}
	~MicroProfileScopeGpuHandler()
	{
		MicroProfileGpuLeave(nToken, nTick);
	}
};




#ifdef MICRO_PROFILE_IMPL

#ifdef _WIN32
#ifndef _VARIADIC_MAX 
#define _VARIADIC_MAX 6 //hrmph
#endif
#include <windows.h>
#define snprintf _snprintf

#pragma warning(push)
#pragma warning(disable: 4244)
int64_t MicroProfileTicksPerSecondCpu()
{
	static int64_t nTicksPerSecond = 0;	
	if(nTicksPerSecond == 0) 
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&nTicksPerSecond);
	}
	return nTicksPerSecond;
}
int64_t MicroProfileGetTick()
{
	int64_t ticks;
	QueryPerformanceCounter((LARGE_INTEGER*)&ticks);
	return ticks;
}

#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <thread>
#include <mutex>
#include <atomic>


#define S g_MicroProfile
#define MICROPROFILE_MAX_TIMERS 1024
#define MICROPROFILE_MAX_GROUPS 48 //dont bump! no. of bits used it bitmask
#define MICROPROFILE_MAX_GRAPHS 5
#define MICROPROFILE_GRAPH_HISTORY 128
#define MICROPROFILE_BUFFER_SIZE (((2048)<<10)/sizeof(MicroProfileLogEntry))
#define MICROPROFILE_MAX_THREADS 32
#define MICROPROFILE_STACK_MAX 32
#define MICROPROFILE_MAX_PRESETS 5
#define MICROPROFILE_DEBUG 0
#define MICROPROFILE_TOOLTIP_MAX_STRINGS (32 + MICROPROFILE_MAX_GROUPS*2)
#define MICROPROFILE_TOOLTIP_STRING_BUFFER_SIZE 1024
#define MICROPROFILE_TOOLTIP_MAX_LOCKED 3
#define MICROPROFILE_MAX_FRAME_HISTORY 512
#define MICROPROFILE_ANIM_DELAY_PRC 0.5f
#define MICROPROFILE_GAP_TIME 50 //extra ms to fetch to close timers from earlier frames

enum MicroProfileDrawMask
{
	MP_DRAW_OFF		= 0x0,
	MP_DRAW_BARS		= 0x1,
	MP_DRAW_DETAILED	= 0x2,
};

enum MicroProfileDrawBarsMask
{
	MP_DRAW_TIMERS 				= 0x1,	
	MP_DRAW_AVERAGE				= 0x2,	
	MP_DRAW_MAX					= 0x4,	
	MP_DRAW_CALL_COUNT			= 0x8,
	MP_DRAW_TIMERS_EXCLUSIVE 	= 0x10,
	MP_DRAW_AVERAGE_EXCLUSIVE 	= 0x20,	
	MP_DRAW_MAX_EXCLUSIVE		= 0x40,
	MP_DRAW_ALL 				= 0x4f,

};

struct MicroProfileTimer
{
	uint64_t nTicks;
	uint32_t nCount;
};

struct MicroProfileGroupInfo
{
	const char* pName;
	uint32_t nGroupIndex;
	uint32_t nNumTimers;
	uint32_t nMaxTimerNameLen;
	MicroProfileTokenType Type;
};

struct MicroProfileTimerInfo
{
	MicroProfileToken nToken;
	uint32_t nTimerIndex;
	uint32_t nGroupIndex;
	const char* pName;
	uint32_t nColor;
};

struct MicroProfileGraphState
{
	int64_t nHistory[MICROPROFILE_GRAPH_HISTORY];
	MicroProfileToken nToken;
	int32_t nKey;
};

struct MicroProfileLogEntry
{
	enum EType
	{
		EEnter, ELeave,
	};
	EType eType;
	uint8_t nStackDepth;
	MicroProfileToken nToken;
	int64_t nTick;
};

struct MicroProfileFrameState
{
	int64_t nFrameStartCpu;
	int64_t nFrameStartGpu;
	uint32_t nLogStart[MICROPROFILE_MAX_GROUPS];
};

struct MicroProfileThreadLog
{
	MicroProfileThreadLog*  pNext;
	MicroProfileLogEntry	Log[MICROPROFILE_BUFFER_SIZE];

	std::atomic<uint32_t>	nPut;
	std::atomic<uint32_t>	nGet;	
	uint32_t 				nActive;
	uint32_t 				nGpu;
	enum
	{
		THREAD_MAX_LEN = 64,
	};
	char					ThreadName[64];
};

struct MicroProfileStringArray
{
	const char* ppStrings[MICROPROFILE_TOOLTIP_MAX_STRINGS];
	char Buffer[MICROPROFILE_TOOLTIP_STRING_BUFFER_SIZE];
	char* pBufferPos;
	uint32_t nNumStrings;
};


struct 
{
	uint32_t nTotalTimers;
	uint32_t nGroupCount;
	uint32_t nAggregateFlip;
	uint32_t nAggregateFlipCount;
	uint32_t nAggregateFrames;
	
	uint32_t nDisplay;
	uint32_t nBars;
	uint64_t nActiveGroup;

	uint64_t nForceGroup;

	//menu/mouse over stuff
	uint64_t nMenuActiveGroup;
	uint32_t nMenuAllGroups;
	uint32_t nMenuAllThreads;
	uint64_t nHoverToken;
	int64_t  nHoverTime;
	int 	 nHoverFrame;
	uint32_t nOverflow;

	uint64_t nGroupMask;
	uint32_t nRunning;
	uint32_t nMaxGroupSize;

	float fGraphBaseTime; //old kill
	float fGraphBaseTimePos; //old kill
	float fReferenceTime;
	float fRcpReferenceTime;
	uint32_t nOpacityBackground;
	uint32_t nOpacityForeground;

	float fDetailedOffset; //display offset relative to start of latest displayable frame.
	float fDetailedRange; //no. of ms to display

	float fDetailedOffsetTarget;
	float fDetailedRangeTarget;

	int nOffsetY;

	uint32_t nWidth;
	uint32_t nHeight;

	uint32_t nBarWidth;
	uint32_t nBarHeight;


	MicroProfileGroupInfo 	GroupInfo[MICROPROFILE_MAX_GROUPS];
	MicroProfileTimerInfo 	TimerInfo[MICROPROFILE_MAX_TIMERS];
	
	MicroProfileTimer 		AggregateTimers[MICROPROFILE_MAX_TIMERS];
	uint64_t				MaxTimers[MICROPROFILE_MAX_TIMERS];
	uint64_t				AggregateTimersExclusive[MICROPROFILE_MAX_TIMERS];
	uint64_t				MaxTimersExclusive[MICROPROFILE_MAX_TIMERS];

	MicroProfileTimer 		Frame[MICROPROFILE_MAX_TIMERS];
	uint64_t				FrameExclusive[MICROPROFILE_MAX_TIMERS];

	MicroProfileTimer 		Aggregate[MICROPROFILE_MAX_TIMERS];
	uint64_t				AggregateMax[MICROPROFILE_MAX_TIMERS];	
	uint64_t				AggregateExclusive[MICROPROFILE_MAX_TIMERS];
	uint64_t				AggregateMaxExclusive[MICROPROFILE_MAX_TIMERS];

	MicroProfileGraphState	Graph[MICROPROFILE_MAX_GRAPHS];
	uint32_t				nGraphPut;

	uint32_t 				nMouseX;
	uint32_t 				nMouseY;
	int						nMouseWheelDelta;
	uint32_t				nMouseDownLeft;
	uint32_t				nMouseDownRight;
	uint32_t 				nMouseLeft;
	uint32_t 				nMouseRight;
	uint32_t 				nMouseLeftMod;
	uint32_t 				nMouseRightMod;
	uint32_t				nModDown;
	uint32_t 				nActiveMenu;

	uint32_t				nThreadActive[MICROPROFILE_MAX_THREADS];
	MicroProfileThreadLog* 	Pool[MICROPROFILE_MAX_THREADS];
	uint32_t				nNumLogs;
	uint32_t 				nMemUsage;

	uint32_t 				nFrameCurrent;
	uint32_t 				nFramePut;

	MicroProfileFrameState Frames[MICROPROFILE_MAX_FRAME_HISTORY];
	
	MicroProfileLogEntry* pDisplayMouseOver;


	uint64_t				nFlipTicks;
	uint64_t				nFlipAggregate;
	uint64_t				nFlipMax;
	uint64_t				nFlipAggregateDisplay;
	uint64_t				nFlipMaxDisplay;
		

	MicroProfileStringArray LockedToolTips[MICROPROFILE_TOOLTIP_MAX_LOCKED];	
	uint32_t  				nLockedToolTipColor[MICROPROFILE_TOOLTIP_MAX_LOCKED];	
	int 					LockedToolTipFront;


} g_MicroProfile;

MicroProfileThreadLog*			g_MicroProfileGpuLog = 0;
MP_THREAD_LOCAL MicroProfileThreadLog* g_MicroProfileThreadLog = 0;
static bool g_bUseLock = false; /// This is used because windows does not support using mutexes under dll init(which is where global initialization is handled)
static uint32_t 				g_nMicroProfileBackColors[2] = {  0x474747, 0x313131 };
static uint32_t g_MicroProfileAggregatePresets[] = {0, 10, 20, 30, 60, 120};
static float g_MicroProfileReferenceTimePresets[] = {5.f, 10.f, 15.f,20.f, 33.33f, 66.66f, 100.f};
static uint32_t g_MicroProfileOpacityPresets[] = {0x40, 0x80, 0xc0, 0xff};
static const char* g_MicroProfilePresetNames[] = 
{
	"Default",
	"Render",
	"GPU",
	"Lighting",
	"AI",
	"Visibility",
	"Sound",
};


MICROPROFILE_DEFINE(g_MicroProfileDetailed, "MicroProfile", "Detailed View", 0x8888000);
MICROPROFILE_DEFINE(g_MicroProfileDrawGraph, "MicroProfile", "Draw Graph", 0xff44ee00);
MICROPROFILE_DEFINE(g_MicroProfileFlip, "MicroProfile", "MicroProfileFlip", 0x3355ee);
MICROPROFILE_DEFINE(g_MicroProfileThreadLoop, "MicroProfile", "ThreadLoop", 0x3355ee);
MICROPROFILE_DEFINE(g_MicroProfileClear, "MicroProfile", "Clear", 0x3355ee);
MICROPROFILE_DEFINE(g_MicroProfileAccumulate, "MicroProfile", "Accumulate", 0x3355ee);
MICROPROFILE_DEFINE(g_MicroProfileDrawBarView, "MicroProfile", "DrawBarView", 0x00dd77);
MICROPROFILE_DEFINE(g_MicroProfileDraw,"MicroProfile", "Draw", 0x737373);



inline std::recursive_mutex& MicroProfileMutex()
{
	static std::recursive_mutex Mutex;
	return Mutex;
}

template<typename T>
T MicroProfileMin(T a, T b)
{ return a < b ? a : b; }

template<typename T>
T MicroProfileMax(T a, T b)
{ return a > b ? a : b; }



void MicroProfileStringArrayClear(MicroProfileStringArray* pArray)
{
	pArray->nNumStrings = 0;
	pArray->pBufferPos = &pArray->Buffer[0];
}

void MicroProfileStringArrayAddLiteral(MicroProfileStringArray* pArray, const char* pLiteral)
{
	pArray->ppStrings[pArray->nNumStrings++] = pLiteral;
}

void MicroProfileStringArrayFormat(MicroProfileStringArray* pArray, const char* fmt, ...)
{
	pArray->ppStrings[pArray->nNumStrings++] = pArray->pBufferPos;
	va_list args;
	va_start (args, fmt);
	pArray->pBufferPos += 1 + vsprintf(pArray->pBufferPos, fmt, args);
	va_end(args);
	MP_ASSERT(pArray->pBufferPos < pArray->Buffer + MICROPROFILE_TOOLTIP_STRING_BUFFER_SIZE);
}
void MicroProfileStringArrayCopy(MicroProfileStringArray* pDest, MicroProfileStringArray* pSrc)
{
	memcpy(&pDest->ppStrings[0], &pSrc->ppStrings[0], sizeof(pDest->ppStrings));
	memcpy(&pDest->Buffer[0], &pSrc->Buffer[0], sizeof(pDest->Buffer));
	for(uint32_t i = 0; i < MICROPROFILE_TOOLTIP_MAX_STRINGS; ++i)
	{
		if(i < pSrc->nNumStrings)
		{
			if(pSrc->ppStrings[i] >= &pSrc->Buffer[0] && pSrc->ppStrings[i] < &pSrc->Buffer[0] + MICROPROFILE_TOOLTIP_STRING_BUFFER_SIZE)
			{
				pDest->ppStrings[i] += &pDest->Buffer[0] - &pSrc->Buffer[0];
			}
		}
	}
	pDest->nNumStrings = pSrc->nNumStrings;
}

MicroProfileThreadLog* MicroProfileCreateThreadLog(const char* pName);
void MicroProfileLoadPreset(const char* pSuffix);
void MicroProfileSavePreset(const char* pSuffix);


inline int64_t MicroProfileMsToTick(float fMs, int64_t nTicksPerSecond)
{
	return (int64_t)(fMs*0.001f*nTicksPerSecond);
}

inline float MicroProfileTickToMsMultiplier(int64_t nTicksPerSecond)
{
	return 1000.f / nTicksPerSecond;
}

inline uint16_t MicroProfileGetGroupIndex(MicroProfileToken t)
{
	return (uint16_t)S.TimerInfo[MicroProfileGetTimerIndex(t)].nGroupIndex;
}


void MicroProfileInit()
{
	std::recursive_mutex& mutex = MicroProfileMutex();
	bool bUseLock = g_bUseLock;
	if(bUseLock)
		mutex.lock();
	static bool bOnce = true;
	if(bOnce)
	{
		S.nMemUsage += sizeof(S);
		bOnce = false;
		memset(&S, 0, sizeof(S));
		S.nGroupCount = 0;
		S.nBarWidth = 100;
		S.nBarHeight = MICROPROFILE_TEXT_HEIGHT;
		S.nActiveGroup = 0;
		S.nForceGroup = 0;
		S.nMenuAllGroups = 0;
		S.nMenuActiveGroup = 0;
		S.nMenuAllThreads = 1;
		S.nAggregateFlip = 30;
		S.nTotalTimers = 0;
		for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
		{
			S.Graph[i].nToken = MICROPROFILE_INVALID_TOKEN;
		}
		S.nBars = MP_DRAW_ALL;
		S.nRunning = 1;
		S.fGraphBaseTime = 40.f;
		S.nWidth = 100;
		S.nHeight = 100;
		S.nActiveMenu = (uint32_t)-1;
		S.fReferenceTime = 33.33f;
		S.fRcpReferenceTime = 1.f / S.fReferenceTime;
		int64_t nTick = MP_TICK();
		for(int i = 0; i < MICROPROFILE_MAX_FRAME_HISTORY; ++i)
		{
			S.Frames[i].nFrameStartCpu = nTick;
			S.Frames[i].nFrameStartGpu = -1;
		}

		MicroProfileThreadLog* pGpu = MicroProfileCreateThreadLog("GPU");
		g_MicroProfileGpuLog = pGpu;
		S.Pool[S.nNumLogs++] = pGpu;
		pGpu->nGpu = 1;

		S.fDetailedOffsetTarget = S.fDetailedOffset = 0.f;
		S.fDetailedRangeTarget = S.fDetailedRange = 50.f;

		S.nOpacityBackground = 0xff<<24;
		S.nOpacityForeground = 0xff<<24;
	}
	if(bUseLock)
		mutex.unlock();
}

MicroProfileThreadLog* MicroProfileCreateThreadLog(const char* pName)
{
	MicroProfileThreadLog* pLog = new MicroProfileThreadLog;
	S.nMemUsage += sizeof(MicroProfileThreadLog);
	memset(pLog, 0, sizeof(*pLog));
	int len = (int)strlen(pName);
	int maxlen = sizeof(pLog->ThreadName)-1;
	len = len < maxlen ? len : maxlen;
	memcpy(&pLog->ThreadName[0], pName, len);
	pLog->ThreadName[len] = '\0';
	return pLog;
}

void MicroProfileOnThreadCreate(const char* pThreadName)
{
	g_bUseLock = true;
	MicroProfileInit();
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());
	MP_ASSERT(g_MicroProfileThreadLog == 0);
	MicroProfileThreadLog* pLog = MicroProfileCreateThreadLog(pThreadName ? pThreadName : MicroProfileGetThreadName());
	MP_ASSERT(pLog);
	g_MicroProfileThreadLog = pLog;
	S.Pool[S.nNumLogs++] = pLog;
}

void MicroProfileInitThreadLog()
{
	MicroProfileOnThreadCreate(nullptr);
}


struct MicroProfileScopeLock
{
	bool bUseLock;
	std::recursive_mutex& m;
	MicroProfileScopeLock(std::recursive_mutex& m) : bUseLock(g_bUseLock), m(m)
	{
		if(bUseLock)
			m.lock();
	}
	~MicroProfileScopeLock()
	{
		if(bUseLock)
			m.unlock();
	}
};

MicroProfileToken MicroProfileFindToken(const char* pGroup, const char* pName)
{
	MicroProfileInit();
	MicroProfileScopeLock L(MicroProfileMutex());
	for(uint32_t i = 0; i < S.nTotalTimers; ++i)
	{
		if(!MP_STRCASECMP(pName, S.TimerInfo[i].pName) && !MP_STRCASECMP(pGroup, S.GroupInfo[S.TimerInfo[i].nGroupIndex].pName))
		{
			return S.TimerInfo[i].nToken;
		}
	}
	return MICROPROFILE_INVALID_TOKEN;
}

uint16_t MicroProfileGetGroup(const char* pGroup, MicroProfileTokenType Type)
{
	for(uint32_t i = 0; i < S.nGroupCount; ++i)
	{
		if(!MP_STRCASECMP(pGroup, S.GroupInfo[i].pName))
		{
			return i;
		}
	}
	uint16_t nGroupIndex = 0xffff;
	S.GroupInfo[S.nGroupCount].pName = pGroup;
	S.GroupInfo[S.nGroupCount].nGroupIndex = S.nGroupCount;
	S.GroupInfo[S.nGroupCount].nNumTimers = 0;
	S.GroupInfo[S.nGroupCount].Type = Type;
	S.GroupInfo[S.nGroupCount].nMaxTimerNameLen = 0;
	nGroupIndex = S.nGroupCount++;
	S.nGroupMask = (S.nGroupMask<<1)|1;
	MP_ASSERT(nGroupIndex < MICROPROFILE_MAX_GROUPS);
	return nGroupIndex;
}

MicroProfileToken MicroProfileGetToken(const char* pGroup, const char* pName, uint32_t nColor, MicroProfileTokenType Type)
{
	MicroProfileInit();
	MicroProfileScopeLock L(MicroProfileMutex());
	MicroProfileToken ret = MicroProfileFindToken(pGroup, pName);
	if(ret != MICROPROFILE_INVALID_TOKEN)
		return ret;
	uint16_t nGroupIndex = MicroProfileGetGroup(pGroup, Type);
	uint16_t nTimerIndex = (uint16_t)(S.nTotalTimers++);
	uint64_t nGroupMask = 1ll << nGroupIndex;
	MicroProfileToken nToken = MicroProfileMakeToken(nGroupMask, nTimerIndex);
	S.GroupInfo[nGroupIndex].nNumTimers++;
	S.GroupInfo[nGroupIndex].nMaxTimerNameLen = MicroProfileMax(S.GroupInfo[nGroupIndex].nMaxTimerNameLen, (uint32_t)strlen(pName));
	MP_ASSERT(S.GroupInfo[nGroupIndex].Type == Type); //dont mix cpu & gpu timers in the same group
	S.nMaxGroupSize = MicroProfileMax(S.nMaxGroupSize, S.GroupInfo[nGroupIndex].nNumTimers);
	S.TimerInfo[nTimerIndex].nToken = nToken;
	S.TimerInfo[nTimerIndex].pName = pName;
	S.TimerInfo[nTimerIndex].nColor = nColor&0xffffff;
	S.TimerInfo[nTimerIndex].nGroupIndex = nGroupIndex;
	return nToken;
}

inline void MicroProfileLogPut(MicroProfileToken nToken_, uint64_t nTick, MicroProfileLogEntry::EType eEntry, MicroProfileThreadLog* pLog)
{
	MP_ASSERT(pLog != 0); //this assert is hit if MicroProfileOnCreateThread is not called
	uint32_t nPos = pLog->nPut.load(std::memory_order_relaxed);
	uint32_t nNextPos = (nPos+1) % MICROPROFILE_BUFFER_SIZE;
	if(nNextPos == pLog->nGet.load(std::memory_order_relaxed))
	{
		S.nOverflow = 100;
	}
	else
	{
		pLog->Log[nPos].nToken = nToken_;
		pLog->Log[nPos].nTick = nTick;
		pLog->Log[nPos].eType = eEntry;
		pLog->nPut.store(nNextPos, std::memory_order_release);
	}
}

uint64_t MicroProfileEnter(MicroProfileToken nToken_)
{
	if(MicroProfileGetGroupMask(nToken_) & S.nActiveGroup)
	{
		if(!g_MicroProfileThreadLog)
		{
			MicroProfileInitThreadLog();
		}
		uint64_t nTick = MP_TICK();
		MicroProfileLogPut(nToken_, nTick, MicroProfileLogEntry::EEnter, g_MicroProfileThreadLog);
		return nTick;
	}
	return MICROPROFILE_INVALID_TICK;
}



void MicroProfileLeave(MicroProfileToken nToken_, uint64_t nTickStart)
{
	if(MICROPROFILE_INVALID_TICK != nTickStart)
	{
		if(!g_MicroProfileThreadLog)
		{
			MicroProfileInitThreadLog();
		}
		uint64_t nTick = MP_TICK();
		MicroProfileLogPut(nToken_, nTick, MicroProfileLogEntry::ELeave, g_MicroProfileThreadLog);
	}
}


uint64_t MicroProfileGpuEnter(MicroProfileToken nToken_)
{
	if(MicroProfileGetGroupMask(nToken_) & S.nActiveGroup)
	{
		uint64_t nTimer = MicroProfileGpuInsertTimeStamp();
		MicroProfileLogPut(nToken_, nTimer, MicroProfileLogEntry::EEnter, g_MicroProfileGpuLog);
		return 1;
	}
	return 0;
}

void MicroProfileGpuLeave(MicroProfileToken nToken_, uint64_t nTickStart)
{
	if(nTickStart)
	{
		uint64_t nTimer = MicroProfileGpuInsertTimeStamp();
		MicroProfileLogPut(nToken_, nTimer, MicroProfileLogEntry::ELeave, g_MicroProfileGpuLog);
	}
}

void MicroProfileGetRange(uint32_t nPut, uint32_t nGet, uint32_t nRange[2][2])
{
	if(nPut > nGet)
	{
		nRange[0][0] = nGet;
		nRange[0][1] = nPut;
		nRange[1][0] = nRange[1][1] = 0;
	}
	else if(nPut != nGet)
	{
		MP_ASSERT(nGet != MICROPROFILE_BUFFER_SIZE);
		uint32_t nCountEnd = MICROPROFILE_BUFFER_SIZE - nGet;
		nRange[0][0] = nGet;
		nRange[0][1] = nGet + nCountEnd;
		nRange[1][0] = 0;
		nRange[1][1] = nPut;
	}
}

void MicroProfileFlip()
{
	MICROPROFILE_SCOPE(g_MicroProfileFlip);
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());

	{
		static int once = 0;
		if(0 == once)
		{
			uint32_t nDisplay = S.nDisplay;
			MicroProfileLoadPreset(g_MicroProfilePresetNames[0]);
			once++;
			S.nDisplay = nDisplay;// dont load display, just state
		}
	}
	S.nActiveGroup = 0;
	if(S.nRunning)
	{
		S.nFramePut = (S.nFramePut+1) % MICROPROFILE_MAX_FRAME_HISTORY;
		S.nFrameCurrent = (S.nFramePut + MICROPROFILE_MAX_FRAME_HISTORY - MICROPROFILE_GPU_FRAME_DELAY - 1) % MICROPROFILE_MAX_FRAME_HISTORY;
		uint32_t nFrameNext = (S.nFrameCurrent+1) % MICROPROFILE_MAX_FRAME_HISTORY;

		MicroProfileFrameState* pFramePut = &S.Frames[S.nFramePut];
		MicroProfileFrameState* pFrameCurrent = &S.Frames[S.nFrameCurrent];
		MicroProfileFrameState* pFrameNext = &S.Frames[nFrameNext];
		
		pFramePut->nFrameStartCpu = MP_TICK();
		pFramePut->nFrameStartGpu = (uint32_t)MicroProfileGpuInsertTimeStamp();
		if(pFrameNext->nFrameStartGpu != (uint64_t)-1)
			pFrameNext->nFrameStartGpu = MicroProfileGpuGetTimeStamp((uint32_t)pFrameNext->nFrameStartGpu);

		if(pFrameCurrent->nFrameStartGpu == (uint64_t)-1)
			pFrameCurrent->nFrameStartGpu = pFrameNext->nFrameStartGpu + 1; 

		uint64_t nFrameStartCpu = pFrameCurrent->nFrameStartCpu;
		uint64_t nFrameEndCpu = pFrameNext->nFrameStartCpu;
		uint64_t nFrameStartGpu = pFrameCurrent->nFrameStartGpu;
		uint64_t nFrameEndGpu = pFrameNext->nFrameStartGpu;

		{
			uint64_t nTick = nFrameEndCpu - nFrameStartCpu;
			S.nFlipTicks = nTick;
			S.nFlipAggregate += nTick;
			S.nFlipMax = MicroProfileMax(S.nFlipMax, nTick);
		}

		for(uint32_t i = 0; i < MICROPROFILE_MAX_THREADS; ++i)
		{
			MicroProfileThreadLog* pLog = S.Pool[i];
			if(!pLog)
			{
				pFramePut->nLogStart[i] = 0;
			}
			else
			{
				pFramePut->nLogStart[i] = pLog->nPut.load(std::memory_order_acquire);
				//need to keep last frame around to close timers. timers more than 1 frame old is ditched.
				pLog->nGet.store(pFrameCurrent->nLogStart[i], std::memory_order_relaxed);
			}
		}

		if(S.nRunning)
		{
			{
				MICROPROFILE_SCOPE(g_MicroProfileClear);
				for(uint32_t i = 0; i < S.nTotalTimers; ++i)
				{
					S.Frame[i].nTicks = 0;
					S.Frame[i].nCount = 0;
					S.FrameExclusive[i] = 0;
				}
			}
			{
				MICROPROFILE_SCOPE(g_MicroProfileThreadLoop);
				for(uint32_t i = 0; i < MICROPROFILE_MAX_THREADS; ++i)
				{
					MicroProfileThreadLog* pLog = S.Pool[i];
					if(!pLog) 
						continue;

					uint32_t nPut = pFrameNext->nLogStart[i];
					uint32_t nGet = pFrameCurrent->nLogStart[i];
					uint32_t nRange[2][2] = { {0, 0}, {0, 0}, };
					MicroProfileGetRange(nPut, nGet, nRange);

					uint32_t nMaxStackDepth = 0;

					uint64_t nFrameStart = pLog->nGpu ? nFrameStartGpu : nFrameStartCpu;
					uint64_t nFrameEnd = pLog->nGpu ? nFrameEndGpu : nFrameEndCpu;
					//fetch gpu results.
					if(pLog->nGpu)
					{
						for(uint32_t j = 0; j < 2; ++j)
						{
							uint32_t nStart = nRange[j][0];
							uint32_t nEnd = nRange[j][1];
							for(uint32_t k = nStart; k < nEnd; ++k)
							{
								
								int64_t nRet = MicroProfileGpuGetTimeStamp((uint32_t)pLog->Log[k].nTick);
								pLog->Log[k].nTick = nRet;

							}
						}
					}
					uint32_t nStack[MICROPROFILE_STACK_MAX];
					int64_t nChildTickStack[MICROPROFILE_STACK_MAX];
					uint32_t nStackPos = 0;
					nChildTickStack[0] = 0;

					for(uint32_t j = 0; j < 2; ++j)
					{
						uint32_t nStart = nRange[j][0];
						uint32_t nEnd = nRange[j][1];
						for(uint32_t k = nStart; k < nEnd; ++k)
						{
							MicroProfileLogEntry& LE = pLog->Log[k];
							switch(LE.eType)
							{
								case MicroProfileLogEntry::EEnter:
								{
									MP_ASSERT(nStackPos < MICROPROFILE_STACK_MAX);								
									nStack[nStackPos++] = k;
									nChildTickStack[nStackPos] = 0;
									
								}
								break;
								case MicroProfileLogEntry::ELeave:
								{
									//todo: reconsider the fallback for Leaves without enters
									int64_t nTickStart = 0 != nStackPos ? pLog->Log[nStack[nStackPos-1]].nTick : nFrameStart;
									int64_t nTicks = LE.nTick - nTickStart;
									int64_t nChildTicks = nChildTickStack[nStackPos];
									if(0 != nStackPos)
									{
										MP_ASSERT(pLog->Log[nStack[nStackPos-1]].nToken == LE.nToken);
										nStackPos--;
										nChildTickStack[nStackPos] += nTicks;
									}
									uint32_t nTimerIndex = MicroProfileGetTimerIndex(LE.nToken);
									S.Frame[nTimerIndex].nTicks += nTicks;								
									S.FrameExclusive[nTimerIndex] += (nTicks-nChildTicks);
									S.Frame[nTimerIndex].nCount += 1;
								}
							}
						}
					}
					//todo: reconsider the fallback for enters without leaves
					for(uint32_t j = 0; j < nStackPos; ++j)
					{
						MicroProfileLogEntry& LE = pLog->Log[nStack[j]];
						uint64_t nTicks = nFrameEnd - LE.nTick;
						uint32_t nTimerIndex = LE.nToken&0xffff;
						S.Frame[nTimerIndex].nTicks += nTicks;
					}
				}
			}
			{
				MICROPROFILE_SCOPE(g_MicroProfileAccumulate);
				for(uint32_t i = 0; i < S.nTotalTimers; ++i)
				{
					S.AggregateTimers[i].nTicks += S.Frame[i].nTicks;				
					S.AggregateTimers[i].nCount += S.Frame[i].nCount;
					S.MaxTimers[i] = MicroProfileMax(S.MaxTimers[i], S.Frame[i].nTicks);
					S.AggregateTimersExclusive[i] += S.FrameExclusive[i];				
					S.MaxTimersExclusive[i] = MicroProfileMax(S.MaxTimersExclusive[i], S.FrameExclusive[i]);
				}
			}
			for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
			{
				if(S.Graph[i].nToken != MICROPROFILE_INVALID_TOKEN)
				{
					MicroProfileToken nToken = S.Graph[i].nToken;
					S.Graph[i].nHistory[S.nGraphPut] = S.Frame[MicroProfileGetTimerIndex(nToken)].nTicks;
				}
			}
			S.nGraphPut = (S.nGraphPut+1) % MICROPROFILE_GRAPH_HISTORY;

		}


		bool bFlipAggregate = false;
		uint32_t nFlipFrequency = S.nAggregateFlip ? S.nAggregateFlip : 30;
		if(S.nRunning && S.nAggregateFlip <= ++S.nAggregateFlipCount)
		{
			memcpy(&S.Aggregate[0], &S.AggregateTimers[0], sizeof(S.Aggregate[0]) * S.nTotalTimers);
			memcpy(&S.AggregateMax[0], &S.MaxTimers[0], sizeof(S.AggregateMax[0]) * S.nTotalTimers);
			memcpy(&S.AggregateExclusive[0], &S.AggregateTimersExclusive[0], sizeof(S.AggregateExclusive[0]) * S.nTotalTimers);
			memcpy(&S.AggregateMaxExclusive[0], &S.MaxTimersExclusive[0], sizeof(S.AggregateMaxExclusive[0]) * S.nTotalTimers);
			
			S.nAggregateFrames = S.nAggregateFlipCount;
			S.nFlipAggregateDisplay = S.nFlipAggregate;
			S.nFlipMaxDisplay = S.nFlipMax;


			if(S.nAggregateFlip) // if 0 accumulate indefinitely
			{
				memset(&S.AggregateTimers[0], 0, sizeof(S.Aggregate[0]) * S.nTotalTimers);
				memset(&S.MaxTimers[0], 0, sizeof(S.MaxTimers[0]) * S.nTotalTimers);
				memset(&S.AggregateTimersExclusive[0], 0, sizeof(S.AggregateExclusive[0]) * S.nTotalTimers);
				memset(&S.MaxTimersExclusive[0], 0, sizeof(S.MaxTimersExclusive[0]) * S.nTotalTimers);
				S.nAggregateFlipCount = 0;
				S.nFlipAggregate = 0;
				S.nFlipMax = 0;
			}
		}
		if(S.nDisplay)
		{
			S.nActiveGroup = S.nMenuAllGroups ? S.nGroupMask : S.nMenuActiveGroup;
		}
	}

	S.nActiveGroup |= S.nForceGroup;
	S.fDetailedOffset = S.fDetailedOffset + (S.fDetailedOffsetTarget - S.fDetailedOffset) * MICROPROFILE_ANIM_DELAY_PRC;
	S.fDetailedRange = S.fDetailedRange + (S.fDetailedRangeTarget - S.fDetailedRange) * MICROPROFILE_ANIM_DELAY_PRC;

}

void MicroProfileSetDisplayMode(int nValue)
{
	nValue = nValue >= 0 && nValue < 3 ? nValue : S.nDisplay;
	S.nDisplay = nValue;
	S.fGraphBaseTime = 40.f;
	S.nOffsetY = 0;
}

void MicroProfileToggleDisplayMode()
{
	switch(S.nDisplay)
	{
		case 2:
		{
			S.nDisplay = 0;
			//S.nActiveGroup = S.nStoredGroup;
		}
		break;
		case 1:
		{
			S.nDisplay = 2;
//			S.nActiveGroup = MICROPROFILE_GROUP_MASK_ALL;
		}
		break;		
		case 0:
		{
			S.nDisplay = 1;
		}
		break;		
	}
	S.fGraphBaseTime = 40.f;
	S.nOffsetY = 0;

}


void MicroProfileFloatWindowSize(const char** ppStrings, uint32_t nNumStrings, uint32_t* pColors, uint32_t& nWidth, uint32_t& nHeight, uint32_t* pStringLengths = 0)
{
	uint32_t* nStringLengths = pStringLengths ? pStringLengths : (uint32_t*)alloca(nNumStrings * sizeof(uint32_t));
	uint32_t nTextCount = nNumStrings/2;
	for(uint32_t i = 0; i < nTextCount; ++i)
	{
		uint32_t i0 = i * 2;
		uint32_t s0, s1;
		nStringLengths[i0] = s0 = (uint32_t)strlen(ppStrings[i0]);
		nStringLengths[i0+1] = s1 = (uint32_t)strlen(ppStrings[i0+1]);
		nWidth = MicroProfileMax(s0+s1, nWidth);
	}
	nWidth = (MICROPROFILE_TEXT_WIDTH+1) * (2+nWidth) + 2 * MICROPROFILE_BORDER_SIZE;
	if(pColors)
		nWidth += MICROPROFILE_TEXT_WIDTH + 1;
	nHeight = (MICROPROFILE_TEXT_HEIGHT+1) * nTextCount + 2 * MICROPROFILE_BORDER_SIZE;
}

void MicroProfileDrawFloatWindow(uint32_t nX, uint32_t nY, const char** ppStrings, uint32_t nNumStrings, uint32_t nColor, uint32_t* pColors = 0)
{
	uint32_t nWidth = 0, nHeight = 0;
	uint32_t* nStringLengths = (uint32_t*)alloca(nNumStrings * sizeof(uint32_t));
	MicroProfileFloatWindowSize(ppStrings, nNumStrings, pColors, nWidth, nHeight, nStringLengths);
	uint32_t nTextCount = nNumStrings/2;
	if(nX + nWidth > S.nWidth)
		nX = S.nWidth - nWidth;
	if(nY + nHeight > S.nHeight)
		nY = S.nHeight - nHeight;
	MicroProfileDrawBox(nX-1, nY-1, nX + nWidth+1, nY + nHeight+1, 0xff000000|nColor);
	MicroProfileDrawBox(nX, nY, nX + nWidth, nY + nHeight, 0xff000000);
	if(pColors)
	{
		nX += MICROPROFILE_TEXT_WIDTH+1;
		nWidth -= MICROPROFILE_TEXT_WIDTH+1;
	}
	for(uint32_t i = 0; i < nTextCount; ++i)
	{
		int i0 = i * 2;
		if(pColors)
		{
			MicroProfileDrawBox(nX-MICROPROFILE_TEXT_WIDTH, nY, nX, nY + MICROPROFILE_TEXT_WIDTH, pColors[i]|0xff000000);
		}
		MicroProfileDrawText(nX + 1, nY + 1, (uint32_t)-1, ppStrings[i0]);
		MicroProfileDrawText(nX + nWidth - nStringLengths[i0+1] * (MICROPROFILE_TEXT_WIDTH+1), nY + 1, (uint32_t)-1, ppStrings[i0+1]);
		nY += (MICROPROFILE_TEXT_HEIGHT+1);
	}
}

void MicroProfileDrawTextBox(uint32_t nX, uint32_t nY, const char** ppStrings, uint32_t nNumStrings, uint32_t nColor, uint32_t* pColors = 0)
{
	uint32_t nWidth = 0, nHeight = 0;
	uint32_t* nStringLengths = (uint32_t*)alloca(nNumStrings * sizeof(uint32_t));
	for(uint32_t i = 0; i < nNumStrings; ++i)
	{
		nStringLengths[i] = (uint32_t)strlen(ppStrings[i]);
		nWidth = MicroProfileMax(nWidth, nStringLengths[i]);
		nHeight++;
	}
	nWidth = (MICROPROFILE_TEXT_WIDTH+1) * (2+nWidth) + 2 * MICROPROFILE_BORDER_SIZE;
	nHeight = (MICROPROFILE_TEXT_HEIGHT+1) * nHeight + 2 * MICROPROFILE_BORDER_SIZE;
	if(nX + nWidth > S.nWidth)
		nX = S.nWidth - nWidth;
	if(nY + nHeight > S.nHeight)
		nY = S.nHeight - nHeight;
	MicroProfileDrawBox(nX, nY, nX + nWidth, nY + nHeight, 0xff000000);
	for(uint32_t i = 0; i < nNumStrings; ++i)
	{
		MicroProfileDrawText(nX + 1, nY + 1, (uint32_t)-1, ppStrings[i]);
		nY += (MICROPROFILE_TEXT_HEIGHT+1);
	}
}




void MicroProfileDrawFloatTooltip(uint32_t nX, uint32_t nY, uint32_t nToken, uint64_t nTime)
{
	uint32_t nIndex = MicroProfileGetTimerIndex(nToken);
	uint32_t nAggregateFrames = S.nAggregateFrames ? S.nAggregateFrames : 1;
	uint32_t nAggregateCount = S.Aggregate[nIndex].nCount ? S.Aggregate[nIndex].nCount : 1;

	uint32_t nGroupId = MicroProfileGetGroupIndex(nToken);
	uint32_t nTimerId = MicroProfileGetTimerIndex(nToken);
	bool bGpu = S.GroupInfo[nGroupId].Type == MicroProfileTokenTypeGpu;

	float fToMs = MicroProfileTickToMsMultiplier(bGpu ? MicroProfileTicksPerSecondGpu() : MicroProfileTicksPerSecondCpu());

	float fMs = fToMs * (nTime);
	float fFrameMs = fToMs * (S.Frame[nIndex].nTicks);
	float fAverage = fToMs * (S.Aggregate[nIndex].nTicks/nAggregateFrames);
	float fCallAverage = fToMs * (S.Aggregate[nIndex].nTicks / nAggregateCount);
	float fMax = fToMs * (S.AggregateMax[nIndex]);

	float fFrameMsExclusive = fToMs * (S.FrameExclusive[nIndex]);
	float fAverageExclusive = fToMs * (S.AggregateExclusive[nIndex]/nAggregateFrames);
	float fMaxExclusive = fToMs * (S.AggregateMaxExclusive[nIndex]);


	MicroProfileStringArray ToolTip;
	MicroProfileStringArrayClear(&ToolTip);
	const char* pGroupName = S.GroupInfo[nGroupId].pName;
	const char* pTimerName = S.TimerInfo[nTimerId].pName;
	MicroProfileStringArrayFormat(&ToolTip, "%s", pGroupName);
	MicroProfileStringArrayFormat(&ToolTip,"%s", pTimerName);
	
	if(nTime != (uint64_t)0)
	{
		MicroProfileStringArrayAddLiteral(&ToolTip, "Time:");
		MicroProfileStringArrayFormat(&ToolTip,"%6.3fms",  fMs);
		MicroProfileStringArrayAddLiteral(&ToolTip, "");
		MicroProfileStringArrayAddLiteral(&ToolTip, "");
	}

	MicroProfileStringArrayAddLiteral(&ToolTip, "Frame Time:");
	MicroProfileStringArrayFormat(&ToolTip,"%6.3fms",  fFrameMs);

	MicroProfileStringArrayAddLiteral(&ToolTip, "Average:");
	MicroProfileStringArrayFormat(&ToolTip,"%6.3fms",  fAverage);

	MicroProfileStringArrayAddLiteral(&ToolTip, "Max:");
	MicroProfileStringArrayFormat(&ToolTip,"%6.3fms",  fMax);

	MicroProfileStringArrayAddLiteral(&ToolTip, "");
	MicroProfileStringArrayAddLiteral(&ToolTip, "");

	MicroProfileStringArrayAddLiteral(&ToolTip, "Frame Call Average:");
	MicroProfileStringArrayFormat(&ToolTip,"%6.3fms",  fCallAverage);

	MicroProfileStringArrayAddLiteral(&ToolTip, "Frame Call Count:");
	MicroProfileStringArrayFormat(&ToolTip, "%6d",  nAggregateCount / nAggregateFrames);

	MicroProfileStringArrayAddLiteral(&ToolTip, "");
	MicroProfileStringArrayAddLiteral(&ToolTip, "");

	MicroProfileStringArrayAddLiteral(&ToolTip, "Exclusive Frame Time:");
	MicroProfileStringArrayFormat(&ToolTip, "%6.3fms",  fFrameMsExclusive);

	MicroProfileStringArrayAddLiteral(&ToolTip, "Exclusive Average:");
	MicroProfileStringArrayFormat(&ToolTip, "%6.3fms",  fAverageExclusive);

	MicroProfileStringArrayAddLiteral(&ToolTip, "Exclusive Max:");
	MicroProfileStringArrayFormat(&ToolTip, "%6.3fms",  fMaxExclusive);


	MicroProfileDrawFloatWindow(nX, nY+20, &ToolTip.ppStrings[0], ToolTip.nNumStrings, S.TimerInfo[nTimerId].nColor);

	if(S.nMouseLeftMod)
	{
		int nIndex = (S.LockedToolTipFront + MICROPROFILE_TOOLTIP_MAX_LOCKED - 1) % MICROPROFILE_TOOLTIP_MAX_LOCKED;
		S.nLockedToolTipColor[nIndex] = S.TimerInfo[nTimerId].nColor;
		MicroProfileStringArrayCopy(&S.LockedToolTips[nIndex], &ToolTip);
		S.LockedToolTipFront = nIndex;

	}
}

#define MICROPROFILE_FRAME_HISTORY_HEIGHT 50
#define MICROPROFILE_FRAME_HISTORY_WIDTH 7
#define MICROPROFILE_FRAME_HISTORY_COLOR_CPU 0xffff7f27 //255 127 39
#define MICROPROFILE_FRAME_HISTORY_COLOR_GPU 0xff37a0ee //55 160 238
#define MICROPROFILE_FRAME_HISTORY_COLOR_HIGHTLIGHT 0x7733bb44
#define MICROPROFILE_FRAME_COLOR_HIGHTLIGHT 0x20009900
#define MICROPROFILE_NUM_FRAMES (MICROPROFILE_MAX_FRAME_HISTORY - (MICROPROFILE_GPU_FRAME_DELAY+1))

void MicroProfileZoomTo(int64_t nTickStart, int64_t nTickEnd)
{
	int64_t nStart = S.Frames[S.nFrameCurrent].nFrameStartCpu;
	float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
	S.fDetailedOffsetTarget = (nTickStart - nStart) * fToMs;
	S.fDetailedRangeTarget = (nTickEnd - nTickStart) * fToMs;


}

void MicroProfileCenter(int64_t nTickCenter)
{
	int64_t nStart = S.Frames[S.nFrameCurrent].nFrameStartCpu;
	float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
	float fCenter = (nTickCenter - nStart) * fToMs;
	S.fDetailedOffsetTarget = S.fDetailedOffset = fCenter - 0.5f * S.fDetailedRange;
}


void MicroProfileDrawDetailedBars(uint32_t nWidth, uint32_t nHeight, int nBaseY, int nSelectedFrame)
{
	int nY = nBaseY - S.nOffsetY;
	int64_t nNumBoxes = 0;
	int64_t nNumLines = 0;

	uint32_t nFrameNext = (S.nFrameCurrent+1) % MICROPROFILE_MAX_FRAME_HISTORY;
	MicroProfileFrameState* pFrameCurrent = &S.Frames[S.nFrameCurrent];
	MicroProfileFrameState* pFrameNext = &S.Frames[nFrameNext];

	int64_t nRangeBegin = 0, nRangeEnd = 0;
	uint64_t nFrameStartCpu = pFrameCurrent->nFrameStartCpu;
	uint64_t nFrameEndCpu = pFrameNext->nFrameStartCpu;
	uint64_t nFrameStartGpu = pFrameCurrent->nFrameStartGpu;
	uint64_t nFrameEndGpu = pFrameNext->nFrameStartGpu;
	float fToMsCpu = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
	float fToMsGpu = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondGpu());

	float fDetailedOffset = S.fDetailedOffset;
	float fDetailedRange = S.fDetailedRange;
	int64_t nDetailedOffsetTicksCpu = MicroProfileMsToTick(fDetailedOffset, MicroProfileTicksPerSecondCpu());
	int64_t nDetailedOffsetTicksGpu = MicroProfileMsToTick(fDetailedOffset, MicroProfileTicksPerSecondGpu());
	int64_t nBaseTicksCpu = nDetailedOffsetTicksCpu + nFrameStartCpu;
	int64_t nBaseTicksGpu = nDetailedOffsetTicksGpu + nFrameStartGpu;
	int64_t nBaseTicksEndCpu = nBaseTicksCpu + MicroProfileMsToTick(fDetailedRange, MicroProfileTicksPerSecondCpu());

	MicroProfileFrameState* pFrameFirst = pFrameCurrent;
	int64_t nGapTime = MicroProfileTicksPerSecondCpu() * MICROPROFILE_GAP_TIME / 1000;
	for(uint32_t i = 0; i < MICROPROFILE_MAX_FRAME_HISTORY - MICROPROFILE_GPU_FRAME_DELAY; ++i)
	{
		uint32_t nNextIndex = (S.nFrameCurrent + MICROPROFILE_MAX_FRAME_HISTORY - i) % MICROPROFILE_MAX_FRAME_HISTORY;
		pFrameFirst = &S.Frames[nNextIndex];
		if(pFrameFirst->nFrameStartCpu <= nBaseTicksCpu-nGapTime)
			break;
	}

	float fMsBase = fToMsCpu * nDetailedOffsetTicksCpu;
	float fMs = fDetailedRange;
	float fMsEnd = fMs + fMsBase;
	float fWidth = (float)nWidth;
	float fMsToScreen = fWidth / fMs;

	{
		float fRate = floor(2*(log10(fMs)-1))/2;
		float fStep = powf(10.f, fRate);
		float fRcpStep = 1.f / fStep;
		int nColorIndex = (int)(floor(fMsBase*fRcpStep));
		float fStart = floor(fMsBase*fRcpStep) * fStep;
		for(float f = fStart; f < fMsEnd; )
		{
			float fStart = f;
			float fNext = f + fStep;
			MicroProfileDrawBox(((fStart-fMsBase) * fMsToScreen), nBaseY, (fNext-fMsBase) * fMsToScreen+1, nBaseY + nHeight, S.nOpacityBackground | g_nMicroProfileBackColors[nColorIndex++ & 1]);
			f = fNext;
		}
	}

	nY += MICROPROFILE_TEXT_HEIGHT+1;
	MicroProfileLogEntry* pMouseOver = S.pDisplayMouseOver;
	MicroProfileLogEntry* pMouseOverNext = 0;
	uint64_t nMouseOverToken = pMouseOver ? pMouseOver->nToken : 0;
	float fMouseX = (float)S.nMouseX;
	float fMouseY = (float)S.nMouseY;
	uint64_t nHoverToken = MICROPROFILE_INVALID_TOKEN;
	int64_t nHoverTime = 0;
	float fHoverDist = 0.5f;
	float fBestDist = 100000.f;

	static int nHoverCounter = 155;
	static int nHoverCounterDelta = 10;
	nHoverCounter += nHoverCounterDelta;
	if(nHoverCounter >= 245)
		nHoverCounterDelta = -10;
	else if(nHoverCounter < 100)
		nHoverCounterDelta = 10;
	uint32_t nHoverColor = (nHoverCounter<<24)|(nHoverCounter<<16)|(nHoverCounter<<8)|nHoverCounter;
	uint32_t nHoverCounterShared = nHoverCounter>>2;
	uint32_t nHoverColorShared = (nHoverCounterShared<<24)|(nHoverCounterShared<<16)|(nHoverCounterShared<<8)|nHoverCounterShared;

	uint32_t nLinesDrawn[MICROPROFILE_STACK_MAX]={0};
	for(uint32_t i = 0; i < MICROPROFILE_MAX_THREADS; ++i)
	{
		MicroProfileThreadLog* pLog = S.Pool[i];
		if(!pLog)
			continue;

		uint32_t nPut = pFrameNext->nLogStart[i];
		///note: this may display new samples as old data, but this will only happen when
		// 		 unpaused, where the detailed view is hardly perceptible
		uint32_t nFront = S.Pool[i]->nPut.load(std::memory_order_relaxed); 
		MicroProfileFrameState* pFrameLogFirst = pFrameCurrent;
		MicroProfileFrameState* pFrameLogLast = pFrameNext;
		uint32_t nGet = pFrameLogFirst->nLogStart[i];
		do
		{
			//uprintf("frame log is %p\n", pFrameLogFirst);
			MP_ASSERT(pFrameLogFirst >= &S.Frames[0] && pFrameLogFirst < &S.Frames[MICROPROFILE_MAX_FRAME_HISTORY]);
			uint32_t nNewGet = pFrameLogFirst->nLogStart[i];
			//todo: fix
			bool bIsValid = false;
			if(nPut < nFront)
			{
				bIsValid = nNewGet < nPut || nNewGet > nFront;
			}
			else
			{
				bIsValid = nNewGet < nPut && nNewGet > nFront;
			}
			if(bIsValid)
			{
				nGet = nNewGet;
				if(pFrameLogFirst->nFrameStartCpu > nBaseTicksEndCpu)
				{
					pFrameLogLast = pFrameLogFirst;//pick the last frame that ends after 
				}


				pFrameLogFirst--;
				if(pFrameLogFirst < &S.Frames[0])
					pFrameLogFirst = &S.Frames[MICROPROFILE_MAX_FRAME_HISTORY-1]; 
			}
			else
			{
				break;
			}
		}while(pFrameLogFirst != pFrameFirst);


		if(nGet == (uint32_t)-1)
			continue;
		MP_ASSERT(nGet != (uint32_t)-1);

		nPut = pFrameLogLast->nLogStart[i];

		uint32_t nRange[2][2] = { {0, 0}, {0, 0}, };

		MicroProfileGetRange(nPut, nGet, nRange);
		if(nPut == nGet) 
			continue;
		if(0==S.nThreadActive[i] && 0==S.nMenuAllThreads)
			continue;
		uint32_t nSize = pLog->nPut != pLog->nGet;
		uint32_t nMaxStackDepth = 0;

		uint64_t nFrameStart = pLog->nGpu ? nFrameStartGpu : nFrameStartCpu;
		uint64_t nFrameEnd = pLog->nGpu ? nFrameEndGpu : nFrameEndCpu;

		bool bGpu = pLog->nGpu != 0;
		float fToMs = bGpu ? fToMsGpu : fToMsCpu;
		int64_t nBaseTicks = bGpu ? nBaseTicksGpu : nBaseTicksCpu;

		nY += 3;
		MicroProfileDrawText(0, nY, (uint32_t)-1, &pLog->ThreadName[0]);
		nY += 3;
		nY += MICROPROFILE_TEXT_HEIGHT + 1;
		uint32_t nYDelta = MICROPROFILE_DETAILED_BAR_HEIGHT;
		uint32_t nStack[MICROPROFILE_STACK_MAX];
		uint32_t nStackPos = 0;
		for(uint32_t j = 0; j < 2; ++j)
		{
			uint32_t nStart = nRange[j][0];
			uint32_t nEnd = nRange[j][1];
			for(uint32_t k = nStart; k < nEnd; ++k)
			{
				MicroProfileLogEntry* pEntry = pLog->Log + k;
				if(MicroProfileLogEntry::EEnter == pEntry->eType)
				{
					MP_ASSERT(nStackPos < MICROPROFILE_STACK_MAX);
					nStack[nStackPos++] = k;
				}
				else
				{
					if(0 == nStackPos)
					{
						continue;
					}

					MP_ASSERT(MicroProfileLogEntry::ELeave == pEntry->eType);
					MicroProfileLogEntry* pEntryEnter = pLog->Log + nStack[nStackPos-1];
					if(pEntryEnter->nToken != pEntry->nToken)
					{
						//uprintf("mismatch %llx %llx\n", pEntryEnter->nToken, pEntry->nToken);
						continue;
					}
					int64_t nTickStart = pEntryEnter->nTick;
					int64_t nTickEnd = pEntry->nTick;
					uint32_t nColor = S.TimerInfo[ MicroProfileGetTimerIndex(pEntry->nToken) ].nColor;
					if(nMouseOverToken == pEntry->nToken)
					{
						if(pEntry == pMouseOver)
						{
							nColor = nHoverColor;
							nRangeBegin = pEntryEnter->nTick;
							nRangeEnd = pEntry->nTick;
						}
						else
						{
							nColor = nHoverColorShared;
						}
					}

					nMaxStackDepth = MicroProfileMax(nMaxStackDepth, nStackPos);
					float fMsStart = fToMs * (nTickStart - nBaseTicks);
					float fMsEnd = fToMs * (nTickEnd - nBaseTicks);
					MP_ASSERT(fMsStart <= fMsEnd);
					float fXStart = fMsStart * fMsToScreen;
					float fXEnd = fMsEnd * fMsToScreen;
					float fYStart = (float)(nY + nStackPos * nYDelta);
					float fYEnd = fYStart + (MICROPROFILE_DETAILED_BAR_HEIGHT);
					float fXDist = MicroProfileMax(fXStart - fMouseX, fMouseX - fXEnd);
					bool bHover = fXDist < fHoverDist && fYStart <= fMouseY && fMouseY <= fYEnd && nBaseY < fMouseY;
					uint32_t nIntegerWidth = (uint32_t)(fXEnd - fXStart);
					if(nIntegerWidth)
					{
						if(bHover && S.nActiveMenu == -1)
						{
							nHoverToken = (uint32_t)pEntry->nToken;
							nHoverTime = nTickEnd - nTickStart;
							pMouseOverNext = pEntry;
							if(S.nMouseRight)
							{
								MicroProfileZoomTo(nTickStart, nTickEnd);
							}
						}
						MicroProfileDrawBox(fXStart, fYStart, fXEnd, fYEnd, nColor|S.nOpacityForeground, MicroProfileBoxTypeBar);
						++nNumBoxes;
					}
					else
					{
						float fXAvg = 0.5f * (fXStart + fXEnd);
						int nLineX = (int)floor(fXAvg+0.5f);
						if(nLineX != (int)nLinesDrawn[nStackPos])
						{
							if(bHover && S.nActiveMenu == -1)
							{
								nHoverToken = (uint32_t)pEntry->nToken;
								nHoverTime = nTickEnd - nTickStart;
								pMouseOverNext = pEntry;
							}
							nLinesDrawn[nStackPos] = nLineX;
							MicroProfileDrawLineVertical(nLineX, fYStart + 0.5f, fYEnd + 0.5f, nColor|S.nOpacityForeground);
							++nNumLines;
						}
					}
					nStackPos--;
				}
			}
		}
		nY += nMaxStackDepth * nYDelta + MICROPROFILE_DETAILED_BAR_HEIGHT+1;
	}
	S.pDisplayMouseOver = pMouseOverNext;

	if(nHoverToken != MICROPROFILE_INVALID_TOKEN && nHoverTime)
	{
		S.nHoverToken = nHoverToken;
		S.nHoverTime = nHoverTime;
	}


	if(nSelectedFrame != -1)
	{
		nRangeBegin = S.Frames[nSelectedFrame].nFrameStartCpu;
		nRangeEnd = S.Frames[(nSelectedFrame+1)%MICROPROFILE_MAX_FRAME_HISTORY].nFrameStartCpu;
	}
	if(nRangeBegin != nRangeEnd)
	{
		float fMsStart = fToMsCpu * (nRangeBegin - nBaseTicksCpu);
		float fMsEnd = fToMsCpu * (nRangeEnd - nBaseTicksCpu);
		float fXStart = fMsStart * fMsToScreen;
		float fXEnd = fMsEnd * fMsToScreen;	
		MicroProfileDrawBox(fXStart, nBaseY, fXEnd, nHeight, MICROPROFILE_FRAME_COLOR_HIGHTLIGHT, MicroProfileBoxTypeFlat);
		MicroProfileDrawLineVertical(fXStart, nBaseY, nHeight, MICROPROFILE_FRAME_COLOR_HIGHTLIGHT | 0x44000000);
		MicroProfileDrawLineVertical(fXEnd, nBaseY, nHeight, MICROPROFILE_FRAME_COLOR_HIGHTLIGHT | 0x44000000);

		fMsStart += fDetailedOffset;
		fMsEnd += fDetailedOffset;
		char sBuffer[32];
		uint32_t nLenStart = snprintf(sBuffer, sizeof(sBuffer)-1, "%.2fms", fMsStart);
		float fStartTextWidth = (float)((1+MICROPROFILE_TEXT_WIDTH) * nLenStart);
		float fStartTextX = fXStart - fStartTextWidth - 2;
		MicroProfileDrawBox(fStartTextX, nBaseY, fStartTextX + fStartTextWidth + 2, MICROPROFILE_TEXT_HEIGHT + 2 + nBaseY, 0x33000000, MicroProfileBoxTypeFlat);
		MicroProfileDrawText(fStartTextX+1, nBaseY, (uint32_t)-1, sBuffer);
		uint32_t nLenEnd = snprintf(sBuffer, sizeof(sBuffer)-1, "%.2fms", fMsEnd);
		MicroProfileDrawBox(fXEnd+1, nBaseY, fXEnd+1+(1+MICROPROFILE_TEXT_WIDTH) * nLenEnd + 3, MICROPROFILE_TEXT_HEIGHT + 2 + nBaseY, 0x33000000, MicroProfileBoxTypeFlat);
		MicroProfileDrawText(fXEnd+2, nBaseY+1, (uint32_t)-1, sBuffer);

	}
}


void MicroProfileDrawDetailedFrameHistory(uint32_t nWidth, uint32_t nHeight, uint32_t nBaseY, uint32_t nSelectedFrame)
{
	const uint32_t nBarWidth = MICROPROFILE_FRAME_HISTORY_WIDTH;
	const uint32_t nBarHeight = MICROPROFILE_FRAME_HISTORY_HEIGHT;
	uint32_t nBaseX = nWidth - nBarWidth;
	float fBaseX = (float)nWidth;
	float fDx = fBaseX / MICROPROFILE_NUM_FRAMES;

	uint32_t nLastIndex =  (S.nFrameCurrent+1) % MICROPROFILE_MAX_FRAME_HISTORY;
	MicroProfileDrawBox(0, nBaseY, nWidth, nBaseY+MICROPROFILE_FRAME_HISTORY_HEIGHT, 0xff000000 | g_nMicroProfileBackColors[0], MicroProfileBoxTypeFlat);
	float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu()) * S.fRcpReferenceTime;
	float fToMsGpu = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondGpu()) * S.fRcpReferenceTime;

	
	uint32_t nFrameNext = (S.nFrameCurrent+1) % MICROPROFILE_MAX_FRAME_HISTORY;
	MicroProfileFrameState* pFrameCurrent = &S.Frames[S.nFrameCurrent];
	MicroProfileFrameState* pFrameNext = &S.Frames[nFrameNext];	
	uint64_t nFrameStartCpu = pFrameCurrent->nFrameStartCpu;
	int64_t nDetailedOffsetTicksCpu = MicroProfileMsToTick(S.fDetailedOffset, MicroProfileTicksPerSecondCpu());
	int64_t nCpuStart = nDetailedOffsetTicksCpu + nFrameStartCpu;
	int64_t nCpuEnd = nCpuStart + MicroProfileMsToTick(S.fDetailedRange, MicroProfileTicksPerSecondCpu());;


	float fSelectionStart = (float)nWidth;
	float fSelectionEnd = 0.f;
	for(uint32_t i = 0; i < MICROPROFILE_NUM_FRAMES; ++i)
	{
		uint32_t nIndex = (S.nFrameCurrent + MICROPROFILE_MAX_FRAME_HISTORY - i) % MICROPROFILE_MAX_FRAME_HISTORY;
		MicroProfileFrameState* pCurrent = &S.Frames[nIndex];
		MicroProfileFrameState* pNext = &S.Frames[nLastIndex];
		
		int64_t nTicks = pNext->nFrameStartCpu - pCurrent->nFrameStartCpu;
		int64_t nTicksGpu = pNext->nFrameStartGpu - pCurrent->nFrameStartGpu;
		float fScale = fToMs * nTicks;
		float fScaleGpu = fToMsGpu * nTicksGpu;
		fScale = fScale > 1.f ? 0.f : 1.f - fScale;
		fScaleGpu = fScaleGpu > 1.f ? 0.f : 1.f - fScaleGpu;
		float fXEnd = fBaseX;
		float fXStart = fBaseX - fDx;
		fBaseX = fXStart;
		uint32_t nColor = MICROPROFILE_FRAME_HISTORY_COLOR_CPU;
		if(nIndex == nSelectedFrame)
			nColor = (uint32_t)-1;
		MicroProfileDrawBox(fXStart, nBaseY + fScale * nBarHeight, fXEnd, nBaseY+MICROPROFILE_FRAME_HISTORY_HEIGHT, nColor, MicroProfileBoxTypeBar);
		if(pNext->nFrameStartCpu > nCpuStart)
		{
			fSelectionStart = fXStart;
		}
		if(pCurrent->nFrameStartCpu < nCpuEnd && fSelectionEnd == 0.f)
		{
			fSelectionEnd = fXEnd;
		}
		nLastIndex = nIndex;
	}
	MicroProfileDrawBox(fSelectionStart, nBaseY, fSelectionEnd, nBaseY+MICROPROFILE_FRAME_HISTORY_HEIGHT, MICROPROFILE_FRAME_HISTORY_COLOR_HIGHTLIGHT, MicroProfileBoxTypeFlat);
}
void MicroProfileDrawDetailedView(uint32_t nWidth, uint32_t nHeight)
{
	MICROPROFILE_SCOPE(g_MicroProfileDetailed);
	uint32_t nBaseY = S.nBarHeight + 1;

	const float fMousePrc = MicroProfileMax((float)S.nMouseX / S.nWidth ,0.f);
	int nSelectedFrame = -1;
	if(S.nMouseY > nBaseY && S.nMouseY <= nBaseY + MICROPROFILE_FRAME_HISTORY_HEIGHT && S.nActiveMenu == -1)
	{

		nSelectedFrame = ((MICROPROFILE_NUM_FRAMES) * (S.nWidth-S.nMouseX) / S.nWidth);
		nSelectedFrame = (S.nFrameCurrent + MICROPROFILE_MAX_FRAME_HISTORY - nSelectedFrame) % MICROPROFILE_MAX_FRAME_HISTORY;
		S.nHoverFrame = nSelectedFrame;
		if(S.nMouseRight)
		{
			int64_t nRangeBegin = S.Frames[nSelectedFrame].nFrameStartCpu;
			int64_t nRangeEnd = S.Frames[(nSelectedFrame+1)%MICROPROFILE_MAX_FRAME_HISTORY].nFrameStartCpu;
			MicroProfileZoomTo(nRangeBegin, nRangeEnd);
		}
		if(S.nMouseDownLeft)
		{
			uint64_t nFrac = (1024 * (MICROPROFILE_NUM_FRAMES) * (S.nMouseX) / S.nWidth) % 1024;
			int64_t nRangeBegin = S.Frames[nSelectedFrame].nFrameStartCpu;
			int64_t nRangeEnd = S.Frames[(nSelectedFrame+1)%MICROPROFILE_MAX_FRAME_HISTORY].nFrameStartCpu;
			MicroProfileCenter(nRangeBegin + (nRangeEnd-nRangeBegin) * nFrac / 1024);
		}
	}
	else
	{
		S.nHoverFrame = -1;
	}

	MicroProfileDrawDetailedBars(nWidth, nHeight, nBaseY + MICROPROFILE_FRAME_HISTORY_HEIGHT, nSelectedFrame);
	MicroProfileDrawDetailedFrameHistory(nWidth, nHeight, nBaseY, nSelectedFrame);
}



void MicroProfileLoopActiveGroups(uint32_t nX, uint32_t nY, const char* pName, std::function<void (uint32_t, uint32_t, uint64_t, uint32_t, uint32_t, float)> CB)
{
	if(pName)
		MicroProfileDrawText(nX, nY, (uint32_t)-1, pName);

	nY += S.nBarHeight + 2;
	uint32_t nGroup = (uint32_t)S.nActiveGroup;
	uint64_t nGroupMask = (uint64_t)-1;
	uint32_t nCount = 0;
	for(uint32_t j = 0; j < MICROPROFILE_MAX_GROUPS; ++j)
	{
		uint64_t nMask = 1ll << j;
		if(nMask & nGroup)
		{
			nY += S.nBarHeight + 1;
			float fToMs = MicroProfileTickToMsMultiplier(S.GroupInfo[j].Type == MicroProfileTokenTypeGpu ? MicroProfileTicksPerSecondGpu() : MicroProfileTicksPerSecondCpu());

			for(uint32_t i = 0; i < S.nTotalTimers;++i)
			{
				uint64_t nTokenMask = MicroProfileGetGroupMask(S.TimerInfo[i].nToken);
				if(nTokenMask & nMask)
				{
					CB(i, nCount, nMask, nX, nY, fToMs);
					nCount += 2;
					nY += S.nBarHeight + 1;
				}
			}
		}
	}
}


void MicroProfileCalcTimers(float* pTimers, float* pAverage, float* pMax, float* pCallAverage, float* pExclusive, float* pAverageExclusive, float* pMaxExclusive, uint64_t nGroup, uint32_t nSize)
{
	MicroProfileLoopActiveGroups(0, 0, 0, 
		[=](uint32_t nTimer, uint32_t nIdx, uint64_t nGroupMask, uint32_t nX, uint32_t nY, float fToMs){
			uint32_t nAggregateFrames = S.nAggregateFrames ? S.nAggregateFrames : 1;
			uint32_t nAggregateCount = S.Aggregate[nTimer].nCount ? S.Aggregate[nTimer].nCount : 1;
			float fToPrc = S.fRcpReferenceTime;
			float fMs = fToMs * (S.Frame[nTimer].nTicks);
			float fPrc = MicroProfileMin(fMs * fToPrc, 1.f);
			float fAverageMs = fToMs * (S.Aggregate[nTimer].nTicks / nAggregateFrames);
			float fAveragePrc = MicroProfileMin(fAverageMs * fToPrc, 1.f);
			float fMaxMs = fToMs * (S.AggregateMax[nTimer]);
			float fMaxPrc = MicroProfileMin(fMaxMs * fToPrc, 1.f);
			float fCallAverageMs = fToMs * (S.Aggregate[nTimer].nTicks / nAggregateCount);
			float fCallAveragePrc = MicroProfileMin(fCallAverageMs * fToPrc, 1.f);
			float fMsExclusive = fToMs * (S.FrameExclusive[nTimer]);
			float fPrcExclusive = MicroProfileMin(fMsExclusive * fToPrc, 1.f);
			float fAverageMsExclusive = fToMs * (S.AggregateExclusive[nTimer] / nAggregateFrames);
			float fAveragePrcExclusive = MicroProfileMin(fAverageMsExclusive * fToPrc, 1.f);
			float fMaxMsExclusive = fToMs * (S.AggregateMaxExclusive[nTimer]);
			float fMaxPrcExclusive = MicroProfileMin(fMaxMsExclusive * fToPrc, 1.f);
			pTimers[nIdx] = fMs;
			pTimers[nIdx+1] = fPrc;
			pAverage[nIdx] = fAverageMs;
			pAverage[nIdx+1] = fAveragePrc;
			pMax[nIdx] = fMaxMs;
			pMax[nIdx+1] = fMaxPrc;
			pCallAverage[nIdx] = fCallAverageMs;
			pCallAverage[nIdx+1] = fCallAveragePrc;
			pExclusive[nIdx] = fMsExclusive;
			pExclusive[nIdx+1] = fPrcExclusive;
			pAverageExclusive[nIdx] = fAverageMsExclusive;
			pAverageExclusive[nIdx+1] = fAveragePrcExclusive;
			pMaxExclusive[nIdx] = fMaxMsExclusive;
			pMaxExclusive[nIdx+1] = fMaxPrcExclusive;
		}
	);
}

#define SBUF_MAX 32

uint32_t MicroProfileDrawBarArray(uint32_t nX, uint32_t nY, float* pTimers, const char* pName, uint32_t nTotalHeight)
{
	const uint32_t nHeight = S.nBarHeight;
	const uint32_t nWidth = S.nBarWidth;
	const uint32_t nTextWidth = 6 * (1+MICROPROFILE_TEXT_WIDTH);
	const float fWidth = (float)S.nBarWidth;

	MicroProfileDrawLineVertical(nX-5, nY, nTotalHeight, S.nOpacityBackground|g_nMicroProfileBackColors[0]|g_nMicroProfileBackColors[1]);

	MicroProfileLoopActiveGroups(nX, nY, pName, 
		[=](uint32_t nTimer, uint32_t nIdx, uint64_t nGroupMask, uint32_t nX, uint32_t nY, float fToMs){
			char sBuffer[SBUF_MAX];
			snprintf(sBuffer, SBUF_MAX-1, "%5.2f", pTimers[nIdx]);
			MicroProfileDrawBox(nX + nTextWidth, nY, nX + nTextWidth + fWidth * pTimers[nIdx+1], nY + nHeight, S.nOpacityForeground|S.TimerInfo[nTimer].nColor, MicroProfileBoxTypeBar);
			MicroProfileDrawText(nX, nY, (uint32_t)-1, sBuffer);
			
		});
	return nWidth + 5 + nTextWidth;

}

uint32_t MicroProfileDrawBarCallCount(uint32_t nX, uint32_t nY, const char* pName)
{
	MicroProfileLoopActiveGroups(nX, nY, pName, 
		[](uint32_t nTimer, uint32_t nIdx, uint64_t nGroupMask, uint32_t nX, uint32_t nY, float fToMs){
			char sBuffer[SBUF_MAX];
			snprintf(sBuffer, SBUF_MAX-1, "%5d", S.Frame[nTimer].nCount);//fix
			MicroProfileDrawText(nX, nY, (uint32_t)-1, sBuffer);
		});
	uint32_t nTextWidth = 6 * MICROPROFILE_TEXT_WIDTH;
	return 5 + nTextWidth;
}



uint32_t MicroProfileDrawBarLegend(uint32_t nX, uint32_t nY, uint32_t nTotalHeight)
{
	MicroProfileDrawLineVertical(nX-5, nY, nTotalHeight, S.nOpacityBackground | g_nMicroProfileBackColors[0]|g_nMicroProfileBackColors[1]);
	MicroProfileLoopActiveGroups(nX, nY, 0, 
		[](uint32_t nTimer, uint32_t nIdx, uint64_t nGroupMask, uint32_t nX, uint32_t nY, float fToMs){
			MicroProfileDrawText(nX, nY, S.TimerInfo[nTimer].nColor, S.TimerInfo[nTimer].pName);
			if(S.nMouseY >= nY && S.nMouseY < nY + MICROPROFILE_TEXT_HEIGHT+1  && S.nMouseX < nX + 20 * (MICROPROFILE_TEXT_WIDTH+1))
			{
				S.nHoverToken = nTimer;
				S.nHoverTime = 0;
			}
		});
	return nX;
}

bool MicroProfileDrawGraph(uint32_t nScreenWidth, uint32_t nScreenHeight)
{
	MICROPROFILE_SCOPE(g_MicroProfileDrawGraph);
	bool bEnabled = false;
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
		if(S.Graph[i].nToken != MICROPROFILE_INVALID_TOKEN)
			bEnabled = true;
	if(!bEnabled)
		return false;
	
	uint32_t nX = nScreenWidth - MICROPROFILE_GRAPH_WIDTH;
	uint32_t nY = nScreenHeight - MICROPROFILE_GRAPH_HEIGHT;
	MicroProfileDrawBox(nX, nY, nX + MICROPROFILE_GRAPH_WIDTH, nY + MICROPROFILE_GRAPH_HEIGHT, S.nOpacityBackground | g_nMicroProfileBackColors[0]|g_nMicroProfileBackColors[1]);
	bool bMouseOver = S.nMouseX >= nX && S.nMouseY >= nY;
	float fMouseXPrc =(float(S.nMouseX - nX)) / MICROPROFILE_GRAPH_WIDTH;
	if(bMouseOver)
	{
		float fXAvg = fMouseXPrc * MICROPROFILE_GRAPH_WIDTH + nX;
		float fLine[] = {
			fXAvg, (float)nY,
			fXAvg, (float)nY + MICROPROFILE_GRAPH_HEIGHT,

		};
		MicroProfileDrawLineVertical(fXAvg, nY, nY + MICROPROFILE_GRAPH_HEIGHT, (uint32_t)-1);
	}

	
	float fY = (float)nScreenHeight;
	float fDX = MICROPROFILE_GRAPH_WIDTH * 1.f / MICROPROFILE_GRAPH_HISTORY;  
	float fDY = MICROPROFILE_GRAPH_HEIGHT;
	uint32_t nPut = S.nGraphPut;
	float* pGraphData = (float*)alloca(sizeof(float)* MICROPROFILE_GRAPH_HISTORY*2);
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
	{
		if(S.Graph[i].nToken != MICROPROFILE_INVALID_TOKEN)
		{
			uint32_t nGroupId = MicroProfileGetGroupIndex(S.Graph[i].nToken);
			bool bGpu = S.GroupInfo[nGroupId].Type == MicroProfileTokenTypeGpu;
			float fToMs = MicroProfileTickToMsMultiplier(bGpu ? MicroProfileTicksPerSecondGpu() : MicroProfileTicksPerSecondCpu());
			float fToPrc = fToMs * S.fRcpReferenceTime * 3 / 4;

			float fX = (float)nX;
			for(uint32_t j = 0; j < MICROPROFILE_GRAPH_HISTORY; ++j)
			{
				float fWeigth = MicroProfileMin(fToPrc * (S.Graph[i].nHistory[(j+nPut)%MICROPROFILE_GRAPH_HISTORY]), 1.f);
				pGraphData[(j*2)] = fX;
				pGraphData[(j*2)+1] = fY - fDY * fWeigth;
				fX += fDX;
			}
			MicroProfileDrawLine2D(MICROPROFILE_GRAPH_HISTORY, pGraphData, S.TimerInfo[MicroProfileGetTimerIndex(S.Graph[i].nToken)].nColor);
		}
	}
	{
		float fY1 = 0.25f * MICROPROFILE_GRAPH_HEIGHT + nY;
		float fY2 = 0.50f * MICROPROFILE_GRAPH_HEIGHT + nY;
		float fY3 = 0.75f * MICROPROFILE_GRAPH_HEIGHT + nY;
		float fLine[] = {
			(float)nX, fY1,
			(float)nX + MICROPROFILE_GRAPH_WIDTH, fY1,
			(float)nX, fY2,
			(float)nX + MICROPROFILE_GRAPH_WIDTH, fY2,
			(float)nX, fY3,
			(float)nX + MICROPROFILE_GRAPH_WIDTH, fY3,
		};
		MicroProfileDrawLineHorizontal(nX, nX + MICROPROFILE_GRAPH_WIDTH, fY1, 0xffdd4444);
		MicroProfileDrawLineHorizontal(nX, nX + MICROPROFILE_GRAPH_WIDTH, fY2, 0xff000000| g_nMicroProfileBackColors[0]);
		MicroProfileDrawLineHorizontal(nX, nX + MICROPROFILE_GRAPH_WIDTH, fY3, 0xff000000|g_nMicroProfileBackColors[0]);

		char buf[32];
		snprintf(buf, sizeof(buf)-1, "%5.2fms", S.fReferenceTime);
		MicroProfileDrawText(nX+1, fY1 - (2+MICROPROFILE_TEXT_HEIGHT), (uint32_t)-1, buf);
	}



	if(bMouseOver)
	{
		uint32_t pColors[MICROPROFILE_MAX_GRAPHS];
		MicroProfileStringArray Strings;
		MicroProfileStringArrayClear(&Strings);
		uint32_t nTextCount = 0;
		uint32_t nGraphIndex = (S.nGraphPut + MICROPROFILE_GRAPH_HISTORY - int(MICROPROFILE_GRAPH_HISTORY*(1.f - fMouseXPrc))) % MICROPROFILE_GRAPH_HISTORY;

		const uint32_t nBoxSize = MICROPROFILE_TEXT_HEIGHT;
		uint32_t nX = S.nMouseX;
		uint32_t nY = S.nMouseY + 20;

		float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
		for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
		{
			if(S.Graph[i].nToken != MICROPROFILE_INVALID_TOKEN)
			{
				uint32_t nGroupId = MicroProfileGetGroupIndex(S.Graph[i].nToken);
				bool bGpu = S.GroupInfo[nGroupId].Type == MicroProfileTokenTypeGpu;
				float fToMs = MicroProfileTickToMsMultiplier(bGpu ? MicroProfileTicksPerSecondGpu() : MicroProfileTicksPerSecondCpu());
				uint32_t nIndex = MicroProfileGetTimerIndex(S.Graph[i].nToken);
				uint32_t nColor = S.TimerInfo[nIndex].nColor;
				const char* pName = S.TimerInfo[nIndex].pName;
				pColors[nTextCount++] = nColor;
				MicroProfileStringArrayAddLiteral(&Strings, pName);
				MicroProfileStringArrayFormat(&Strings, "%5.2fms", fToMs * (S.Graph[i].nHistory[nGraphIndex]));
			}
		}
		if(nTextCount)
		{
			MicroProfileDrawFloatWindow(nX, nY, Strings.ppStrings, Strings.nNumStrings, 0, pColors);
		}

		if(S.nMouseRight)
		{
			for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
			{
				S.Graph[i].nToken = MICROPROFILE_INVALID_TOKEN;
			}
		}
	}

	return bMouseOver;
}

void MicroProfileDrawBarView(uint32_t nScreenWidth, uint32_t nScreenHeight)
{
	if(!S.nActiveGroup)
		return;
	MICROPROFILE_SCOPE(g_MicroProfileDrawBarView);

	const uint32_t nHeight = S.nBarHeight;
	const uint32_t nWidth = S.nBarWidth;
	int nColorIndex = 0;
	uint32_t nX = 0;
	uint32_t nY = nHeight + 1 - S.nOffsetY;	
	uint32_t nNumTimers = 0;
	uint32_t nNumGroups = 0;
	uint32_t nMaxTimerNameLen = 1;
	for(uint32_t j = 0; j < MICROPROFILE_MAX_GROUPS; ++j)
	{
		if(S.nActiveGroup & (1ll << j))
		{
			nNumTimers += S.GroupInfo[j].nNumTimers;
			nNumGroups += 1;
			nMaxTimerNameLen = MicroProfileMax(nMaxTimerNameLen, S.GroupInfo[j].nMaxTimerNameLen);
		}
	}
	uint32_t nBlockSize = 2 * nNumTimers;
	float* pTimers = (float*)alloca(nBlockSize * 7 * sizeof(float));
	float* pAverage = pTimers + nBlockSize;
	float* pMax = pTimers + 2 * nBlockSize;
	float* pCallAverage = pTimers + 3 * nBlockSize;
	float* pTimersExclusive = pTimers + 4 * nBlockSize;
	float* pAverageExclusive = pTimers + 5 * nBlockSize;
	float* pMaxExclusive = pTimers + 6 * nBlockSize;
	MicroProfileCalcTimers(pTimers, pAverage, pMax, pCallAverage, pTimersExclusive, pAverageExclusive, pMaxExclusive, S.nActiveGroup, nNumTimers);
	{
		uint32_t nWidth = 0;
		for(uint32_t i = 1; i < MP_DRAW_ALL; i <<= 1)
		{
			if(S.nBars & i)
			{
				nWidth += S.nBarWidth + 5 + 6 * (1+MICROPROFILE_TEXT_WIDTH);
				if(i & MP_DRAW_CALL_COUNT)
					nWidth += 5 + 6 * MICROPROFILE_TEXT_WIDTH;
			}
		}
		nWidth += (1+nMaxTimerNameLen) * (MICROPROFILE_TEXT_WIDTH+1);
		for(uint32_t i = 0; i < nNumTimers+nNumGroups+1; ++i)
		{
			int nY0 = nY + i * (nHeight + 1);
			MicroProfileDrawBox(nX, nY0, nWidth, nY0 + (nHeight+1)+1, S.nOpacityBackground | g_nMicroProfileBackColors[nColorIndex++ & 1]);
		}
	}
	int nTotalHeight = (nNumTimers+nNumGroups+2) * (nHeight+1);
	uint32_t nLegendOffset = 1;
	for(uint32_t j = 0; j < MICROPROFILE_MAX_GROUPS; ++j)
	{
		if(S.nActiveGroup & (1ll << j))
		{
			MicroProfileDrawText(nX, nY + (1+nHeight) * nLegendOffset, (uint32_t)-1, S.GroupInfo[j].pName);
			nLegendOffset += S.GroupInfo[j].nNumTimers+1;
		}
	}
	if(S.nBars & MP_DRAW_TIMERS)		
		nX += MicroProfileDrawBarArray(nX, nY, pTimers, "Time", nTotalHeight) + 1;
	if(S.nBars & MP_DRAW_AVERAGE)		
		nX += MicroProfileDrawBarArray(nX, nY, pAverage, "Average", nTotalHeight) + 1;
	if(S.nBars & MP_DRAW_MAX)		
		nX += MicroProfileDrawBarArray(nX, nY, pMax, "Max Time", nTotalHeight) + 1;
	if(S.nBars & MP_DRAW_CALL_COUNT)		
	{
		nX += MicroProfileDrawBarArray(nX, nY, pCallAverage, "Call Average", nTotalHeight) + 1;
		nX += MicroProfileDrawBarCallCount(nX, nY, "Count") + 1; 
	}
	if(S.nBars & MP_DRAW_TIMERS_EXCLUSIVE)		
		nX += MicroProfileDrawBarArray(nX, nY, pTimersExclusive, "Exclusive Time", nTotalHeight) + 1;
	if(S.nBars & MP_DRAW_AVERAGE_EXCLUSIVE)		
		nX += MicroProfileDrawBarArray(nX, nY, pAverageExclusive, "Exclusive Average", nTotalHeight) + 1;
	if(S.nBars & MP_DRAW_MAX_EXCLUSIVE)		
		nX += MicroProfileDrawBarArray(nX, nY, pMaxExclusive, "Exclusive Max Time", nTotalHeight) + 1;
	nX += MicroProfileDrawBarLegend(nX, nY, nTotalHeight) + 1;
}

void MicroProfileDrawMenu(uint32_t nWidth, uint32_t nHeight)
{
	uint32_t nX = 0;
	uint32_t nY = 0;
	bool bMouseOver = S.nMouseY < MICROPROFILE_TEXT_HEIGHT + 1;
#define SBUF_SIZE 256
	char buffer[256];
	MicroProfileDrawBox(nX, nY, nX + nWidth, nY + (S.nBarHeight+1)+1, 0xff000000|g_nMicroProfileBackColors[1]);

#define MICROPROFILE_MENU_MAX 16
	const char* pMenuText[MICROPROFILE_MENU_MAX] = {0};
	uint32_t 	nMenuX[MICROPROFILE_MENU_MAX] = {0};
	uint32_t nNumMenuItems = 0;

	snprintf(buffer, 127, "MicroProfile");
	MicroProfileDrawText(nX, nY, (uint32_t)-1, buffer);
	nX += (sizeof("MicroProfile")+2) * (MICROPROFILE_TEXT_WIDTH+1);
	pMenuText[nNumMenuItems++] = "Mode";
	pMenuText[nNumMenuItems++] = "Groups";
	char AggregateText[64];
	snprintf(AggregateText, sizeof(AggregateText)-1, "Aggregate[%d]", S.nAggregateFlip ? S.nAggregateFlip : S.nAggregateFlipCount);
	pMenuText[nNumMenuItems++] = &AggregateText[0];
	pMenuText[nNumMenuItems++] = "Timers";
	pMenuText[nNumMenuItems++] = "Options";
	pMenuText[nNumMenuItems++] = "Preset";
	const int nPauseIndex = nNumMenuItems;
	pMenuText[nNumMenuItems++] = S.nRunning ? "Pause" : "Unpause";
	pMenuText[nNumMenuItems++] = "Help";

	if(S.nOverflow)
	{
		pMenuText[nNumMenuItems++] = "!BUFFERSFULL!";
	}


	struct SOptionDesc
	{
		SOptionDesc(){}
		SOptionDesc(uint8_t nSubType, uint8_t nIndex, const char* fmt, ...):nSubType(nSubType), nIndex(nIndex)
		{
			va_list args;
			va_start (args, fmt);
			vsprintf(Text, fmt, args);
			va_end(args);
		}
		char Text[32];
		uint8_t nSubType;
		uint8_t nIndex;
		bool bSelected;
	};
	static const int nNumReferencePresets = sizeof(g_MicroProfileReferenceTimePresets)/sizeof(g_MicroProfileReferenceTimePresets[0]);
	static const int nNumOpacityPresets = sizeof(g_MicroProfileOpacityPresets)/sizeof(g_MicroProfileOpacityPresets[0]);
	static const int nOptionSize = nNumReferencePresets + nNumOpacityPresets * 2 + 3;
	static SOptionDesc Options[nOptionSize];
	static bool bOptionInit = false;
	if(!bOptionInit)
	{
		bOptionInit = true;
		int nIndex = 0;
		Options[nIndex++] = SOptionDesc(0xff, 0, "%s", "Reference");
		for(int i = 0; i < nNumReferencePresets; ++i)
		{
			Options[nIndex++] = SOptionDesc(0, i, "  %6.2fms", g_MicroProfileReferenceTimePresets[i]);
		}
		Options[nIndex++] = SOptionDesc(0xff, 0, "%s", "BG Opacity");		
		for(int i = 0; i < nNumOpacityPresets; ++i)
		{
			Options[nIndex++] = SOptionDesc(1, i, "  %7d%%", (i+1)*25);
		}
		Options[nIndex++] = SOptionDesc(0xff, 0, "%s", "FG Opacity");		
		for(int i = 0; i < nNumOpacityPresets; ++i)
		{
			Options[nIndex++] = SOptionDesc(2, i, "  %7d%%", (i+1)*25);
		}


		MP_ASSERT(nIndex == nOptionSize);
	}



	typedef std::function<const char* (int, bool&)> SubmenuCallback; 
	typedef std::function<void(int)> ClickCallback;
	SubmenuCallback GroupCallback[] = 
	{	[] (int index, bool& bSelected) -> const char*{
			switch(index)
			{
				case 0: 
					bSelected = S.nDisplay == MP_DRAW_DETAILED;
					return "Detailed";
				case 1:
					bSelected = S.nDisplay == MP_DRAW_BARS; 
					return "Timers";
				case 2:
					bSelected = false; 
					return "Off";

				default: return 0;
			}
		},
		[] (int index, bool& bSelected) -> const char*{
			if(index == 0)
			{
				bSelected = S.nMenuAllGroups != 0;
				return "ALL";
			}
			else
			{
				index = index-1;
				bSelected = 0 != (S.nMenuActiveGroup & (1ll << index));
				if(index < MICROPROFILE_MAX_GROUPS && S.GroupInfo[index].pName)
					return S.GroupInfo[index].pName;
				else
					return 0;
			}
		},
		[] (int index, bool& bSelected) -> const char*{
			if(index < sizeof(g_MicroProfileAggregatePresets)/sizeof(g_MicroProfileAggregatePresets[0]))
			{
				int val = g_MicroProfileAggregatePresets[index];
				bSelected = (int)S.nAggregateFlip == val;
				if(0 == val)
					return "Infinite";
				else
				{
					static char buf[128];
					snprintf(buf, sizeof(buf)-1, "%7d", val);
					return buf;
				}
			}
			return 0;
		},
		[] (int index, bool& bSelected) -> const char*{
			bSelected = 0 != (S.nBars & (1 << index));
			switch(index)
			{
				case 0: return "Time";				
				case 1: return "Average";				
				case 2: return "Max";
				case 3: return "Call Count";
				case 4: return "Exclusive Timers";
				case 5: return "Exclusive Average";
				case 6: return "Exclusive Max";
			}
			return 0;
		},
		[] (int index, bool& bSelected) -> const char*{
			if(index >= nOptionSize) return 0;
			switch(Options[index].nSubType)
			{
			case 0:
				bSelected = S.fReferenceTime == g_MicroProfileReferenceTimePresets[Options[index].nIndex];
				break;
			case 1:
				bSelected = S.nOpacityBackground>>24 == g_MicroProfileOpacityPresets[Options[index].nIndex];
				break;
			case 2:
				bSelected = S.nOpacityForeground>>24 == g_MicroProfileOpacityPresets[Options[index].nIndex];
				break;
			}
			return Options[index].Text;
		},

		[] (int index, bool& bSelected) -> const char*{
			static char buf[128];
			bSelected = false;
			int nNumPresets = sizeof(g_MicroProfilePresetNames) / sizeof(g_MicroProfilePresetNames[0]);
			int nIndexSave = index - nNumPresets - 1;
			if(index == nNumPresets)
				return "--";
			else if(nIndexSave >=0 && nIndexSave <nNumPresets)
			{
				snprintf(buf, sizeof(buf)-1, "Save '%s'", g_MicroProfilePresetNames[nIndexSave]);
				return buf;
			}
			else if(index < nNumPresets)
			{
				snprintf(buf, sizeof(buf)-1, "Load '%s'", g_MicroProfilePresetNames[index]);
				return buf;
			}
			else
			{
				return 0;
			}
		},

		[] (int index, bool& bSelected) -> const char*{
			return 0;
		},
		[] (int index, bool& bSelected) -> const char*{
			return 0;
		},
		[] (int index, bool& bSelected) -> const char*{
			return 0;
		},


	};
	ClickCallback CBClick[] = 
	{
		[](int nIndex)
		{
			switch(nIndex)
			{
				case 0:
					S.nDisplay = MP_DRAW_DETAILED;
					break;
				case 1:
					S.nDisplay = MP_DRAW_BARS;
					break;
				case 2:
					S.nDisplay = 0;
					break;
			}
		},
		[](int nIndex)
		{
			if(nIndex == 0)
				S.nMenuAllGroups = 1-S.nMenuAllGroups;
			else
				S.nMenuActiveGroup ^= (1ll << (nIndex-1));
		},
		[](int nIndex)
		{
			S.nAggregateFlip = g_MicroProfileAggregatePresets[nIndex];
			if(0 == S.nAggregateFlip)
			{
				memset(S.AggregateTimers, 0, sizeof(S.AggregateTimers));
				memset(S.MaxTimers, 0, sizeof(S.MaxTimers));
				memset(S.AggregateTimersExclusive, 0, sizeof(S.AggregateTimersExclusive));
				memset(S.MaxTimersExclusive, 0, sizeof(S.MaxTimersExclusive));
				S.nFlipAggregate = 0;
				S.nFlipMax = 0;
				S.nAggregateFlipCount = 0;
			}
		},
		[](int nIndex)
		{
			S.nBars ^= (1 << nIndex);
		},
		[](int nIndex)
		{
			switch(Options[nIndex].nSubType)
			{
			case 0:
				S.fReferenceTime = g_MicroProfileReferenceTimePresets[Options[nIndex].nIndex];
				S.fRcpReferenceTime = 1.f / S.fReferenceTime;
				break;
			case 1:
				S.nOpacityBackground = g_MicroProfileOpacityPresets[Options[nIndex].nIndex]<<24;
				break;
			case 2:
				S.nOpacityForeground = g_MicroProfileOpacityPresets[Options[nIndex].nIndex]<<24;
				break;
			}
		},
		[](int nIndex)
		{
			int nNumPresets = sizeof(g_MicroProfilePresetNames) / sizeof(g_MicroProfilePresetNames[0]);
			int nIndexSave = nIndex - nNumPresets - 1;
			if(nIndexSave >= 0 && nIndexSave < nNumPresets)
			{
				MicroProfileSavePreset(g_MicroProfilePresetNames[nIndexSave]);
			}
			else if(nIndex >= 0 && nIndex < nNumPresets)
			{
				MicroProfileLoadPreset(g_MicroProfilePresetNames[nIndex]);
			}
		},
		[](int nIndex)
		{
		},
		[](int nIndex)
		{
		},
		[](int nIndex)
		{
		},
	};

	uint32_t nSelectMenu = (uint32_t)-1;
	for(uint32_t i = 0; i < nNumMenuItems; ++i)
	{
		nMenuX[i] = nX;
		uint32_t nLen = (uint32_t)strlen(pMenuText[i]);
		uint32_t nEnd = nX + nLen * (MICROPROFILE_TEXT_WIDTH+1);
		if(S.nMouseY <= MICROPROFILE_TEXT_HEIGHT && S.nMouseX <= nEnd && S.nMouseX >= nX)
		{
			MicroProfileDrawBox(nX-1, nY, nX + nLen * (MICROPROFILE_TEXT_WIDTH+1), nY +(S.nBarHeight+1)+1, 0xff888888);
			nSelectMenu = i;
			if((S.nMouseLeft || S.nMouseRight) && i == (int)nPauseIndex)
			{
				S.nRunning = !S.nRunning;
			}
		}
		MicroProfileDrawText(nX, nY, (uint32_t)-1, pMenuText[i]);
		nX += (nLen+1) * (MICROPROFILE_TEXT_WIDTH+1);
	}
	uint32_t nMenu = nSelectMenu != (uint32_t)-1 ? nSelectMenu : S.nActiveMenu;
	S.nActiveMenu = nMenu;
	if((uint32_t)-1 != nMenu)
	{
		nX = nMenuX[nMenu];
		nY += MICROPROFILE_TEXT_HEIGHT+1;
		SubmenuCallback CB = GroupCallback[nMenu];
		int nNumLines = 0;
		bool bSelected = false;
		const char* pString = CB(nNumLines, bSelected);
		uint32_t nWidth = 0, nHeight = 0;
		while(pString)
		{
			nWidth = MicroProfileMax<int>(nWidth, (int)strlen(pString));
			nNumLines++;
			pString = CB(nNumLines, bSelected);
		}
		nWidth = (2+nWidth) * (MICROPROFILE_TEXT_WIDTH+1);
		nHeight = nNumLines * (MICROPROFILE_TEXT_HEIGHT+1);
		if(S.nMouseY <= nY + nHeight+0 && S.nMouseY >= nY-0 && S.nMouseX <= nX + nWidth + 0 && S.nMouseX >= nX - 0)
		{
			S.nActiveMenu = nMenu;
		}
		else if(nSelectMenu == (uint32_t)-1)
		{
			S.nActiveMenu = (uint32_t)-1;
		}
		MicroProfileDrawBox(nX, nY, nX + nWidth, nY + nHeight, 0xff000000|g_nMicroProfileBackColors[1]);
		for(int i = 0; i < nNumLines; ++i)
		{
			bool bSelected = false;
			const char* pString = CB(i, bSelected);
			if(S.nMouseY >= nY && S.nMouseY < nY + MICROPROFILE_TEXT_HEIGHT + 1)
			{
				bMouseOver = true;
				if(S.nMouseLeft || S.nMouseRight)
				{
					CBClick[nMenu](i);
				}
				MicroProfileDrawBox(nX, nY, nX + nWidth, nY + MICROPROFILE_TEXT_HEIGHT + 1, 0xff888888);
			}
			snprintf(buffer, SBUF_SIZE-1, "%c %s", bSelected ? '*' : ' ' ,pString);
			MicroProfileDrawText(nX, nY, (uint32_t)-1, buffer);
			nY += MICROPROFILE_TEXT_HEIGHT+1;
		}
	}


	{
		static char FrameTimeMessage[64];
		float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
		uint32_t nAggregateFrames = S.nAggregateFrames ? S.nAggregateFrames : 1;
		float fMs = fToMs * (S.nFlipTicks);
		float fAverageMs = fToMs * (S.nFlipAggregateDisplay / nAggregateFrames);
		float fMaxMs = fToMs * S.nFlipMaxDisplay;
		int nLen = snprintf(FrameTimeMessage, sizeof(FrameTimeMessage)-1, "Time[%6.2f] Avg[%6.2f] Max[%6.2f]", fMs, fAverageMs, fMaxMs);
		pMenuText[nNumMenuItems++] = &FrameTimeMessage[0];
		MicroProfileDrawText(nWidth - nLen * (MICROPROFILE_TEXT_WIDTH+1), 0, -1, FrameTimeMessage);
	}
}


void MicroProfileMoveGraph()
{
	int nZoom = S.nMouseWheelDelta;
	int nPanX = 0;
	int nPanY = 0;
	static int X = 0, Y = 0;
	if(S.nMouseDownLeft && !S.nModDown)
	{
		nPanX = S.nMouseX - X;
		nPanY = S.nMouseY - Y;
	}
	X = S.nMouseX;
	Y = S.nMouseY;

	if(nZoom)
	{
		float fOldRange = S.fDetailedRange;
		if(nZoom>0)
		{
			S.fDetailedRangeTarget = S.fDetailedRange *= S.nModDown ? 1.40 : 1.05f;
		}
		else
		{
			S.fDetailedRangeTarget = S.fDetailedRange /= S.nModDown ? 1.40 : 1.05f;
		}

		float fDiff = fOldRange - S.fDetailedRange;
		float fMousePrc = MicroProfileMax((float)S.nMouseX / S.nWidth ,0.f);
		S.fDetailedOffsetTarget = S.fDetailedOffset += fDiff * fMousePrc;

	}
	if(nPanX)
	{
		S.fDetailedOffsetTarget = S.fDetailedOffset += -nPanX * S.fDetailedRange / S.nWidth;
	}
	S.nOffsetY -= nPanY;
	if(S.nOffsetY<0)
		S.nOffsetY = 0;
}

bool MicroProfileIsDrawing()
{
	return S.nDisplay != 0;
}
void MicroProfileDraw(uint32_t nWidth, uint32_t nHeight)
{
	MICROPROFILE_SCOPE(g_MicroProfileDraw);

	if(S.nDisplay)
	{
		MicroProfileScopeLock L(MicroProfileMutex());
		S.nWidth = nWidth;
		S.nHeight = nHeight;
		S.nHoverToken = MICROPROFILE_INVALID_TOKEN;
		S.nHoverTime = 0;
		S.nHoverFrame = -1;

		MicroProfileMoveGraph();


		if(S.nDisplay & MP_DRAW_DETAILED)
		{
			MicroProfileDrawDetailedView(nWidth, nHeight);
		}
		else if(0 != (S.nDisplay & MP_DRAW_BARS) && S.nBars)
		{
			MicroProfileDrawBarView(nWidth, nHeight);
		}

		MicroProfileDrawMenu(nWidth, nHeight);
		bool bMouseOverGraph = MicroProfileDrawGraph(nWidth, nHeight);

		uint32_t nLockedToolTipX = 3;
		bool bDeleted = false;
		for(int i = 0; i < MICROPROFILE_TOOLTIP_MAX_LOCKED; ++i)
		{
			int nIndex = (S.LockedToolTipFront + i) % MICROPROFILE_TOOLTIP_MAX_LOCKED;
			if(S.LockedToolTips[nIndex].ppStrings[0])
			{
				uint32_t nToolTipWidth = 0, nToolTipHeight = 0;
				MicroProfileFloatWindowSize(S.LockedToolTips[nIndex].ppStrings, S.LockedToolTips[nIndex].nNumStrings, 0, nToolTipWidth, nToolTipHeight, 0);
				uint32_t nStartY = nHeight - nToolTipHeight - 2;
				if(!bDeleted && S.nMouseY > nStartY && S.nMouseX > nLockedToolTipX && S.nMouseX <= nLockedToolTipX + nToolTipWidth && (S.nMouseLeft || S.nMouseRight) )
				{
					bDeleted = true;
					int j = i;
					for(; j < MICROPROFILE_TOOLTIP_MAX_LOCKED-1; ++j)
					{
						int nIndex0 = (S.LockedToolTipFront + j) % MICROPROFILE_TOOLTIP_MAX_LOCKED;
						int nIndex1 = (S.LockedToolTipFront + j+1) % MICROPROFILE_TOOLTIP_MAX_LOCKED;
						MicroProfileStringArrayCopy(&S.LockedToolTips[nIndex0], &S.LockedToolTips[nIndex1]);
					}
					MicroProfileStringArrayClear(&S.LockedToolTips[(S.LockedToolTipFront + j) % MICROPROFILE_TOOLTIP_MAX_LOCKED]);
				}
				else
				{
					MicroProfileDrawFloatWindow(nLockedToolTipX, nHeight-nToolTipHeight-2, &S.LockedToolTips[nIndex].ppStrings[0], S.LockedToolTips[nIndex].nNumStrings, S.nLockedToolTipColor[nIndex]);
					nLockedToolTipX += nToolTipWidth + 4;
				}
			}
		}

		if(S.nActiveMenu == 7)
		{
			if(S.nDisplay & MP_DRAW_DETAILED)
			{
				MicroProfileStringArray DetailedHelp;
				MicroProfileStringArrayClear(&DetailedHelp);
				MicroProfileStringArrayFormat(&DetailedHelp, "%s", MICROPROFILE_HELP_LEFT);
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Toggle Graph");
				MicroProfileStringArrayFormat(&DetailedHelp, "%s", MICROPROFILE_HELP_ALT);
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Zoom");
				MicroProfileStringArrayFormat(&DetailedHelp, "%s + %s", MICROPROFILE_HELP_MOD, MICROPROFILE_HELP_LEFT);
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Lock Tooltip");
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Drag");
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Pan View");
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Mouse Wheel");
				MicroProfileStringArrayAddLiteral(&DetailedHelp, "Zoom");
				MicroProfileDrawFloatWindow(nWidth, MICROPROFILE_FRAME_HISTORY_HEIGHT+20, DetailedHelp.ppStrings, DetailedHelp.nNumStrings, 0xff777777);

				MicroProfileStringArray DetailedHistoryHelp;
				MicroProfileStringArrayClear(&DetailedHistoryHelp);
				MicroProfileStringArrayFormat(&DetailedHistoryHelp, "%s", MICROPROFILE_HELP_LEFT);
				MicroProfileStringArrayAddLiteral(&DetailedHistoryHelp, "Center View");
				MicroProfileStringArrayFormat(&DetailedHistoryHelp, "%s", MICROPROFILE_HELP_ALT);
				MicroProfileStringArrayAddLiteral(&DetailedHistoryHelp, "Zoom to frame");
				MicroProfileDrawFloatWindow(nWidth, 20, DetailedHistoryHelp.ppStrings, DetailedHistoryHelp.nNumStrings, 0xff777777);



			}
			else if(0 != (S.nDisplay & MP_DRAW_BARS) && S.nBars)
			{
				MicroProfileStringArray BarHelp;
				MicroProfileStringArrayClear(&BarHelp);
				MicroProfileStringArrayFormat(&BarHelp, "%s", MICROPROFILE_HELP_LEFT);
				MicroProfileStringArrayAddLiteral(&BarHelp, "Toggle Graph");
				MicroProfileStringArrayFormat(&BarHelp, "%s + %s", MICROPROFILE_HELP_MOD, MICROPROFILE_HELP_LEFT);
				MicroProfileStringArrayAddLiteral(&BarHelp, "Lock Tooltip");
				MicroProfileStringArrayAddLiteral(&BarHelp, "Drag");
				MicroProfileStringArrayAddLiteral(&BarHelp, "Pan View");
				MicroProfileDrawFloatWindow(nWidth, MICROPROFILE_FRAME_HISTORY_HEIGHT+20, BarHelp.ppStrings, BarHelp.nNumStrings, 0xff777777);

			}
			MicroProfileStringArray Debug;
			MicroProfileStringArrayClear(&Debug);
			MicroProfileStringArrayAddLiteral(&Debug, "Memory Usage");
			MicroProfileStringArrayFormat(&Debug, "%4.2fmb", S.nMemUsage / (1024.f * 1024.f));
			uint32_t nFrameNext = (S.nFrameCurrent+1) % MICROPROFILE_MAX_FRAME_HISTORY;
			MicroProfileFrameState* pFrameCurrent = &S.Frames[S.nFrameCurrent];
			MicroProfileFrameState* pFrameNext = &S.Frames[nFrameNext];


			MicroProfileStringArrayAddLiteral(&Debug, "");
			MicroProfileStringArrayAddLiteral(&Debug, "");
			MicroProfileStringArrayAddLiteral(&Debug, "Usage");
			MicroProfileStringArrayAddLiteral(&Debug, "markers [frames] ");
			for(int i = 0; i < MICROPROFILE_MAX_GROUPS; ++i)
			{
				if(pFrameCurrent->nLogStart[i] && S.Pool[i])
				{
					uint32_t nEnd = pFrameNext->nLogStart[i];
					uint32_t nStart = pFrameCurrent->nLogStart[i];
					uint32_t nUsage = nStart < nEnd ? (nEnd - nStart) : (nEnd + MICROPROFILE_BUFFER_SIZE - nStart);
					uint32_t nFrameSupport = MICROPROFILE_BUFFER_SIZE / nUsage;
					MicroProfileStringArrayFormat(&Debug, "%s", &S.Pool[i]->ThreadName[0]);
					MicroProfileStringArrayFormat(&Debug, "%9d [%7d]", nUsage, nFrameSupport);
				}
			}

			MicroProfileDrawFloatWindow(0, nHeight-10, Debug.ppStrings, Debug.nNumStrings, 0xff777777);
		}



		if(S.nActiveMenu == -1 && !bMouseOverGraph)
		{
			if(S.nHoverToken != MICROPROFILE_INVALID_TOKEN)
			{
				MicroProfileDrawFloatTooltip(S.nMouseX, S.nMouseY, S.nHoverToken, S.nHoverTime);
			}
			else if(S.nHoverFrame != -1)
			{
				uint32_t nNextFrame = (S.nHoverFrame+1)%MICROPROFILE_MAX_FRAME_HISTORY;
				int64_t nTick = S.Frames[S.nHoverFrame].nFrameStartCpu;
				int64_t nTickNext = S.Frames[nNextFrame].nFrameStartCpu;
				int64_t nTickGpu = S.Frames[S.nHoverFrame].nFrameStartGpu;
				int64_t nTickNextGpu = S.Frames[nNextFrame].nFrameStartGpu;

				float fToMs = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondCpu());
				float fToMsGpu = MicroProfileTickToMsMultiplier(MicroProfileTicksPerSecondGpu());
				float fMs = fToMs * (nTickNext - nTick);
				float fMsGpu = fToMsGpu * (nTickNextGpu - nTickGpu);
				MicroProfileStringArray ToolTip;
				MicroProfileStringArrayClear(&ToolTip);
				MicroProfileStringArrayFormat(&ToolTip, "Frame %d", S.nHoverFrame);
#if MICROPROFILE_DEBUG
				MicroProfileStringArrayFormat(&ToolTip, "%p", &S.Frames[S.nHoverFrame]);
#else
				MicroProfileStringArrayAddLiteral(&ToolTip, "");
#endif
				MicroProfileStringArrayAddLiteral(&ToolTip, "CPU Time");
				MicroProfileStringArrayFormat(&ToolTip, "%6.2fms", fMs);
				MicroProfileStringArrayAddLiteral(&ToolTip, "GPU Time");
				MicroProfileStringArrayFormat(&ToolTip, "%6.2fms", fMsGpu);
				#if MICROPROFILE_DEBUG
				for(int i = 0; i < MICROPROFILE_MAX_GROUPS; ++i)
				{
					if(S.Frames[S.nHoverFrame].nLogStart[i])
					{
						MicroProfileStringArrayFormat(&ToolTip, "%d", i);
						MicroProfileStringArrayFormat(&ToolTip, "%d", S.Frames[S.nHoverFrame].nLogStart[i]);
					}
				}
				#endif
				MicroProfileDrawFloatWindow(S.nMouseX, S.nMouseY+20, &ToolTip.ppStrings[0], ToolTip.nNumStrings, -1);
			}
			if(S.nMouseLeft)
			{
				if(S.nHoverToken != MICROPROFILE_INVALID_TOKEN)
					MicroProfileToggleGraph(S.nHoverToken);
			}
		}
#if MICROPROFILE_DRAWCURSOR
		float fCursor[8] = 
		{
			MicroProfileMax(0, (int)S.nMouseX-3), S.nMouseY,
			MicroProfileMin(nWidth, S.nMouseX+3), S.nMouseY,
			S.nMouseX, MicroProfileMax((int)S.nMouseY-3, 0),
			S.nMouseX, MicroProfileMin(nHeight, S.nMouseY+3),

		};
		MicroProfileDrawLine2D(2, &fCursor[0], 0xff00ff00);
		MicroProfileDrawLine2D(2, &fCursor[4], 0xff00ff00);
#endif

	}
	S.nMouseLeft = S.nMouseRight = 0;
	S.nMouseLeftMod = S.nMouseRightMod = 0;
	S.nMouseWheelDelta = 0;
	if(S.nOverflow)
		S.nOverflow--;

}
void MicroProfileMousePosition(uint32_t nX, uint32_t nY, int nWheelDelta)
{
	S.nMouseX = nX;
	S.nMouseY = nY;
	S.nMouseWheelDelta = nWheelDelta;
}

void MicroProfileModKey(uint32_t nKeyState)
{
	S.nModDown = nKeyState ? 1 : 0;
}

void MicroProfileClearGraph()
{
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
	{
		if(S.Graph[i].nToken != 0)
		{
			S.Graph[i].nToken = MICROPROFILE_INVALID_TOKEN;
		}
	}
}
void MicroProfileTogglePause()
{
	S.nRunning = !S.nRunning;
}

void MicroProfileGetState(MicroProfileState* pStateOut)
{
	pStateOut->nDisplay = S.nDisplay;
	pStateOut->nMenuAllGroups = S.nMenuAllGroups;
	pStateOut->nMenuActiveGroup = S.nMenuActiveGroup;
	pStateOut->nMenuAllThreads = S.nMenuAllThreads;
	pStateOut->nAggregateFlip = S.nAggregateFlip;
	pStateOut->nBars = S.nBars;
	pStateOut->fReferenceTime = S.fReferenceTime;
}

void MicroProfileSetState(MicroProfileState* pStateOut)
{
	MicroProfileScopeLock L(MicroProfileMutex());
	S.nDisplay = pStateOut->nDisplay;
	S.nMenuAllGroups = pStateOut->nMenuAllGroups;
	S.nMenuActiveGroup = pStateOut->nMenuActiveGroup;
	S.nMenuAllThreads = pStateOut->nMenuAllThreads;
	S.nAggregateFlip = pStateOut->nAggregateFlip;
	S.nBars = pStateOut->nBars;
	S.fReferenceTime = pStateOut->fReferenceTime;
	S.fRcpReferenceTime = 1.f / S.fReferenceTime;
}

void MicroProfileToggleGraph(MicroProfileToken nToken)
{
	nToken &= 0xffff;
	int32_t nMinSort = 0x7fffffff;
	int32_t nFreeIndex = -1;
	int32_t nMinIndex = 0;
	int32_t nMaxSort = 0x80000000;
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
	{
		if(S.Graph[i].nToken == MICROPROFILE_INVALID_TOKEN)
			nFreeIndex = i;
		if(S.Graph[i].nToken == nToken)
		{
			S.Graph[i].nToken = MICROPROFILE_INVALID_TOKEN;
			return;
		}
		if(S.Graph[i].nKey < nMinSort)
		{
			nMinSort = S.Graph[i].nKey;
			nMinIndex = i;
		}
		if(S.Graph[i].nKey > nMaxSort)
		{
			nMaxSort = S.Graph[i].nKey;
		}
	}
	int nIndex = nFreeIndex > -1 ? nFreeIndex : nMinIndex;
	S.Graph[nIndex].nToken = nToken;
	S.Graph[nIndex].nKey = nMaxSort+1;
	memset(&S.Graph[nIndex].nHistory[0], 0, sizeof(S.Graph[nIndex].nHistory));
}
void MicroProfileMouseButton(uint32_t nLeft, uint32_t nRight)
{
	if(0 == nLeft && S.nMouseDownLeft)
	{
		if(S.nModDown)
			S.nMouseLeftMod = 1;
		else
			S.nMouseLeft = 1;
	}

	if(0 == nRight && S.nMouseDownRight)
	{
		if(S.nModDown)
			S.nMouseRightMod = 1;
		else
			S.nMouseRight = 1;
	}

	S.nMouseDownLeft = nLeft;
	S.nMouseDownRight = nRight;
	
}

#include <stdio.h>

#define MICROPROFILE_PRESET_HEADER_MAGIC 0x28586813
#define MICROPROFILE_PRESET_HEADER_VERSION 0x00000100
struct MicroProfilePresetHeader
{
	uint32_t nMagic;
	uint32_t nVersion;
	//groups, threads, aggregate, reference frame, graphs timers
	uint32_t nGroups[MICROPROFILE_MAX_GROUPS];
	uint32_t nThreads[MICROPROFILE_MAX_THREADS];
	uint32_t nGraphName[MICROPROFILE_MAX_GRAPHS];
	uint32_t nGraphGroupName[MICROPROFILE_MAX_GRAPHS];
	uint32_t nMenuAllGroups;
	uint32_t nMenuAllThreads;
	uint32_t nAggregateFlip;
	float fReferenceTime;
	uint32_t nBars;
	uint32_t nDisplay;
	uint32_t nOpacityBackground;
	uint32_t nOpacityForeground;
};

#ifndef MICROPROFILE_PRESET_FILENAME_FUNC
#define MICROPROFILE_PRESET_FILENAME_FUNC MicroProfilePresetFilename
static const char* MicroProfilePresetFilename(const char* pSuffix)
{
	static char filename[512];
	snprintf(filename, sizeof(filename)-1, ".microprofilepreset.%s", pSuffix);
	return filename;
}
#endif

void MicroProfileSavePreset(const char* pPresetName)
{
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());
	FILE* F = fopen(MICROPROFILE_PRESET_FILENAME_FUNC(pPresetName), "w");
	if(!F) return;

	MicroProfilePresetHeader Header;
	memset(&Header, 0, sizeof(Header));
	Header.nAggregateFlip = S.nAggregateFlip;
	Header.nBars = S.nBars;
	Header.fReferenceTime = S.fReferenceTime;
	Header.nMenuAllGroups = S.nMenuAllGroups;
	Header.nMenuAllThreads = S.nMenuAllThreads;
	Header.nMagic = MICROPROFILE_PRESET_HEADER_MAGIC;
	Header.nVersion = MICROPROFILE_PRESET_HEADER_VERSION;
	Header.nDisplay = S.nDisplay;
	Header.nOpacityBackground = S.nOpacityBackground;
	Header.nOpacityForeground = S.nOpacityForeground;
	fwrite(&Header, sizeof(Header), 1, F);
	uint64_t nMask = 1;
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GROUPS; ++i)
	{
		if(S.nMenuActiveGroup & nMask)
		{
			uint32_t offset = ftell(F);
			const char* pName = S.GroupInfo[i].pName;
			int nLen = (int)strlen(pName)+1;
			fwrite(pName, nLen, 1, F);
			Header.nGroups[i] = offset;
		}
		nMask <<= 1;
	}
	for(uint32_t i = 0; i < MICROPROFILE_MAX_THREADS; ++i)
	{
		MicroProfileThreadLog* pLog = S.Pool[i];
		if(pLog && S.nThreadActive[i])
		{
			uint32_t nOffset = ftell(F);
			const char* pName = &pLog->ThreadName[0];
			int nLen = (int)strlen(pName)+1;
			fwrite(pName, nLen, 1, F);
			Header.nThreads[i] = nOffset;
		}
	}
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
	{
		MicroProfileToken nToken = S.Graph[i].nToken;
		if(nToken != MICROPROFILE_INVALID_TOKEN)
		{
			uint32_t nGroupIndex = MicroProfileGetGroupIndex(nToken);
			uint32_t nTimerIndex = MicroProfileGetTimerIndex(nToken);
			const char* pGroupName = S.GroupInfo[nGroupIndex].pName;
			const char* pTimerName = S.TimerInfo[nTimerIndex].pName;
			MP_ASSERT(pGroupName);
			MP_ASSERT(pTimerName);
			int nGroupLen = (int)strlen(pGroupName)+1;
			int nTimerLen = (int)strlen(pTimerName)+1;

			uint32_t nOffsetGroup = ftell(F);
			fwrite(pGroupName, nGroupLen, 1, F);
			uint32_t nOffsetTimer = ftell(F);
			fwrite(pTimerName, nTimerLen, 1, F);
			Header.nGraphName[i] = nOffsetTimer;
			Header.nGraphGroupName[i] = nOffsetGroup;
		}
	}
	fseek(F, 0, SEEK_SET);
	fwrite(&Header, sizeof(Header), 1, F);

	fclose(F);

}



void MicroProfileLoadPreset(const char* pSuffix)
{
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());
	FILE* F = fopen(MICROPROFILE_PRESET_FILENAME_FUNC(pSuffix), "r");
	if(!F)
	{
	 	return;
	}
	fseek(F, 0, SEEK_END);
	int nSize = ftell(F);
	char* const pBuffer = (char*)alloca(nSize);
	fseek(F, 0, SEEK_SET);
	int nRead = (int)fread(pBuffer, nSize, 1, F);
	fclose(F);
	if(1 != nRead)
		return;
	
	MicroProfilePresetHeader& Header = *(MicroProfilePresetHeader*)pBuffer;

	if(Header.nMagic != MICROPROFILE_PRESET_HEADER_MAGIC || Header.nVersion != MICROPROFILE_PRESET_HEADER_VERSION)
	{
		return;
	}

	S.nAggregateFlip = Header.nAggregateFlip;
	S.nBars = Header.nBars;
	S.fReferenceTime = Header.fReferenceTime;
	S.fRcpReferenceTime = 1.f / Header.fReferenceTime;
	S.nMenuAllGroups = Header.nMenuAllGroups;
	S.nMenuAllThreads = Header.nMenuAllThreads;
	S.nDisplay = Header.nDisplay;
	S.nMenuActiveGroup = 0;
	S.nOpacityBackground = Header.nOpacityBackground;
	S.nOpacityForeground = Header.nOpacityForeground;

	memset(&S.nThreadActive[0], 0, sizeof(S.nThreadActive));

	for(uint32_t i = 0; i < MICROPROFILE_MAX_GROUPS; ++i)
	{
		if(Header.nGroups[i])
		{
			const char* pGroupName = pBuffer + Header.nGroups[i];	
			for(uint32_t j = 0; j < MICROPROFILE_MAX_GROUPS; ++j)
			{
				if(S.GroupInfo[j].pName && 0 == MP_STRCASECMP(pGroupName, S.GroupInfo[j].pName))
				{
					S.nMenuActiveGroup |= (1ll << j);
				}
			}
		}
	}
	for(uint32_t i = 0; i < MICROPROFILE_MAX_THREADS; ++i)
	{
		if(Header.nThreads[i])
		{
			const char* pThreadName = pBuffer + Header.nThreads[i];
			for(uint32_t j = 0; j < MICROPROFILE_MAX_THREADS; ++j)
			{
				MicroProfileThreadLog* pLog = S.Pool[j];
				if(pLog && 0 == MP_STRCASECMP(pThreadName, &pLog->ThreadName[0]))
				{
					S.nThreadActive[j] = 1;
				}
			}
		}
	}
	for(uint32_t i = 0; i < MICROPROFILE_MAX_GRAPHS; ++i)
	{
		MicroProfileToken nPrevToken = S.Graph[i].nToken;
		S.Graph[i].nToken = MICROPROFILE_INVALID_TOKEN;
		if(Header.nGraphName[i] && Header.nGraphGroupName[i])
		{
			const char* pGraphName = pBuffer + Header.nGraphName[i];
			const char* pGraphGroupName = pBuffer + Header.nGraphGroupName[i];
			for(uint32_t j = 0; j < S.nTotalTimers; ++j)
			{
				uint64_t nGroupIndex = S.TimerInfo[j].nGroupIndex;
				if(0 == MP_STRCASECMP(pGraphName, S.TimerInfo[j].pName) && 0 == MP_STRCASECMP(pGraphGroupName, S.GroupInfo[nGroupIndex].pName))
				{
					MicroProfileToken nToken = MicroProfileMakeToken(1ll << nGroupIndex, (uint16_t)j);
					S.Graph[i].nToken = nToken;
					if(nToken != nPrevToken)
					{
						memset(&S.Graph[i].nHistory, 0, sizeof(S.Graph[i].nHistory));
					}
					break;
				}
			}
		}
	}
}

void MicroProfileDrawLineVertical(int nX, int nTop, int nBottom, uint32_t nColor)
{
	MicroProfileDrawBox(nX, nTop, nX + 1, nBottom, nColor);
}

void MicroProfileDrawLineHorizontal(int nLeft, int nRight, int nY, uint32_t nColor)
{
	MicroProfileDrawBox(nLeft, nY, nRight, nY + 1, nColor);
}

float MicroProfileGetTime(const char* pGroup, const char* pName)
{
	MicroProfileToken nToken = MicroProfileFindToken(pGroup, pName);
	if(nToken == MICROPROFILE_INVALID_TOKEN)
	{
		return 0.f;
	}
	uint32_t nTimerIndex = MicroProfileGetTimerIndex(nToken);
	uint32_t nGroupIndex = MicroProfileGetGroupIndex(nToken);
	float fToMs = MicroProfileTickToMsMultiplier(S.GroupInfo[nGroupIndex].Type == MicroProfileTokenTypeGpu ? MicroProfileTicksPerSecondGpu() : MicroProfileTicksPerSecondCpu());
	return S.Frame[nTimerIndex].nTicks * fToMs;
}
void MicroProfileForceEnableGroup(const char* pGroup, MicroProfileTokenType Type)
{
	MicroProfileInit();
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());
	uint16_t nGroup = MicroProfileGetGroup(pGroup, Type);
	S.nForceGroup |= (1ll << nGroup);
}

void MicroProfileForceDisableGroup(const char* pGroup, MicroProfileTokenType Type)
{
	MicroProfileInit();
	std::lock_guard<std::recursive_mutex> Lock(MicroProfileMutex());
	uint16_t nGroup = MicroProfileGetGroup(pGroup, Type);
	S.nForceGroup &= ~(1ll << nGroup);
}

#undef S

#ifdef _WIN32
#pragma warning(pop)
#endif
#endif
#endif


