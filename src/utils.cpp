// MIT License
//
// Copyright (c) 2023 rzyzai, and caozhanhao
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

#include "lead/utils.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <random>
#include <unordered_set>

namespace lead::utils
{
  std::string get_string_from_file(const std::string &path)
  {
    std::ifstream file{path, std::ios::binary};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }
  
  bool begin_with(const std::string &a, const std::string &b)
  {
    if (a.size() < b.size()) return false;
    for (size_t i = 0; i < b.size(); ++i)
    {
      if (a[i] != b[i])
      {
        return false;
      }
    }
    return true;
  }
  
  std::string effect(const std::string &str, Effect effect_)
  {
    if (str.empty()) return "";
    if (effect_ == utils::Effect::bg_shadow)
    {
      return "\033[48;5;7m" + str + "\033[49m";
    }
    else if (effect_ == utils::Effect::bg_strong_shadow)
    {
      return "\033[48;5;8m" + str + "\033[49m";
    }
    
    int effect = static_cast<int>(effect_);
    int end = 0;
    if (effect >= 1 && effect <= 7)
    {
      end = 0;
    }
    else if (effect >= 30 && effect <= 37)
    {
      end = 39;
    }
    else if (effect >= 40 && effect <= 47)
    {
      end = 49;
    }
    return "\033[" + std::to_string(effect) + "m" + str + "\033[" + std::to_string(end) + "m";
  }
  
  std::string red(const std::string &str)
  {
    return effect(str, Effect::fg_red);
  }
  
  std::string green(const std::string &str)
  {
    return effect(str, Effect::fg_green);
  }
  
  std::string yellow(const std::string &str)
  {
    return effect(str, Effect::fg_yellow);
  }
  
  std::string blue(const std::string &str)
  {
    return effect(str, Effect::fg_blue);
  }
  
  std::string magenta(const std::string &str)
  {
    return effect(str, Effect::fg_magenta);
  }
  
  std::string cyan(const std::string &str)
  {
    return effect(str, Effect::fg_cyan);
  }
  
  std::string white(const std::string &str)
  {
    return effect(str, Effect::fg_white);
  }
  
  int get_edit_distance(const std::string &s1, const std::string &s2)
  {
    std::size_t n = s1.size();
    std::size_t m = s2.size();
    if (n * m == 0) return static_cast<int>(n + m);
    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i < n + 1; i++)
    {
      D[i][0] = i;
    }
    for (int j = 0; j < m + 1; j++)
    {
      D[0][j] = j;
    }
    
    for (int i = 1; i < n + 1; i++)
    {
      for (int j = 1; j < m + 1; j++)
      {
        int left = D[i - 1][j] + 1;
        int down = D[i][j - 1] + 1;
        int left_down = D[i - 1][j - 1];
        if (s1[i - 1] != s2[j - 1]) left_down += 1;
        D[i][j] = (std::min)(left, (std::min)(down, left_down));
      }
    }
    return D[n][m];
  }
}