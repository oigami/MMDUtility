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

void OpenConsole()
{
  FILE *in, *out;
  AllocConsole();
  freopen_s(&out, "CONOUT$", "w", stdout);//CONOUT$
  freopen_s(&in, "CONIN$", "r", stdin);
}

namespace
{
  constexpr float eps = 1e-7f;

  bool compare(float a, float b)
  {
    return a - eps <= b && b <= a + eps;
  }

  struct Float3
  {
    float x, y, z;

    bool operator==(const Float3& o) const
    {
      return compare(x, o.x) && compare(y, o.y) && compare(z, o.z);
    }

    bool operator!=(const Float3& o) const
    {
      return !(*this == o);
    }
  };
}

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

struct CameraKeyFrameData
{
  int frame_no;
  int pre_index;
  int next_index; // 次のキーフレームがあるときに0以外になる
  float length;
  Float3 xyz;
  Float3 rxyz;
  char hokan1_x[6]; // x, y, z, 回転, 距離, 視野角
  char hokan1_y[6];
  char hokan2_x[6];
  char hokan2_y[6];
  int is_perspective;
  int view_angle;
  int is_selected; // 1で選択している。0で選択していない
  int __unknown2[2];
};

struct MMDMainData
{
  int __unknown10[2];
  int mouse_x, mouse_y;
  int pre_mouse_x, pre_mouse_y;
  int key_up;
  int key_down;
  int key_left;
  int key_right;
  int key_shift; // keyは0から3までの数値を取る。押している間は3になる。
  int key_space;
  int key_f9;
  int key_x_or_f11; // f11の場合は2になる
  int key_z;
  int key_c;
  int key_v;
  int key_d;
  int key_a;
  int key_b;
  int key_g;
  int key_s;
  int key_i;
  int key_h;
  int key_k;
  int key_p;
  int key_u;
  int key_j;
  int key_f;
  int key_r;
  int key_l;
  int key_close_bracket;
  int key_backslash;
  int key_tab;
  int __unknown20[14];
  int key_enter;
  int key_ctrl;
  int key_alt;
  int __unknown30;
  void* __unknown_pointer;
  int __unknown40[155];
  Float3 rxyz;
  int __unknown49[2];
  float counter_f;
  int counter;
  int __unknown50[3];
  Float3 xyz;
  int __unknown60[22];
  CameraKeyFrameData (&camera_key_frame)[10000];
  void* __unknown_pointer20[258];
  void* __unknown_pointer30[255]; // 起動時nullモデルを読み込むと順番にポインタが入る
  int select_model;

  enum class SelectBoneType : int
  {
    Select,
    Box,
    Camera,
    Rotate,
    Move
  };

  SelectBoneType select_bone_type; // 0:選択、1:BOX選択、2:カメラモード、3:回転、4:移動
  int __unknown70[4];
  float __unknown71;
  int mouse_over_move; // xyz回転(9,10,11)、xyz移動(12,13,14)
  int __unknown80[17];
  int left_frame;
  int __unknown90;
  int pre_left_frame;
  int now_frame;
  int __unknown100[163785];
  char is_camera_select;
  char is_model_bone_select[127];
  int __unknown110[318];
  int output_size_x;
  int output_size_y;
  float length;
};

constexpr int pointer_size = offsetof(MMDMainData, length);
constexpr int size = sizeof(CameraKeyFrameData);
static_assert(5200 == offsetof(MMDMainData, now_frame), "");

static_assert(660344 == offsetof(MMDMainData, is_camera_select), "");
static_assert(661752 == offsetof(MMDMainData, length), "");

class IUndoRedo
{
  virtual IUndoRedo* next() = 0;
  virtual void load() = 0;
  virtual void save() = 0;
};

namespace
{
  MMDMainData* getMMDMainData()
  {
    auto pointer = (BYTE**) ((BYTE*) GetModuleHandleW(nullptr) + 0x1445F8);
    if ( IsBadReadPtr(pointer, sizeof(INT_PTR)) != 0 )
    {
      std::wstring error = L"内部エラー\nポインタの読み込みに失敗しました\npointer=" + std::to_wstring((INT_PTR) pointer);
      MessageBoxW(nullptr, error.c_str(), L"エラー", MB_OK);
      return nullptr;
    }
    return (MMDMainData*) *pointer;
  }
}

class CameraUndoRedo
{
public:
  CameraUndoRedo(const CameraKeyFrameData& d, int i) : changed_data(d), index(i) {}

  CameraUndoRedo() = default;

  bool compare() const
  {
    auto mmd_data = getMMDMainData();
    if ( changed_data.xyz != mmd_data->xyz )
    {
      return false;
    }
    if ( changed_data.rxyz != mmd_data->rxyz )
    {
      return false;
    }
    return true;
  }

  void updateState()
  {
    int x = 100, y;
    RECT rect;
    GetClientRect(getHWND(), &rect);
    y = rect.bottom - 256;
    SendMessage(getHWND(), WM_MOUSEMOVE, 0, MAKELPARAM(x, y));
    SendMessage(getHWND(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    SendMessage(getHWND(), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(x, y));
  }

  void load()
  {
    auto mmd_data = getMMDMainData();
    if ( index >= 0 )
    {
      auto& camera_data = mmd_data->camera_key_frame[index];
      camera_data = changed_data;
      if ( state == NewData )
      {
        mmd_data->xyz = camera_data.xyz;
        mmd_data->rxyz = camera_data.rxyz;
        camera_data.xyz = { 0,0,0 };
        camera_data.rxyz = { 0,0,0 };

        mmd_data->camera_key_frame[camera_data.pre_index].next_index = camera_data.is_perspective;
        camera_data.pre_index = 0;
        camera_data.is_perspective = 1;

        if ( camera_data.next_index >= 0 ) mmd_data->camera_key_frame[camera_data.next_index].pre_index = camera_data.is_selected;
        camera_data.next_index = 0;
        camera_data.is_selected = 1;
      }
      else if ( state == Overwrite )
      {
        mmd_data->now_frame = changed_data.frame_no;
        updateState();
        mmd_data->xyz = camera_data.xyz;
        mmd_data->rxyz = camera_data.rxyz;
        mmd_data->camera_key_frame[camera_data.pre_index].next_index = index;

        if ( camera_data.next_index > 0 ) mmd_data->camera_key_frame[camera_data.next_index].pre_index = index;
      }
    }
    else
    {
      mmd_data->xyz = changed_data.xyz;
      mmd_data->now_frame = changed_data.frame_no;
      mmd_data->rxyz = changed_data.rxyz;
      mmd_data->length = changed_data.length;
    }
    mmd_data->left_frame = left_frame;
    mmd_data->pre_left_frame = left_frame;
    updateState();
  }

  void save()
  {
    auto& camera_data = getMMDMainData()->camera_key_frame[index];
    changed_data = camera_data;
  }

  CameraKeyFrameData changed_data;
  int index;
  int left_frame = 0;

  enum State
  {
    NewData,
    Overwrite,
    Key,
    Wheel,
  } state = State::Key;
};

class CameraKeyFrameUndoRedo
{
  std::vector<std::pair<int, CameraUndoRedo>> list;
  int now_undo_cnt;

  int now_frame = -1;
  Float3 xyz;
  Float3 rxyz;
  bool is_update = false;
  bool is_enter = false;
  static constexpr int undo_data_size = 2000;
  static constexpr int undo_memory_free_size = 500;
public:

  CameraKeyFrameUndoRedo() : now_undo_cnt(0)
  {
    list.reserve(undo_data_size);
  }

  void print()
  {
    //system("cls");
    for ( auto& i : list )
    {
      using namespace std;
      auto& data = i.second.changed_data.xyz;
      cout << (data.x) << ' ' << data.y << ' ' << data.z << endl;
    }
  }

  void keyUpdate(WPARAM wparam, LPARAM lparam)
  {
    //print();
    auto mmd_data = getMMDMainData();
    if ( now_frame == -1 )
    {
      now_frame = mmd_data->now_frame;
      list.push_back({ 1,CameraUndoRedo(mmd_data->camera_key_frame[0], -1) });
      now_undo_cnt++;
    }
    if ( GetAsyncKeyState(VK_CONTROL) < 0 && LOWORD(lparam) == 1 && (lparam & 0x80000000) == 0 )
    {
      if ( wparam == 'Z' )
      {
        undo();
      }
      else if ( wparam == 'Y' )
      {
        redo();
      }
    }

    if ( mmd_data->select_bone_type != MMDMainData::SelectBoneType::Camera ) return;
    if ( (wparam == VK_RIGHT || wparam == VK_LEFT) )
    {
      if ( is_update )
      {
        pushCurent();
        is_update = false;
      }
      xyz = mmd_data->xyz;
      rxyz = mmd_data->rxyz;
    }

    if ( wparam == VK_RETURN )
    {
      if ( LOWORD(lparam) == 1 && (lparam & 0x40000000) == 0 )
      {
        pushNowframe();
      }
      else if ( (lparam & 0x80000000) )
      {
        pushNowframe();
      }
      printf("enter %llu %x %d\n", wparam, lparam, (lparam & 0x80000000));
    }
  }

  std::pair<int, CameraKeyFrameData&> getNowFrame()
  {
    auto mmd_data = getMMDMainData();
    auto& key_frame = mmd_data->camera_key_frame;
    for ( int i = 0; i < 10000; )
    {
      if ( key_frame[i].frame_no == mmd_data->now_frame ) return { i, key_frame[i] };

      if ( key_frame[i].next_index == 0 )
      {
        for ( int j = 1; j < 10000; j++ )
        {
          if ( key_frame[j].frame_no == 0 ) return { j, key_frame[j] };
        }
      }
      else
      {
        i = key_frame[i].next_index;
      }
    }
    MessageBoxW(nullptr, L"アンドゥ機能でエラーが発生しました。\n現在のキーフレーム情報が取得できませんでした。", L"エラー", MB_OK);
    return { 0, key_frame[0] };
  }

  void push(int num, const CameraUndoRedo& p)
  {
    list.resize(now_undo_cnt);
    list.push_back({ num,p });
    now_undo_cnt++;
    if ( list.size() >= undo_data_size - undo_memory_free_size )
    {
      list.erase(list.begin(), list.begin() + undo_memory_free_size);
      now_undo_cnt -= undo_memory_free_size;
    }
  }

  void pushNowframe()
  {
    auto mmd_data = getMMDMainData();
    CameraUndoRedo camera;
    camera.state = CameraUndoRedo::Key;
    auto p = getNowFrame();
    camera.changed_data = p.second;
    camera.index = p.first;
    if ( p.second.frame_no == 0 )
    {
      camera.state = CameraUndoRedo::NewData;
      camera.changed_data.xyz = mmd_data->xyz;
      camera.changed_data.rxyz = mmd_data->rxyz;

      auto& key_frame = mmd_data->camera_key_frame;
      constexpr int INF = 1000000;
      std::pair<int, int> maxi = { 0,0 }, mini = { INF, INF };
      for ( int j = 1; j < 10000; j++ )
      {
        auto i = key_frame[j];
        if ( i.frame_no == 0 ) continue;
        if ( i.frame_no < mmd_data->now_frame )
        {
          maxi = std::max(maxi, { i.frame_no, j });
        }
        else
        {
          mini = std::min(mini, { i.frame_no, j });
        }
      }
      camera.changed_data.pre_index = maxi.second;
      camera.changed_data.is_perspective = key_frame[maxi.second].next_index;
      if ( mini.first != INF )
      {
        camera.changed_data.next_index = mini.second;
        camera.changed_data.is_selected = key_frame[mini.second].pre_index;
      }
      else
      {
        camera.changed_data.next_index = -1;
      }
    }
    else
    {
      camera.state = CameraUndoRedo::Overwrite;
      camera.changed_data.xyz = mmd_data->xyz;
      camera.changed_data.rxyz = mmd_data->rxyz;
    }
    camera.left_frame = mmd_data->left_frame;
    push(2, camera);
    printf("push%d %f %f %f\n", now_undo_cnt, mmd_data->xyz.x, mmd_data->xyz.y, mmd_data->xyz.z);
  }

  void pushCurent(CameraUndoRedo::State state = CameraUndoRedo::Key, int num = 1)
  {
    auto mmd_data = getMMDMainData();
    CameraUndoRedo camera;
    camera.state = state;
    camera.changed_data = mmd_data->camera_key_frame[mmd_data->now_frame];
    camera.changed_data.xyz = mmd_data->xyz;
    camera.changed_data.rxyz = mmd_data->rxyz;
    camera.changed_data.length = mmd_data->length;
    camera.index = -1;
    camera.changed_data.frame_no = mmd_data->now_frame;
    camera.left_frame = mmd_data->left_frame;
    push(num, camera);
    printf("push%d %f %f %f\n", now_undo_cnt, mmd_data->xyz.x, mmd_data->xyz.y, mmd_data->xyz.z);
    is_update = true;
  }

  void mouseUpdate(WPARAM wparam, const MOUSEHOOKSTRUCT* lparam)
  {
    //print();
    if ( lparam->wHitTestCode != HTCLIENT ) return;
    static int lbuttonup = false;
    auto mmd_data = getMMDMainData();
    if ( now_frame == -1 )
    {
      now_frame = mmd_data->now_frame;
      list.push_back({ 1,{ mmd_data->camera_key_frame[0],-1 } });
      now_undo_cnt++;
    }
    if ( mmd_data->select_bone_type != MMDMainData::SelectBoneType::Camera ) return;

    if ( wparam == WM_MOUSEWHEEL )
    {
      if ( list.back().second.state == CameraUndoRedo::Wheel )
      {
        list.back().second.changed_data.length = mmd_data->length;
      }
      else
      {
        pushCurent(CameraUndoRedo::Wheel);
      }
    }
    if ( wparam == WM_LBUTTONDOWN || wparam == WM_RBUTTONDOWN || wparam == WM_MBUTTONDOWN )
    {
      xyz = mmd_data->xyz;
      rxyz = mmd_data->rxyz;
      lbuttonup = true;
    }
    if ( lbuttonup && (wparam == WM_LBUTTONUP || wparam == WM_RBUTTONUP || wparam == WM_MBUTTONUP) )
    {
      if ( mmd_data->xyz != xyz || mmd_data->rxyz != rxyz )
      {
        pushCurent();
      }
      lbuttonup = false;
    }
  }


  void undo()
  {
    if ( now_undo_cnt > 1 )
    {
      for ( int i = list[now_undo_cnt - 2].first - 1; i >= 0; i-- )
      {
        now_undo_cnt--;
        is_update = true;
        printf("undo%d\n", now_undo_cnt);
        list[now_undo_cnt - 1].second.load();
      }
    }
    //auto updateFunc = (void(*)())((BYTE*) GetModuleHandleW(nullptr) + 0x64110);
    //updateFunc();
  }

  void redo()
  {
    if ( now_undo_cnt < list.size() )
    {
      for ( int i = 0, len = list[now_undo_cnt - 1].first; i < len; i++ )
      {
        printf("redo%d\n", now_undo_cnt);
        list[now_undo_cnt - 1].second.load();
        now_undo_cnt++;
      }
    }

    //auto updateFunc = (void(*)())((BYTE*) GetModuleHandleW(nullptr) + 0x64110);
    //updateFunc();
  }
};

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
    camera_undo_redo.keyUpdate(wParam, lParam);
  }

  void MouseProc(WPARAM wParam, const MOUSEHOOKSTRUCT* param) override
  {
    camera_undo_redo.mouseUpdate(wParam, param);
  }

private:
  CameraKeyFrameUndoRedo camera_undo_redo;
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
