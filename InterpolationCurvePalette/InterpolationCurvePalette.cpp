// InterpolationCurvePalette.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"

extern HMODULE g_module;
class InterpolationCurvePalette : public MMDPluginDLL3
{
public:
  InterpolationCurvePalette() {}

  const char* getPluginTitle() const override { return "MMDUtility_Interpolation_Curve_Palette"; }

  void start() override
  {
    MMDUtility* utility = dynamic_cast<MMDUtility*>(mmp::getDLL3Object("MMDUtility"));
    if ( utility == nullptr ) return;


    auto menu = utility->getUitilityMenu();
    auto ctrl = utility->getControl();

    {
      auto window = new control::MenuCheckBox(ctrl);
      window->command = [=](auto args)
      {
        window->reverseCheck();
        if ( window->isChecked() )
        {
          ShowWindow(window_handle_, SW_SHOW);
        }
        else
        {
          ShowWindow(window_handle_, SW_HIDE);
        }
      };
      menu->AppendChild(L"補間曲線パレット", window);
    }
    {
      auto window = new control::MenuCheckBox(ctrl);
      window->command = [=](auto args)
      {
        unsigned char* p = *(unsigned char**) 0x00007FF60F6145F8;
        p += 0x9E4D5;
        p[0] = 50;
        p[1] = 50;
        p[2] = 100;
        p[3] = 100;
        RECT rect;
        HWND hwnd = getHWND(), hWnd;
        hWnd = hwnd;
        GetClientRect(hwnd, &rect);
        rect.bottom -= 8;
        rect.bottom -= 127 - p[1] + 8;
        POINT pos = { p[0] + 8,rect.bottom };
        while ( true )
        {
          HWND next = ChildWindowFromPoint(hWnd, pos);
          if ( next == NULL || hWnd == next )
            break;
          hWnd = next;
        }

        PostMessage(hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pos.x, pos.y));
        Sleep(1000);
        PostMessage(hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(pos.x, pos.y));
        Sleep(1000);
        PostMessage(hWnd, WM_LBUTTONUP, 0, MAKELPARAM(pos.x, pos.y));
      };
      menu->AppendChild(L"テスト", window);
    }
    menu->AppendSeparator();
    auto mainDlgProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM) -> INT_PTR
    {
      static constexpr int button_id[] = {
        IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3, IDC_BUTTON4, IDC_BUTTON5,
        IDC_BUTTON6, IDC_BUTTON7, IDC_BUTTON8, IDC_BUTTON9, IDC_BUTTON10 };

      switch ( msg )
      {
      case WM_COMMAND:
      {
        int num = 0;
        for ( int i = 0; i < 10; i++ )
        {
          if ( button_id[i] == LOWORD(wparam) )
          {
            num = i;
            break;
          }
        }
        unsigned char* p = *(unsigned char**) 0x00007FF60F6145F8;
        p += 0x9E4D5;
        POINT next[] = { {0,10},{0,30} , {0,50}, {0,100}, {0,127},
        {10,0}, {30,0}, {50, 0}, {100,0}, {0,127} };
        RECT rect;
        HWND hwnd = getHWND();
        GetClientRect(hwnd, &rect);
        rect.bottom -= 8;
        for ( int j = 0; j < 2; j++ )
        {
          if ( j == 1 ) next[num].x = 127 - next[num].x;
          POINT pos = { next[num].x + 8, rect.bottom - (127 - next[num].y) };
          if ( j == 1 ) next[num].x = 127 - next[num].x;
          int y = rect.bottom - (127 - p[1 + j * 2]);
          {
            SendMessage(hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(p[0 + j * 2] + 8, y));
            SendMessage(hwnd, WM_MOUSEACTIVATE, (WPARAM) hwnd, 0x2010001);
            SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p[0 + j * 2] + 8, y));
            SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos.x, pos.y + 1));
            SendMessage(hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos.x, pos.y + 0));
            SendMessage(hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y + 0));
          }
        }
      }
      break;
      default:
        break;
      }
      return 0;
    };
    window_handle_ = CreateDialogParamW(g_module, MAKEINTRESOURCE(PALETTE_WINDOW_ID), getHWND(), mainDlgProc, (LPARAM)this);
  }

private:

  HWND window_handle_;
};


int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device) { return new InterpolationCurvePalette(); }
