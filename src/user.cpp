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
    j =  p.points;
  }
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p)
  {
    j.get_to(p.points);
  }
  
  User::User(const std::string &voc_dir_path, const std::string &record_dir_path)
  {
    std::cout << "Loading vocabulary at '" << voc_dir_path << "'." << std::endl;
    vocabulary.load(voc_dir_path + "/" + "voc.json");
    std::filesystem::path record_path_fs = record_dir_path;
    record_path_fs /= "record.json";
    record_path = record_path_fs;
    std::cout << "Loading record at '" << record_path << "'." << std::endl;
    std::ifstream record_file(record_path);
    
    try
    {
      nlohmann::json record = nlohmann::json::parse(record_file);
      if (record["record"].size() != vocabulary.size())
      {
        std::cerr << "Records don't match with vocabulary. Ignored '" << record_path << "'." << std::endl;
        word_records.insert(word_records.end(), vocabulary.size(), {});
      }
      else
        word_records = record["record"].get<std::vector<WordRecord>>();
      marked_words = record["marked_words"].get<std::vector<size_t>>();
    }
    catch(...)
    {
      std::cerr << "Exception occurred when parsing 'record.json'. Ignored '" << record_path << "'." << std::endl;
      word_records.insert(word_records.end(), vocabulary.size(), {});
    }
    record_file.close();
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
  
  WordRef User::memorize_word()
  {
    return get_random_word();
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
    std::vector<nlohmann::json> words;
    for (auto &r: wr)
    {
      WordRef word = vocabulary.at(r);
      words.emplace_back(nlohmann::json{{"word", word.word->word},
                                        {"word_index", r},
                                        {"meaning", word.word->meaning},
                                        {"explanation", get_explanation(r)},
                                        {"is_marked", is_marked(r)}});
    }
    if (!wr.empty())
    {
      return {{"status",       "success"},
              {"words",        words},
              {"message",      "找到了" + std::to_string(wr.size()) + "个结果"}};
    }
    return {{"status",  "failed"},
            {"message", "没有找到" + word}};
  }
  
  int User::mark_word(size_t index)
  {
    if(std::find(marked_words.begin(), marked_words.end(), index) != marked_words.end())
      return -1;
    marked_words.emplace_back(index);
    return 0;
  }
  
  int User::unmark_word(size_t index)
  {
    if(auto it = std::find(marked_words.begin(), marked_words.end(), index); it != marked_words.end())
      marked_words.erase(it);
    else
      return -1;
    return 0;
  }
  
  bool User::is_marked(size_t index) const
  {
    return std::find(marked_words.begin(), marked_words.end(), index) != marked_words.end();
  }
  
  nlohmann::json User::get_record() const
  {
    size_t passed = 0;
    for (size_t i = 0; i < word_records.size(); ++i)
    {
      if (word_records[i].points == 0)
        ++passed;
    }
  
    std::vector<nlohmann::json> ret_marked_words;
    for (auto &r: marked_words)
    {
      WordRef word = vocabulary.at(r);
      ret_marked_words.emplace_back(nlohmann::json{{"word",        word.word->word},
                                                   {"word_index", word.index},
                                                   {"meaning",     word.word->meaning},
                                                   {"explanation", get_explanation(r)}});
    }
    
    return {{"status",            "success"},
            {"passed_word_count", passed},
            {"word_count",        word_records.size()},
            {"marked_words", ret_marked_words}};
  }
  
  void User::write_records()
  {
    std::fstream record_file(record_path, std::ios::out | std::ios::trunc);
    record_file << nlohmann::json{{"record", word_records}, {"marked_words", marked_words}}.dump();
    record_file.close();
  }
}
