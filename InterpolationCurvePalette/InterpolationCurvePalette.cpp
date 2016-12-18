// InterpolationCurvePalette.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"
#include <windowsx.h>
#include <iostream>
#undef max
#undef min
extern HMODULE g_module;

#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;

static filesystem::path getDLlParentPath()
{
  char module_path[MAX_PATH + 1];
  GetModuleFileNameA(g_module, module_path, MAX_PATH);
  return filesystem::path(module_path).parent_path();
}

template<class T>
void println(std::ostream& ofs, T t)
{
  ofs << t << std::endl;
}

template<class A, class...T>
void println(std::ostream& ofs, A a, T ... t)
{
  ofs << a << ' ';
  println(ofs, t...);
}


class IUndoRedo
{
  virtual IUndoRedo* next() = 0;
  virtual void load() = 0;
  virtual void save() = 0;
};

using namespace mmp;


class InterpolationCurvePalette : public MMDPluginDLL3
{
public:
  struct ICData
  {
    POINT xy1, xy2;
  };

  std::array<ICData, 10> ic_data_;
  bool use_shortcut_;

  InterpolationCurvePalette() {}

  const char* getPluginTitle() const override { return "MMDUtility_Interpolation_Curve_Palette"; }

  void useShortcut(bool use)
  {
    Button_SetCheck(GetDlgItem(window_handle_, IDC_CHECK1), use);
    use_shortcut_ = use;
  }

  void loadData()
  {
    std::ifstream ifs(getDLlParentPath() / "interpolationCuveData.txt");
    if ( ifs.is_open() == false )
    {
      ic_data_[0].xy1 = { 64, 127 };
      ic_data_[0].xy2 = { 64, 0, };
      ic_data_[1].xy1 = { 48, 117 };
      ic_data_[1].xy2 = { 80, 10 };
      ic_data_[2].xy1 = { 38, 107 };
      ic_data_[2].xy2 = { 90, 20 };
      ic_data_[3].xy1 = { 97, 127 };
      ic_data_[3].xy2 = { 30, 0 };
      ic_data_[4].xy1 = { 0, 64 };
      ic_data_[4].xy2 = { 64, 0 };

      ic_data_[5].xy1 = { 64, 127 };
      ic_data_[5].xy2 = { 127, 64 };
      ic_data_[6].xy1 = { 0, 30 };
      ic_data_[6].xy2 = { 127, 97 };
      ic_data_[7].xy1 = { 20, 89 };
      ic_data_[7].xy2 = { 107, 37 };
      ic_data_[8].xy1 = { 10, 79 };
      ic_data_[8].xy2 = { 117, 47 };
      ic_data_[9].xy1 = { 0, 64 };
      ic_data_[9].xy2 = { 127, 64 };

      useShortcut(true);
      return;
    }
    int version;
    ifs >> version;
    for ( auto& i : ic_data_ )
    {
      ifs >> i.xy1.x >> i.xy1.y >> i.xy2.x >> i.xy2.y;
    }
    int is_shortcut;
    ifs >> is_shortcut;
    useShortcut(is_shortcut != 0);
  }

  void saveData()
  {
    std::ofstream ofs(getDLlParentPath() / "interpolationCuveData.txt");
    println(ofs, "001");
    for ( auto& i : ic_data_ )
    {
      println(ofs, i.xy1.x, i.xy1.y, i.xy2.x, i.xy2.y);
    }
    println(ofs, use_shortcut_ ? 1 : 0);
  }


  static unsigned char* getMMDICPointer()
  {
    auto p = (unsigned char*) getMMDMainData();
    p += 0x9E4D5;
    if ( IsBadReadPtr(p, sizeof(char[4])) != 0 || IsBadWritePtr(p, sizeof(char[4])) != 0 )
    {
      std::wstring error = L"内部エラー\n補間曲線の読み込みに失敗しました\np=" + std::to_wstring((INT_PTR) p);
      MessageBoxW(nullptr, error.c_str(), L"エラー", MB_OK);
      return nullptr;
    }
    return p;
  }

  void stop() override
  {
    saveData();
  }

  void start() override
  {
    MMDUtility* utility = dynamic_cast<MMDUtility*>(mmp::getDLL3Object("MMDUtility"));
    if ( utility == nullptr ) return;


    auto menu = utility->getUitilityMenu();
    auto ctrl = utility->getControl();

    {
      palette_menu_ = new control::MenuCheckBox(ctrl);
      palette_menu_->command = [=](auto args)
        {
          palette_menu_->reverseCheck();
          if ( palette_menu_->isChecked() )
          {
            ShowWindow(window_handle_, SW_SHOW);
          }
          else
          {
            ShowWindow(window_handle_, SW_HIDE);
          }
        };
      menu->AppendChild(L"補間曲線パレット", palette_menu_);
    }
    menu->AppendSeparator();

    auto mainDlgProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> INT_PTR
      {
        static constexpr int button_id[] = {
          IDC_BUTTON1, IDC_BUTTON2, IDC_BUTTON3, IDC_BUTTON4, IDC_BUTTON5,
          IDC_BUTTON6, IDC_BUTTON7, IDC_BUTTON8, IDC_BUTTON9, IDC_BUTTON10 };
        static InterpolationCurvePalette* this_;
        if ( msg == WM_INITDIALOG ) this_ = (InterpolationCurvePalette*) lparam;
        if ( this_ == nullptr ) return 0;

        int num = 0;
        switch ( msg )
        {
        case WM_CLOSE:

          ShowWindow(hwnd, SW_HIDE);
          this_->palette_menu_->check(false);
          break;

        case WM_COMMAND:
        {
          if ( LOWORD(wparam) == IDC_CHECK1 )
          {
            this_->useShortcut(BST_CHECKED == Button_GetCheck(GetDlgItem(hwnd, IDC_CHECK1)));
            return 0;
          }
          if ( LOWORD(wparam) == IDC_BUTTON11 )
          {
            auto dlgProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> INT_PTR
              {
                static constexpr int edit_id[] = {
                  EDIT_RADIOBOX1 , EDIT_RADIOBOX2 , EDIT_RADIOBOX3 , EDIT_RADIOBOX4 , EDIT_RADIOBOX5 ,
                  EDIT_RADIOBOX6 , EDIT_RADIOBOX7 , EDIT_RADIOBOX8 , EDIT_RADIOBOX9 , EDIT_RADIOBOX10 ,
                };
                static InterpolationCurvePalette* this_;
                if ( msg == WM_INITDIALOG ) this_ = (InterpolationCurvePalette*) lparam;
                if ( this_ == nullptr ) return 0;

                auto mmd_data = getMMDMainData();

                static int id = -1;
                switch ( msg )
                {
                case WM_CLOSE:
                  EndDialog(hwnd, 0);
                  this_->recvIC(id);
                  id = -1;
                  break;
                case WM_COMMAND:
                  for ( int i = 0; i < 10; i++ )
                  {
                    if ( edit_id[i] == LOWORD(wparam) )
                    {
                      this_->recvIC(id);
                      id = i;
                      this_->sendIC(id);
                      break;
                    }
                  }
                default:
                  break;
                }
                return 0;
              };
            CreateDialogParamW(g_module, MAKEINTRESOURCE(IC_SETTING_WINDOW_ID), getHWND(), dlgProc, (LPARAM) this_);
            break;
          }
          for ( int i = 0; i < 10; i++ )
          {
            if ( button_id[i] == LOWORD(wparam) )
            {
              num = i;
              break;
            }
          }
          this_->sendIC(num);
        }
          break;
        default:
          break;
        }
        return 0;
      };
    window_handle_ = CreateDialogParamW(g_module, MAKEINTRESOURCE(PALETTE_WINDOW_ID), getHWND(), mainDlgProc, (LPARAM)this);
    loadData();
  }

  void sendIC(int num)
  {
    RECT rect;
    HWND mmd_hwnd = getHWND();
    GetClientRect(mmd_hwnd, &rect);
    rect.bottom -= 8;

    auto& data = ic_data_[num];
    std::array<POINT, 2> pos = {
      POINT{ data.xy1.x + 8, rect.bottom - (127 - data.xy1.y) },
      { data.xy2.x + 8, rect.bottom - (127 - data.xy2.y) } };

    auto p = getMMDICPointer();
    if ( p == nullptr ) return;
    p[2] = 127;
    p[3] = 127;
    for ( int j = 1; j >= 0; j-- )
    {
      p[0] = 0;
      p[1] = 0;
      int y = rect.bottom - (127 - p[1 + j * 2]);
      SendMessage(mmd_hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(p[0 + j * 2] + 8, y));
      SendMessage(mmd_hwnd, WM_MOUSEACTIVATE, (WPARAM) mmd_hwnd, 0x2010001);
      SendMessage(mmd_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p[0 + j * 2] + 8, y));
      SendMessage(mmd_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 1));
      SendMessage(mmd_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 0));
      SendMessage(mmd_hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 0));
    }
  }


  void recvIC(int id)
  {
    if ( id == -1 ) return;
    auto p = getMMDICPointer();
    if ( p == nullptr ) return;
    ic_data_[id].xy1.x = p[0];
    ic_data_[id].xy1.y = p[1];
    ic_data_[id].xy2.x = p[2];
    ic_data_[id].xy2.y = p[3];
  }

  void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
  {
    if ( use_shortcut_ == false ) return;
    if ( VK_F1 <= wParam && wParam <= VK_F9 )
    {
      if ( (lParam & 0x80000000) == 0 )
      {
        sendIC(wParam - VK_F1);
      }
    }
  }

private:
  control::MenuCheckBox* palette_menu_;
  HWND window_handle_;
};


int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
#ifndef NDEBUG
  OpenConsole();
#endif // !NDEBUG
  return new InterpolationCurvePalette();
}
