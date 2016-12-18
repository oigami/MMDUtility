// CameraUndo.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
using namespace mmp;

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
    auto mmd_data = mmp::getMMDMainData();
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
