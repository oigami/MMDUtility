// AutoSave.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"
#include <io.h>
#include <fcntl.h>
#include <mutex>
#include <experimental/filesystem>
#include <fstream>
#include <iomanip>
#include <atomic>
namespace filesystem = std::experimental::filesystem;
using char8 = std::int8_t;
extern HMODULE g_module;
static_assert(sizeof(float) == 4, "");
#pragma pack(push, 1) // アライメント1に設定
struct PMMFile
{
  /* ヘッダ */
  char8 format_ID[30];
  std::int32_t view_width;
  std::int32_t view_height;
  std::int32_t frame_width;
  float edit_view_angle;
  BYTE __unknown10[7];

  /* モデル */

  BYTE model_count;

  struct ModelData
  {
    std::int32_t number;
    std::string name;
    std::string name_en;
    char8 path[256];
    BYTE __unknown;
    std::int32_t bone_count;
    std::vector<std::string> bone_name; // [bone_count]
    std::int32_t morph_count;
    std::vector<std::string> morph_name; // [morph_count]
    std::int32_t ik_count;
    std::vector<std::int32_t> ik_index; // [ik_count]
    std::int32_t op_count; // 外部親設定
    std::vector<std::int32_t> op_index; // [op_count]
    BYTE draw_order;
    BYTE edit_is_display;
    std::int32_t edit_selected_bone;
    std::int32_t skin_panel[4];
    BYTE frame_count;
    std::vector<BYTE> is_frame_open; // [frame_count]
    std::int32_t vscroll;
    std::int32_t last_frame;

    struct BoneInitFrame
    {
      std::int32_t frame_number;
      std::int32_t pre_index;
      std::int32_t next_index;
      BYTE interpolation_x[4];
      BYTE interpolation_y[4];
      BYTE interpolation_z[4];
      BYTE interpolation_rotation[4];
      float translation[3];
      float rotation_q[4];
      BYTE is_selected;
      BYTE physical_disabled;
    };

    std::vector<BoneInitFrame> bone_init_frame; // [bone_count]
    struct BoneKeyFrame
    {
      std::int32_t data_index;
      BoneInitFrame data;
    };

    std::int32_t bone_key_frame_count;
    std::vector<BoneKeyFrame> bone_key_frame; // [bone_key_frame_count]
    struct MorphInitFrame
    {
      std::int32_t frame_number;
      std::int32_t pre_index;
      std::int32_t next_index;
      float value;
      BYTE is_selected;
    };

    std::vector<MorphInitFrame> morph_init_frame;

    struct MorphKeyFrame
    {
      std::int32_t data_index;
      MorphInitFrame data;
    };

    std::int32_t morph_key_frame_count;
    std::vector<MorphKeyFrame> morph_key_frame;

    struct OPInitFrame
    {
      std::int32_t frame_number;
      std::int32_t pre_index;
      std::int32_t next_index;
      BYTE is_display;
      std::vector<BYTE> is_ik_enabled; // [ik_count]
      std::vector<std::int32_t[2]> op_data; // [op_count]
      BYTE is_selected;
    };

    std::vector<OPInitFrame> op_init_frame;

    struct OPKeyFrame
    {
      std::int32_t data_index;
      OPInitFrame data;
    };

    std::int32_t op_key_frame_count;
    std::vector<OPKeyFrame> op_key_frame; // [op_key_frame_count]

    struct BoneCurrentData
    {
      float translation[3];
      float rotatation_q[4];
      BYTE is_edit_uncomitted;
      BYTE is_physics_disabled;
      BYTE is_row_selected;
    };

    std::vector<BoneCurrentData> bone_current_data; // [bone_count]

    std::vector<float> morph_current_data; // [morph_count]

    std::vector<BYTE> is_ik_enabled;

    struct OPCurrentData
    {
      std::int32_t key_frame_begin;
      std::int32_t key_frame_end;
      std::int32_t model_index;
      std::int32_t parent_bone_index;
    };

    std::vector<OPCurrentData> op_current_data; // [op_count]

    BYTE is_blend; // 加算合成
    float edge_width;
    BYTE is_self_shadow_enabled;
    BYTE calc_order;
  };

  std::vector<ModelData> model_data; // [model_count]

  /* カメラ */

  struct CameraInitFrame
  {
    std::int32_t frame_number;
    std::int32_t pre_index;
    std::int32_t next_index;
    std::int32_t distance;
    float eye_position[3];
    float rotation[3];
    std::int32_t looking_model_index; // 非選択時-1
    std::int32_t looking_bone_index;
    BYTE interpolation_x[4];
    BYTE interpolation_y[4];
    BYTE interpolation_z[4];
    BYTE interpolation_rotation[4];
    BYTE interpolation_distance[4];
    BYTE interpolation_angle_view[4];
    BYTE is_parse;
    std::int32_t angle_view;
    BYTE is_selected;
  };

  CameraInitFrame camera_init_frame;
  std::int32_t camera_key_frame_count;

  struct CameraKeyFrame
  {
    std::int32_t data_index;
    CameraInitFrame data;
  };

  std::vector<CameraKeyFrame> camera_key_frame; // [camera_key_frame_count]

  struct CameraCurrentData
  {
    float eye_position[3];
    float target_position[3];
    float rotation[3];
  };

  CameraCurrentData camera_current_data;

  /* ライティング */
  struct LightInitFrame
  {
    std::int32_t frame_number;
    std::int32_t pre_index;
    std::int32_t next_index;
    float r, g, b;
    float x, y, z;;
    BYTE is_selected;
  };

  std::int32_t light_key_frame_count;

  struct LightKeyFrame
  {
    std::int32_t data_index;
    LightInitFrame data;
  };

  std::vector<LightKeyFrame> light_key_frame; // [light_key_frame_count]

  struct LightCurrentData
  {
    float r, g, b;
    float x, y, z;
    BYTE is_selected;
  };

  LightCurrentData light_current_data;


  /* アクセサリ */
  BYTE select_accessory_index;
  std::int32_t vscroll;
  BYTE accessory_count;

  std::vector<char[100]> accessory_name;

  struct AccData
  {
    BYTE index;
    char name[100];
    char path[256];
    BYTE draw_order;

    struct DataBody
    {
      BYTE opacity : 7;
      BYTE is_visible : 1;
      std::int32_t parent_model_index;
      std::int32_t parent_bone_index;
      float translation[3];
      float rotation[3];
      float scale;
      BYTE is_shadow_enabled;
    };

    static_assert(sizeof(DataBody) == 38, "a");

    struct InitFrame
    {
      std::int32_t frame;
      std::int32_t pre_index;
      std::int32_t next_index;
      DataBody body;
      BYTE is_selected;
    };

    std::int32_t key_frame_count;

    struct KeyFrame
    {
      std::int32_t data_index;
      InitFrame data;
    };

    std::vector<KeyFrame> key_frame; // [key_frame_count]

    DataBody accessory_current_data;

    BYTE is_blend; // 加算合成
  };

  std::vector<AccData> accessory_data; // [accessory_count]

  std::int32_t current_frame_number;
  std::int32_t hscroll_frame_number;
  std::int32_t hscroll_frame_scale;
  std::int32_t bone_operation_kind;

  enum class LookingAt : std::int32_t
  {
    None,
    Model,
    Bone,
  };

  LookingAt looking_at;
  BYTE is_repeat;

  BYTE play_from_frame;
  BYTE play_to_frame;
  std::int32_t play_start_frame;
  std::int32_t play_end_frame;

  BYTE is_wav_enabled;
  char8 wav_path[256];

  std::int32_t avi_offset_x;
  std::int32_t avi_offset_y;
  float avi_scale;
  char8 avi_path[256];
  std::int32_t is_show_avi; // 1で表示、それ以外は非表示

  std::int32_t bgimage_offset_x;
  std::int32_t bgimage_offset_y;
  float bgimage_scale;
  char8 bgimage_path[256];
  BYTE is_show_bgimage;

  BYTE is_show_information;
  BYTE is_show_axis;
  BYTE is_show_ground_shadow;
  float fps_limit;
  std::int32_t screen_caputure_mode;
  std::int32_t accessory_number_render_after_models;
  float ground_shadow_brightness;
  BYTE is_transparent_ground_shadow;
  BYTE physics_mode;

  /* 重力 */
  struct GravityCurrentData
  {
    float acceleration;
    std::int32_t noize_amount;
    float direction_x;
    float direction_y;
    float direction_z;
    BYTE add_noize;
  };

  GravityCurrentData gravity_current_data;

  struct GravityInitFrame
  {
    std::int32_t frame_number;
    std::int32_t pre_index;
    std::int32_t next_index;
    BYTE add_noize;
    std::int32_t noize_amount;
    float acceleration;
    float direction_x;
    float direction_y;
    float direction_z;
    BYTE is_selected;
  };

  GravityInitFrame gravity_init_data;

  std::int32_t gravity_key_frame_count;

  struct GravityKeyFrame
  {
    std::int32_t data_index;
    GravityInitFrame data;
  };

  std::vector<GravityKeyFrame> gravity_key_frame; // [gravity_key_frame_count]

  BYTE is_show_self_shadow;

  float self_shadow_current_distance;

  struct SelfShadowInitFrame
  {
    std::int32_t frame_number;
    std::int32_t pre_index;
    std::int32_t next_index;
    BYTE mode;
    float distance;
    BYTE is_selected;
  };

  SelfShadowInitFrame self_shadow_init_frame;
  std::int32_t self_shadow_key_frame_count;

  struct SelfShadowKeyFrame
  {
    std::int32_t data_index;
    SelfShadowInitFrame data;
  };

  std::vector<SelfShadowKeyFrame> self_shadow_key_frame; // [self_shadow_key_frame]

  std::int32_t edge_color_r;
  std::int32_t edge_color_g;
  std::int32_t edge_color_b;

  BYTE is_black_background;

  std::int32_t camera_current_looking_model;
  std::int32_t camera_current_looking_bone;

  float unknown[4 * 4];

  BYTE is_view_lookat_enabled;

  BYTE unknown10;

  BYTE is_physics_ground_enabled;

  std::int32_t frame_textbox;

  BYTE selector_choice_selection_following;

  struct SelectorChoiceSelection
  {
    BYTE model_index;
    std::int32_t selector_choice;
  };

  std::vector<SelectorChoiceSelection> selector_choice_selection; // [model_count]
};


#pragma pack(pop)
namespace
{
  std::atomic<bool> is_saving;
  int max_file_num_ = 0;
}

class AutoSave :public MMDPluginDLL3
{
public:
  bool is_runnable = false;
  std::chrono::system_clock::time_point start_time = std::chrono::system_clock::now();
  std::chrono::minutes interval_time;

  static std::size_t fileCountNative(const filesystem::path& path)
  {
    WIN32_FIND_DATA wfd = {};
    HANDLE handle = FindFirstFile(path.c_str(), &wfd);
    if ( handle == INVALID_HANDLE_VALUE ) return 0;

    std::size_t result = 0;
    do
    {
      if ( lstrcmpW(wfd.cFileName, L".") != 0 && lstrcmpW(wfd.cFileName, L"..") != 0 )
      {
        ++result;
        //if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) result += fileCountNative(src + ('\\') + path);
      }
    }
    while ( FindNextFile(handle, &wfd) );
    FindClose(handle);

    return result;
  }

  static void maxFileErase(const filesystem::path& path)
  {
    auto size = fileCountNative(path);
    if ( size < max_file_num_ ) return;
    filesystem::directory_iterator it(path.parent_path()), last;
    for ( ; max_file_num_ < size && it != last; ++it )
    {
      if ( it->path().has_extension() && it->path().extension() == L".pmm" )
      {
        filesystem::remove(it->path());
        size--;
      }
    }
  }

  void save()
  {
    if ( is_runnable == false ) return;
    auto save_func = (void(*)(mmp::MMDMainData*))((char*) GetModuleHandleW(nullptr) + 0x750D0);
    is_saving = true;
    save_func(mmp::getMMDMainData());
  }

  void start() override
  {
    auto path = mmp::getDLLPath(g_module).parent_path();
    path /= L"autosave/";
    if ( filesystem::exists(path) == false )
    {
      filesystem::create_directories(path);
    }
    path /= L"setting.txt";
    if ( filesystem::exists(path) == false )
    {
      std::ofstream ofs(path);
      ofs << 5 << std::endl;
      ofs << 50 << std::endl;
    }
    std::ifstream ifs(path);
    int interval_minute;
    ifs >> interval_minute >> max_file_num_;
    if ( interval_minute >= 1 && max_file_num_ >= 0 )
    {
      is_runnable = true;
    }
    interval_time = std::chrono::minutes(interval_minute);
    ifs.close();

    auto utility = dynamic_cast<MMDUtility*>(mmp::getDLL3Object("MMDUtility"));
    if ( utility == nullptr ) return;

    auto menu = utility->getUitilityMenu();
    auto ctrl = utility->getControl();
    auto save_menu = new control::MenuDelegate(ctrl);
    save_menu->SetType(control::IMenu::Type::Command);
    save_menu->command = [this](auto args)
      {
        save();
      };
    menu->AppendChild(L"保存", save_menu);
    menu->AppendSeparator();
  }

  const char* getPluginTitle() const override { return "MMDUtlity_AutoSave"; }

  void MouseProc(WPARAM, const MOUSEHOOKSTRUCT*) override
  {
    auto end = std::chrono::system_clock::now();

    auto diff = end - start_time;
    if ( diff > interval_time )
    {
      start_time = std::chrono::system_clock::now();
      save();
    }
  }
};

namespace
{
  std::ofstream save_writer;
  mmp::WinAPIHooker<decltype(_wsopen_s)*> f_wsopen_s;
  char save_buf[1024 * 1024 * 10];

  errno_t Mywsopen_s(int* pfh, const wchar_t* filename, int oflag, int shflag, int pmode)
  {
    filesystem::path path = filename;
    if ( oflag & _O_WRONLY && pfh && path.has_extension() && path.extension() == ".pmm" && is_saving.exchange(false) )
    {
      *pfh = -1;
      filesystem::path save_path = mmp::getDLLPath(g_module).parent_path();
      const auto fname = path.filename().stem();
      save_path /= L"autosave/";
      save_path /= fname;
      AutoSave::maxFileErase(save_path / L"*.pmm");

      if ( filesystem::exists(save_path) == false )
      {
        filesystem::create_directories(save_path);
      }

      wchar_t buf[256];
      const time_t now_time = std::time(nullptr);
      const tm* now = std::localtime(&now_time);
      int len = swprintf_s(buf, L"_%d-%02d-%02d_%02d-%02d-%02d.pmm", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
      save_path /= fname;
      save_path.concat(buf, buf + len);
      std::ofstream ofs(save_path, std::ios::binary);
      ofs.rdbuf()->pubsetbuf(save_buf, sizeof(save_buf));
      if ( ofs.is_open() == false )
      {
        MessageBoxW(nullptr, L"保存ファイルの作成に失敗しました", L"MMDUtility AutoSave", MB_OK);
      }
      save_writer = std::move(ofs);
      return 0;
    }
    return f_wsopen_s(pfh, filename, oflag, shflag, pmode);
  }

  mmp::WinAPIHooker<decltype(_write)*> f_write;

  int __cdecl MyWrite(int fd, const void* buffer, unsigned int count)
  {
    if ( -1 == fd )
    {
      save_writer.write((const char*) buffer, count);
      return count;
    }
    return f_write(fd, buffer, count);
  }

  mmp::WinAPIHooker<decltype(_close)*> f_close;


  errno_t Myclose(int pfh)
  {
    if ( -1 == pfh )
    {
      std::thread flush_thread([]()
        {
          save_writer.flush();
          save_writer.close();
        });
      flush_thread.detach();
      return 0;
    }
    return f_close(pfh);
  }
}

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
  f_write.hook("MSVCR90.DLL", "_write", MyWrite);
  f_wsopen_s.hook("MSVCR90.DLL", "_wsopen_s", Mywsopen_s);
  f_close.hook("MSVCR90.DLL", "_close", Myclose);

#ifndef NDEBUG
  OpenConsole();
#endif // !NDEBUG
  return new AutoSave();
}
