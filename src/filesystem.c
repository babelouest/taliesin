/**
 *
 * Taliesin - Media server
 * 
 * Filesystem functions definitions
 *
 * Copyright 2017-2018 Nicolas Mora <mail@babelouest.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU GENERAL PUBLIC LICENSE
 * License as published by the Free Software Foundation;
 * version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU GENERAL PUBLIC LICENSE for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include "taliesin.h"

/**
 * Check if path exist and is readable
 */
int is_fs_directory_readable(const char * path) {
  int res;
  
  DIR * dir = opendir(path);
  if (dir == NULL && ENOENT == errno) {
    res = 0;
  } else if (dir != NULL) {
    if (access(path, R_OK) != 0) {
      res = 0;
    } else {
      res = 1;
    }
    closedir(dir);
  } else {
    res = 0;
  }
  return res;
}

/**
 * Return the list of files of the specified folder
 */
json_t * fs_directory_read(const char * path) {
  json_t * j_folder = json_array();
  DIR *dir;
  struct dirent *ent;
  struct stat file_stat;
  char * full_path;
  
  if (j_folder != NULL) {
    if ((dir = opendir (path)) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        full_path = msprintf("%s/%s", path, ent->d_name);
        if (stat(full_path, &file_stat) == 0) {
          if (S_ISDIR(file_stat.st_mode) && o_strcmp(ent->d_name, ".") != 0 && o_strcmp(ent->d_name, "..") != 0) {
            // Folder
            json_array_append_new(j_folder, json_pack("{sssssi}", "type", "folder", "name", ent->d_name, "last_modified", file_stat.st_mtime));
          } else if (S_ISREG(file_stat.st_mode)) {
            // File
            json_array_append_new(j_folder, json_pack("{sssssi}", "type", "file", "name", ent->d_name, "last_modified", file_stat.st_mtime));
          }
        } else {
          y_log_message(Y_LOG_LEVEL_ERROR, "fs_directory_read - error reading directory %s", full_path);
        }
        o_free(full_path);
      }
      closedir(dir);
    } else {
      y_log_message(Y_LOG_LEVEL_ERROR, "fs_directory_read - error opening directory %s", path);
    }
  } else {
    y_log_message(Y_LOG_LEVEL_ERROR, "fs_directory_read - error allocating resources for j_folder");
  }
  return j_folder;
}

/**
 * Count the number of files in a folder and all its subfolders
 */
ssize_t fs_directory_count_files_recursive(const char * path) {
  DIR *dir;
  struct dirent *ent;
  struct stat file_stat;
  char * full_path;
  ssize_t counter = 0, tmp;
  
  if (path != NULL) {
    if ((dir = opendir (path)) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        full_path = msprintf("%s/%s", path, ent->d_name);
        if (stat(full_path, &file_stat) == 0) {
          if (S_ISDIR(file_stat.st_mode) && o_strcmp(ent->d_name, ".") != 0 && o_strcmp(ent->d_name, "..") != 0) {
            // Folder
            tmp = fs_directory_count_files_recursive(full_path);
            if (tmp >= 0) {
              counter += tmp;
            }
          } else if (S_ISREG(file_stat.st_mode)) {
            // File
            counter++;
          }
        } else {
          counter = -1;
          y_log_message(Y_LOG_LEVEL_ERROR, "fs_directory_count_files_recursive - error reading directory %s", full_path);
        }
        o_free(full_path);
      }
      closedir(dir);
    } else {
      counter = -1;
      y_log_message(Y_LOG_LEVEL_ERROR, "fs_directory_count_files_recursive - error opening directory %s", path);
    }
  } else {
    counter = -1;
  }
  return counter;
}
