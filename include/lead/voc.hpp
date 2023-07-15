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
  class Word
  {
  public:
    int reviewed_times;
    int planned_times;
    std::string word;
    std::string meaning;
    std::string pronunciation;
    std::vector<std::tuple<std::string, std::string>> examples;
    
    Word(std::string word_, std::string meaning_, std::string pronunciation_)
        : reviewed_times(0), planned_times(10), word(std::move(word_)),
        meaning(std::move(meaning_)), pronunciation(pronunciation_) {}
  };
  
  void to_json(nlohmann::json &j, const lead::Word &p)
  {
    j = nlohmann::json{{"word",           p.word},
                       {"meaning",        p.meaning},
                       {"reviewed_times", p.reviewed_times},
                       {"planned_times",  p.planned_times},
                       {"examples",       p.examples}};
  }
  
  struct WordRef
  {
    Word * word;
    size_t pos;
  };
  
  void to_json(nlohmann::json &j, const lead::WordRef &p)
  {
    j = nlohmann::json{{"word", *p.word},
                       {"pos",  p.pos}};
  }
  
  class VOC
  {
  private:
    std::vector<Word> vocabulary;
    std::set<size_t> current;
  public:
    void load(const std::vector<Word> &word)
    {
      vocabulary = word;
      current.clear();
      for (size_t i = 0; i < vocabulary.size(); ++i)
        current.insert(i);
    }
    
    void load(const std::string &path)
    {
      std::ifstream f(path);
      nlohmann::json data = nlohmann::json::parse(f);
      for (auto &r: data)
      {
        vocabulary.emplace_back(Word{r["word"].get<std::string>(), r["meaning"].get<std::string>(),
            r["pronunciation"].get<std::string>()});
        current.insert(vocabulary.size() - 1);
      }
    }
    
    const WordRef get_a_word()
    {
      size_t pos = utils::randint(0, vocabulary.size());
      return {&vocabulary[pos], pos};
    }
  
     WordRef at(size_t w)
    {
      return {&vocabulary[w], w};
    }
    
    void pass(size_t pos)
    {
      current.erase(pos);
    }
    
    const auto &get_voc() const { return vocabulary; }
    
    nlohmann::json generate_a_quiz(WordRef wr) const
    {
      int a = 0;
      do utils::randint(0, vocabulary.size());
      while(a == wr.pos && vocabulary.size() > 1);
      int b = 0;
      do b = utils::randint(0, vocabulary.size());
      while((b == a || b == wr.pos) && vocabulary.size() > 2);
      int c = 0;
      do c = utils::randint(0, vocabulary.size());
      while((c == b || c == a || c == wr.pos) && vocabulary.size() > 3);
      std::vector<std::string> opt{"A", "B", "C", "D"};
      std::random_shuffle(opt.begin(), opt.end());
      return {{"options", {
        {opt[0], vocabulary[a].meaning},
        {opt[1], vocabulary[b].meaning},
        {opt[2], vocabulary[c].meaning},
        {opt[3], wr.word->meaning}}},
              {"answer", opt[3]}
      };
    }
  };
}
#endif