#include "mmd_utility.h"
#ifdef NDEBUG
#define printf(...) (void)0

#endif // !NDEBUG

namespace control
{
  int ReferenceCounter::AddRef() const
  {
    ref_cnt_++;
    return ref_cnt_;
  }

  int ReferenceCounter::Release() const
  {
    if ( --ref_cnt_ == 0 )
    {
      delete this;
      return 0;
    }
    return ref_cnt_;
  }

  bool MenuFlag::isGrayed() const { return (flag_ & MF_GRAYED) != 0; }

  bool MenuFlag::isDisabled() const { return (flag_ & MF_DISABLED) != 0; }

  bool MenuFlag::isBitmap() const { return (flag_ & MF_BITMAP) != 0; }

  bool MenuFlag::isPopUp() const { return (flag_ & MF_POPUP) != 0; }

  bool MenuFlag::isHilite() const { return (flag_ & MF_HILITE) != 0; }

  bool MenuFlag::isOwnerDraw() const { return (flag_ & MF_OWNERDRAW) != 0; }

  bool MenuFlag::isSystemMenu() const { return (flag_ & MF_SYSMENU) != 0; }

  bool MenuFlag::isMouseSelect() const { return (flag_ & MF_MOUSESELECT) != 0; }

  struct IMenu::Pimpl
  {
    int id_;
    HMENU menu_handle_ = ::CreateMenu();
    std::vector<IMenu*> child_menu_;
    HWND window_handle_ = nullptr;
    Type type;
  };

  class Control
  {
    using ID = int;
    std::unordered_map<ID, IMenu*> menu_;
    int menu_select_id_ = -1;
    Control(const Control&) = delete;
    void operator=(const Control&) = delete;
  public:
    Control() = default;

    void AddObj(IMenu* menu);

    void WndProc(int code, const MSG* param);
  };

  IMenu::IMenu(Control* ctrl): IMenu(ctrl, createWM_APP_ID()) { }

  IMenu::IMenu(Control* ctrl, int id)
  {
    pimpl = new Pimpl();
    pimpl->id_ = id;
    pimpl->type = Type::List;
    ctrl->AddObj(this);
  }

  void IMenu::SetType(Type t)
  {
    pimpl->type = t;
  }

  HMENU IMenu::getMenuHandle() const { return pimpl->menu_handle_; }

  int IMenu::id() const { return pimpl->id_; }

  void IMenu::SetWindow(HWND hwnd, LPWSTR lpszItemName)
  {
    pimpl->window_handle_ = hwnd;
    auto hmenu = GetMenu(hwnd);
    MENUITEMINFOW mii;

    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_ID | MIIM_TYPE;
    mii.wID = pimpl->id_;
    mii.fType = MFT_STRING;
    mii.dwTypeData = lpszItemName;
    mii.hSubMenu = pimpl->menu_handle_;
    mii.fMask |= MIIM_SUBMENU;

    InsertMenuItemW(hmenu, mii.wID, FALSE, &mii);
  }

  IMenu::~IMenu() {}

  void IMenu::AppendChild(MENUITEMINFOW& mii)
  {
    InsertMenuItemW(pimpl->menu_handle_, 0, FALSE, &mii);
  }

  void IMenu::AppendSeparator()
  {
    MENUITEMINFOW mii;

    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_ID | MIIM_TYPE;
    mii.wID = 0;
    mii.fType = MFT_SEPARATOR;
    mii.hSubMenu = nullptr;

    InsertMenuItemW(pimpl->menu_handle_, 0, FALSE, &mii);
  }

  void IMenu::AppendChild(LPWSTR lpszItemName, IMenu* hmenuSub)
  {
    assert(lpszItemName != NULL);
    MENUITEMINFOW mii;

    mii.cbSize = sizeof(MENUITEMINFOW);
    mii.fMask = MIIM_ID | MIIM_TYPE;

    mii.fType = MFT_STRING;
    mii.dwTypeData = lpszItemName;

    if ( hmenuSub != nullptr )
    {
      hmenuSub->AddRef();
      mii.wID = hmenuSub->id();
      pimpl->child_menu_.push_back(hmenuSub);
      switch ( hmenuSub->pimpl->type )
      {
      case Type::List:
        mii.fMask |= MIIM_SUBMENU;
        mii.hSubMenu = hmenuSub->getMenuHandle();
        break;
      case Type::Command:
        mii.fType |= MFT_RADIOCHECK;
        break;
      case Type::CheckBox:

        break;
      }
    }
    else
    {
      mii.wID = createWM_APP_ID();
    }

    InsertMenuItemW(pimpl->menu_handle_, mii.wID, FALSE, &mii);
  }

  void MenuCheckBox::check(bool is_check)
  {
    CheckMenuItem(GetMenu(getHWND()), id(), MF_BYCOMMAND | (is_check ? MF_CHECKED : MF_UNCHECKED));
  }

  void MenuCheckBox::reverseCheck() { check(!isChecked()); }

  bool MenuCheckBox::isChecked() const
  {
    auto uState = GetMenuState(GetMenu(getHWND()), id(), MF_BYCOMMAND);
    return (uState & MFS_CHECKED) != 0;
  }

  void Control::AddObj(IMenu* menu)
  {
    if ( menu_.insert({ menu->id(), menu }).second == false ) throw std::runtime_error("すでにIDが使われています。");
  }

  void Control::WndProc(int /*code*/, const MSG* param)
  {
    //menu_select_id_ = -1;
    switch ( param->message )
    {
    case WM_LBUTTONUP:
    {
      auto it = menu_.find(menu_select_id_);
      if ( it != menu_.end() )
      {
        IMenu::CommandArgs args;
        args.window_hwnd = param->hwnd;
        args.item_id = LOWORD(param->wParam);
        args.notify_code = HIWORD(param->wParam);
        args.control_hwnd = (HWND) param->lParam;
        it->second->Command(args);
      }
      break;
    }
    case WM_MENUSELECT:
    {
      menu_select_id_ = LOWORD(param->wParam);
      auto it = menu_.find(menu_select_id_);
      if ( it != menu_.end() )
      {
        it->second->MenuSelect(param->hwnd, LOWORD(param->wParam), { HIWORD(param->wParam) }, (HMENU) param->lParam);
      }
      break;
    }
    default:
      break;
    }
  }
}

MMDUtility::MMDUtility(IDirect3DDevice9* device) : device_(device)
{
  auto hwnd = getHWND();
  ctrl_ = new control::Control();
  auto menu = new control::MenuDelegate(ctrl_);
  menu->SetWindow(hwnd, L"MMDUtility");
  top_menu_ = menu;
  DrawMenuBar(hwnd);
}

void MMDUtility::WndProc(const CWPSTRUCT* param)
{
  //ctrl_.WndProc(param);
  switch ( param->message )
  {
  case WM_COMMAND:
    //printf("%x %d %d %d %d %d\n", param->message, HIWORD(param->lParam),LOWORD(param->lParam),HIWORD(param->wParam), LOWORD(param->wParam));

  default:
    break;
  }
  //printf("%x %x %x\n", param->message, param->lParam, param->wParam);
}

void MMDUtility::MsgProc(int code, const MSG* param)
{
  ctrl_->WndProc(code, param);
  //printf("%x %lld %lld\n", param->message, param->lParam, param->wParam);
}

std::pair<bool, LRESULT> MMDUtility::WndProc(HWND, UINT, WPARAM, LPARAM) { return { false,0 }; }

void OpenConsole()
{
  FILE *in, *out;
  AllocConsole();
  freopen_s(&out, "CONOUT$", "w", stdout);//CONOUT$
  freopen_s(&in, "CONIN$", "r", stdin);
}

int version() { return 3; }

MMDPluginDLL3* create3(IDirect3DDevice9* device)
{
#ifndef NDEBUG
  //OpenConsole();
#endif // NDEBUG
  return new MMDUtility(device);
}

void destroy3(MMDPluginDLL3* p)
{
  delete p;
}
