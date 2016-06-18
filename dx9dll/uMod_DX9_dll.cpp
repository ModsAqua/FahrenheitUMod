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


#include "uMod_DX9_dll.h"
#include "uMod_IDirect3D9.h"


/*
 * global variable which are not linked external
 */
HINSTANCE             gl_hOriginal_DX9_Dll = NULL;

typedef IDirect3D9 *(APIENTRY *Direct3DCreate9_type)(UINT);




/*
 * global variable which are linked external
 */

void InitDX9(void)
{
  LoadOriginal_DX9_Dll();
}

void ExitDX9(void)
{
  // Release the system's d3d9.dll
  if (gl_hOriginal_DX9_Dll!=NULL)
  {
    FreeLibrary(gl_hOriginal_DX9_Dll);
    gl_hOriginal_DX9_Dll = NULL;
  }
}

void LoadOriginal_DX9_Dll(void)
{
  char buffer[MAX_PATH];
  if (gl_hOriginal_DX9_Dll==NULL)
  {
    GetSystemDirectory(buffer,MAX_PATH); //get the system directory, we need to open the original d3d9.dll

    // Append dll name
    strcat_s( buffer, MAX_PATH,"\\d3d9.dll");

    // try to load the system's d3d9.dll
    gl_hOriginal_DX9_Dll = LoadLibrary(buffer);
  }
}


/*
 * We do not inject, the game loads this dll by itself thus we must include the Direct3DCreate9 function
 */

IDirect3D9* WINAPI  Direct3DCreate9(UINT SDKVersion)
{
  Message("WINAPI  Direct3DCreate9\n");

	if (gl_hOriginal_DX9_Dll==NULL) LoadOriginal_DX9_Dll(); // looking for the "original d3d9.dll"
	if (gl_hOriginal_DX9_Dll==NULL) return (NULL);
	
	// find original function in original d3d9.dll
	Direct3DCreate9_type D3DCreate9_fn = (Direct3DCreate9_type) GetProcAddress( gl_hOriginal_DX9_Dll, "Direct3DCreate9");
    

	if (!D3DCreate9_fn) 
  {
	  Message("Direct3DCreate9: original function not found in dll\n");
    return (NULL);
  }
	

	//Create originale IDirect3D9 object
	IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(SDKVersion);

	//create our uMod_IDirect3D9 object
	uMod_IDirect3D9 *pIDirect3D9 = new uMod_IDirect3D9( pIDirect3D9_orig, gl_TextureServer);

	// Return pointer to our object instead of "real one"
	return (pIDirect3D9);
}

typedef IDirect3D9 *(APIENTRY *Direct3DCreate9_type)(UINT);
typedef HRESULT (APIENTRY *Direct3DCreate9Ex_type)(UINT SDKVersion, IDirect3D9Ex **ppD3D);

typedef int (WINAPI *D3DPERF_BeginEvent_type)( D3DCOLOR , LPCWSTR  );
int WINAPI D3DPERF_BeginEvent( D3DCOLOR col, LPCWSTR wszName )
{
  D3DPERF_BeginEvent_type fn = (D3DPERF_BeginEvent_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_BeginEvent");
  return fn( col, wszName);
}

typedef int (WINAPI *D3DPERF_EndEvent_type)( void );
int WINAPI D3DPERF_EndEvent( void )
{
  D3DPERF_EndEvent_type fn = (D3DPERF_EndEvent_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_EndEvent");
  return fn();
}

typedef void (WINAPI *D3DPERF_SetMarker_type)( D3DCOLOR , LPCWSTR );
void WINAPI D3DPERF_SetMarker( D3DCOLOR col, LPCWSTR wszName )
{
  D3DPERF_SetMarker_type fn = (D3DPERF_SetMarker_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_SetMarker");
  return fn( col, wszName);
}

typedef void (WINAPI *D3DPERF_SetRegion_type)( D3DCOLOR, LPCWSTR );
void WINAPI D3DPERF_SetRegion( D3DCOLOR col, LPCWSTR wszName )
{
  D3DPERF_SetRegion_type fn = (D3DPERF_SetRegion_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_SetRegion");
  return fn( col, wszName);
}

typedef BOOL (WINAPI *D3DPERF_QueryRepeatFrame_type)( void );
BOOL WINAPI D3DPERF_QueryRepeatFrame( void )
{
  D3DPERF_QueryRepeatFrame_type fn = (D3DPERF_QueryRepeatFrame_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_QueryRepeatFrame");
  return fn( );
}

typedef void (WINAPI *D3DPERF_SetOptions_type)( DWORD );
void WINAPI D3DPERF_SetOptions( DWORD dwOptions )
{
  D3DPERF_SetOptions_type fn = (D3DPERF_SetOptions_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_SetOptions");
  return fn( dwOptions);
}

typedef DWORD (WINAPI *D3DPERF_GetStatus_type)( void );
DWORD WINAPI D3DPERF_GetStatus( void )
{
  D3DPERF_GetStatus_type fn = (D3DPERF_GetStatus_type) GetProcAddress( gl_hOriginal_DX9_Dll, "D3DPERF_GetStatus");
  return fn( );
}

