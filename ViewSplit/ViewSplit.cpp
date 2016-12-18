// ViewSplit.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//
#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd/MMDExport.h"
#include "../MMDUtility/mmd_utility.h"
#include "resource.h"
#include <Commdlg.h>
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

typedef struct
{
  float x, y, z, rhw;
  DWORD diff;
} D3DVERTEX;

struct Rect
{
private:
  // 正規化済み
  float x_, y_, width_, height_;
  float scale_ = 1.0f;
public:

  Rect() : x_(0), y_(0), width_(0.5f), height_(0.5f) {}

  Rect(float x, float y, float width, float height)
    : x_(x),
      y_(y),
      width_(width),
      height_(height) { }

  void setScale(float scale)
  {
    scale = 1.0f;
    scale_ = std::max(scale_, 0.1f);
  }

  float getScale() const { return scale_; }

  void scale(float d_scale)
  {
    scale_ += d_scale;
    scale_ = std::max(scale_, 0.1f);
  }

  void move(const D3DVIEWPORT9& viewport, int x, int y)
  {
    x_ += float(x) / viewport.Width;
    y_ += float(y) / viewport.Height;
  }

  bool contains(const D3DVIEWPORT9& viewport, POINT pos) const
  {
    const int cx = x(viewport);
    const int cy = y(viewport);
    const int cwidth = width(viewport);
    const int cheight = height(viewport);

    return cx <= pos.x && pos.x <= cx + cwidth && cy <= pos.y && pos.y <= cy + cheight;
  }

  int x(const D3DVIEWPORT9& viewport) const
  {
    int res = ax(viewport);
    if ( res < 0 ) return 0;
    return res;
  }

  int y(const D3DVIEWPORT9& viewport) const
  {
    int res = ay(viewport);
    if ( res < 0 ) return 0;
    return res;
  }

  int ax(const D3DVIEWPORT9& viewport) const
  {
    return static_cast<int>(viewport.X + x_ * viewport.Width);
  }

  int ay(const D3DVIEWPORT9& viewport) const
  {
    return static_cast<int>(viewport.Y + y_ * viewport.Height);
  }


  int width(const D3DVIEWPORT9& viewport) const
  {
    int sx = ax(viewport);
    int res = static_cast<int>(width_ * viewport.Width * scale_);
    if ( sx < 0 ) res += sx;
    return res;
  }

  int height(const D3DVIEWPORT9& viewport) const
  {
    int sy = ay(viewport);
    int res = static_cast<int>(height_ * viewport.Height * scale_);
    if ( sy < 0 ) res += sy;
    return res;
  }

  D3DVIEWPORT9 getViewport(const D3DVIEWPORT9& viewport) const
  {
    D3DVIEWPORT9 v = viewport;
    v.Width = width(viewport);
    v.Height = height(viewport);
    v.X = x(viewport);
    v.Y = y(viewport);
    return v;
  }

  void save(std::ostream& ofs) const
  {
    println(ofs, x_, y_, width_, height_, scale_);
  }

  void load(std::istream& ifs, int version, const D3DVIEWPORT9& viewport)
  {
    ifs >> x_ >> y_ >> width_ >> height_ >> scale_;
    if ( version == 1 )
    {
      x_ = (x_ - viewport.X) / viewport.Width;
      y_ = (y_ - viewport.Y) / viewport.Height;
      width_ /= viewport.Width;
      height_ /= viewport.Height;
    }
  }
};


struct ViewData
{
  DirectX::XMVECTOR pre_position;
  Rect view;
  bool is_use_;
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


  void save(std::ofstream& ofs)
  {
    println(ofs, is_use_ ? "1" : "0");
    view.save(ofs);
    println(ofs, alpha_val);

    println(ofs, (int) camera_type);
    DirectX::XMFLOAT3A lookat;
    DirectX::XMStoreFloat3A(&lookat, camera.lookat);
    println(ofs, lookat.x, lookat.y, lookat.z);
    println(ofs, camera.rotate.x, camera.rotate.y);
    println(ofs, camera.distance);
  }

  void load(std::ifstream& ifs, const D3DVIEWPORT9& viewport, int version)
  {
    int is_use;
    ifs >> is_use;
    is_use_ = is_use == 1;
    view.load(ifs, version, viewport);
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
  explicit ViewSplit(IDirect3DDevice9* device) : device_(device) {}

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

  void ResetDefaultSetting(D3DVIEWPORT9)
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
    data_[0].view = Rect(0, 0, 0.5f, 0.5f);
    data_[1].view = Rect(0.5f, 0, 0.5f, 0.5f);
    data_[2].view = Rect(0, 0.5f, 0.5f, 0.5f);
    data_[3].view = Rect(0.5f, 0.5f, 0.5f, 0.5f);


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
    szPath = (mmp::getDLLPath(g_module) / L"split_view_setting").wstring();
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
    GetModuleFileNameA(g_module, module_path, MAX_PATH);
    auto path = filesystem::path(module_path).parent_path() / L"split_view_setting";
    if ( filesystem::exists(path) == false )
    {
      filesystem::create_directories(path);
    }
    filesystem::directory_iterator it(path), last;
    auto menu = GetDlgItem(dialog_hwnd, LOAD_MENU);
    int cnt = static_cast<int>(SendMessage(menu, CB_GETCOUNT, 0, 0));
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
          Button_SetCheck(GetDlgItem(dialog_hwnd, IS_USE_SPLIT_VIEW), is_split_);
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
        case WM_CLOSE:
          this_->form_dialog_->command({});
          break;
        case WM_COMMAND:
        {
          if ( LOWORD(wparam) == PARAM_LOAD )
          {
            wchar_t str[1024] = {};
            auto menu = GetDlgItem(hwnd, LOAD_MENU);
            int select = static_cast<int>(SendMessage(menu, CB_GETCURSEL, 0L, 0L));
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
                this_->data_[i].load(ifs, this_->back_viewport_, version);

                Button_SetCheck(GetDlgItem(this_->dialog_hwnd, is_use_id[i]), this_->data_[i].is_use_);
                int id = -1;
                if ( this_->data_[i].camera_type == ViewData::CameraType::Fix )
                {
                  id = is_fix_id[i];
                }
                else
                {
                  id = is_tracking_id[i];
                }
                CheckRadioButton(this_->dialog_hwnd, is_fix_id[i], is_tracking_id[i], id);
              }
            }
          }
          else if ( LOWORD(wparam) == PARAM_SAVE )
          {
            auto str = funcFileSave(hwnd);
            filesystem::path path = str;
            std::ofstream ofs(path.string());
            ofs << "002" << std::endl;
            for ( int i = 0; i < 4; i++ )
            {
              this_->data_[i].save(ofs);
              ofs << std::endl;
            }

            auto menu = GetDlgItem(hwnd, LOAD_MENU);
            int select = static_cast<int>(SendMessage(menu, CB_GETCURSEL, 0L, 0L));
            SendMessage(menu, CB_GETLBTEXT, select, (LPARAM) str.c_str());
            this_->UpdateSettingMenu();
          }
          //if ( HIWORD(wparam) == CBN_SELENDOK && LOWORD(wparam) == model_select_id )
          {
            auto model_selector = GetDlgItem(getHWND(), model_select_id);
            this_->model_selected_id = static_cast<int>(SendMessage(model_selector, CB_GETCURSEL, 0L, 0L));
          }

          for ( int i = 0; i < 4; ++i )
          {
            auto& data = this_->data_[i];
            bool is_fix = IsDlgButtonChecked(hwnd, is_fix_id[i]) == BST_CHECKED;
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
            this_->data_[i].alpha_val = 100 - static_cast<int>(SendDlgItemMessage(hwnd, alpha_val[i], TBM_GETPOS, 0, 0));
          }
          break;

        default:
          break;
        }
        return INT_PTR();
      };

    {
      form_dialog_ = new control::MenuCheckBox(ctrl);
      form_dialog_->SetType(control::IMenu::Type::Command);
      form_dialog_->command = [mainDlgProc, this](const control::IMenu::CommandArgs&)
        {
          form_dialog_->reverseCheck();
          if ( form_dialog_->isChecked() )
          {
            ShowWindow(dialog_hwnd, SW_SHOW);
          }
          else
          {
            ShowWindow(dialog_hwnd, SW_HIDE);
          }
        };
      menu->AppendChild(L"画面分割設定ウィンドウ", form_dialog_);
      dialog_hwnd = CreateDialogParamW(g_module, MAKEINTRESOURCE(IDD_FORMVIEW), getHWND(), mainDlgProc, (LPARAM)this);
      Button_SetCheck(GetDlgItem(dialog_hwnd, IS_USE_SPLIT_VIEW), true);
      UpdateSettingMenu();
    }
    menu->AppendSeparator();
    menu->Release();
    DrawMenuBar(getHWND());
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
            auto model_selector = GetDlgItem(getHWND(), model_select_id);
            model_selected_id = static_cast<int>(SendMessage(model_selector, CB_GETCURSEL, 0L, 0L));
          }
          if ( 1 <= model_selected_id && model_selected_id <= ExpGetPmdNum() )
          {
            auto new_pos = (toMatrix(ExpGetPmdBoneWorldMat(model_selected_id - 1, 0)) * DirectX::XMMatrixInverse(nullptr, rotate)).r[3];
            auto diff = DirectX::XMVectorSubtract(new_pos, data.pre_position);

            data.camera.lookat = DirectX::XMVectorAdd(data.camera.lookat, diff);

            data.pre_position = new_pos;
          }
        }
        if ( data.is_use_ == false ) return;

        // TODO: シェーダに渡されているデータもできれば変えて、再生時も対応できるようにする。
        auto w = DirectX::XMMatrixTranslationFromVector(data.camera.lookat);
        w *= rotate;
        D3DMATRIX tmp = toMatrix(DirectX::XMMatrixInverse(nullptr, w));
        auto pos = toMatrix(DirectX::XMMatrixTranslation(0, 0, data.camera.distance));
        device_->SetTransform(D3DTS_VIEW, &pos);
        device_->SetTransform(D3DTS_WORLD, &tmp);

        auto v = data.view.getViewport(back_viewport_);
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
        color = D3DCOLOR_RGBA(a, a, a, a);
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
            { static_cast<float>(v.X), static_cast<float>(v.Y), 0.0001f, 1.0f, 0x30FFFF00 } ,
            { static_cast<float>(v.X + v.Width),static_cast<float>(v.Y), 0.0001f , 1.0f, 0x30FFFF00 } ,
            { static_cast<float>(v.X + v.Width),static_cast<float>(v.Y + v.Height), 0.0001f , 1.0f , 0x30FFFF00 },
            { static_cast<float>(v.X), static_cast<float>(v.Y + v.Height), 0.0001f , 1.0f, 0x30FFFF00 },
            { static_cast<float>(v.X), static_cast<float>(v.Y) , 0.0001f, 1.0f, 0x30FFFF00 } ,
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
        if ( ResetDepthBuffer() )
        {
          ResetDefaultSetting(viewport);
        }
        for ( int i = 0; i < 3; i++ )
        {
          device_->SetDepthStencilSurface(depth_buf[i]);
          DrawView(data_[i]);
        }

        if ( is_camera_right_bottom )
        {
          viewport = data_[3].view.getViewport(back_viewport_);
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
    for ( auto& i : depth_buf )
    {
      device_->SetDepthStencilSurface(i);
      device_->Clear(Count, pRects, Flags, Color, Z, Stencil);
    }

    device_->SetDepthStencilSurface(tmp);
    if ( tmp ) tmp->Release();
    now_clear = 0;
  }


  const char* getPluginTitle() const override { return "MMDUtility_ViewSplit"; }

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
        if ( d.contains(back_viewport_, pt) )
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
        int dy = pt.y - mouse_data.y;
        data_[mouse_data.id].view.move(back_viewport_, dx, dy);
        mouse_data.x = pt.x;
        mouse_data.y = pt.y;
      }
        break;
      case MoveMouseData::Type::ViewScale:
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].view.scale(dx / 500.0f);
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
        float dx = static_cast<float>(pt.x - mouse_data.x);
        float dy = static_cast<float>(pt.y - mouse_data.y);
        DirectX::XMVECTOR div = { -(20.0f * data_[mouse_data.id].view.getScale()) ,(20.0f * data_[mouse_data.id].view.getScale()) ,1,1 };
        data_[mouse_data.id].camera.lookat = DirectX::XMVectorAdd(data_[mouse_data.id].camera.lookat, DirectX::XMVectorDivide(DirectX::XMVectorSet(dx, dy, 0.0f, 0.0f), div));

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

  void Reset(D3DPRESENT_PARAMETERS* /*pPresentationParameters*/) override
  {
    if ( tmp_depth ) tmp_depth->Release();
    for ( auto& i : depth_buf )
    {
      if ( i ) i->Release();
      i = nullptr;
    }
  }

  void PostReset(D3DPRESENT_PARAMETERS* /*pPresentationParameters*/, HRESULT& /*res*/) override
  {
    ResetDepthBuffer();
  }

  bool ResetDepthBuffer()
  {
    if ( depth_buf[0] ) return false;

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
      return true;
    }
    return false;
  }

  std::array<IDirect3DSurface9*, 5> depth_buf = {};
  D3DVIEWPORT9 back_viewport_ = { 0 };
  IDirect3DSurface9* tmp_depth = nullptr;
  IDirect3DDevice9* device_;
  control::MenuCheckBox* form_dialog_;
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
