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



#include "uMod_Main.h"
#include "uMod_TextureClient_DX9.h"


uMod_TextureClient_DX9::uMod_TextureClient_DX9( IDirect3DDevice9* device, const int version)
  : uMod_TextureClient( version)
{
  Message("uMod_TextureClient_DX9::uMod_TextureClient_DX9(void): %p\n", this);
  D3D9Device = device;
  Bool_CheckAgainNonAdded = false;
}

uMod_TextureClient_DX9::~uMod_TextureClient_DX9(void)
{
  Message("uMod_TextureClient_DX9::~uMod_TextureClient(void): %p\n", this);
}


int uMod_TextureClient_DX9::AddTexture( uMod_IDirect3DTexture9* pTexture)
{
  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedTexture(NULL); //this texture must no be added twice

  if (pTexture->FAKE) return (RETURN_OK); // this is a fake texture

  Message("uMod_TextureClient_DX9::AddTexture( %p): %p (thread: %lu)\n", pTexture, this, GetCurrentThreadId());

  if (int ret = pTexture->ComputetHash())
  {
    Bool_CheckAgainNonAdded = true;
    NonAdded_OriginalTextures.Add( pTexture);
    return (ret);
  }

  if (gl_ErrorState & uMod_ERROR_FATAL) return (RETURN_FATAL_ERROR);

  OriginalTextures.Add( pTexture); // add the texture to the list of original texture

  return (LookUpToMod(pTexture)); // check if this texture should be modded
}

int uMod_TextureClient_DX9::CheckAgainNonAdded(void)
{
  Message("uMod_TextureClient_DX9::CheckAgainNonAdded( %u): %p\n", NonAdded_OriginalTextures.GetNumber(), this);

  Bool_CheckAgainNonAdded = false;

  int num = NonAdded_OriginalTextures.GetNumber();
  for (int i=num-1; i>=0; i--)
  {
    uMod_IDirect3DTexture9* pTexture = NonAdded_OriginalTextures[i];

    if (pTexture->ComputetHash() == RETURN_OK)
    {
      NonAdded_OriginalTextures.Remove(pTexture);
      OriginalTextures.Add( pTexture); // add the texture to the list of original texture
      LookUpToMod(pTexture); // check if this texture should be modded
    }
  }

  Message("uMod_TextureClient_DX9::CheckAgainNonAdded() END: %p\n", this);
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::RemoveTexture( uMod_IDirect3DTexture9* pTexture) // is called from a texture, if it is finally released
{
  Message("uMod_TextureClient_DX9::RemoveTexture( %p, %#llX): %p\n", pTexture, pTexture->CRC64, this);

  if (gl_ErrorState & uMod_ERROR_FATAL) return (RETURN_FATAL_ERROR);
  if (pTexture->FAKE)
  {
    // we need to set the corresponding FileToMod[X].pTexture to NULL, to avoid a link to a non existing texture object
    int ref = pTexture->Reference;
    if (ref>=0 && ref<NumberToMod)
    {
      for (int i=0; i<FileToMod[ref].NumberOfTextures; i++) if (FileToMod[ref].Textures[i] == pTexture)
      {
        FileToMod[ref].NumberOfTextures--;
        for (int j=i; j<FileToMod[ref].NumberOfTextures; j++) FileToMod[ref].Textures[j] = FileToMod[ref].Textures[j+1];
        FileToMod[ref].Textures[FileToMod[ref].NumberOfTextures] = NULL;
        break;
      }
	  if (FileToMod[ref].NumberOfTextures == 0)
	  {
		  if (FileToMod[ref].pData)
		  {
			  delete[] FileToMod[ref].pData;
			  FileToMod[ref].pData = NULL;
		  }
	  }
    }
  }
  else
  {
    if (int ret = OriginalTextures.Remove( pTexture)) return (ret); // remove this texture form the original list
    return (NonAdded_OriginalTextures.Remove( pTexture)); // also try to remove this texture form the non_added list
  }
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::LookUpToMod( uMod_IDirect3DTexture9* pTexture, int num_index_list, int *index_list) // should only be called for original textures
{
  Message("uMod_TextureClient_DX9::LookUpToMod( %p): hash: %#llX,  %p\n", pTexture, pTexture->CRC64, this);
  if (pTexture->CrossRef_D3Dtex!=NULL) return (RETURN_OK); // bug, this texture is already switched
  int index = GetIndex( pTexture->CRC64, num_index_list, index_list);
  if (index>=0)
  {
    uMod_IDirect3DTexture9 *fake_Texture;
    if (int ret = LoadTexture( & (FileToMod[index]), &fake_Texture)) return (ret);
    if (SwitchTextures( fake_Texture, pTexture))
    {
      Message("uMod_TextureClient_DX9::LookUpToMod(): textures not switched %#llX\n", FileToMod[index].Hash);
      fake_Texture->Release();
    }
    else
    {
      IDirect3DBaseTexture9 **temp = new IDirect3DBaseTexture9*[FileToMod[index].NumberOfTextures+1];
      for (int j=0; j<FileToMod[index].NumberOfTextures; j++) temp[j] =  (IDirect3DBaseTexture9*) FileToMod[index].Textures[j];

      if (FileToMod[index].Textures!=NULL) delete [] FileToMod[index].Textures;
      FileToMod[index].Textures = (void**) temp;

      FileToMod[index].Textures[FileToMod[index].NumberOfTextures++] = fake_Texture;
      fake_Texture->Reference = index;
    }
  }
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::LoadFile(char *path, char **buffer, UINT64 size)
{
	FILE *file;
	int result = RETURN_TEXTURE_NOT_LOADED;

	*buffer = new char[(int)size];
	if (*buffer == NULL)
		return result;

	fopen_s(&file, path, "rb");
	if (file == NULL)
	{
		Message("LoadFile( %s, %d) NOT found\n", path, size);
		delete[] *buffer;
		return result;
	}

	Message("LoadFile( %s, %d) found\n", path, size);

	if (fread(*buffer, 1, (size_t)size, file) == size)
		result = RETURN_OK;

	fclose(file);

	return result;
}


int uMod_TextureClient_DX9::LoadTexture( TextureFileStruct* file_in_memory, uMod_IDirect3DTexture9 **ppTexture) // to load fake texture from a file in memory
{
  Message("LoadTexture( %s, %p, %#llX): %p\n", file_in_memory->filePath, ppTexture, file_in_memory->Hash, this);
  if (RETURN_OK != LoadFile(file_in_memory->filePath, &file_in_memory->pData, file_in_memory->Size))
  {
	  *ppTexture = NULL;
	  return (RETURN_TEXTURE_NOT_LOADED);
  }

  if (D3D_OK != D3DXCreateTextureFromFileInMemoryEx( D3D9Device, file_in_memory->pData, file_in_memory->Size, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, (IDirect3DTexture9 **) ppTexture))
  {
    *ppTexture=NULL;
	delete[] file_in_memory->pData;
	file_in_memory->pData = NULL;
	return (RETURN_TEXTURE_NOT_LOADED);
  }
  (*ppTexture)->FAKE = true;

  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedTexture(NULL); //this texture must no be added twice

  Message("LoadTexture( %p, %#llX): DONE\n", *ppTexture, file_in_memory->Hash);
  return (RETURN_OK);
}
