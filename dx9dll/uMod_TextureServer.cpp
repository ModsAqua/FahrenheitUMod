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

uMod_TextureServer::uMod_TextureServer(wchar_t *game)
{
  Message("uMod_TextureServer(void): %p\n", this);

  Mutex = CreateMutex(NULL, false, NULL);

  Clients = NULL;
  NumberOfClients = 0;
  LenghtOfClients = 0;
}

uMod_TextureServer::~uMod_TextureServer(void)
{
  Message("~uMod_TextureServer(void): %p\n", this);
  if (Mutex != NULL) CloseHandle(Mutex);

  //delete the files in memory
  int num = CurrentMod.GetNumber();
  for (int i = 0; i < num; i++)
  {
    if (CurrentMod[i]->pData) delete [] CurrentMod[i]->pData; //delete the file content of the texture
    delete CurrentMod[i]; //delete the structure
  }

  num = OldMod.GetNumber();
  for (int i = 0; i < num; i++)
  {
    if (OldMod[i]->pData) delete [] OldMod[i]->pData; //delete the file content of the texture
    delete OldMod[i]; //delete the structure
  }

  if (Clients != NULL) delete [] Clients;
}

int uMod_TextureServer::AddClient(uMod_TextureClient *client, TextureFileStruct* &update, int &number, const int version) // called from a client
{
  Message("AddClient(%p): %p\n", client, this);
  if (int ret = LockMutex())
  {
    gl_ErrorState |= uMod_ERROR_SERVER;
    return (ret);
  }

  if (int ret = PrepareUpdate( update, number)) return (ret); // get a copy of all texture to be modded


  if (NumberOfClients == LenghtOfClients) //allocate more memory
  {
    uMod_TextureClient** temp = NULL;
    try {temp = new uMod_TextureClient*[LenghtOfClients + 10];}
    catch (...)
    {
      gl_ErrorState |= uMod_ERROR_MEMORY | uMod_ERROR_SERVER;
      if (int ret = UnlockMutex()) return (ret);
      return (RETURN_NO_MEMORY);
    }
    for (int i=0; i<LenghtOfClients; i++) temp[i] = Clients[i];
    if (Clients!=NULL) delete [] Clients;
    Clients = temp;
    LenghtOfClients += 10;
  }
  Clients[NumberOfClients++] = client;

  if (int ret = UnlockMutex())  return (ret);

  return (RETURN_OK);
}

int uMod_TextureServer::RemoveClient(uMod_TextureClient *client, const int version) // called from a client
{
  Message("RemoveClient(%p): %p\n", client);
  if (int ret = LockMutex())
  {
    gl_ErrorState |= uMod_ERROR_SERVER;
    return (ret);
  }

  bool hit = false;
  for (int i = 0; i < NumberOfClients; i++) if (client == Clients[i])
  {
    hit = true;
    NumberOfClients--;
    Clients[i] = Clients[NumberOfClients];
    break;
  }

  int ret = UnlockMutex();
  if (!hit) return ret;
  if (ret!=RETURN_OK) return (ret);

  return (RETURN_OK);
}

int uMod_TextureServer::AddFile( char* file_name, DWORD64 size, DWORD64 hash, bool force) // called from Mainloop()
{
  Message("uMod_TextureServer::AddFile( %s %#llX, %llu, %d): %p\n", file_name, hash, size, force, this);

  if (int ret = LockMutex())
  {
    gl_ErrorState |= uMod_ERROR_SERVER;
    return (ret);
  }
  TextureFileStruct* temp = NULL;

  int num = CurrentMod.GetNumber();
  for (int i=0; i<num; i++) if (CurrentMod[i]->Hash == hash) //look through all current textures
  {
    if (force) {temp = CurrentMod[i]; break;} // we need to reload it
    else
    {
      return (RETURN_OK); // we still have added this texture
    }
  }
  if (temp==NULL) // if not found, look through all old textures
  {
    num = OldMod.GetNumber();
    for (int i=0; i<num; i++) if (OldMod[i]->Hash == hash)
    {
      temp = OldMod[i];
      OldMod.Remove(temp);
      CurrentMod.Add(temp);
      if (force) break; // we must reload it
      else
      {
        return (RETURN_OK); // we should not reload it
      }
    }
  }

  bool new_file = true;
  if (temp!=NULL) //if it was found, we delete the old file content
  {
    new_file = false;
    if (temp->pData!=NULL) delete [] temp->pData;
    temp->pData = NULL;
  }
  else //if it was not found, we need to create a new object
  {
    new_file = true;
    temp = new TextureFileStruct;
    temp->Reference = -1;
  }

  strcpy_s((char *)&temp->filePath, MAX_PATH, file_name);

  temp->pData = NULL;
  temp->Size = (unsigned int) size;
  temp->NumberOfTextures = 0;
  temp->Textures = NULL;
  temp->Hash = hash;

  temp->ForceReload = force;

  Message("uMod_TextureServer::End AddFile(%#llX)\n", hash);
  if (new_file) CurrentMod.Add(temp); // new files must be added to the list of the CurrentMod+

  return (UnlockMutex());
}

int uMod_TextureServer::PropagateUpdate(uMod_TextureClient* client) // called from Mainloop(), send the update to all clients
{
  Message("PropagateUpdate(%p): %p\n", client, this);
  if (int ret = LockMutex())
  {
    gl_ErrorState |= uMod_ERROR_TEXTURE;
    return (ret);
  }
  if (client != NULL)
  {
    TextureFileStruct* update;
    int number;
    if (int ret = PrepareUpdate( update, number)) return (ret);
    client->AddUpdate(update, number);
  }
  else
  {
    for (int i=0; i<NumberOfClients; i++)
    {
      TextureFileStruct* update;
      int number;
      if (int ret = PrepareUpdate( update, number)) return (ret);
      Clients[i]->AddUpdate(update, number);
    }
  }
  return (UnlockMutex());
}

#define cpy_file_struct( a, b) \
{  \
  a.ForceReload = b.ForceReload; \
  strcpy_s(a.filePath, MAX_PATH, b.filePath); \
  a.pData = b.pData; \
  a.Size = b.Size; \
  a.NumberOfTextures = b.NumberOfTextures; \
  a.Reference = b.Reference; \
  a.Textures = b.Textures; \
  a.Hash = b.Hash; }

int TextureFileStruct_Compare( const void * elem1, const void * elem2 )
{
  TextureFileStruct *tex1 = (TextureFileStruct*)elem1;
  TextureFileStruct *tex2 = (TextureFileStruct*)elem2;
  if (tex1->Hash < tex2->Hash) return (-1);
  if (tex1->Hash > tex2->Hash) return (+1);
  return (0);
}

int uMod_TextureServer::PrepareUpdate(TextureFileStruct* &update, int &number) // called from the PropagateUpdate() and AddClient.
// Prepare an update for one client. The allocated memory must deleted by the client.
{
  update = NULL;
  number = 0;

  TextureFileStruct* temp = NULL;
  int num = CurrentMod.GetNumber();
  if (num>0)
  {
    try {temp = new TextureFileStruct[num];}
    catch (...)
    {
      gl_ErrorState |= uMod_ERROR_MEMORY | uMod_ERROR_SERVER;
      return (RETURN_NO_MEMORY);
    }

    for (int i=0; i<num; i++) cpy_file_struct(temp[i], (*(CurrentMod[i])));
    qsort( temp, num, sizeof(TextureFileStruct), TextureFileStruct_Compare);
  }

  update = temp;
  number = num;

  Message("PrepareUpdate(%p, %d): %p\n", update, number, this);
  return (RETURN_OK);
}
#undef cpy_file_struct

int uMod_TextureServer::LockMutex(void)
{
  if (( gl_ErrorState & (uMod_ERROR_FATAL | uMod_ERROR_MUTEX) )) return (RETURN_NO_MUTEX);
  if (WAIT_OBJECT_0!=WaitForSingleObject( Mutex, 100)) return (RETURN_MUTEX_LOCK); //waiting 100ms, to wait infinite pass INFINITE
  return (RETURN_OK);
}

int uMod_TextureServer::UnlockMutex(void)
{
  if (ReleaseMutex( Mutex) == 0) return (RETURN_MUTEX_UNLOCK);
  return (RETURN_OK);
}

int uMod_TextureServer::MainLoop(void) // run as a separated thread
{
  Message("MainLoop: begin\n");

  unsigned long num = 0u;
  DWORD64 texture_size = 0u;
  char texture_path[MAX_PATH];
  DWORD64 texture_hash = 0u;
  unsigned long width = 0u, height = 0u, format = 0u;
  WIN32_FIND_DATA ffd;
  HANDLE file;

  file = FindFirstFile("textures\\Indigo Prophecy_*.dds", &ffd);
  if (file == INVALID_HANDLE_VALUE)
	  return (RETURN_OK);

  do {
	  texture_size = (ffd.nFileSizeHigh * ((DWORD64)MAXDWORD + 1)) + ffd.nFileSizeLow;
	  sprintf_s(texture_path, "textures\\%s", ffd.cFileName);
	  sscanf_s(ffd.cFileName, "Indigo Prophecy_W%u_H%u_F%u_T_0X%llx", &width, &height, &format, &texture_hash);
	  AddFile(texture_path, texture_size, texture_hash, true);
  } while (FindNextFile(file, &ffd) != 0);

  FindClose(file);

  PropagateUpdate();

  return (RETURN_OK);
}
