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



#ifndef uMod_TEXTURECLIENT_DX9_HPP
#define uMod_TEXTURECLIENT_DX9_HPP

#include "uMod_Error.h"
#include "uMod_TextureClient.h"
#include "uMod_IDirect3DTexture9.h"
#include "uMod_IDirect3DDevice9.h"


class uMod_TextureServer;

/*
 *  An object of this class is owned by each d3d9 device.
 *  functions called by the Server are called from the server thread instance.
 *  All other functions are called from the render thread instance of the game itself.
 */

class uMod_TextureClient_DX9 : public uMod_TextureClient
{
public:
  uMod_TextureClient_DX9( IDirect3DDevice9* device, const int version);
  virtual ~uMod_TextureClient_DX9(void);

  /**
   * called from uMod_IDirect3DDevice9::CreateTexture(...) or uMod_IDirect3DDevice9::BeginScene()
   * @param[in] tex texture to be added
   * @return
   */
  int AddTexture( uMod_IDirect3DTexture9* tex);

  /**
   * called from  uMod_IDirect3DTexture9::Release()
   * @param[in] tex
   * @return
   */
  int RemoveTexture( uMod_IDirect3DTexture9* tex);

  int LookUpToMod( uMod_IDirect3DTexture9* pTexture, int num_index_list=0, int *index_list=(int*)0); // called at the end AddTexture(...) and from Device->UpdateTexture(...)

  uMod_TextureHandler<uMod_IDirect3DTexture9> OriginalTextures; // stores the pointer to the uMod_IDirect3DTexture9 objects created by the game


  // if the has could not be calculated for a texture it is not considered in the original texture list,
  // but it is now added into the following lists and we try to calculate the hash once again later
  int CheckAgainNonAdded(void);
  bool Bool_CheckAgainNonAdded;
  uMod_TextureHandler<uMod_IDirect3DTexture9> NonAdded_OriginalTextures; // stores the pointer to the uMod_IDirect3DTexture9 objects created by the game

private:
  IDirect3DDevice9* D3D9Device;

  int LoadFile(char *path, char **buffer, UINT64 size);

  int LoadTexture( TextureFileStruct* file_in_memory, uMod_IDirect3DTexture9 **ppTexture); // called if a target texture is found
};



#endif /* uMod_TEXTUREHANDLER_HPP_ */
