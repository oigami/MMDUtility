// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
HMODULE g_module;

void OpenConsole()
{
  FILE *in, *out;
  AllocConsole();
  freopen_s(&out, "CONOUT$", "w", stdout);//CONOUT$
  freopen_s(&in, "CONIN$", "r", stdin);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
  switch ( ul_reason_for_call )
  {
  case DLL_PROCESS_ATTACH:
#ifndef NDEBUG
    OpenConsole();
#endif // !NDEBUG
    g_module = hModule;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
