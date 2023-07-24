// MIT License
//
// Copyright (c) 2023 caozhanhao
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef LEAD_UTILS_HPP
#define LEAD_UTILS_HPP
#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <random>

namespace lead::utils
{
  template<typename T>
  T randnum(T a, T b)// [a, b)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<T> u(a, b - 1);
    return u(gen);
  }
  
  std::string get_string_from_file(const std::string &path);
  
  bool begin_with(const std::string &a, const std::string &b);
  
  enum class Effect : std::size_t
  {
    bold = 1, faint, italic, underline, slow_blink, rapid_blink, color_reverse,
    fg_black = 30, fg_red, fg_green, fg_yellow, fg_blue, fg_magenta, fg_cyan, fg_white,
    bg_black = 40, bg_red, bg_green, bg_yellow, bg_blue, bg_magenta, bg_cyan, bg_white,
    bg_shadow, bg_strong_shadow
  };
  
  std::string effect(const std::string &str, Effect effect_);
  
  std::string red(const std::string &str);
  
  std::string green(const std::string &str);
  
  std::string yellow(const std::string &str);
  
  std::string blue(const std::string &str);
  
  std::string magenta(const std::string &str);
  
  std::string cyan(const std::string &str);
  
  std::string white(const std::string &str);
  
  int get_edit_distance(const std::string &s1, const std::string &s2);
  
  template<typename T>
  T split(const std::string& str, const std::string& delims = " ")
  {
    T ret;
    size_t first = 0;
    while (first < str.size())
    {
      const auto second = str.find_first_of(delims, first);
      if (first != second)
        ret.insert(ret.end(), str.substr(first, second - first));
      if (second == std::string::npos)
        break;
      first = second + 1;
    }
    return ret;
  }
}
#endif