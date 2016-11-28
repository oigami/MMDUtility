#include "mmd_utility.h"
template class std::function<void(const control::IMenu::CommandArgs&)>;

bool control::MenuFlag::isGrayed() const { return (flag_ & MF_GRAYED) != 0; }

bool control::MenuFlag::isDisabled() const { return (flag_ & MF_DISABLED) != 0; }

bool control::MenuFlag::isBitmap() const { return (flag_ & MF_BITMAP) != 0; }

bool control::MenuFlag::isPopUp() const { return (flag_ & MF_POPUP) != 0; }

bool control::MenuFlag::isHilite() const { return (flag_ & MF_HILITE) != 0; }

bool control::MenuFlag::isOwnerDraw() const { return (flag_ & MF_OWNERDRAW) != 0; }

bool control::MenuFlag::isSystemMenu() const { return (flag_ & MF_SYSMENU) != 0; }

bool control::MenuFlag::isMouseSelect() const { return (flag_ & MF_MOUSESELECT) != 0; }

control::IMenu::IMenu() : id_(createWM_APP_ID()) {}

control::IMenu::IMenu(int id) : id_(id) {}

HMENU control::IMenu::getMenuHandle() const { return menu_handle_; }

int control::IMenu::id() const { return id_; }

void control::IMenu::SetWindow(HWND hwnd, LPWSTR lpszItemName)
{
  window_handle_ = hwnd;
  auto hmenu = GetMenu(hwnd);
  MENUITEMINFOW mii;

  mii.cbSize = sizeof(MENUITEMINFOW);
  mii.fMask = MIIM_ID | MIIM_TYPE;
  mii.wID = id_;
  mii.fType = MFT_STRING;
  mii.dwTypeData = lpszItemName;
  mii.hSubMenu = menu_handle_;
  mii.fMask |= MIIM_SUBMENU;

  InsertMenuItemW(hmenu, mii.wID, FALSE, &mii);
}

control::IMenu::~IMenu() {}

void control::IMenu::AppendChild(MENUITEMINFOW& mii)
{
  InsertMenuItemW(menu_handle_, 0, FALSE, &mii);
}

void control::IMenu::AppendSeparator()
{
  MENUITEMINFOW mii;

  mii.cbSize = sizeof(MENUITEMINFOW);
  mii.fMask = MIIM_ID | MIIM_TYPE;
  mii.wID = 0;
  mii.fType = MFT_SEPARATOR;
  mii.hSubMenu = nullptr;

  InsertMenuItemW(menu_handle_, 0, FALSE, &mii);
}

void control::IMenu::AppendChild(LPWSTR lpszItemName, std::shared_ptr<IMenu> hmenuSub)
{
  assert(lpszItemName != NULL);
  MENUITEMINFOW mii;

  mii.cbSize = sizeof(MENUITEMINFOW);
  mii.fMask = MIIM_ID | MIIM_TYPE;

  mii.fType = MFT_STRING;
  mii.dwTypeData = lpszItemName;

  if ( hmenuSub != nullptr )
  {
    mii.wID = hmenuSub->id();
    child_menu_.push_back(hmenuSub);
    switch ( hmenuSub->type )
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

  InsertMenuItemW(menu_handle_, mii.wID, FALSE, &mii);
}

control::MenuCheckBox::MenuCheckBox() { type = Type::CheckBox; }

void control::MenuCheckBox::check(bool is_check)
{
  CheckMenuItem(GetMenu(getHWND()), id(), MF_BYCOMMAND | (is_check ? MF_CHECKED : MF_UNCHECKED));
}

void control::MenuCheckBox::reverseCheck() { check(!isChecked()); }

bool control::MenuCheckBox::isChecked() const
{
  auto uState = GetMenuState(GetMenu(getHWND()), id(), MF_BYCOMMAND);
  return (uState & MFS_CHECKED) != 0;
}

template<class T>
std::shared_ptr<T> control::Control::createMenu()
{
  auto menu = std::make_shared<T>();
  if ( menu_.insert({ menu->id(), menu }).second == false ) throw std::runtime_error("すでにIDが使われています。");
  return menu;
}

std::shared_ptr<control::MenuDelegate> control::Control::createMenu()
{
  return createMenu<MenuDelegate>();
}

std::shared_ptr<control::MenuDelegate> control::Control::createMenuCommand()
{
  auto menu = createMenu();
  menu->type = IMenu::Type::Command;
  return menu;
}

std::shared_ptr<control::MenuCheckBox> control::Control::createMenuCheckBox()
{
  return createMenu<MenuCheckBox>();
}

void control::Control::WndProc(int /*code*/, const MSG* param)
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
    printf("WM_MENUSELECT %x %x %lld\n", param->message, LOWORD(param->wParam), param->lParam);
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

MMDUtility::MMDUtility(IDirect3DDevice9* device) : device_(device)
{
  auto hwnd = getHWND();
  auto menu = ctrl_.createMenu();
  menu->SetWindow(hwnd, L"MMDUtility");
  top_menu_ = menu;
  DrawMenuBar(hwnd);
}

void MMDUtility::WndProc(const CWPSTRUCT* /*param*/)
{
  //ctrl_.WndProc(param);
  //printf("%x %x %x\n", param->message, param->lParam, param->wParam);
}

void MMDUtility::WndProc(int code, const MSG* param)
{
  ctrl_.WndProc(code, param);
  //printf("%x %x %x\n", param->message, param->lParam, param->wParam);
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
  OpenConsole();
  return new MMDUtility(device);
}

void destroy3(MMDPluginDLL3* p)
{
  delete p;
}
