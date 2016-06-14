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

  Pipe.In = INVALID_HANDLE_VALUE;
  Pipe.Out = INVALID_HANDLE_VALUE;
}

uMod_TextureServer::~uMod_TextureServer(void)
{
  Message("~uMod_TextureServer(void): %p\n", this);
  if (Mutex != NULL) CloseHandle(Mutex);

  //delete the files in memory
  int num = CurrentMod.GetNumber();
  for (int i = 0; i < num; i++)
  {
    delete [] CurrentMod[i]->pData; //delete the file content of the texture
    delete CurrentMod[i]; //delete the structure
  }

  num = OldMod.GetNumber();
  for (int i = 0; i < num; i++)
  {
    delete [] OldMod[i]->pData; //delete the file content of the texture
    delete OldMod[i]; //delete the structure
  }

  if (Clients != NULL) delete [] Clients;

  if (Pipe.In != INVALID_HANDLE_VALUE ) CloseHandle(Pipe.In);
  Pipe.In = INVALID_HANDLE_VALUE;
  if (Pipe.Out != INVALID_HANDLE_VALUE) CloseHandle(Pipe.Out);
  Pipe.Out = INVALID_HANDLE_VALUE;
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

  if (Pipe.Out != INVALID_HANDLE_VALUE)
  {
    MsgStruct msg;
    msg.Control = CONTROL_ADD_CLIENT;
    msg.Value = version;

    unsigned long num;
    bool ret2 = WriteFile( Pipe.Out, (const void*) &msg, sizeof(msg), &num, NULL);
    if (!ret2 || sizeof(msg)!=num) {return (RETURN_PIPE_ERROR);}
    if (!FlushFileBuffers(Pipe.Out)) {return (RETURN_PIPE_ERROR);}
  }
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


  if (Pipe.Out != INVALID_HANDLE_VALUE)
  {
    MsgStruct msg;
    msg.Control = CONTROL_REMOVE_CLIENT;
    msg.Value = version;

    unsigned long num;
    bool ret2 = WriteFile( Pipe.Out, (const void*) &msg, sizeof(msg), &num, NULL);
    if (!ret2 || sizeof(msg)!=num) {return (RETURN_PIPE_ERROR);}
    if (!FlushFileBuffers(Pipe.Out)) {return (RETURN_PIPE_ERROR);}
  }

  return (RETURN_OK);
}

int uMod_TextureServer::AddFile( char* buffer, DWORD64 size,  DWORD64 hash, bool force) // called from Mainloop()
{
  Message("uMod_TextureServer::AddFile( %p %llu, %#llX, %d): %p\n", buffer, size, hash, force, this);

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
      if (buffer!=NULL) delete [] buffer;
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
        if (buffer!=NULL) delete [] buffer;
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

  temp->pData = buffer;

  for (unsigned int i=0; i<size; i++) temp->pData[i] = buffer[i];

  temp->Size = (unsigned int) size;
  temp->NumberOfTextures = 0;
  temp->Textures = NULL;
  temp->Hash = hash;

  temp->ForceReload = force;

  Message("uMod_TextureServer::End AddFile(%#llX)\n", hash);
  if (new_file) CurrentMod.Add(temp); // new files must be added to the list of the CurrentMod+

  return (UnlockMutex());
}


int uMod_TextureServer::RemoveFile(DWORD64 hash) // called from Mainloop()
{
  Message("RemoveFile( %#llx): %p\n", hash, this);

  if (int ret = LockMutex())
  {
    gl_ErrorState |= uMod_ERROR_SERVER;
    return (ret);
  }
  int num = CurrentMod.GetNumber();
  for (int i = 0; i < num; i++) if (CurrentMod[i]->Hash == hash)
  {
    TextureFileStruct* temp = CurrentMod[i];
    CurrentMod.Remove(temp);
    return (OldMod.Add(temp));
  }
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
  if (Pipe.In == INVALID_HANDLE_VALUE) return (RETURN_PIPE_NOT_OPENED);
  char *buffer;
  try {buffer = new char[BIG_BUFSIZE];}
  catch (...) {return (RETURN_NO_MEMORY);}

  unsigned long num = 0u;
  bool update_textures = false;

  DWORD64 texture_size = 0u;
  DWORD64 texture_received = 0u;
  char *texture_data = (char*)0;
  bool texture_force = false;
  DWORD64 texture_hash = 0u;

  Message("MainLoop: started\n");
  while (1)
  {
    Message("MainLoop: run\n");
    bool ret = ReadFile(Pipe.In, // pipe handle
        buffer, // buffer to receive reply
        BIG_BUFSIZE, // size of buffer
        &num, // number of bytes read
        NULL); // not overlapped

    Message("MainLoop: read something (%lu)\n", num);
    if (ret || GetLastError() == ERROR_MORE_DATA)
    {
      DWORD64 pos = 0;
      MsgStruct *commands;
      DWORD64 size=0u;
      while (pos < num)
      {
        if (texture_received<texture_size)
        {
          while (pos<num && texture_received<texture_size) texture_data[texture_received++] = buffer[pos++];
          if (texture_received==texture_size)
          {
            AddFile( texture_data, texture_size, texture_hash, texture_force);
            texture_data = (char*)0;
            texture_size = 0u;
            texture_received = 0u;
          }
        }
        else
        {
        commands = (MsgStruct*) &buffer[pos];

        switch (commands->Control)
        {
        case CONTROL_END_TEXTURES:
        {
          Message("MainLoop: CONTROL_END_TEXTURES (): %p\n", this);
          update_textures=true;
          break;
        }
        case CONTROL_FORCE_RELOAD_TEXTURE_DATA:
        {
          texture_size = commands->Value;
          texture_received = 0u;
          texture_hash = commands->Hash;
          texture_force=true;
          try
          {
            texture_data = new char[(unsigned int) texture_size];
          }
          catch (...)
          {
            texture_size = 0u;
            texture_data = (char*)0;
            gl_ErrorState |= uMod_ERROR_MEMORY | uMod_ERROR_SERVER;
          }
          Message("MainLoop: CONTROL_FORCE_RELOAD_TEXTURE_DATA (%#llX  %llu, %p): %p\n", texture_hash, texture_size, texture_data, this);
          break;
        }
        case CONTROL_ADD_TEXTURE_DATA:
        {
          texture_size = commands->Value;
          texture_received = 0u;
          texture_hash = commands->Hash;
          texture_force=true;
          try
          {
            texture_data = new char[(unsigned int) texture_size];
          }
          catch (...)
          {
            texture_size = 0u;
            texture_data = (char*)0;
            gl_ErrorState |= uMod_ERROR_MEMORY | uMod_ERROR_SERVER;
          }
          Message("MainLoop: CONTROL_ADD_TEXTURE_DATA (%#llX  %llu, %p): %p\n", texture_hash, texture_size, texture_data, this);
          break;
        }
        case CONTROL_REMOVE_TEXTURE:
        {
          Message("MainLoop: CONTROL_REMOVE_TEXTURE (%#llX): %p\n", commands->Hash, this);
          RemoveFile(commands->Hash);
          break;
        }
		default:
        {
          Message("MainLoop: DEFAULT: %lu  %#llX  %#llX: %p\n", commands->Control, commands->Value, commands->Hash, this);
          break;
        }
        }
        pos += sizeof(MsgStruct) + size;
      }
      }
      if (update_textures) {PropagateUpdate(); update_textures=false;}
    }
    else
    {
      Message("MainLoop: error in ReadFile()\n");
      delete [] buffer;
      ClosePipe();
      return (RETURN_OK);
    }
  }

  delete [] buffer;
  return (RETURN_OK);
}

int uMod_TextureServer::OpenPipe(wchar_t *game, int injection_method) // called from InitInstance()
{
  Message("OpenPipe: Out\n")
  // open first outgoing pipe !!
  Pipe.Out = CreateFileW(PIPE_Game2uMod, // pipe name
      GENERIC_WRITE, // write access
      0, // no sharing
      NULL, // default security attributes
      OPEN_EXISTING, // opens existing pipe
      0, // default attributes
      NULL); // no template file

  // Exit if an error other than ERROR_PIPE_BUSY occurs.
  if (Pipe.Out == INVALID_HANDLE_VALUE) return (RETURN_PIPE_NOT_OPENED);

  unsigned int len = 0u;
  while (game[len]) len++;
  len++; //to send also the zero
  unsigned long num;
  //send name of this game to uMod_GUI

  char *buffer=(char*)0;
  GetMemory(buffer, sizeof(int) + len * sizeof(wchar_t));
  *((int*) buffer) = injection_method;
  char *p_game = (char*) game;

  for (unsigned int i=0; i<len*sizeof(wchar_t); i++) buffer[i+sizeof(int)] = p_game[i];
  WriteFile(Pipe.Out, (const void*) buffer, len * sizeof(wchar_t) + sizeof(int), &num, NULL);
  delete [] buffer;

  // now we can open the pipe for reading
  Message("OpenPipe: In\n");
  Pipe.In = CreateFileW(PIPE_uMod2Game, // pipe name
      GENERIC_READ, // read access
      0, // no sharing
      NULL, // default security attributes
      OPEN_EXISTING, // opens existing pipe
      0, // default attributes
      NULL); // no template file

  if (Pipe.In == INVALID_HANDLE_VALUE)
  {
    CloseHandle(Pipe.In);
    Pipe.In = INVALID_HANDLE_VALUE;
    return (RETURN_PIPE_NOT_OPENED);
  }

  Message("OpenPipe: Done\n");
  return (RETURN_OK);
}

int uMod_TextureServer::ClosePipe(void) //called from ExitInstance, this must be done, otherwise the Mainloop will wait endless on the ReadFile()
{
  Message("ClosePipe:\n");



  // We close the outgoing pipe first.
  // The GUI will notice that the opposite side of it incoming pipe is closed
  // and closes it outgoing (our incoming) pipe and thus cancel the ReadFile() in the Mainloop()

  if (Pipe.Out != INVALID_HANDLE_VALUE)
  {

    MsgStruct msg;
    msg.Control = CONTROL_GAME_EXIT;

    unsigned long num;
    bool ret2 = WriteFile( Pipe.Out, (const void*) &msg, sizeof(msg), &num, NULL);
    if (!ret2 || sizeof(msg)!=num) {return (RETURN_PIPE_ERROR);}
    if (!FlushFileBuffers(Pipe.Out)) {return (RETURN_PIPE_ERROR);}

    DisconnectNamedPipe(Pipe.Out);
    CloseHandle(Pipe.Out);
    Pipe.Out = INVALID_HANDLE_VALUE;
  }

  if (Pipe.In != INVALID_HANDLE_VALUE)
  {
    DisconnectNamedPipe(Pipe.In);
    CloseHandle(Pipe.In);
    Pipe.In = INVALID_HANDLE_VALUE;
  }

  return (RETURN_OK);
}
