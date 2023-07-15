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
  int randint(int a, int b)// [a, b)
  {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> u(a, b - 1);
    for (int i = 0; i < 10; ++i)
    {
      return u(gen);
    }
    return 0;
  }
  
  std::string get_string_from_file(const std::string &path)
  {
    std::ifstream file{path, std::ios::binary};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }
}
#endif