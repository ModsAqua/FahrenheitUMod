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



#ifndef uMod_DEFINES_H_
#define uMod_DEFINES_H_


#ifdef LOG_MESSAGE
extern FILE *gl_File;

#define Message(...) {if (gl_File!=NULL) {fprintf( gl_File, __VA_ARGS__); fflush(gl_File);}}
#define OpenMessage(...) {if (fopen_s( &gl_File, "uMod_log.txt", "wt")) gl_File=NULL;}
#define CloseMessage(...) {if (gl_File!=NULL) {fclose(gl_File); gl_File=NULL;}}


#else
/**
 * Open the file for logging (if LOG_MESSAGE=1 was passed during compilation)
 */
#define OpenMessage(...)
/**
 * Print a message (like printf) into the file ant flush the content to disk (if LOG_MESSAGE=1 was passed during compilation)
 */
#define Message(...)
/**
 * Close the file for logging (if LOG_MESSAGE=1 was passed during compilation)
 */
#define CloseMessage(...)
#endif


#endif /* uMod_DEFINES_H_ */
