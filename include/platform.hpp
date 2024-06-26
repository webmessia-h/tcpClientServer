// platfrom
#pragma once
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <stdexcept>
#include <string.h>
#include <unistd.h>
#include <vector>
struct file_info {
  char *buffer;
  size_t file_size;
};
std::string resolve_relative_path(const std::string &relative_path);
std::vector<std::pair<std::string, long long>>
list_directory(const std::string &path);
file_info read_file(const char *file_path);
void write_file(const char *filename, const char *buffer, size_t buffer_size);
bool cleanup_handler(char *buffer);
