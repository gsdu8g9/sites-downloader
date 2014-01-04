#include <string>

#pragma once

bool is_slash(const std::string& str);
std::string get_path(const std::string& current, const std::string& target);
std::string absolute_path(const std::string& path);
std::string without_end_after_slash(const std::string& str);
bool is_wrong_name(const std::string& str);
bool compare_with_end(const std::string& str, const std::string& end);
std::string to_shell(const std::string& str);