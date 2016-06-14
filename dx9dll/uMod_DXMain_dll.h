/*
This file is part of Universal Modding Engine.


Universal Modding Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Universal Modding Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Universal Modding Engine.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef uMod_DXMAIN_DLL_H_
#define uMod_DXMAIN_DLL_H_

#include "uMod_DX9_dll.h"

/**
 * This function is called during the dll load sequence (dll entry)
 * @param[in] hModule
 */
void InitInstance(HINSTANCE hModule);

/**
 * This function is called, when the dll gets unloaded.
 */
void ExitInstance(void);

/**
 * Returns true if this game shall be injected (DirectX hook). It returns also the name and path of the game executable.
 * This function always return true, when compiled for "Direct Injection" of for "No Injection".
 * @param[out] ret buffer where the name and path of the game executable should be stored
 * @param[in] max_len length of the buffer
 * @return
 */
bool HookThisProgram( wchar_t *ret, const int max_len);

/**
 * This function is called, when the server thread is created. It only calls the mainloop function of the \a TextureServer.
 * @param[in] lpParam Pointer to the \a TextureServer.
 */
DWORD WINAPI ServerThread( LPVOID lpParam);


#endif
