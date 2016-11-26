// MMDUtility.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"


#include <mmd_plugin.h>

class MMDUtility : public MMDPluginDLL3
{
public:
  MMDUtility(IDirect3DDevice9* device) : deivce_(device) { }

  void WndProc(const CWPSTRUCT* param) override
  {
    if ( WM_COMMAND == param->message )
    {
      printf("WM_COMMAND %x\n", LOWORD(param->wParam));
    }
    printf("%d %d %d\n", param->message, param->lParam, param->wParam);
  }

  std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override { return { false,0 }; }

  IDirect3DDevice9* deivce_;
};

MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
  return new MMDUtility(device);
}

void destroy3(MMDPluginDLL3* p)
{
  delete p;
}
