// utils.cpp
#include "utils.hpp"
#include <algorithm>
#include <sstream>


// 去除字符串两端的空格
std::string trim(const std::string& s) {
    // 如果字符串为空，则返回空字符串
    if (s.empty()) return "";

    // 定义字符串的起始和结束位置
    size_t start = 0;
    size_t end = s.size() - 1;

    // 从字符串开头开始，找到第一个非空格字符的位置
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    // 从字符串末尾开始，找到第一个非空格字符的位置
    while (end >= start && std::isspace(static_cast<unsigned char>(s[end]))) {
        --end;
    }

    // 返回去除两端空格后的字符串
    return s.substr(start, end - start + 1);
}


std::vector<std::string> split_lines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream iss(text);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }
    return lines;
}