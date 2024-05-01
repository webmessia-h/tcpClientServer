#include "../../include/platform.hpp"
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#include "sys/stat.h"
#include <dirent.h>
#endif

std::string resolve_relative_path(const std::string &relative_path) {
  if (relative_path.empty()) {
    throw std::invalid_argument("Empty path provided");
  }

  // Check if the path starts with a tilde (~)
  if (relative_path[0] == '~') {
    // Get the home directory of the current user
    const char *home_dir = getenv("HOME");
    if (!home_dir) {
      struct passwd *pw = getpwuid(getuid());
      if (pw && pw->pw_dir) {
        home_dir = pw->pw_dir;
      } else {
        throw std::runtime_error("Failed to determine home directory");
      }
    }
    // Construct the absolute path
    std::string absolute_path = std::string(home_dir) + relative_path.substr(1);
    // Return the absolute path
    return absolute_path;
  }

  // Otherwise, the path is already absolute
  return relative_path;
}

std::vector<std::pair<std::string, long long>>
list_directory(const std::string &path) {
  std::string absolute_path = resolve_relative_path(path);
  std::vector<std::pair<std::string, long long>> files;
#ifdef _WIN32
  WIN32_FIND_DATA findFileData;
  HANDLE hFind = FindFirstFile((path + "/*").c_str(), &findFileData);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      if (!(findFileData.dwFileAttributes &
            FILE_ATTRIBUTE_DIRECTORY)) { // Check if it's not a directory
        std::string filename = findFileData.cFileName;
        LARGE_INTEGER size;
        size.LowPart = findFileData.nFileSizeLow;
        size.HighPart = findFileData.nFileSizeHigh;
        files.push_back(std::make_pair(filename, size.QuadPart));
      }
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
  }
#else
  DIR *dir = opendir(absolute_path.c_str());
  if (dir == nullptr) {
    std::cerr << "Error opening directory " << path << strerror(errno)
              << std::endl;
    return files;
  }
  dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string filename = entry->d_name;
    if (filename != "." && filename != "..") {
      std::string fullpath = absolute_path + "/" + filename;
      struct stat st;
      if (stat(fullpath.c_str(), &st) == 0) {
        if (S_ISREG(st.st_mode)) { // Check if it's a regular file
          files.push_back(std::make_pair(filename, st.st_size));
        }
      }
    }
  }
  closedir(dir);
#endif
  return files;
}

file_info read_file(const char *filename) {
  std::string absolute_path = resolve_relative_path(filename);
  std::ifstream file(absolute_path, std::ios::binary);
  file_info FILE;
  if (!file.is_open()) {
    FILE.buffer = nullptr;
    FILE.file_size = 0;
    std::cerr << "Error: Failed to open file" << strerror(errno) << std::endl;
    return FILE;
  }
  // Determine the size of the file
  file.seekg(0, std::ios::end);
  size_t file_size = file.tellg();
  file.seekg(0, std::ios::beg);
  // Resize the buffer to hold the file contents
  FILE.buffer = new char[file_size];
  FILE.file_size = file_size;
  // Read the file contents into the buffer
  if (!file.read(FILE.buffer, file_size)) {
    // Failed to read the file
    delete[] FILE.buffer;
    FILE.buffer = nullptr;
    FILE.file_size = 0;
    std::cerr << "Error: Failed to read the file" << strerror(errno)
              << std::endl;
  }
  file.close();
  return FILE;
}

void write_file(const char *filename, const char *buffer, size_t buffer_size) {
  std::string absolute_path = resolve_relative_path(filename);
  std::ofstream file(absolute_path, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Failed to open file for writing." << strerror(errno)
              << std::endl;
    return;
  }
  file.write(buffer, buffer_size);
  if (!file.good()) {
    std::cerr << "Error: Failed to write to file." << strerror(errno)
              << std::endl;
    return;
  }
  file.close();

  std::cout << "File '" << filename << "' has been successfully saved."
            << std::endl;
}

bool cleanup_handler(char *buffer) {
  if (buffer != nullptr)
    delete[] buffer;
  return false;
  return true;
}
