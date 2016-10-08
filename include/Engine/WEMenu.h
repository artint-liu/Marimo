#ifndef _GRAP_X_WINE_MENU_H_
#define _GRAP_X_WINE_MENU_H_

class GAMEENGINE_API CWEMenu
{
private:
  GXHMENU    m_hMenu;
public:
  GXHMENU    Get();
  CWEMenu(GXHWND hWnd);
};

#endif // _GRAP_X_WINE_MENU_H_