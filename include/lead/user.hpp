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
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace lead
{
  constexpr size_t planned_review_times = 30;
  
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
    std::vector<WordRecord> word_records;
    std::vector<size_t> marked_words;
    std::string record_path;
    size_t plan_pos;
  public:
    User(const std::string &voc_dir_path, const std::string &record_dir_path);
    
    WordRef get_word(size_t w) const;
    
    WordRef get_random_word() const;
  
    WordRef get_memorize_word();
    
    WordRef prev_memorize_word();
  
    WordRef set_memorize_word(size_t index);
    
    WordRef curr_memorize_word() const;
    
    WordRecord *word_record(size_t w);
    
    std::string get_explanation(size_t index) const;
    
    nlohmann::json get_quiz(WordRef wr) const;
  
    nlohmann::json search(const std::string &word) const;
    
    void clear_records();
    
    void clear_marks();
  
    int mark_word(size_t index);
  
    int unmark_word(size_t index);
    
    bool is_marked(size_t index) const;
  
    nlohmann::json get_marked() const;
    
    nlohmann::json get_passed() const;
    
    nlohmann::json get_plan() const;
  
    void write_records();
  
    VOC vocabulary;
  };
}
#endif