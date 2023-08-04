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
#include <vector>
#include <set>
#include <functional>

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
  
  void VOC::load(const std::string &voc_path)
  {
    std::ifstream voc_file(voc_path);
    nlohmann::json voc = nlohmann::json::parse(voc_file);
    for (auto &r: voc)
    {
      vocabulary.emplace_back(
          Word{
              .word = r["word"].get<std::string>(),
              .meaning = r["meaning"].get<std::string>(),
              .detail = r["detail"]
          });
    }
    voc_file.close();
  }
  
  void parse_example(const nlohmann::json &data, std::string &ret);
  
  void parse_pattern(const nlohmann::json &data, std::string &ret);
  
  void parse_note(const nlohmann::json &data, std::string &ret);
  
  void parse_explanation(const nlohmann::json &data, std::string &ret, const std::string &mark = "");
  
  void parse_discrimination(const nlohmann::json &data, std::string &ret);
  
  void parse_collocation(const nlohmann::json &data, std::string &ret);
  
  void parse_quiz_question(const nlohmann::json &data, std::string &ret);
  
  void parse_quiz_answer(const nlohmann::json &data, std::string &ret);
  
  void try_parse_examples(const nlohmann::json &data, std::string &ret);
  
  
  void try_parse_patterns(const nlohmann::json &data, std::string &ret);
  
  
  void try_parse_notes(const nlohmann::json &data, std::string &ret);
  
  
  void try_parse_discriminations(const nlohmann::json &data, std::string &ret);
  
  
  void try_parse_collocations(const nlohmann::json &data, std::string &ret);
  
  void try_parse_explanations(const nlohmann::json &data, std::string &ret);
  
  void parse_example(const nlohmann::json &data, std::string &ret)
  {
    auto en = data["en"].get<std::string>();
    // TODO highlight
    ret += "<div class=\"mdui-typo\"><blockquote><span id=\"example\" class=\"example\">" + en;
    ret += data["chi"].get<std::string>() + "</span>";
    if (data.contains("source"))
      ret += "<footer>" + data["source"].get<std::string>() + "</footer>";
    ret += "</div>";
  }
  
  void parse_note(const nlohmann::json &data, std::string &ret)
  {
    // Examples
    try_parse_examples(data, ret);
    // Sentential Patterns
    try_parse_patterns(data, ret);
  }
  
  void parse_discrimination(const nlohmann::json &data, std::string &ret)
  {
    ret += "<span class=\"discrimination\">辨析 ";
    for (auto &word: data["words"])
      ret += word["word"].get<std::string>() + ", ";
    ret.pop_back();
    ret.pop_back();
    ret += "";
    ret += "</span><br/>";
    // Explanation
    if (data.contains("explanation"))
      ret += data["explanation"].get<std::string>();
    // Examples
    try_parse_examples(data, ret);
    // Discrimination word
    ret += "<ol class=\"mdui-list\">";
    for (auto &r: data["words"])
    {
      ret += "<strong>" + r["word"].get<std::string>() + "</strong>" + r["desc"].get<std::string>();
      if (r.contains("examples"))
      {
        for (auto &t: r["examples"])
          parse_example(t, ret);
      }
    }
    ret += "</ol>";
  }
  
  void parse_pattern(const nlohmann::json &data, std::string &ret)
  {
    ret += data["pattern"].get<std::string>();
    // Examples
    try_parse_examples(data, ret);
    
    // Notes
    try_parse_notes(data, ret);
    
    // Discriminations
    try_parse_discriminations(data, ret);
  }
  
  
  void parse_collocation(const nlohmann::json &data, std::string &ret)
  {
    ret += "<span class=\"collocation\">" + data["word"].get<std::string>() + "</span>";
    // Sentential Patterns
    try_parse_explanations(data, ret);
  }
  
  void parse_explanation(const nlohmann::json &data, std::string &ret, const std::string &mark)
  {
    // Meaning
    ret += "<span class=\"meaning\">" + data["chi"].get<std::string>() + "</span>";
    if (data["vital"].get<bool>())
      ret += " ★ ";
    ret += mark;
    // English translation
    if (data.contains("en"))
      ret += "(" + data["en"].get<std::string>() + ")";
    
    // Synonym
    if (data.contains("Synonym"))
      ret += "近 " + data["Synonym"].get<std::string>();
    
    // Antonym
    if (data.contains("antonym"))
      ret += "反 " + data["antonym"].get<std::string>();
    
    // Examples
    try_parse_examples(data, ret);
    
    // Sentential Patterns
    try_parse_patterns(data, ret);
    
    // Notes
    try_parse_notes(data, ret);
    
    // Discriminations
    try_parse_discriminations(data, ret);
    
    // Collocations
    try_parse_collocations(data, ret);
  }
  
  void parse_derivative(const nlohmann::json &data, std::string &ret)
  {
    ret += data["word"].get<std::string>();
    if (data.contains("ph"))
      ret += " " + data["ph"].get<std::string>();
    if (data.contains("pos"))
      ret += " " + data["pos"].get<std::string>();
    if (data.contains("usage"))
      ret += " " + data["usage"].get<std::string>();
    
    // Examples
    try_parse_examples(data, ret);
    
    // Sentential Patterns
    try_parse_patterns(data, ret);
    
    // Collocations
    try_parse_collocations(data, ret);
  }
  
  void parse_quiz_question(const nlohmann::json &data, std::string &ret)
  {
    // Subject
    ret += data["subject"].get<std::string>();
    
    // Source
    if (data.contains("source"))
      ret += " (" + data["source"].get<std::string>() + ")";
    
    // Options
    if (data.contains("option_A"))
      ret += "A. " + data["option_A"].get<std::string>();
    
    if (data.contains("option_B"))
      ret += "B. " + data["option_B"].get<std::string>();
    
    if (data.contains("option_C"))
      ret += "C. " + data["option_C"].get<std::string>();
    
    if (data.contains("option_D"))
      ret += "D. " + data["option_D"].get<std::string>();
    
  }
  
  void parse_quiz_answer(const nlohmann::json &data, std::string &ret)
  {
    ret += data["answer"].get<std::string>() + "  ";
  }
  
  
  void parse_list(const nlohmann::json &data, std::string &ret, const std::string &tag,
                  const std::function<void(const nlohmann::json &, std::string &)> &fn)
  {
    if (data.contains(tag))
    {
      ret += "<ul class=\"mdui-list\">";
      for (auto &r: data[tag])
      {
        ret += "<li>";
        fn(r, ret);
        ret += "</li>";
      }
      ret += "</ul>";
    }
  }
  
  void try_parse_examples(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "examples", parse_example);
  }
  
  void try_parse_patterns(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "patterns", parse_pattern);
  }
  
  void try_parse_notes(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "notes", parse_note);
  }
  
  void try_parse_discriminations(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "discriminations", parse_discrimination);
  }
  
  void try_parse_collocations(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "collocations", parse_collocation);
  }
  
  void try_parse_explanations(const nlohmann::json &data, std::string &ret)
  {
    parse_list(data, ret, "explanations", [](const nlohmann::json &data, std::string &ret){parse_explanation(data, ret);});
  }
  
  std::string VOC::get_explanation(size_t index) const
  {
    std::string ret;
    auto &detail = vocabulary[index].detail;
    // Word
    ret += "<h1>" + vocabulary[index].word;
    // part of speech
    if (detail.contains("pos"))
      ret += "<i>  " + detail["pos"].get<std::string>() + "</i>";
    // vital
    if (detail["vital"].get<bool>())
      ret += " ★";
    ret += "</h1>";
    // Pronunciation
    if (detail.contains("en_ph"))
      ret += "英 /" + detail["en_ph"].get<std::string>() + "/  ";
    if (detail.contains("usa_ph"))
      ret += "美 /" + detail["usa_ph"].get<std::string>() + "/  ";
    
    // Usage
    if (detail.contains("usage"))
      ret += detail["usage"].get<std::string>();
    
    // Explanations
    if (detail.contains("explanations"))
    {
      ret += "<h3>释义</h3>";
      ret += "<ol class=\"mdui-list\">";
      for (auto &r: detail["explanations"])
      {
        ret += "<li><span class=\"meaning\"><strong>";
        parse_explanation(r, ret, "</strong>");
        ret += "</span></li>";
      }
      ret += "</ol>";
    }
    
    // Collocations
    if (detail.contains("collocations"))
    {
      ret += "<h3>固定搭配</h3>";
      ret += "<ul class=\"mdui-list\">";
      for (auto &r: detail["collocations"])
      {
        ret += "<li>";
        parse_collocation(r, ret);
        ret += "</li>";
      }
      ret += "</ul>";
    }
    
    // Derivatives
    if (detail.contains("derivatives"))
    {
      ret += "<h3>派生词汇</h3>";
      ret += "<ul class=\"mdui-list\">";
      for (auto &r: detail["derivatives"])
      {
        ret += "<li>";
        parse_derivative(r, ret);
        ret += "</li>";
      }
      ret += "</ul>";
    }
    
    // Quizzes
    if (detail.contains("quizzes"))
    {
      ret += "<h3>题目解析</h3>";
      ret += "<ol class=\"mdui-list\">";
      for (auto &r: detail["quizzes"])
      {
        ret += "<li><span class=\"explanation_quiz\">";
        parse_quiz_question(r, ret);
        ret += "</span></li>";
      }
      ret += "</ol>";
      ret += "<h4>答案</h4>";
      ret += "<ol class=\"mdui-list\">";
      for (auto &r: detail["quizzes"])
      {
        ret += "<li><span class=\"explanation_quiz\">";
        parse_quiz_answer(r, ret);
        ret += "</span></li>";
      }
      ret += "</ol>";
    }
    return "<div class=\"mdui-typo\">" + ret + "</div>";
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
      for (auto &r: similiar)
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
    bool is_word = true;
    for(auto& r : w)
    {
      if(!std::isalpha(r))
        is_word = false;
    }
    for (size_t i = 0; i < vocabulary.size(); ++i)
    {
      if(is_word)
      {
        if (vocabulary[i].word == w)
          ret.emplace_back(i);
      }
      else
      {
        if (vocabulary[i].meaning.find(w) != std::string::npos)
          ret.emplace_back(i);
      }
    }
    return ret;
  }
  
  size_t VOC::size() const { return vocabulary.size(); }
  
}