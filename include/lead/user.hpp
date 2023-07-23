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
#ifndef LEAD_USER_HPP
#define LEAD_USER_HPP
#pragma once

#include "voc.hpp"
#include "leveldb/db.h"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace lead
{
  constexpr size_t planned_review_times = 10;
  
  struct WordRecord
  {
    size_t points;
    
    WordRecord() : points(planned_review_times) {}
  };
  
  void to_json(nlohmann::json &j, const lead::WordRecord &p);
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p);
  
  class User
  {
  private:
    VOC vocabulary;
    std::vector<WordRecord> word_records;
  public:
    User(const std::string &voc_dir_path);
    
    WordRef get_word(size_t w) const;
    
    WordRef get_random_word() const;
    
    WordRecord &word_record(size_t w);
    
    std::string get_explanation(size_t index) const;
    
    nlohmann::json get_quiz(WordRef wr) const;
    
    nlohmann::json search(const std::string &word) const;
    
    nlohmann::json get_progress() const;
  };
}
#endif