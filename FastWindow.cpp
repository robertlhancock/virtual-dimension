/* 
 * Fast Window - A fast and convenient message dispatching window procedure.
 * Copyright (C) 2003 Francois Ferrand
 *
 * This program is free software; you can redistribute it and/or modify it under 
 * the terms of the GNU General Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple 
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
 
#include "StdAfx.h"
#include "fastwindow.h"

FastWindow::FastWindow()
{
   m_hWnd = NULL;
}

FastWindow::~FastWindow(void)
{
   if (IsWindow(m_hWnd)) 
      DestroyWindow(m_hWnd);

   m_commandMap.clear();
   m_messageMap.clear();
   m_notifyMap.clear();
   m_syscommandMap.clear();
}

HWND FastWindow::Create( LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, 
                         int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance)
{ 
   if (IsValid())
      return NULL;

   m_hWnd = ::CreateWindow( lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, 
                            hWndParent, hMenu, hInstance, this);

   return m_hWnd;
}

HWND FastWindow::Create( DWORD dwStyleEx, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, 
                         int x, int y, int nWidth, int nHeight, 
                         HWND hWndParent, HMENU hMenu, HINSTANCE hInstance)
{
   if (IsValid())
      return NULL;

   m_hWnd = ::CreateWindowEx( dwStyleEx, lpClassName, lpWindowName, dwStyle, x, y, 
                              nWidth, nHeight, hWndParent, hMenu, hInstance, this); 

   return m_hWnd;
}

ATOM FastWindow::RegisterClassEx(LPWNDCLASSEX lpwcex)
{
   lpwcex->lpfnWndProc = (WNDPROC)WndProc;
   if (lpwcex->cbWndExtra != 0)
   {
      MessageBox(NULL, "FastWindow does not support user data", "error", MB_OK);
      return 0;
   }
   lpwcex->cbWndExtra = sizeof(FastWindow*);
   return ::RegisterClassEx(lpwcex);
}

ATOM FastWindow::RegisterClass(LPWNDCLASS lpwc)
{
   lpwc->lpfnWndProc	= (WNDPROC)WndProc;
   if (lpwc->cbWndExtra != 0)
   {
      MessageBox(NULL, "FastWindow does not support user data", "error", MB_OK);
      return 0;
   }
   lpwc->cbWndExtra = sizeof(FastWindow*);
   return ::RegisterClass(lpwc);
}

LRESULT CALLBACK FastWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   FastWindow * window = (FastWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

   //Perform initialization if it has not been done yet
   if (window == NULL)
   {
      switch(message)
      {
      case WM_CREATE:
      case WM_NCCREATE:
         {
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;

            window = (FastWindow *)lpcs->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)window);
         }
         break;

      default:
         //Somewhat confusing !!!! We should never come here...
         return DefWindowProc(hWnd, message, wParam, lParam);
      }
   }

   //Do the actual message dispatching
   MessageMap::iterator it;
   it = window->m_messageMap.find(message);
   if (it == window->m_messageMap.end())
      return DefWindowProc(hWnd, message, wParam, lParam);
   else
      return (*it).second(hWnd, message, wParam, lParam);
}

LRESULT FastWindow::CommandHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam)
{
   MessageMap::iterator it;
   it = m_commandMap.find(wParam);
   if (it == m_commandMap.end())
      return DefWindowProc(hWnd, code, wParam, lParam);
   else
      return (*it).second(hWnd, code, wParam, lParam);
}

LRESULT FastWindow::SysCommandHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam)
{
   MessageMap::iterator it;
   it = m_syscommandMap.find(wParam);
   if (it == m_syscommandMap.end())
      return DefWindowProc(hWnd, code, wParam, lParam);
   else
      return (*it).second(hWnd, code, wParam, lParam);
}

LRESULT FastWindow::NotifyHandler(HWND hWnd, UINT code, WPARAM wParam, LPARAM lParam)
{
   MessageMap::iterator it;
   it = m_notifyMap.find(wParam);
   if (it == m_notifyMap.end())
      return DefWindowProc(hWnd, code, wParam, lParam);
   else
      return (*it).second(hWnd, code, wParam, lParam);
}