// InterpolationCurvePalette.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "resource.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"
#include <windowsx.h>
extern HMODULE g_module;


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
    std::ifstream ifs("interpolationCuveData.txt");
    if ( ifs.is_open() == false )
    {
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
    std::ofstream ofs("interpolationCuveData.txt");
    println(ofs, "001");
    for ( auto& i : ic_data_ )
    {
      println(ofs, i.xy1.x, i.xy1.y, i.xy2.x, i.xy2.y);
    }
  }

  static unsigned char* getMMDICPointer()
  {
    auto pointer = ((BYTE*) GetModuleHandleW(nullptr) + 0x1445F8);
    if ( IsBadReadPtr(pointer, sizeof(INT_PTR)) != 0 )
    {
      std::wstring error = L"内部エラー\nポインタの読み込みに失敗しました\npointer=" + std::to_wstring((INT_PTR) pointer);
      MessageBoxW(nullptr, error.c_str(), L"エラー", MB_OK);
      return nullptr;
    }
    auto p = *(unsigned char**) pointer;
    p += 0x9E4D5;
    if ( IsBadReadPtr(pointer, sizeof(char[4])) != 0 )
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

                static int id = 0;
                switch ( msg )
                {
                case WM_CLOSE:
                  EndDialog(hwnd, 0);
                  break;
                case WM_COMMAND:
                  if ( LOWORD(wparam) == BUTTON_EXIT )
                  {
                    EndDialog(hwnd, 0);
                    break;
                  }
                  if ( LOWORD(wparam==BUTTON_SET) )
                  {
                    auto p = getMMDICPointer();
                    if ( p == nullptr ) return 0;
                    this_->ic_data_[id].xy1.x = p[0];
                    this_->ic_data_[id].xy1.y = p[1];
                    this_->ic_data_[id].xy2.x = p[2];
                    this_->ic_data_[id].xy2.y = p[3];
                    break;
                  }
                  for ( int i = 0; i < 10; i++ )
                  {
                    if ( edit_id[i] == LOWORD(wparam) )
                    {
                      id = i;
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
        case WM_HOTKEY:
          if ( WM_HOTKEY == msg )
          {
            num = 0xc000 ^ wparam;
            if ( this_->use_shortcut_ == false ) return 0;
          }


          RECT rect;
          HWND mmd_hwnd = getHWND();
          GetClientRect(mmd_hwnd, &rect);
          rect.bottom -= 8;

          const auto& data = this_->ic_data_[num];
          std::array<POINT, 2> pos = {
            POINT{ data.xy1.x + 8, rect.bottom - (127 - data.xy1.y) },
            { data.xy2.x + 8, rect.bottom - (127 - data.xy2.y) } };

          auto p = getMMDICPointer();
          if ( p == nullptr ) return 0;

          for ( int j = 0; j < 2; j++ )
          {
            int y = rect.bottom - (127 - p[1 + j * 2]);
            SendMessage(mmd_hwnd, WM_MOUSEMOVE, 0, MAKELPARAM(p[0 + j * 2] + 8, y));
            SendMessage(mmd_hwnd, WM_MOUSEACTIVATE, (WPARAM) mmd_hwnd, 0x2010001);
            SendMessage(mmd_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(p[0 + j * 2] + 8, y));
            SendMessage(mmd_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 1));
            SendMessage(mmd_hwnd, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 0));
            SendMessage(mmd_hwnd, WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos[j].x, pos[j].y + 0));
          }
        }
          break;
        default:
          break;
        }
        return 0;
      };
    window_handle_ = CreateDialogParamW(g_module, MAKEINTRESOURCE(PALETTE_WINDOW_ID), getHWND(), mainDlgProc, (LPARAM)this);

    for ( int i = 1; i < 10; i++ )
    {
      RegisterHotKey(window_handle_, 0xc000 | (i - 1), 0, '0' + i);
    }
    RegisterHotKey(window_handle_, 0xc000 | 9, 0, '0');

    loadData();
  }

private:
  control::MenuCheckBox* palette_menu_;
  HWND window_handle_;
};


int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device) { return new InterpolationCurvePalette(); }
