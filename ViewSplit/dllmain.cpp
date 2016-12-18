// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include <d3dx9.h>
#include "../MMDUtility/mmd_utility.h"
HMODULE g_module;

mmp::WinAPIHooker<decltype(D3DXCreateEffectFromFileA)*> a;

HRESULT WINAPI
MyD3DXCreateEffectFromFileA(
  LPDIRECT3DDEVICE9 pDevice,
  LPCSTR pSrcFile,
  CONST D3DXMACRO* pDefines,
  LPD3DXINCLUDE pInclude,
  DWORD Flags,
  LPD3DXEFFECTPOOL pPool,
  LPD3DXEFFECT* ppEffect,
  LPD3DXBUFFER* ppCompilationErrors)
{
  return a(pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
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

    a.hook("d3dx9_43.dll", "D3DXCreateEffectFromFileA", MyD3DXCreateEffectFromFileA);
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
