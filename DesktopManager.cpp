/* 
 * Virtual Dimension -  a free, fast, and feature-full virtual desktop manager 
 * for the Microsoft Windows platform.
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
#include "desktopmanager.h"
#include "settings.h"
#include "hotkeymanager.h"

bool deskOrder(Desktop * first, Desktop * second)
{  return first->m_index < second->m_index;  }

DesktopManager::DesktopManager(void)
{
   Settings settings;

   m_currentDesktop = NULL;

   //Load the number of columns
   m_nbColumn = settings.LoadNbCols();

   LoadDesktops();
}

DesktopManager::~DesktopManager(void)
{
   Settings settings;
   int index;

   index = 0;
   while(!m_desks.empty())
   {
      Desktop * desk;

      desk = m_desks.front();
      desk->m_index = index;

      desk->Save();

      m_desks.pop_front();
      delete desk;

      index ++;
   }

   settings.SaveNbCols(m_nbColumn);
}

void DesktopManager::resize(int width, int height)
{
   m_width = width; 
   m_height = height; 
}

void DesktopManager::paint(HDC hDc)
{
   list<Desktop*>::const_iterator it;
   int x, y;            //Position of the top/left corner of a desktop representation
   int i;
   int deltaX, deltaY;  //Width and height of a desktop

   if (m_desks.size() == 0)
      return;

   deltaX = m_width / min(m_nbColumn, (int)m_desks.size());
   deltaY = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);

   x = 0;
   y = 0;
   i = 0;
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      RECT rect;

      // Calculate boundign rectangle for the current desktop representation
      rect.top = y+2;
      rect.bottom = y+deltaY-2;
      rect.left = x+2;
      rect.right = x+deltaX-2;

      // Draw the desktop
      (*it)->Draw(hDc, &rect);

      // Calculate x and y for the next iteration
      i++;
      if (i % m_nbColumn == 0)
      {
         x = 0;
         y += deltaY;
      }
      else
         x += deltaX;
   }
}

Desktop * DesktopManager::AddDesktop(Desktop * desk)
{
   /* If needed, create the object */
   if (desk == NULL)
   {
      desk = new Desktop;
      sprintf(desk->m_name, "Desk%i", (int)m_desks.size());
   }

   /* Add the desktop to the list */
   m_desks.push_back(desk);

   /* If this is the first desktop, activate it */
   if (m_currentDesktop == NULL)
   {
      m_currentDesktop = desk;
      desk->Activate();
   }

   return desk;
}

Desktop * DesktopManager::GetFirstDesktop()
{
   m_deskIterator = m_desks.begin();

   return (m_deskIterator == m_desks.end()) ? NULL : *m_deskIterator;
}

Desktop * DesktopManager::GetNextDesktop()
{
   m_deskIterator ++;
   return (m_deskIterator == m_desks.end()) ? NULL : *m_deskIterator;
}

void DesktopManager::RemoveDesktop(Desktop * desk)
{
   /* remove from the list */
   m_desks.remove(desk);

   /* remove from the registry */
   desk->Remove();

   /* change the current desktop, if needed */
   if (m_currentDesktop == desk)
   {
      if (m_desks.empty())
         m_currentDesktop = NULL;
      else
      {
         m_currentDesktop = m_desks.front();
         m_currentDesktop->Activate();
      }
   }

   /* and remove the object from memory */
   delete desk;
}

void DesktopManager::Sort()
{
   m_desks.sort(deskOrder);
}

Desktop* DesktopManager::GetDesktopFromPoint(int X, int Y)
{
   list<Desktop*>::const_iterator it;
   int x, y;            //Position of the top/left corner of a desktop representation
   int i;
   int deltaX, deltaY;  //Width and height of a desktop

   if (m_desks.size() == 0)
      return NULL;

   deltaX = m_width / min(m_nbColumn, (int)m_desks.size());
   deltaY = m_height / (((int)m_desks.size()+m_nbColumn-1) / m_nbColumn);

   x = 0;
   y = 0;
   i = 0;
   for(it = m_desks.begin(); it != m_desks.end(); it ++)
   {
      if ((X < x+deltaX) && (Y < y+deltaY))
         return *it;

      // Calculate x and y for the next iteration
      i++;
      if (i % m_nbColumn == 0)
      {
         x = 0;
         y += deltaY;
      }
      else
         x += deltaX;
   }

   return NULL;
}

void DesktopManager::SwitchToDesktop(Desktop * desk)
{
   if (m_currentDesktop == desk)
      return;

   if (m_currentDesktop != NULL)
      m_currentDesktop->Desactivate();

   m_currentDesktop = desk;
   m_currentDesktop->Activate();
}

void DesktopManager::LoadDesktops()
{
   Desktop * desk;
   Settings settings;
   Settings::Desktop deskSettings(&settings);

   //Temporary, to prevent AddDesktop from changing the current desktop
   m_currentDesktop = (Desktop*)1;

   //Load desktops from registry
   for(int i=0; deskSettings.Open(i); i++)
   {
      desk = new Desktop(&deskSettings);
      AddDesktop(desk);

      deskSettings.Close();
   }

   //Sort the desktops according to their index
   Sort();

   //Activate the first desktop
   if (m_desks.empty())
      m_currentDesktop = NULL;
   else
   {
      m_currentDesktop = m_desks.front();
      m_currentDesktop->Activate();
   }
}
