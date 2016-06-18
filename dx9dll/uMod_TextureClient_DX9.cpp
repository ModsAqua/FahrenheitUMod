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

int uMod_TextureClient_DX9::AddTexture( uMod_IDirect3DVolumeTexture9* pTexture)
{
  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedVolumeTexture(NULL); //this texture must no be added twice

  if (pTexture->FAKE) return (RETURN_OK); // this is a fake texture

  Message("uMod_TextureClient_DX9::AddTexture( Volume: %p): %p (thread: %lu)\n", pTexture, this, GetCurrentThreadId());

  if (int ret = pTexture->ComputetHash())
  {
    Bool_CheckAgainNonAdded = true;
    NonAdded_OriginalVolumeTextures.Add( pTexture);
    return (ret);
  }

  if (gl_ErrorState & uMod_ERROR_FATAL) return (RETURN_FATAL_ERROR);

  OriginalVolumeTextures.Add( pTexture); // add the texture to the list of original texture

  return (LookUpToMod(pTexture)); // check if this texture should be modded
}

int uMod_TextureClient_DX9::AddTexture( uMod_IDirect3DCubeTexture9* pTexture)
{
  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedCubeTexture(NULL); //this texture must no be added twice

  if (pTexture->FAKE) return (RETURN_OK); // this is a fake texture

  Message("uMod_TextureClient_DX9::AddTexture( Cube: %p): %p (thread: %lu)\n", pTexture, this, GetCurrentThreadId());

  if (int ret = pTexture->ComputetHash())
  {
    Bool_CheckAgainNonAdded = true;
    NonAdded_OriginalCubeTextures.Add( pTexture);
    return (ret);
  }

  if (gl_ErrorState & uMod_ERROR_FATAL) return (RETURN_FATAL_ERROR);

  OriginalCubeTextures.Add( pTexture); // add the texture to the list of original texture

  return (LookUpToMod(pTexture)); // check if this texture should be modded
}


int uMod_TextureClient_DX9::CheckAgainNonAdded(void)
{
  Message("uMod_TextureClient_DX9::CheckAgainNonAdded( %u, %u, %u): %p\n", NonAdded_OriginalTextures.GetNumber(),
      NonAdded_OriginalVolumeTextures.GetNumber(), NonAdded_OriginalCubeTextures.GetNumber(), this);

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

  num = NonAdded_OriginalVolumeTextures.GetNumber();
  for (int i=num-1; i>=0; i--)
  {
    uMod_IDirect3DVolumeTexture9* pTexture = NonAdded_OriginalVolumeTextures[i];

    if (pTexture->ComputetHash() == RETURN_OK)
    {
      NonAdded_OriginalVolumeTextures.Remove(pTexture);
      OriginalVolumeTextures.Add( pTexture); // add the texture to the list of original texture
      LookUpToMod(pTexture); // check if this texture should be modded
    }
  }

  num = NonAdded_OriginalCubeTextures.GetNumber();
  for (int i=num-1; i>=0; i--)
  {
    uMod_IDirect3DCubeTexture9* pTexture = NonAdded_OriginalCubeTextures[i];

    if (pTexture->ComputetHash() == RETURN_OK)
    {
      NonAdded_OriginalCubeTextures.Remove(pTexture);
      OriginalCubeTextures.Add( pTexture); // add the texture to the list of original texture
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

int uMod_TextureClient_DX9::RemoveTexture( uMod_IDirect3DVolumeTexture9* pTexture) // is called from a texture, if it is finally released
{
  Message("uMod_TextureClient_DX9::RemoveTexture( Volume %p, %#llX): %p\n", pTexture, pTexture->CRC64, this);

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
    if (int ret = OriginalVolumeTextures.Remove( pTexture)) return (ret); // remove this texture form the original list
    return (NonAdded_OriginalVolumeTextures.Remove( pTexture)); // also try to remove this texture form the non_added list
  }
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::RemoveTexture( uMod_IDirect3DCubeTexture9* pTexture) // is called from a texture, if it is finally released
{
  Message("uMod_TextureClient_DX9::RemoveTexture( Cube %p, %#llX): %p\n", pTexture, pTexture->CRC64, this);

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
    if (int ret = OriginalCubeTextures.Remove( pTexture)) return (ret); // remove this texture form the original list
    return (NonAdded_OriginalCubeTextures.Remove( pTexture)); // also try to remove this texture form the non_added list
  }
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::MergeUpdate(void)
{
  if (NumberOfUpdate<0) {return (RETURN_OK);}
  if (int ret = LockMutex()) {gl_ErrorState |= uMod_ERROR_TEXTURE ; return (ret);}

  Message("uMod_TextureClient_DX9::MergeUpdate(): %p\n", this);

  for (int i=0; i<NumberOfUpdate; i++) {Update[i].NumberOfTextures=0; Update[i].Textures = NULL;} // this is already done, but safety comes first ^^

  int pos_old=0;
  int pos_new=0;
  int *to_lookup = NULL;
  if (NumberOfUpdate>0) to_lookup = new int[NumberOfUpdate];
  int num_to_lookup = 0;

  /*
   * FileToMod contains the old files (textures) which should replace the target textures (if they are loaded by the game)
   * Update contains the new files (textures) which should replace the target textures (if they are loaded by the game)
   *
   * Both arrays (FileToMod and Update) are sorted according to their hash values.
   *
   * First we go through both arrays linearly and
   * 1) take over the old entry if the hash is the same,
   * 2) release old fake texture (if target texture exist and is not in the Update)
   * 3) or mark newly added fake texture (if they are not in FileToMod)
   */

  while (pos_old<NumberToMod && pos_new<NumberOfUpdate)
  {
    if (FileToMod[pos_old].Hash > Update[pos_new].Hash) // this fake texture is new
    {
      to_lookup[num_to_lookup++] = pos_new++; // keep this fake texture in mind, we must search later for it through all original textures
      // we increase only the new counter by one
    }
    else if (FileToMod[pos_old].Hash < Update[pos_new].Hash) // this fake texture is not in the update
    {
      for (int i=FileToMod[pos_old].NumberOfTextures-1; i>=0; i--)
          ((IDirect3DBaseTexture9*) FileToMod[pos_old].Textures[i])->Release(); // we release the fake textures
      if (FileToMod[pos_old].Textures!=NULL) delete [] FileToMod[pos_old].Textures; // we delete the memory
      FileToMod[pos_old].NumberOfTextures = 0;
      FileToMod[pos_old].Textures = NULL;

      pos_old++; // we increase only the old counter by one
    }
    else // the hash value is the same, thus this texture is in the array FileToMod as well as in the array Update
    {
      if (Update[pos_new].ForceReload)
      {
        if (FileToMod[pos_old].NumberOfTextures>0)
        {
          Update[pos_new].Textures = (void**) new IDirect3DBaseTexture9*[FileToMod[pos_old].NumberOfTextures];
        }
        for (int i=0; i<FileToMod[pos_old].NumberOfTextures; i++)
        {
          IDirect3DBaseTexture9 *base_texture;
          int ret = ((IDirect3DBaseTexture9*) FileToMod[pos_old].Textures[i])->QueryInterface( IID_IDirect3D9, (void**)&base_texture);
          switch (ret)
          {
            case 0x01000000L:
            {
              uMod_IDirect3DTexture9 *pTexture = (uMod_IDirect3DTexture9*) FileToMod[pos_old].Textures[i];//
              uMod_IDirect3DTexture9 *pRefTexture = pTexture->CrossRef_D3Dtex;
              pTexture->Release();
              i--; //after the Release of the old fake texture FileToMod[pos_old].Textures[i] is overwritten by entries with index greater than i

              uMod_IDirect3DTexture9 *fake_Texture;
              if (int ret = LoadTexture( & (Update[pos_new]), &fake_Texture)) return (ret);
              if (SwitchTextures( fake_Texture, pRefTexture))
              {
                Message("MergeUpdate(): textures not switched %#llX\n", pRefTexture->CRC64);
                fake_Texture->Release();
              }
              else
              {
                Update[pos_new].Textures[Update[pos_new].NumberOfTextures++] = fake_Texture;
                fake_Texture->Reference = pos_new;
              }
              break;
            }
            case 0x01000001L:
            {
              uMod_IDirect3DVolumeTexture9 *pTexture = (uMod_IDirect3DVolumeTexture9*) FileToMod[pos_old].Textures[i];//
              uMod_IDirect3DVolumeTexture9 *pRefTexture = pTexture->CrossRef_D3Dtex;
              pTexture->Release();
              i--; //after the Release of the old fake texture FileToMod[pos_old].Textures[i] is overwritten by entries with index greater than i

              uMod_IDirect3DVolumeTexture9 *fake_Texture;
              if (int ret = LoadTexture( & (Update[pos_new]), &fake_Texture)) return (ret);
              if (SwitchTextures( fake_Texture, pRefTexture))
              {
                Message("MergeUpdate(): textures not switched %#llX\n", pRefTexture->CRC64);
                fake_Texture->Release();
              }
              else
              {
                Update[pos_new].Textures[Update[pos_new].NumberOfTextures++] = fake_Texture;
                fake_Texture->Reference = pos_new;
              }
              break;
            }
            case 0x01000002L:
            {
              uMod_IDirect3DCubeTexture9 *pTexture = (uMod_IDirect3DCubeTexture9*) FileToMod[pos_old].Textures[i];//
              uMod_IDirect3DCubeTexture9 *pRefTexture = pTexture->CrossRef_D3Dtex;
              pTexture->Release();
              i--; //after the Release of the old fake texture FileToMod[pos_old].Textures[i] is overwritten by entries with index greater than i

              uMod_IDirect3DCubeTexture9 *fake_Texture;
              if (int ret = LoadTexture( & (Update[pos_new]), &fake_Texture)) return (ret);
              if (SwitchTextures( fake_Texture, pRefTexture))
              {
                Message("MergeUpdate(): textures not switched %#llX\n", pRefTexture->CRC64);
                fake_Texture->Release();
              }
              else
              {
                Update[pos_new].Textures[Update[pos_new].NumberOfTextures++] = fake_Texture;
                fake_Texture->Reference = pos_new;
              }
              break;
            }
            default:
              break; // this is no fake texture and QueryInterface failed, because IDirect3DBaseTexture9 object cannot be a IDirect3D9 object ;)
          }
        }
      }
      else // the texture might be loaded or not
      {
        Update[pos_new].NumberOfTextures = FileToMod[pos_old].NumberOfTextures;
        Update[pos_new].Textures = FileToMod[pos_old].Textures;
        FileToMod[pos_old].NumberOfTextures = 0;
        FileToMod[pos_old].Textures = NULL;
      }
      // we increase both counters by one
      pos_old++;
      pos_new++;
    }
  }

  while (pos_old<NumberToMod) //this fake textures are not in the Update
  {
    for (int i=FileToMod[pos_old].NumberOfTextures-1; i>=0; i--)
      ((IDirect3DBaseTexture9*) FileToMod[pos_old].Textures[i])->Release(); // we release the fake textures
    //for (int i=0; i<FileToMod[pos_old].NumberOfTextures; i++) FileToMod[pos_old].Textures[i]->Release(); // we release the fake textures
    if (FileToMod[pos_old].Textures!=NULL) delete [] FileToMod[pos_old].Textures; // we delete the memory
    FileToMod[pos_old].Textures = NULL;
    pos_old++;
  }
  while (pos_new<NumberOfUpdate) //this fake textures are newly added
  {
    to_lookup[num_to_lookup++] = pos_new++; //keep this fake texture in mind, we must search later for it through all original textures
  }


  if (FileToMod!=NULL)
  {
    delete [] FileToMod;
  }

  FileToMod = Update;
  NumberToMod = NumberOfUpdate;

  NumberOfUpdate = -1;
  Update = NULL;


  if (num_to_lookup>0)
  {
    uMod_IDirect3DTexture9* single_texture;
    single_texture = ((uMod_IDirect3DDevice9*)D3D9Device)->GetSingleTexture(); //this texture must no be added twice

    int num = OriginalTextures.GetNumber();
    for (int i=0; i<num; i++) if (OriginalTextures[i]->CrossRef_D3Dtex==NULL || OriginalTextures[i]->CrossRef_D3Dtex==single_texture)
    {
      UnswitchTextures(OriginalTextures[i]); //this we can do always, so we unswitch the single texture
      LookUpToMod( OriginalTextures[i], num_to_lookup, to_lookup);
    }

    uMod_IDirect3DVolumeTexture9 *single_volume_texture;
    single_volume_texture = ((uMod_IDirect3DDevice9*)D3D9Device)->GetSingleVolumeTexture(); //this texture must no be added twice
    num = OriginalVolumeTextures.GetNumber();
    for (int i=0; i<num; i++) if (OriginalVolumeTextures[i]->CrossRef_D3Dtex==NULL || OriginalVolumeTextures[i]->CrossRef_D3Dtex==single_volume_texture)
    {
      UnswitchTextures(OriginalVolumeTextures[i]); //this we can do always, so we unswitch the single texture
      LookUpToMod( OriginalVolumeTextures[i], num_to_lookup, to_lookup);
    }

    uMod_IDirect3DCubeTexture9 *single_cube_texture;
    single_cube_texture = ((uMod_IDirect3DDevice9*)D3D9Device)->GetSingleCubeTexture(); //this texture must no be added twice
    num = OriginalCubeTextures.GetNumber();
    for (int i=0; i<num; i++) if (OriginalCubeTextures[i]->CrossRef_D3Dtex==NULL || OriginalCubeTextures[i]->CrossRef_D3Dtex==single_cube_texture)
    {
      UnswitchTextures(OriginalCubeTextures[i]); //this we can do always, so we unswitch the single texture
      LookUpToMod( OriginalCubeTextures[i], num_to_lookup, to_lookup);
    }
  }



  if (to_lookup != NULL) delete [] to_lookup;

  Message("uMod_TextureClient_DX9::MergeUpdate() END: %p\n", this);

  CheckAgainNonAdded();
  return (UnlockMutex());
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

int uMod_TextureClient_DX9::LookUpToMod( uMod_IDirect3DVolumeTexture9* pTexture, int num_index_list, int *index_list) // should only be called for original textures
{
  Message("uMod_TextureClient_DX9::LookUpToMod( Volume %p): hash: %#llX,  %p\n", pTexture, pTexture->CRC64, this);
  if (pTexture->CrossRef_D3Dtex!=NULL) return (RETURN_OK); // bug, this texture is already switched
  int index = GetIndex( pTexture->CRC64, num_index_list, index_list);
  if (index>=0)
  {
    uMod_IDirect3DVolumeTexture9 *fake_Texture;
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

int uMod_TextureClient_DX9::LookUpToMod( uMod_IDirect3DCubeTexture9* pTexture, int num_index_list, int *index_list) // should only be called for original textures
{
  Message("uMod_TextureClient_DX9::LookUpToMod( Cube %p): hash: %#llX,  %p\n", pTexture, pTexture->CRC64, this);
  if (pTexture->CrossRef_D3Dtex!=NULL) return (RETURN_OK); // bug, this texture is already switched
  int index = GetIndex( pTexture->CRC64, num_index_list, index_list);
  if (index>=0)
  {
    uMod_IDirect3DCubeTexture9 *fake_Texture;
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

	*buffer = new char[size];
	if (*buffer == NULL)
		return result;

	file = fopen(path, "rb");
	if (file == NULL)
	{
		Message("LoadFile( %s, %d) NOT found\n", path, size);
		delete[] *buffer;
		return result;
	}

	Message("LoadFile( %s, %d) found\n", path, size);

	if (fread(*buffer, 1, size, file) == size)
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

int uMod_TextureClient_DX9::LoadTexture( TextureFileStruct* file_in_memory, uMod_IDirect3DVolumeTexture9 **ppTexture) // to load fake texture from a file in memory
{
  Message("LoadTexture( Volume %s, %p, %#llX): %p\n", file_in_memory->filePath, ppTexture, file_in_memory->Hash, this);
  if (RETURN_OK != LoadFile(file_in_memory->filePath, &file_in_memory->pData, file_in_memory->Size))
  {
	  *ppTexture = NULL;
	  return (RETURN_TEXTURE_NOT_LOADED);
  }
  if (D3D_OK != D3DXCreateVolumeTextureFromFileInMemoryEx( D3D9Device, file_in_memory->pData, file_in_memory->Size, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, (IDirect3DVolumeTexture9 **) ppTexture))
  {
    *ppTexture=NULL;
	delete[] file_in_memory->pData;
	file_in_memory->pData = NULL;
	return (RETURN_TEXTURE_NOT_LOADED);
  }
  (*ppTexture)->FAKE = true;

  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedVolumeTexture(NULL); //this texture must no be added twice

  Message("LoadTexture( Volume %p, %#llX): DONE\n", *ppTexture, file_in_memory->Hash);
  return (RETURN_OK);
}

int uMod_TextureClient_DX9::LoadTexture( TextureFileStruct* file_in_memory, uMod_IDirect3DCubeTexture9 **ppTexture) // to load fake texture from a file in memory
{
  Message("LoadTexture( Cube %s, %p, %#llX): %p\n", file_in_memory->filePath, ppTexture, file_in_memory->Hash, this);
  if (RETURN_OK != LoadFile(file_in_memory->filePath, &file_in_memory->pData, file_in_memory->Size))
  {
	  *ppTexture = NULL;
	  return (RETURN_TEXTURE_NOT_LOADED);
  }
  if (D3D_OK != D3DXCreateCubeTextureFromFileInMemoryEx( D3D9Device, file_in_memory->pData, file_in_memory->Size, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, (IDirect3DCubeTexture9 **) ppTexture))
  {
    *ppTexture=NULL;
	delete[] file_in_memory->pData;
	file_in_memory->pData = NULL;
    return (RETURN_TEXTURE_NOT_LOADED);
  }
  (*ppTexture)->FAKE = true;

  ((uMod_IDirect3DDevice9*)D3D9Device)->SetLastCreatedCubeTexture(NULL); //this texture must no be added twice

  Message("LoadTexture( Cube %p, %#llX): DONE\n", *ppTexture, file_in_memory->Hash);
  return (RETURN_OK);
}




