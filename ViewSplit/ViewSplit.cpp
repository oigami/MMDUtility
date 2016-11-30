// ViewSplit.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//
#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd/MMDExport.h"
#include "../MMDUtility/mmd_utility.h"
#include "resource.h"
extern HMODULE g_module;
typedef struct
{
  float x, y, z, rhw;
  DWORD diff;
} D3DVERTEX;

class ViewSplit : public MMDPluginDLL3
{
public:
  explicit ViewSplit(IDirect3DDevice9* device): device_(device) {}

  bool is_split_ = false;
  bool is_camera_right_bottom = false;
  control::MenuCheckBox* check_use_menu;
  HWND dialog_hwnd;
  struct Rect
  {
    int x, y, width, height;
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
      return v;

    }
  };

  struct ViewData
  {
    bool is_use_;
    Rect view;
    float scale = 1.0f;
    int alpha_val = 100;
  };

  std::array<ViewData, 4> data_;

  struct MoveMouseData
  {
    int x, y;
    int type;
    int id = -1;
  } mouse_data;

  static constexpr int is_use_id[] = { LU_IS_USE , RU_IS_USE,LB_IS_USE,RB_IS_USE };

  void ResetDefaultSetting(D3DVIEWPORT9 viewport)
  {
    for ( int i = 0; i < 4; i++ )
    {
      data_[i].is_use_ = true;
      Button_SetCheck(GetDlgItem(dialog_hwnd, is_use_id[i]), TRUE);
    }

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
      check_use_menu= new control::MenuCheckBox(ctrl);
      check_use_menu->command = [this](const control::IMenu::CommandArgs&)
        {
          check_use_menu->reverseCheck();
          is_split_ = check_use_menu->isChecked();
          Button_SetCheck(GetDlgItem(dialog_hwnd,IS_USE_SPLIT_VIEW), true);
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
        static constexpr int alpha_val[] = { LU_ALPHA, RU_ALPHA, LB_ALPHA2, RB_ALPHA };
        static ViewSplit* this_;
        static bool is_move = false;
        if ( msg == WM_INITDIALOG ) this_ = (ViewSplit*) lParam;
        if ( this_ == nullptr ) return INT_PTR();
        switch ( msg )
        {
        case WM_COMMAND:
        {
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

    auto DrawView = [=](const ViewData& data, DirectX::XMMATRIX worldx)
      {
        const Rect& viewport = data.view;
        // TODO: シェーダに渡されているデータもできれば変えて、再生時も対応できるようにする。
        auto tmp = toMatrix(worldx);
        device_->SetTransform(D3DTS_WORLD, &tmp);
        auto v = data.view.getViewport(back_viewport_.MinZ, back_viewport_.MaxZ);
        v.Width *= data.scale;
        v.Height *= data.scale;
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
        device_->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_BLENDFACTOR);

        device_->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

        device_->SetRenderState(D3DRS_BLENDFACTOR, back_color);
        device_->SetRenderState(D3DRS_ALPHABLENDENABLE, is_alphablend);
        device_->SetRenderState(D3DRS_SRCBLEND, src_blend);
        device_->SetRenderState(D3DRS_DESTBLEND, dest_blend);
        device_->SetRenderState(D3DRS_SRCBLENDALPHA, src_alpha);


#define FVF_VERTEX   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)
        D3DVERTEX pt[] = {
          { v.X , v.Y, 0.0001, 1 , 0xFFFFFF00 } ,
          { v.X + v.Width, v.Y, 0.0001 , 1, 0xFFFFFF00 } ,
          { v.X + v.Width, v.Y + v.Height, 0.0001 , 1 , 0xFFFFFF00 },
          { v.X, v.Y + v.Height, 0.0001 , 1 , 0xFFFFFF00 },
          { v.X , v.Y , 0.0001, 1, 0xFFFFFF00 } ,
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
        if ( data_[0].is_use_ ) DrawView(data_[0], DirectX::XMMatrixIdentity());

        device_->SetDepthStencilSurface(depth_buf[1]);
        if ( data_[1].is_use_ ) DrawView(data_[1], DirectX::XMMatrixRotationY(DirectX::XM_PI / 2));

        device_->SetDepthStencilSurface(depth_buf[2]);
        if ( data_[2].is_use_ ) DrawView(data_[2], DirectX::XMMatrixRotationY(-DirectX::XM_PI / 2));

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
            DrawView(data_[3], DirectX::XMMatrixRotationX(-DirectX::XM_PI / 2));
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
        if ( d.x <= pt.x && pt.x <= d.x + d.width * data_[i].scale && d.y <= pt.y && pt.y <= d.y + d.height * data_[i].scale )
        {
          if ( GetAsyncKeyState(VK_CONTROL) < 0 )
          {
            // 移動
            mouse_data.type = 1;
          }
          else if ( GetAsyncKeyState(VK_SHIFT) < 0 )
          {
            // 拡大縮小
            mouse_data.type = 2;
          }
          else
          {
            // 回転
            mouse_data.type = 3;
          }
          mouse_data.id = i;
          mouse_data.x = pt.x;
          mouse_data.y = pt.y;
          break;
        }
      }
      break;
    }

    case WM_MOUSEMOVE:
    {
      if ( mouse_data.id == -1 ) break;
      auto pt = mouseObj->pt;
      ScreenToClient(getHWND(), &pt);
      if ( mouse_data.type == 1 )
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].view.x += dx;
        mouse_data.x = pt.x;

        int dy = pt.y - mouse_data.y;
        mouse_data.y = pt.y;
        data_[mouse_data.id].view.y += dy;
      }
      else if ( mouse_data.type == 2 )
      {
        int dx = pt.x - mouse_data.x;
        data_[mouse_data.id].scale += dx / 200.0f;
        data_[mouse_data.id].scale = std::max(data_[mouse_data.id].scale, 0.1f);
        mouse_data.x = pt.x;
      }
      else { }
      break;
    }

    case WM_LBUTTONUP:
    {
      mouse_data.id = -1;
      break;
    }
    default:
      break;
    }
  }

  std::array<IDirect3DSurface9*,5> depth_buf = {};
  D3DVIEWPORT9 back_viewport_;
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
