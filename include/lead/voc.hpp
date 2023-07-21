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
  
  void to_json(nlohmann::json &j, const lead::Word &p)
  {
    j = nlohmann::json{{"word",     p.word},
                       {"meaning",  p.meaning}};
  }
  
  struct WordRef
  {
    Word *word;
    size_t index;
    
    WordRef(Word *w, size_t p) : word(w), index(p) {}
    
    WordRef() : word(nullptr), index(0) {}
    
    bool is_valid() { return word != nullptr; };
  };
  
  void to_json(nlohmann::json &j, const lead::WordRef &p)
  {
    j = nlohmann::json{{"word", *p.word},
                       {"index",  p.index}};
  }
  class VOC
  {
  private:
    std::string name;
    std::vector<Word> vocabulary;
  public:
    void set_name(const std::string &name_)
    {
      name = name_;
    }
    
    void load(const std::vector<Word> &word)
    {
      vocabulary = word;
    }
    
    void load(const std::string &voc_index_path, const std::string &voc_data_path)
    {
      std::ifstream index_file(voc_index_path);
      std::ifstream data_file(voc_data_path);
      nlohmann::json voc_index = nlohmann::json::parse(index_file);
      nlohmann::json voc_data = nlohmann::json::parse(data_file);
      for (auto &r: voc_index)
      {
        vocabulary.emplace_back(
            Word{
              .word = r["word"].get<std::string>(),
              .meaning = r["meanings"].get<std::string>(),
              .explanation = voc_data[std::to_string(r["index"].get<int>())].get<std::string>()
        });
      }
      index_file.close();
      data_file.close();
    }
    
    std::string get_explanation(size_t index)
    {
      return vocabulary[index].explanation;
    }
    
    std::vector<WordRef> get_similiar_words(WordRef wr, size_t n, const std::function<bool(WordRef)>& selector)
    {
      std::vector<WordRef> ret;
      const auto distance_cmp = [](auto &&p1, auto &&p2) { return p1.second < p2.second; };
      std::multiset<std::pair<size_t, int>, decltype(distance_cmp)> similiar(distance_cmp); // index, distance
      const auto is_ambiguous = [this, &wr, &similiar](size_t i) -> bool
      {
         if(vocabulary[i].word == wr.word->word) return true;
         for(auto& r : similiar)
         {
           if (vocabulary[r.first].word == vocabulary[i].word)
             return true;
         }
         return false;
      };
      
      for (size_t i = 0; i < vocabulary.size(); ++i)
      {
        if (is_ambiguous(i) || !selector(at(i))) continue;
        auto d = utils::get_edit_distance(vocabulary[i].word, wr.word->word);
        if (similiar.size() < n)
        {
          similiar.insert({i, d});
        }
        else if ( d < similiar.rbegin()->second)
        {
          similiar.erase(std::prev(similiar.end()));
          similiar.insert({i, d});
        }
      }
      for (auto &r: similiar)
      {
        ret.emplace_back(WordRef{&vocabulary[r.first], r.first});
      }
      return ret;
    }
    
    WordRef at(size_t w)
    {
      return {&vocabulary[w], w};
    }
    
    std::vector<size_t> search(const std::string &w)
    {
      std::vector<size_t> ret;
      for (size_t i = 0; i < vocabulary.size(); ++i)
      {
        if (vocabulary[i].word == w)
          ret.emplace_back(i);
      }
      return ret;
    }
    
    size_t size() const { return vocabulary.size(); }
    
    const auto &get_voc() const { return vocabulary; }
  };
}
#endif