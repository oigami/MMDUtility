// AutoSave.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"

class AutoSave:public MMDPluginDLL3
{
public:
  const char* getPluginTitle() const override { return "MMDUtlity_AutoSave"; }
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
#ifndef NDEBUG
  OpenConsole();
#endif // !NDEBUG
  return new AutoSave();
}
