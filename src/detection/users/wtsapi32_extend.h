#pragma once

//at the time of writing, <wtsapi32.h> of MinGW doesn't have the definition of WTSEnumerateSessionsExW and friends

#if (_WIN32_WINNT >= 0x0601)

typedef struct _WTS_SESSION_INFO_1A {
  DWORD ExecEnvId;
  WTS_CONNECTSTATE_CLASS State;
  DWORD SessionId;
  LPSTR pSessionName;
  LPSTR pHostName;
  LPSTR pUserName;
  LPSTR pDomainName;
  LPSTR pFarmName;
} WTS_SESSION_INFO_1A, *PWTS_SESSION_INFO_1A;

typedef struct _WTS_SESSION_INFO_1W {
  DWORD ExecEnvId;
  WTS_CONNECTSTATE_CLASS State;
  DWORD SessionId;
  LPWSTR pSessionName;
  LPWSTR pHostName;
  LPWSTR pUserName;
  LPWSTR pDomainName;
  LPWSTR pFarmName;
} WTS_SESSION_INFO_1W, * PWTS_SESSION_INFO_1W;

#define WTS_SESSION_INFO_1 __MINGW_NAME_AW(WTS_SESSION_INFO_1)
#define PWTS_SESSION_INFO_1 __MINGW_NAME_AW(PWTS_SESSION_INFO_1)

WINBOOL WINAPI WTSEnumerateSessionsExA(HANDLE hServer,DWORD* pLevel,DWORD Filter,PWTS_SESSION_INFO_1A* ppSessionInfo,DWORD* pCount);
WINBOOL WINAPI WTSEnumerateSessionsExW(HANDLE hServer,DWORD* pLevel,DWORD Filter,PWTS_SESSION_INFO_1W* ppSessionInfo,DWORD* pCount);
#define WTSEnumerateSessionsEx __MINGW_NAME_AW(WTSEnumerateSessionsEx)

typedef enum _WTS_TYPE_CLASS {
  WTSTypeProcessInfoLevel0,
  WTSTypeProcessInfoLevel1,
  WTSTypeSessionInfoLevel1
} WTS_TYPE_CLASS;
BOOL WTSFreeMemoryExA(WTS_TYPE_CLASS WTSTypeClass,PVOID pMemory,ULONG NumberOfEntries);
BOOL WTSFreeMemoryExW(WTS_TYPE_CLASS WTSTypeClass,PVOID pMemory,ULONG NumberOfEntries);
#define WTSFreeMemoryEx __MINGW_NAME_AW(WTSFreeMemoryEx)

#endif /*(_WIN32_WINNT >= 0x0601)*/
