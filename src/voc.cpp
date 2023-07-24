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
#include "lead/voc.hpp"
#include "lead/utils.hpp"
#include "bundled/nlohmann/json.hpp"
#include <tuple>
#include <string>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>
#include <random>

namespace lead
{
  void to_json(nlohmann::json &j, const lead::Word &p)
  {
    j = nlohmann::json{{"word",    p.word},
                       {"meaning", p.meaning}};
  }
  
  void to_json(nlohmann::json &j, const lead::WordRef &p)
  {
    j = nlohmann::json{{"word",  *p.word},
                       {"index", p.index}};
  }
  
  void VOC::load(const std::vector<Word> &word)
  {
    vocabulary = word;
  }
  
  void VOC::load(const std::string &voc_index_path, const std::string &voc_data_path)
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
  
  std::string VOC::get_explanation(size_t index) const
  {
    return vocabulary[index].explanation;
  }
  
  std::vector<WordRef> VOC::get_similiar_words(WordRef wr, size_t n, const std::function<bool(WordRef)> &selector) const
  {
    std::vector<WordRef> ret;
    const auto distance_cmp = [](auto &&p1, auto &&p2) { return p1.second < p2.second; };
    std::multiset<std::pair<size_t, int>, decltype(distance_cmp)> similiar(distance_cmp); // index, distance
    const auto is_ambiguous = [&wr, &similiar](size_t i) -> bool
    {
      int min_distance = 0;
      min_distance = (std::abs)(static_cast<int>(wr.index - i));
      for(auto& r : similiar)
        min_distance = (std::min)((std::abs)(static_cast<int>(r.first - i)), min_distance);
      return (min_distance < 10);
    };
    
    for (size_t i = 0; i < vocabulary.size(); ++i)
    {
      if (is_ambiguous(i) || !selector(at(i))) continue;
      auto d = utils::get_edit_distance(vocabulary[i].word, wr.word->word);
      if (similiar.size() < n)
      {
        similiar.insert({i, d});
      }
      else if (d < similiar.rbegin()->second)
      {
        similiar.erase(std::prev(similiar.end()));
        similiar.insert({i, d});
      }
    }

    for (auto &r: similiar)
      ret.emplace_back(WordRef{&vocabulary[r.first], r.first});
    return ret;
  }
  
  WordRef VOC::at(size_t w) const
  {
    return {&vocabulary[w], w};
  }
  
  std::vector<size_t> VOC::search(const std::string &w) const
  {
    std::vector<size_t> ret;
    for (size_t i = 0; i < vocabulary.size(); ++i)
    {
      if (vocabulary[i].word == w || vocabulary[i].meaning.find(w) != std::string::npos)
        ret.emplace_back(i);
    }
    return ret;
  }
  
  size_t VOC::size() const { return vocabulary.size(); }
}