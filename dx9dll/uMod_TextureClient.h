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



#ifndef uMod_TEXTURECLIENT_HPP
#define uMod_TEXTURECLIENT_HPP

#include <d3d9.h>
#include "uMod_GlobalDefines.h"
#include "uMod_Error.h"


class uMod_TextureServer;

/**
 *  An object of this class is owned by each d3dXX device.
 *  functions called by the Server are called from the server thread instance.
 *  All other functions are called from the render thread instance of the game itself.
 */

class uMod_TextureClient
{
public:
  uMod_TextureClient(const int version);
  virtual ~uMod_TextureClient(void);

  /**
   * Connect to the server. (called from the instance, which creates the client, e.g. uMod_IDirect3DDevice9::uMod_IDirect3DDevice9)
   * @return RETURN_OK on success
   */
  int ConnectToServer(uMod_TextureServer* server);

  /**
   * The server add an update to the client.(called from server)
   * @param update Pointer to an array of TextureFileStruct, the client \b must delete this array!
   * @param number number of entries
   * @return RETURN_OK on success
   */
  int AddUpdate(TextureFileStruct* update, int number);

  const int Version;
  uMod_TextureServer* Server; //!< Pointer to the server

  TextureFileStruct* Update; //!< array which stores the file in memory and the hash of each texture to be modded
  int NumberOfUpdate; //!< number of texture to be modded

  /**
   * Lock the mutex. The mutex protect the AddUpdate and MergeUpdate function to be called simultaneously.
   * @return RETURN_OK on success
   */
  int LockMutex();
  /**
   * Unloock the mutex. The mutex protect the AddUpdate and MergeUpdate function to be called simultaneously.
   * @return RETURN_OK on success
   */
  int UnlockMutex();
  HANDLE Mutex; //!< The mutex protect the AddUpdate and MergeUpdate function to be called simultaneously.

  int NumberToMod; //!< number of texture to be modded
  TextureFileStruct* FileToMod; //!< array which stores the file in memory and the hash of each texture to be modded

  /**
   * Find the given hash value in the \a FileToMod list. A vector with index can be passed. If so, only these index are considered in the search
   * @param hash hash value
   * @param num_index_list number of index vector
   * @param index_list pointer to the index vector
   * @return index or a negative value if the hash value was not found
   */
  int GetIndex( DWORD64 hash, int num_index_list=0, int *index_list=(int*)0); // called from LookUpToMod(...);


};



#endif /* uMod_TEXTUREHANDLER_HPP_ */
