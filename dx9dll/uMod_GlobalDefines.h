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

#ifndef uMod_GlobalDefines_H_
#define uMod_GlobalDefines_H_


#define BIG_BUFSIZE 1<<24
#define SMALL_BUFSIZE 1<<10

/**
 * This struct is the basic of the communication between dll and GUI. For each command send through the pipe a control value is set to MsgStruct::Control.
 * The MsgStruct::Value can store various information (e.g. key values, boolean, or length of data).
 *
 * If mor date should be send, Control is set to one of the intended values and value to the amount of data in byte.
 * After the MsgStruct is send also the data is send through the pipe.
 */
typedef struct
{
  DWORD32 Control; //!< stores the control value
  DWORD64 Value; //!< various meanings
  DWORD64 Hash;
} MsgStruct;


#define uMod_VERSION_char "uMod V 2.0 alpha (r51)"
#define uMod_VERSION L"uMod V 2.0 alpha (r51)"

#define CONTROL_ADD_TEXTURE 1
#define CONTROL_FORCE_RELOAD_TEXTURE 2
#define CONTROL_REMOVE_TEXTURE 3
#define CONTROL_FORCE_RELOAD_TEXTURE_DATA 4
#define CONTROL_ADD_TEXTURE_DATA 5
#define CONTROL_MORE_TEXTURES 6
#define CONTROL_END_TEXTURES 7


#define CONTROL_GAME_EXIT 100
#define CONTROL_ADD_CLIENT 101
#define CONTROL_REMOVE_CLIENT 102

#define VERSION_DX9     90
#define VERSION_DX9EX   91

#endif
