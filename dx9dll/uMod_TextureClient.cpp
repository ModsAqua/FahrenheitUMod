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


uMod_TextureClient::uMod_TextureClient(const int version) : Version(version)
{
  Message("uMod_TextureClient::uMod_TextureClient(void): %lu\n", this);
  Server = NULL;

  NumberToMod = 0;
  FileToMod = NULL;

  Mutex = CreateMutex(NULL, false, NULL);

  Update = NULL;
  NumberOfUpdate = -1;
}

uMod_TextureClient::~uMod_TextureClient(void)
{
  Message("uMod_TextureClient::~uMod_TextureClient(void): %lu\n", this);
  if (Server!=NULL) Server->RemoveClient(this, Version);

  if (Mutex!=NULL) CloseHandle(Mutex);

  if (Update!=NULL) delete [] Update;
  if (FileToMod!=NULL)
  {
    for (int i=0; i<NumberToMod; i++) if (FileToMod[i].Textures!=NULL) delete [] FileToMod[i].Textures;
    delete [] FileToMod;
  }
}


int uMod_TextureClient::ConnectToServer(uMod_TextureServer* server)
{
  Server = server;
  if (Server!=NULL)
  {
    if (Server->AddClient( this, FileToMod, NumberToMod, Version))
    {
      Server=NULL;
      NumberToMod = 0;
      FileToMod = NULL;
    }
    else
    {
      for (int i=0; i<NumberToMod; i++) {FileToMod[i].NumberOfTextures=0; FileToMod[i].Textures = NULL;}
    }
  }
  return (RETURN_OK);
}


int uMod_TextureClient::AddUpdate(TextureFileStruct* update, int number)  //client must delete the update array
{
  Message("AddUpdate( %lu, %d): %lu\n", update, number, this);
  if (int ret = LockMutex()) {gl_ErrorState |= uMod_ERROR_TEXTURE; return (ret);}
  if (Update!=NULL) delete [] Update;
  Update = update;
  NumberOfUpdate = number;
  return (UnlockMutex());
}


int uMod_TextureClient::LockMutex(void)
{
  if (( gl_ErrorState & (uMod_ERROR_FATAL | uMod_ERROR_MUTEX) )) return (RETURN_NO_MUTEX);
  if (WAIT_OBJECT_0!=WaitForSingleObject( Mutex, 100)) return (RETURN_MUTEX_LOCK); //waiting 100ms, to wait infinite pass INFINITE
  return  (RETURN_OK);
}

int uMod_TextureClient::UnlockMutex(void)
{
  if (ReleaseMutex(Mutex)==0) return (RETURN_MUTEX_UNLOCK);
  return  (RETURN_OK);
}


int uMod_TextureClient::GetIndex( DWORD64 hash, int num_index_list, int *index_list)
{
  if(NumberToMod>0)
  {
    if (index_list==NULL || num_index_list==0)
    {
      if (hash<FileToMod[0].Hash || hash>FileToMod[NumberToMod-1].Hash) return (-1);
      int pos = NumberToMod/2;
      int begin = 0;
      int end = NumberToMod-1;

      // We look in the middle of the interval and each step we halve the interval,
      // unless we find the texture or the size of the interval is less than 3.
      // Note: contradicting to normal C-code here the interval includes the index "begin" and "end"!
      while (begin+1<end) // as long as the interval is longer than two
      {
        if (hash > FileToMod[pos].Hash) // the new interval is the right half of the actual interval
        {
          begin = pos+1; // the new interval does not contain the index "pos"
          pos = (begin + end)/2; // set "pos" somewhere inside the new interval
        }
        else if (hash < FileToMod[pos].Hash) // the new interval is the left half of the actual interval
        {
          end = pos-1; // the new interval does not contain the index "pos"
          pos = (begin + end)/2; // set "pos" somewhere inside the new interval
        }
        else {return (pos); break;} // we hit the correct hash
      }
      for ( pos=begin; pos<=end; pos++) if (FileToMod[pos].Hash==hash) return (pos);
    }
    else
    {
      if (hash<FileToMod[index_list[0]].Hash || hash>FileToMod[index_list[num_index_list-1]].Hash) return (-1);
      int pos = num_index_list/2;
      int begin = 0;
      int end = num_index_list-1;

      // We look in the middle of the interval and each step we halve the interval,
      // unless we find the texture or the size of the interval is less than 3.
      // Note: contradicting to normal C-code here the interval includes the index "begin" and "end"!
      while (begin+1<end) // as long as the interval is longer than two
      {
        if (hash > FileToMod[index_list[pos]].Hash) // the new interval is the right half of the actual interval
        {
          begin = pos+1; // the new interval does not contain the index "pos"
          pos = (begin + end)/2; // set "pos" somewhere inside the new interval
        }
        else if (hash < FileToMod[index_list[pos]].Hash) // the new interval is the left half of the actual interval
        {
          end = pos-1; // the new interval does not contain the index "pos"
          pos = (begin + end)/2; // set "pos" somewhere inside the new interval
        }
        else {return (index_list[pos]); break;} // we hit the correct hash
      }
      for ( pos=begin; pos<=end; pos++) if (FileToMod[index_list[pos]].Hash==hash) return (index_list[pos]);
    }
  }
  return (-1);
}
