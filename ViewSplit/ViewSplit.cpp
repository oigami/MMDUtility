// ViewSplit.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//
#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd/MMDExport.h"
#include "../MMDUtility/mmd_utility.h"
#include "resource.h"
#include <Commdlg.h>
extern HMODULE g_module;

typedef struct
{
  float x, y, z, rhw;
  DWORD diff;
} D3DVERTEX;

struct Rect
{
  int x, y, width, height;
  float scale = 1.0f;

  D3DVIEWPORT9 getViewport(float min_z, float max_z) const
  {
    D3DVIEWPORT9 v;
    v.MinZ = min_z;
    v.MaxZ = max_z;
    if ( x < 0 )
    {
      v.X = 0;
      v.Width = width + x;
    }
    else
    {
      v.X = x;
      v.Width = width;
    }
    if ( y < 0 )
    {
      v.Y = 0;
      v.Height = height + y;
    }
    else
    {
      v.Y = y;
      v.Height = height;
    }
    v.Height *= scale;
    v.Width *= scale;
    return v;
  }
};

struct ViewData
{
  bool is_use_;
  Rect view;
  int alpha_val = 100;

  struct Camera
  {
    DirectX::XMVECTOR lookat;
    DirectX::XMFLOAT2A rotate;
    float distance;
  } camera;

  enum class CameraType
  {
    Fix,
    ModelTracking,
  } camera_type;

  DirectX::XMVECTOR pre_position;

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

  void save(std::ofstream& ofs)
  {
    println(ofs, is_use_ ? "1" : "0");
    println(ofs, view.x, view.y, view.width, view.height, view.scale);
    println(ofs, alpha_val);

    println(ofs, (int) camera_type);
    DirectX::XMFLOAT3A lookat;
    DirectX::XMStoreFloat3A(&lookat, camera.lookat);
    println(ofs, lookat.x, lookat.y, lookat.z);
    println(ofs, camera.rotate.x, camera.rotate.y);
    println(ofs, camera.distance);
  }

  void load(std::ifstream& ifs)
  {
    int is_use;
    ifs >> is_use;
    is_use_ = is_use == 1;
    ifs >> view.x >> view.y >> view.width >> view.height >> view.scale;
    ifs >> alpha_val;
    ifs >> (int&) camera_type;
    DirectX::XMFLOAT3A lookat;
    ifs >> lookat.x >> lookat.y >> lookat.z;
    camera.lookat = DirectX::XMLoadFloat3A(&lookat);
    ifs >> camera.rotate.x >> camera.rotate.y >> camera.distance;
  }
};

class ViewSplit : public MMDPluginDLL3
{
public:
  explicit ViewSplit(IDirect3DDevice9* device): device_(device) {}

  bool is_split_ = false;
  bool is_camera_right_bottom = false;
  control::MenuCheckBox* check_use_menu;
  HWND dialog_hwnd;
  int model_selected_id = 0;

  std::array<ViewData, 4> data_;

  struct MoveMouseData
  {
    int x, y;

    enum class Type
    {
      ViewMove,
      ViewScale,
      CameraRotate,
      CameraXYMove,
      CameraDistanceMove,
      Unkown,
    } type = Type::Unkown;

    int id = -1;
  } mouse_data;

  static constexpr int is_use_id[] = { LU_IS_USE , RU_IS_USE,LB_IS_USE,RB_IS_USE };
  static constexpr int alpha_val[] = { LU_ALPHA, RU_ALPHA, LB_ALPHA2, RB_ALPHA };
  static constexpr int is_fix_id[] = { LU_IS_FIX, RU_IS_FIX, LB_IS_FIX, RB_IS_FIX };
  static constexpr int is_tracking_id[] = { LU_IS_TRACKING, RU_IS_TRACKING, LB_IS_TRACKING, RB_IS_TRACKING };
  static constexpr int model_select_id = 436;

  void ResetDefaultSetting(D3DVIEWPORT9 viewport)
  {
    for ( int i = 0; i < 4; i++ )
    {
      data_[i] = ViewData();
      data_[i].is_use_ = true;
      Button_SetCheck(GetDlgItem(dialog_hwnd, is_use_id[i]), TRUE);
    }
    for ( int i = 0; i < 4; i++ )
    {
      CheckRadioButton(dialog_hwnd, is_fix_id[i], is_tracking_id[i], is_fix_id[i]);
      data_[i].camera_type = ViewData::CameraType::Fix;
    }
    // ビューポートの設定
    data_[0].view.x = viewport.X;
    data_[0].view.y = viewport.Y;
    data_[0].view.width = viewport.Width;
    data_[0].view.height = viewport.Height;

    data_[1].view = data_[0].view;
    data_[1].view.x += data_[1].view.width;

    data_[2] = data_[0];
    data_[2].view.y += data_[0].view.height;

    data_[3] = data_[2];
    data_[3].view.x += data_[3].view.width;

    // カメラの設定
    data_[0].camera.distance = 45.0f;
    data_[0].camera.lookat = { 0,10,0 };
    for ( auto& i : data_ )
    {
      i.camera = data_[0].camera;
    }
    data_[1].camera.rotate.x += DirectX::XM_PIDIV2;
    data_[2].camera.rotate.x -= DirectX::XM_PIDIV2;
    data_[3].camera.rotate.y += DirectX::XM_PIDIV2;
    data_[3].camera.lookat = { 0.0f,0.0f,0.0f };
  }

  static std::wstring funcFileSave(HWND hWnd)
  {
    static OPENFILENAME ofn;
    static wchar_t szFile[MAX_PATH];

    std::wstring szPath;
    char module_path[MAX_PATH + 1];
    GetModuleFileNameA(g_module, module_path, MAX_PATH);
    szPath = (filesystem::path(module_path).parent_path() / L"split_view_setting").wstring();
    if ( ofn.lStructSize == 0 )
    {
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hwndOwner = hWnd;
      ofn.lpstrInitialDir = szPath.c_str(); // 初期フォルダ位置
      ofn.lpstrFile = szFile; // 選択ファイル格納
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrDefExt = TEXT(".txt");
      ofn.lpstrFilter = TEXT("txtファイル(*.txt)\0*.txt\0");
      ofn.lpstrTitle = TEXT("設定ファイルを保存します。");
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
    }
    if ( GetSaveFileName(&ofn) )
    {
      return szFile;
    }
    else return {};
  }

  static std::wstring funcPngSave(HWND hWnd)
  {
    static OPENFILENAME ofn;
    static wchar_t szFile[MAX_PATH];

    std::wstring szPath;
    char module_path[MAX_PATH + 1];
    GetModuleFileNameA(g_module, module_path, MAX_PATH);
    szPath = (filesystem::path(module_path).parent_path() / L"split_view_setting").wstring();
    if ( ofn.lStructSize == 0 )
    {
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hwndOwner = hWnd;
      ofn.lpstrInitialDir = szPath.c_str(); // 初期フォルダ位置
      ofn.lpstrFile = szFile; // 選択ファイル格納
      ofn.nMaxFile = MAX_PATH;
      ofn.lpstrDefExt = TEXT(".txt");
      ofn.lpstrFilter = TEXT("txtファイル(*.txt)\0*.txt\0");
      ofn.lpstrTitle = TEXT("設定ファイルを保存します。");
      ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
    }
    if ( GetSaveFileName(&ofn) )
    {
      return szFile;
    }
    else return {};
  }

  void UpdateSettingMenu()
  {
    char module_path[MAX_PATH];
    wchar_t stritem[MAX_PATH * 2];
    GetModuleFileNameA(g_module, module_path, MAX_PATH);
    auto path = filesystem::path(module_path).parent_path() / L"split_view_setting";
    if ( filesystem::exists(path) == false )
    {
      filesystem::create_directories(path);
    }
    filesystem::directory_iterator it(path), last;
    auto menu = GetDlgItem(dialog_hwnd, LOAD_MENU);
    int cnt = SendMessage(menu, CB_GETCOUNT, 0, 0);
    for ( int i = 0; i < cnt; i++ )
    {
      SendMessage(menu, CB_DELETESTRING, 0, 0);
    }
    SendMessage(menu, CB_ADDSTRING, 0, (LPARAM) L"デフォルト");
    for ( ; it != last; it++ )
    {
      SendMessage(menu, CB_ADDSTRING, 0, (LPARAM) it->path().filename().c_str());
    }
    std::ifstream ifs(path.string());
  }

  void start() override
  {
    auto utility_dll = mmp::getDLL3Object("MMDUtility");
    if ( utility_dll == nullptr ) return;
    auto utility = dynamic_cast<MMDUtility*>(utility_dll);
    if ( utility == nullptr ) return;

    auto menu = utility->getUitilityMenu();
    auto ctrl = utility->getControl();
    {
      check_use_menu = new control::MenuCheckBox(ctrl);
      check_use_menu->command = [this](const control::IMenu::CommandArgs&)
        {
          check_use_menu->reverseCheck();
          is_split_ = check_use_menu->isChecked();
          Button_SetCheck(GetDlgItem(dialog_hwnd,IS_USE_SPLIT_VIEW), is_split_);
        };

      menu->AppendChild(L"画面分割", check_use_menu);
    }

    {
      auto check_camera_pos = new control::MenuCheckBox(ctrl);
      check_camera_pos->command = [check_camera_pos, this](const control::IMenu::CommandArgs&)
        {
          check_camera_pos->reverseCheck();
          is_camera_right_bottom = check_camera_pos->isChecked();
        };
      menu->AppendChild(L"カメラを右下にする", check_camera_pos);
    }

    auto mainDlgProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lParam)
      {
        static ViewSplit* this_;
        static bool is_move = false;
        if ( msg == WM_INITDIALOG ) this_ = (ViewSplit*) lParam;
        if ( this_ == nullptr ) return INT_PTR();
        switch ( msg )
        {
        case WM_COMMAND:
        {
          if ( LOWORD(wparam) == PARAM_LOAD )
          {
            wchar_t str[1024] = {};
            auto menu = GetDlgItem(hwnd, LOAD_MENU);
            int select = SendMessage(menu, CB_GETCURSEL, 0L, 0L);
            if ( select <= 0 )
            {
              D3DVIEWPORT9 viewport = this_->back_viewport_;
              viewport.Width = viewport.Width / 2;
              viewport.Height = viewport.Height / 2;
              this_->ResetDefaultSetting(viewport);
            }
            else
            {
              SendMessage(menu, CB_GETLBTEXT, select, (LPARAM) str);
              char module_path[MAX_PATH + 1];
              GetModuleFileNameA(g_module, module_path, MAX_PATH);
              filesystem::path path = filesystem::path(module_path).parent_path() / L"split_view_setting";
              if ( filesystem::exists(path) == false )
              {
                filesystem::create_directories(path);
              }
              path /= str;
              std::ifstream ifs(path.string());
              int version;
              ifs >> version;
              for ( int i = 0; i < 4; i++ )
              {
                this_->data_[i].load(ifs);
              }
            }
          }
          else if ( LOWORD(wparam) == PARAM_SAVE )
          {
            auto str = funcFileSave(hwnd);
            filesystem::path path = str;
            std::ofstream ofs(path.string());
            ofs << "001" << std::endl;
            for ( int i = 0; i < 4; i++ )
            {
              this_->data_[i].save(ofs);
              ofs << std::endl;
            }

            auto menu = GetDlgItem(hwnd, LOAD_MENU);
            int select = SendMessage(menu, CB_GETCURSEL, 0L, 0L);
            SendMessage(menu, CB_GETLBTEXT, select, (LPARAM) str.c_str());
            this_->UpdateSettingMenu();
          }
          //if ( HIWORD(wparam) == CBN_SELENDOK && LOWORD(wparam) == model_select_id )
          {
            auto model_selector = GetDlgItem(getHWND(), model_select_id);
            this_->model_selected_id = SendMessage(model_selector, CB_GETCURSEL, 0L, 0L);
          }

          for ( int i = 0; i < 4; ++i )
          {
            auto& data = this_->data_[i];
            bool is_fix = IsDlgButtonChecked(hwnd, is_fix_id[i]);
            data.camera_type = is_fix ? ViewData::CameraType::Fix : ViewData::CameraType::ModelTracking;
            if ( !is_fix )
            {
              if ( 1 <= this_->model_selected_id && this_->model_selected_id <= ExpGetPmdNum() )
              {
                auto rotate = DirectX::XMMatrixRotationX(data.camera.rotate.y) * DirectX::XMMatrixRotationY(data.camera.rotate.x);
                auto new_pos = (toMatrix(ExpGetPmdBoneWorldMat(this_->model_selected_id - 1, 0)) * DirectX::XMMatrixInverse(nullptr, rotate)).r[3];
                data.pre_position = new_pos;
              }
            }
          }
          for ( int i = 0; i < 4; i++ )
          {
            this_->data_[i].is_use_ = Button_GetCheck(GetDlgItem(hwnd, is_use_id[i])) == BST_CHECKED;
          }
          bool use = Button_GetCheck(GetDlgItem(hwnd, IS_USE_SPLIT_VIEW)) == BST_CHECKED;
          this_->check_use_menu->check(use);
          this_->is_split_ = use;
          break;
        }
        case WM_MOVE:
          is_move = true;
          break;
        case WM_NCLBUTTONUP:
          if ( is_move )
          {
            InvalidateRect(hwnd, nullptr, true);
          }
        case WM_HSCROLL:
          for ( int i = 0; i < 4; i++ )
          {
            this_->data_[i].alpha_val = 100 - SendDlgItemMessage(hwnd, alpha_val[i], TBM_GETPOS, 0, 0);
          }
          break;

        default:
          break;
        }
        return INT_PTR();
      };

    {
      auto form_dialog = new control::MenuDelegate(ctrl);
      form_dialog->SetType(control::IMenu::Type::Command);
      form_dialog->command = [mainDlgProc,form_dialog, this](const control::IMenu::CommandArgs&)
        {
          if ( !IsWindowVisible(dialog_hwnd) )
          {
            ShowWindow(dialog_hwnd, SW_SHOW);
          }
          else
          {
            ShowWindow(dialog_hwnd, SW_HIDE);
          }
        };
      menu->AppendChild(L"ウィンドウ表示", form_dialog);
      dialog_hwnd = CreateDialogParamW(g_module, MAKEINTRESOURCE(IDD_FORMVIEW), getHWND(), mainDlgProc, (LPARAM)this);
      UpdateSettingMenu();
    }

    menu->AppendSeparator();
    menu->Release();
  }

  static DirectX::XMMATRIX toMatrix(D3DMATRIX mat)
  {
    DirectX::XMMATRIX res;
    res = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)mat.m);
    return res;
  }

  static D3DMATRIX toMatrix(DirectX::XMMATRIX mat)
  {
    alignas(16) D3DMATRIX res;
    DirectX::XMStoreFloat4x4A((DirectX::XMFLOAT4X4A*)res.m, mat);
    return res;
  }

  void DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override
  {
    static int now_draw = 0;
    if ( now_draw ) return;

    device_->GetViewport(&back_viewport_);
    if ( is_split_ == false ) return;

    now_draw = 1;
    D3DMATRIX dworld, dview;
    device_->GetTransform(D3DTS_WORLD, &dworld);
    device_->GetTransform(D3DTS_VIEW, &dview);

    auto DrawView = [=](ViewData& data)
      {
        auto rotate = DirectX::XMMatrixRotationX(data.camera.rotate.y) * DirectX::XMMatrixRotationY(data.camera.rotate.x);
        if ( data.camera_type == ViewData::CameraType::ModelTracking && this->mouse_data.type == MoveMouseData::Type::Unkown )
        {
          {
            wchar_t str[1024];
            auto model_selector = GetDlgItem(getHWND(), model_select_id);
            model_selected_id = SendMessage(model_selector, CB_GETCURSEL, 0L, 0L);
          }
          if ( 1 <= model_selected_id && model_selected_id <= ExpGetPmdNum() )
          {
            auto new_pos = (toMatrix(ExpGetPmdBoneWorldMat(model_selected_id - 1, 0)) * DirectX::XMMatrixInverse(nullptr, rotate)).r[3];
            auto diff = DirectX::XMVectorSubtract(new_pos, data.pre_position);

            data.camera.lookat = DirectX::XMVectorAdd(data.camera.lookat, diff);

            data.pre_position = new_pos;
          }
        }
        // TODO: シェーダに渡されているデータもできれば変えて、再生時も対応できるようにする。
        auto w = DirectX::XMMatrixTranslationFromVector(data.camera.lookat);
        w *= rotate;
        D3DMATRIX tmp = toMatrix(DirectX::XMMatrixInverse(nullptr, w));
        auto pos = toMatrix(DirectX::XMMatrixTranslation(0, 0, data.camera.distance));
        device_->SetTransform(D3DTS_VIEW, &pos);
        device_->SetTransform(D3DTS_WORLD, &tmp);

        auto v = data.view.getViewport(back_viewport_.MinZ, back_viewport_.MaxZ);
        device_->SetViewport(&v);

        // アルファブレンド設定
        DWORD is_alphablend, src_blend, dest_blend, src_alpha;
        device_->GetRenderState(D3DRS_ALPHABLENDENABLE, &is_alphablend);
        device_->GetRenderState(D3DRS_SRCBLEND, &src_blend);
        device_->GetRenderState(D3DRS_DESTBLEND, &dest_blend);
        device_->GetRenderState(D3DRS_SRCBLENDALPHA, &src_alpha);

        device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        DWORD color = 0, back_color;
        int a = (int) (data.alpha_val / 100.0f * 255);
        color = D3DCOLOR_RGBA(a,a,a,a);
        device_->GetRenderState(D3DRS_BLENDFACTOR, &back_color);

        device_->SetRenderState(D3DRS_BLENDFACTOR, color);
        device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_BLENDFACTOR);
        device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVBLENDFACTOR);

        device_->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);


        {
          device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
          device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
#define FVF_VERTEX   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
          D3DVERTEX pt[] = {
            { v.X , v.Y, 0.0001, 1 , 0x30FFFF00 } ,
            { v.X + v.Width, v.Y, 0.0001 , 1, 0x30FFFF00 } ,
            { v.X + v.Width, v.Y + v.Height, 0.0001 , 1 , 0x30FFFF00 },
            { v.X, v.Y + v.Height, 0.0001 , 1 , 0x30FFFF00 },
            { v.X , v.Y , 0.0001, 1, 0x30FFFF00 } ,
          };
          DWORD fvf;
          device_->GetFVF(&fvf);
          device_->SetFVF(FVF_VERTEX);

          IDirect3DVertexBuffer9* buffer;
          UINT offset, stride;
          device_->GetStreamSource(0, &buffer, &offset, &stride);
          //頂点データ(v)を渡して描画する
          device_->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, pt, sizeof(D3DVERTEX));
          device_->SetFVF(fvf);
          device_->SetStreamSource(0, buffer, offset, stride);
          if ( buffer ) buffer->Release();
        }

        device_->SetRenderState(D3DRS_BLENDFACTOR, back_color);
        device_->SetRenderState(D3DRS_ALPHABLENDENABLE, is_alphablend);
        device_->SetRenderState(D3DRS_SRCBLEND, src_blend);
        device_->SetRenderState(D3DRS_DESTBLEND, dest_blend);
        device_->SetRenderState(D3DRS_SRCBLENDALPHA, src_alpha);
        device_->SetTransform(D3DTS_VIEW, &dview);
      };
    auto Draw = [=](D3DMATRIX /*center*/)
      {
        D3DVIEWPORT9 viewport = back_viewport_;
        viewport.Width = viewport.Width / 2;
        viewport.Height = viewport.Height / 2;
        if ( depth_buf[0] == nullptr )
        {
          device_->GetDepthStencilSurface(&tmp_depth);
          if ( tmp_depth )
          {
            D3DSURFACE_DESC desc;
            tmp_depth->GetDesc(&desc);
            for ( auto& i : depth_buf )
            {
              device_->CreateDepthStencilSurface(desc.Width, desc.Height, desc.Format, desc.MultiSampleType, desc.MultiSampleQuality,
                                                 TRUE, &i, nullptr);
            }
            ResetDefaultSetting(viewport);
          }
        }
        device_->SetDepthStencilSurface(depth_buf[0]);
        if ( data_[0].is_use_ ) DrawView(data_[0]);

        device_->SetDepthStencilSurface(depth_buf[1]);
        if ( data_[1].is_use_ ) DrawView(data_[1]);

        device_->SetDepthStencilSurface(depth_buf[2]);
        if ( data_[2].is_use_ ) DrawView(data_[2]);

        if ( is_camera_right_bottom )
        {
          viewport = data_[3].view.getViewport(viewport.MinZ, viewport.MaxZ);
          device_->SetViewport(&viewport);
        }
        else
        {
          if ( data_[3].is_use_ )
          {
            device_->SetDepthStencilSurface(depth_buf[3]);
            DrawView(data_[3]);
          }
          device_->SetViewport(&back_viewport_);
        }

        device_->SetDepthStencilSurface(depth_buf[4]);
        device_->SetTransform(D3DTS_WORLD, &dworld);
        device_->SetTransform(D3DTS_VIEW, &dview);
      };

    for ( int i = 0, len = ExpGetAcsNum(); i < len; i++ )
    {
      if ( ExpGetAcsOrder(i) == ExpGetCurrentObject() )
      {
        Draw(ExpGetAcsWorldMat(i));
        break;
      }
    }
    for ( int i = 0, len = ExpGetPmdNum(); i < len; i++ )
    {
      if ( ExpGetPmdOrder(i) == ExpGetCurrentObject() )
      {
        Draw(ExpGetPmdBoneWorldMat(i, 0));
        break;
      }
    }
    now_draw = 0;
  }

  void PostDrawIndexedPrimitive(D3DPRIMITIVETYPE, INT /*BaseVertexIndex*/, UINT /*MinVertexIndex*/, UINT /*NumVertices*/, UINT /*startIndex*/, UINT /*primCount*/, HRESULT& /*res*/) override
  {
    device_->SetViewport(&back_viewport_);
    if ( tmp_depth ) device_->SetDepthStencilSurface(tmp_depth);

    return;
  }

  void Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override
  {
    static int now_clear = 0;
    if ( now_clear ) return;
    now_clear = 1;
    IDirect3DSurface9* tmp;
    device_->GetDepthStencilSurface(&tmp);
    for ( auto& i: depth_buf )
    {
      device_->SetDepthStencilSurface(i);
      device_->Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    device_->SetDepthStencilSurface(tmp);
    if ( tmp ) tmp->Release();
    now_clear = 0;
  }


  const char* getPluginTitle() const override { return "MMDUtility_ViewSplit"; }

  std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override
  {
    switch ( uMsg )
    {
    case WM_MOUSEWHEEL:
      return { true,0 };
    default:
      break;
    }
    return { false,0 };
  }

  void WndProc(const CWPSTRUCT* param) override
  {
    switch ( param->message )
    {
    case WM_LBUTTONDOWN:
      printf("%d", param->hwnd);
    default:
      break;
    }
  }

  void MouseProc(WPARAM wParam, const MOUSEHOOKSTRUCT* mouseObj) override
  {
    switch ( wParam )
    {
    case WM_LBUTTONDOWN:
    {
      auto pt = mouseObj->pt;
      ScreenToClient(getHWND(), &pt);
      for ( int i = 0; i < 4; i++ )
      {
        auto& d = data_[i].view;
        if ( d.x <= pt.x && pt.x <= d.x + d.width * data_[i].view.scale && d.y <= pt.y && pt.y <= d.y + d.height * data_[i].view.scale )
        {
          bool is_shift = GetAsyncKeyState(VK_SHIFT) < 0;
          bool is_control = GetAsyncKeyState(VK_CONTROL) < 0;
          bool is_alt = GetAsyncKeyState(VK_MENU) < 0;
          if ( is_alt && is_control && is_shift )
          {
            mouse_data.type = MoveMouseData::Type::CameraDistanceMove;
          }
          else if ( is_alt && is_shift )
          {
            mouse_data.type = MoveMouseData::Type::CameraXYMove;
          }
          else if ( is_control && is_shift )
          {
            mouse_data.type = MoveMouseData::Type::ViewScale;
          }
          else if ( is_control )
          {
            mouse_data.type = MoveMouseData::Type::ViewMove;
          }
          else if ( is_alt )
          {
            mouse_data.type = MoveMouseData::Type::CameraRotate;
          }
          else
          {
            mouse_data.type = MoveMouseData::Type::Unkown;
          }
          mouse_data.id = i;
          mouse_data.x = pt.x;
          mouse_data.y = pt.y;
          break;
        }
      }
    }
      break;

    case WM_MOUSEMOVE:
    {
      if ( mouse_data.id == -1 ) break;
      auto pt = mouseObj->pt;
      ScreenToClient(getHWND(), &pt);
      switch ( mouse_data.type )
      {
      case MoveMouseData::Type::ViewMove:
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].view.x += dx;
        mouse_data.x = pt.x;

        int dy = pt.y - mouse_data.y;
        mouse_data.y = pt.y;
        data_[mouse_data.id].view.y += dy;
      }
        break;
      case MoveMouseData::Type::ViewScale:
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].view.scale += dx / 500.0f;
        data_[mouse_data.id].view.scale = std::max(data_[mouse_data.id].view.scale, 0.1f);
        mouse_data.x = pt.x;
      }
        break;
      case MoveMouseData::Type::CameraRotate:
      {
        int dx = pt.x - mouse_data.x;
        mouse_data.x = pt.x;
        data_[mouse_data.id].camera.rotate.x += dx / 200.0f;

        int dy = pt.y - mouse_data.y;
        mouse_data.y = pt.y;
        data_[mouse_data.id].camera.rotate.y += dy / 200.0f;
      }
        break;
      case MoveMouseData::Type::CameraXYMove:
      {
        int dx = pt.x - mouse_data.x;
        int dy = pt.y - mouse_data.y;
        DirectX::XMVECTOR div = { -(20.0f * data_[mouse_data.id].view.scale) ,(20.0f * data_[mouse_data.id].view.scale) ,1,1 };
        data_[mouse_data.id].camera.lookat = DirectX::XMVectorAdd(data_[mouse_data.id].camera.lookat, DirectX::XMVectorDivide(DirectX::XMVectorSet(dx, dy, 0, 0), div));

        mouse_data.x = pt.x;
        mouse_data.y = pt.y;
      }
        break;
      case MoveMouseData::Type::CameraDistanceMove:
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].camera.distance += dx / 20.0f;
        mouse_data.x = pt.x;
      }
        break;
      default: break;
      }
    }
      break;
    case WM_LBUTTONUP:
    {
      mouse_data.id = -1;
      mouse_data.type = MoveMouseData::Type::Unkown;
    }
      break;
    default:
      break;
    }
  }

  std::array<IDirect3DSurface9*, 5> depth_buf = {};
  D3DVIEWPORT9 back_viewport_ = { 0 };
  IDirect3DSurface9* tmp_depth = nullptr;
  IDirect3DDevice9* device_;
};

int version() { return 3; }


MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
  return new ViewSplit(device);
}

void destroy3(MMDPluginDLL3* p)
{
  delete p;
}
