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

/*


 NEVER USE THIS CODE FOR ILLEGAL PURPOSE


*/


#include "uMod_Main.h"
/*
 * global variable which are not linked external
 */

HINSTANCE             gl_hThisInstance = NULL;
uMod_TextureServer*    gl_TextureServer = NULL;
HANDLE                gl_ServerThread = NULL;


/*
 * global variable which are linked external
 */
unsigned int          gl_ErrorState = 0u;

#ifdef LOG_MESSAGE
FILE*                 gl_File = NULL;
#endif

void WideScreenFix();

/*
 * dll entry routine, here we initialize or clean up
 */
BOOL WINAPI DllMain( HINSTANCE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  UNREFERENCED_PARAMETER(lpReserved);

  switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
      WideScreenFix();

	  InitInstance(hModule);
		break;
	}
	case DLL_PROCESS_DETACH:
	{
	  ExitInstance();
	  break;
	}
  default:  break;
	}
    
  return (true);
}


DWORD WINAPI ServerThread( LPVOID lpParam )
{
  UNREFERENCED_PARAMETER(lpParam);
  if (gl_TextureServer!=NULL) gl_TextureServer->MainLoop(); //This is and endless mainloop, it sleep till something is written into the pipe.
  return (0);
}

void InitInstance(HINSTANCE hModule)
{

  DisableThreadLibraryCalls( hModule ); //reduce overhead

  gl_hThisInstance = (HINSTANCE)  hModule;

  wchar_t game[10*MAX_PATH];
  if (HookThisProgram( game, 10*MAX_PATH)) //ask if we need to hook this program
  {
    OpenMessage();
    Message("InitInstance: %lu (%ls)\n", hModule, game);

    gl_TextureServer = new uMod_TextureServer(game); //create the server which listen on the pipe and prepare the update for the texture clients

    InitDX9();

    gl_ServerThread = CreateThread( NULL, 0, ServerThread, NULL, 0, NULL); //creating a thread for the mainloop
    if (gl_ServerThread==NULL) {Message("InitInstance: Serverthread not started\n");}
  }
}

void ExitInstance()
{
  if (gl_ServerThread!=NULL)
  {
    CloseHandle(gl_ServerThread); // kill the server thread
    gl_ServerThread = NULL;
  }
  if (gl_TextureServer!=NULL)
  {
    delete gl_TextureServer; //delete the texture server
    gl_TextureServer = NULL;
  }

  ExitDX9();

  CloseMessage();
}


bool HookThisProgram( wchar_t *ret, const int ret_len)
{
  const int max_len = ret_len < MAX_PATH ? MAX_PATH : ret_len;
  wchar_t *Executable = NULL;
  if (GetMemory( Executable, max_len))
  {
    ret[0]=0;
    return false;
  }
  GetModuleFileNameW( GetModuleHandle( NULL ), Executable, max_len ); //ask for name and path of this executable

  // we inject directly
  int i=0;
  while (i<max_len && i<ret_len-1 && Executable[i]) {ret[i]=Executable[i]; i++;}
  ret[i]=0;
  delete [] Executable;
  return true;
}
