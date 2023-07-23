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
#include "lead/user.hpp"
#include "lead/voc.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace lead
{
  void to_json(nlohmann::json &j, const lead::WordRecord &p)
  {
    j = nlohmann::json{p.points};
  }
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p)
  {
    j.get_to(p.points);
  }
  
  User::User(const std::string &voc_dir_path)
  {
    vocabulary.load(voc_dir_path + "/" + "index.json", voc_dir_path + "/" + "data.json");
    word_records.insert(word_records.end(), vocabulary.size(), {});
  }
  
  WordRef User::get_word(size_t w) const
  {
    return vocabulary.at(w);
  }
  
  WordRef User::get_random_word() const
  {
    std::vector<size_t> candidate;
    for (size_t i = 0; i < word_records.size(); ++i)
    {
      if (word_records[i].points != 0)
        candidate.emplace_back(i);
    }
    if (candidate.empty()) return {};
    size_t index = candidate[utils::randnum<size_t>(0, candidate.size())];
    return vocabulary.at(index);
  }
  
  WordRecord &User::word_record(size_t w)
  {
    return word_records[w];
  }
  
  std::string User::get_explanation(size_t index) const
  {
    return vocabulary.get_explanation(index);
  }
  
  nlohmann::json User::get_quiz(WordRef wr) const
  {
    auto words = vocabulary.get_similiar_words(wr, 3, [this](WordRef wr) -> bool
    {
      if (word_records[wr.index].points == 0) return false;
      return true;
    });
    std::vector<std::string> opt{"A", "B", "C", "D"};
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(opt.begin(), opt.end(), g);
    static bool flag = true;
    if (flag)
    {
      flag = false;
      return {
          {"status",   "success"},
          {"question", wr.word->word},
          {"options",  {
                           {opt[0], words[0].word->meaning},
                           {opt[1], words[1].word->meaning},
                           {opt[2], words[2].word->meaning},
                           {opt[3], wr.word->meaning}}},
          {"indexes",  {
                           {opt[0], words[0].index},
                           {opt[1], words[1].index},
                           {opt[2], words[2].index},
                           {opt[3], wr.index}}},
          {"answer",   opt[3]}
      };
    }
    else
    {
      flag = true;
      return {
          {"status",   "success"},
          {"question", wr.word->meaning},
          {"options",  {
                           {opt[0], words[0].word->word},
                           {opt[1], words[1].word->word},
                           {opt[2], words[2].word->word},
                           {opt[3], wr.word->word}}},
          {"indexes",  {
                           {opt[0], words[0].index},
                           {opt[1], words[1].index},
                           {opt[2], words[2].index},
                           {opt[3], wr.index}}},
          {"answer",   opt[3]}
      };
    }
    return {};
  }
  
  nlohmann::json User::search(const std::string &word) const
  {
    auto wr = vocabulary.search(word);
    std::vector<std::string> explanations;
    for (auto &r: wr)
      explanations.emplace_back(get_explanation(r));
    if (!wr.empty())
    {
      return {{"status",       "success"},
              {"indexes",      wr},
              {"explanations", explanations},
              {"message",      "找到了" + std::to_string(wr.size()) + "个结果"}};
    }
    return {{"status",  "failed"},
            {"message", "没有找到" + word}};
  }
  
  nlohmann::json User::get_progress() const
  {
    size_t passed = 0;
    for (size_t i = 0; i < word_records.size(); ++i)
    {
      if (word_records[i].points == 0)
        ++passed;
    }
    return {{"status",            "success"},
            {"passed_word_count", passed},
            {"word_count",        word_records.size()}};
  }
}
