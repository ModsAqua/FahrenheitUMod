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


#ifndef uMod_TEXTURESERVER_H_
#define uMod_TEXTURESERVER_H_

#include "uMod_GlobalDefines.h"
#include "uMod_ArrayHandler.h"


/**
 *  An object of this class is created only once.
 *  The Mainloop functions is executed by a server thread,
 *  which listen on a pipe.
 *
 *  Functions called by the Client are called from the a thread instance of the game itself.
 *  Nearly all other functions are called from the server thread instance.
 */


class uMod_TextureClient;

class uMod_TextureServer
{
public:
  /**
   * The server is created from the dll entry routine.
   * @param[in] name Name of the game executable (without the extension)
   */
  uMod_TextureServer(wchar_t *name);
  ~uMod_TextureServer(void);

  /**
   * Each client connect itself to the server via this function.
   * @param[in] client This-pointer of the client
   * @param[out] update Current texture to be modded
   * @param[out] number Number of modded textures
   * @param[in] version The device version (DX9, DX9EX, DX10, or DX101)
   * @return RETURN_OK on success
   */
  int AddClient(uMod_TextureClient *client, TextureFileStruct* &update, int &number, const int version); // called from a Client

  /**
   * On destruction of client, it disconnect from the server.
   * @param[in] client This-pointer of the client
   * @param[in] version The device version (DX9, DX9EX, DX10, or DX101)
   * @return RETURN_OK on success
   */
  int RemoveClient(uMod_TextureClient *client, const int version); // called from a Client

  /**
   * The mainloop reads from the pipe (blocking reading). It is running in a separate server thread.
   * @return RETURN_OK on success
   */
  int MainLoop(void); // is executed in a server thread



private:
  /**
   * Add a file to the list of texture to be modded (called from the mainloop).
   * @param[in] buffer hold the file content of the texture
   * @param[in] size size of the file content
   * @param[in] hash hash of the texture to be replaced
   * @param[in] force set to TRUE to force a reload of the texture
   * @return RETURN_OK on success
   */
  int AddFile( char* file_name, DWORD64 size,  DWORD64 hash, bool force); // called from Mainloop(), if the content of the texture is sent

  /**
   * Send the files to be modded (Update) to a client (called from the mainloop).
   * @param[in] client Pointer to a client (if NULL is passed, the data is send to all clients)
   * @return
   */
  int PropagateUpdate(uMod_TextureClient* client=NULL); // called from Mainloop() if texture are loaded or removed

  /**
   * Prepare the texture data for the clients (e.g. Load the texture from disk, sort the texture according the hash values) (called from the mainloop).
   * @param[out] update
   * @param[out] number
   * @return
   */
  int PrepareUpdate(TextureFileStruct* &update, int &number);

  /**
   * Locks the mutex.
   * @return
   */
  int LockMutex();
  /**
   * Locks the mutex.
   * @return
   */
  int UnlockMutex();
  HANDLE Mutex; //!< Mutex protects the simultaneously add or remove of multiple clients.


  uMod_TextureClient** Clients;
  int NumberOfClients;
  int LenghtOfClients;

  uMod_FileHandler CurrentMod;  // hold the file content of texture
  uMod_FileHandler OldMod; // hold the file content of texture which were added previously but are not needed any more
  // this is needed, because a texture clients might not have merged the last update and thus hold pointers to the file content of old textures
};


#endif /* uMod_TEXTURESERVER_H_ */
