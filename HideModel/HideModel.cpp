// HideModel.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "../MMDUtility/mmdplugin/mmd_plugin.h"
#include "../MMDUtility/mmd_utility.h"


class HideModel : public MMDPluginDLL3
{
  control::MenuCheckBox* check = nullptr;
  bool pre_check = false;
public:

  const char* getPluginTitle() const override { return "MMDUtility_HideModel"; }

  void start() override
  {
    auto utility = MMDUtility::getObject();
    if ( utility == nullptr ) return;

    auto ctrl = utility->getControl();
    check = new control::MenuCheckBox(ctrl);

    check->command = [=](auto)
      {
        check->reverseCheck();
      };

    auto menu = utility->getUitilityMenu();
    menu->AppendChild(L"選択モデル以外非表示", check);
    menu->AppendSeparator();
  }

  void BeginScene() override
  {
    auto mmd = mmp::getMMDMainData();
    if ( check == nullptr || check->isChecked() == false || mmd->select_bone_type == mmp::MMDMainData::SelectBoneType::Camera )
    {
      if ( pre_check == true )
      {
        auto now_frame = mmd->now_frame;
        for ( int i = 0; i < 255; i++ )
        {
          auto model = mmd->model_data[i];
          if ( model == nullptr ) continue;
          auto key_frame = model->configuration_keyframe;
          auto it = key_frame;
          if ( it->next_index == 0 )
          {
            model->is_visible = it->is_visible;
          }
          for ( ; it->next_index; it = key_frame + it->next_index )
          {
            if ( now_frame < key_frame[it->next_index].frame_number )
            {
              model->is_visible = it->is_visible;
              break;
            }
          }
        }
        pre_check = false;
      }
      return;
    }

    int selected_model = mmd->select_model;

    for ( int i = 0; i < 255; i++ )
    {
      auto model = mmd->model_data[i];
      if ( model == nullptr ) continue;
      model->is_visible = false;
      if ( i == selected_model )
      {
        auto now_frame = mmd->now_frame;
        auto key_frame = model->configuration_keyframe;
        auto it = key_frame;
        if ( it->next_index == 0 )
        {
          model->is_visible = it->is_visible;
        }
        for ( ; it->next_index; it = key_frame + it->next_index )
        {
          if ( now_frame < key_frame[it->next_index].frame_number )
          {
            model->is_visible = it->is_visible;
            break;
          }
        }
      }
    }
    pre_check = true;
  }

  void KeyBoardProc(WPARAM wParam, LPARAM lParam) override
  {
    if ( wParam == 'E' )
    {
      if ( (lParam & 0x80000000) == 0 )
      {
        check->reverseCheck();
      }
    }
  }
};

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9*)
{
  return new HideModel();
}
