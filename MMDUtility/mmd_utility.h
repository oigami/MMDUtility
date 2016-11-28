#pragma once
// MMDUtility.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"


#include "mmdplugin/mmd_plugin.h"
#ifdef MAKE_MMD_UTILITY
# define MMD_UTILITY_DLL_FUNC_API __declspec(dllexport)
#else
# define MMD_UTILITY_DLL_FUNC_API __declspec(dllimport)
#pragma comment(lib,"MMDUtility")
#endif // MAKE_MMD_UTILITY


#include <d3d9.h>

namespace control
{
  struct MenuFlag
  {
    UINT flag_;

    bool isGrayed() const;

    bool isDisabled() const;

    bool isBitmap() const;

    bool isPopUp() const;

    bool isHilite() const;

    bool isOwnerDraw() const;

    bool isSystemMenu() const;

    bool isMouseSelect() const;
  };

  class IMenu
  {
    int id_;
    HMENU menu_handle_ = ::CreateMenu();
    std::vector<std::shared_ptr<IMenu>> child_menu_;
    HWND window_handle_ = nullptr;

  protected:
    IMenu();

    explicit IMenu(int id);

  public:
    enum class Type
    {
      CheckBox,
      List,
      Command,
    };

    Type type = Type::List;

    MMD_UTILITY_DLL_FUNC_API HMENU getMenuHandle() const;

    MMD_UTILITY_DLL_FUNC_API int id() const;

    void SetWindow(HWND hwnd, LPWSTR lpszItemName);

    virtual ~IMenu();

    virtual void MenuSelect(HWND hwnd, UINT item_id_or_index, MenuFlag flag, HMENU hMenu) = 0;

    struct CommandArgs
    {
      HWND window_hwnd;
      UINT item_id;
      int notify_code;
      HWND control_hwnd;
    };

    virtual void Command(const CommandArgs& args) = 0;

    MMD_UTILITY_DLL_FUNC_API void AppendChild(MENUITEMINFOW& mii);

    MMD_UTILITY_DLL_FUNC_API void AppendSeparator();

    MMD_UTILITY_DLL_FUNC_API void AppendChild(LPWSTR lpszItemName, std::shared_ptr<IMenu> hmenuSub);
  };


  struct MenuDelegate : IMenu
  {
    void MenuSelect(HWND hwnd, UINT item_id_or_index, MenuFlag flag, HMENU hMenu) override
    {
      if ( menu_select ) menu_select(hwnd, item_id_or_index, flag, hMenu);
    }

    void Command(const CommandArgs& args) override
    {
      if ( command ) command(args);
    }

    std::function<void(const CommandArgs&)> command;
    std::function<void(HWND, UINT, MenuFlag, HMENU)> menu_select;
  };

  struct MenuCheckBox : MenuDelegate
  {
    MMD_UTILITY_DLL_FUNC_API MenuCheckBox();

    MMD_UTILITY_DLL_FUNC_API void check(bool is_check);

    MMD_UTILITY_DLL_FUNC_API void reverseCheck();

    MMD_UTILITY_DLL_FUNC_API bool isChecked() const;
  };

  class Control
  {
    using ID = int;
    std::unordered_map<ID, std::shared_ptr<IMenu>> menu_;
    int menu_select_id_ = -1;
    Control(const Control&) = delete;
    void operator=(const Control&) = delete;
  public:
    Control() = default;

    template<class T>
    std::shared_ptr<T> createMenu();

    MMD_UTILITY_DLL_FUNC_API std::shared_ptr<MenuDelegate> createMenu();

    MMD_UTILITY_DLL_FUNC_API std::shared_ptr<MenuDelegate> createMenuCommand();

    MMD_UTILITY_DLL_FUNC_API std::shared_ptr<MenuCheckBox> createMenuCheckBox();

    MMD_UTILITY_DLL_FUNC_API void WndProc(int code, const MSG* param);
  };
}


class MMDUtility : public MMDPluginDLL3
{
  control::Control ctrl_;
  std::shared_ptr<control::IMenu> top_menu_;
public:

  control::Control& getControl() { return ctrl_; }

  std::shared_ptr<control::IMenu> getUitilityMenu() const { return top_menu_; }


  explicit MMDUtility(IDirect3DDevice9* device);

  void WndProc(const CWPSTRUCT* param) override;

  void WndProc(int code, const MSG* param) override;

  std::pair<bool, LRESULT> WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

  const char* getPluginTitle() const override { return "MMDUtility"; }

private:

  IDirect3DDevice9* device_;
};

void OpenConsole();
