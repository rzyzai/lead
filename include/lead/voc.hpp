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
#ifndef LEAD_VOC_HPP
#define LEAD_VOC_HPP
#pragma once

#include "bundled/nlohmann/json.hpp"
#include "utils.hpp"
#include <tuple>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <set>
#include <random>

namespace lead
{
  struct Word
  {
    std::string word;
    std::string meaning;
    std::string explanation;
  };
  
  void to_json(nlohmann::json &j, const lead::Word &p);
  
  struct WordRef
  {
    const Word *word;
    size_t index;
    
    WordRef(const Word *w, size_t p) : word(w), index(p) {}
    
    WordRef() : word(nullptr), index(0) {}
    
    WordRef(const WordRef &) = default;
    
    WordRef &operator=(const WordRef &) = default;
    
    bool is_valid() { return word != nullptr; };
  };
  
  void to_json(nlohmann::json &j, const lead::WordRef &p);
  
  class VOC
  {
  private:
    std::vector<Word> vocabulary;
  public:
    void load(const std::vector<Word> &word);
    
    void load(const std::string &voc_index_path, const std::string &voc_data_path);
    
    std::string get_explanation(size_t index) const;
    
    std::vector<WordRef> get_similiar_words(WordRef wr, size_t n, const std::function<bool(WordRef)> &selector) const;
    
    WordRef at(size_t w) const;
  
    std::vector<size_t> search(const std::string &w) const;
    
    size_t size() const;
  };
}
#endif