// MIT License
//
// Copyright (c) 2023 rzyzai, and caozhanhao
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
#include <vector>
#include <deque>
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
    j = nlohmann::json{{"word",  p.word->word},
                       {"word_index", p.index},
                       {"meaning", p.word->meaning}};
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
// explanation generation test
//    for(size_t i = 0; i < vocabulary.size(); ++i)
//      get_explanation(i);
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
    if (data.contains("chi"))
    {
      ret += +"<br/>" + data["chi"].get<std::string>();
    }
    
    ret += "</span>";
    if (data.contains("source"))
    {
      ret += "<footer>" + data["source"].get<std::string>() + "</footer>";
    }
    ret += "</div>";
  }
  
  void parse_note(const nlohmann::json &data, std::string &ret)
  {
    ret += "<span class=\"notes\">注：" + data["note"].get<std::string>();
    // Examples
    try_parse_examples(data, ret);
    // Sentential Patterns
    try_parse_patterns(data, ret);
    ret += "</span>";
  }
  
  void parse_discrimination(const nlohmann::json &data, std::string &ret)
  {
    ret += "<span class=\"discrimination\">辨析 ";
    if (data["words"].is_string())
    {
      ret += data["words"].get<std::string>();
    }
    else
    {
      for (auto &word: data["words"])
      {
        ret += word["word"].get<std::string>() + ", ";
      }
      if (ret[ret.size() - 2] == ',' && ret.back() == ' ')
      {
        ret.pop_back();
        ret.pop_back();
      }
    }
    ret += "</span><br/>";
    // Explanation
    if (data.contains("explanation"))
    {
      ret += data["explanation"].get<std::string>();
    }
    // Examples
    try_parse_examples(data, ret);
    // Discrimination word
    if (data["words"].is_array())
    {
      ret += "<ol class=\"mdui-list\">";
      for (auto &r: data["words"])
      {
        ret += "<strong>" + r["word"].get<std::string>() + "</strong>";
        if (r.contains("desc"))
        {
          ret += r["desc"].get<std::string>();
        }
        if (r.contains("examples"))
        {
          for (auto &t: r["examples"])
          {
            parse_example(t, ret);
          }
        }
      }
      ret += "</ol>";
    }
  }
  
  void parse_pattern(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("pattern"))
    {
      ret += data["pattern"].get<std::string>();
    }
    
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
    {
      ret += " ★ ";
    }
    ret += mark;
    // English translation
    if (data.contains("en"))
    {
      ret += "(" + data["en"].get<std::string>() + ")";
    }
    
    // Synonym
    if (data.contains("synonym"))
    {
      ret += "近 " + data["synonym"].get<std::string>();
    }
    
    // Antonym
    if (data.contains("antonym"))
    {
      ret += "反 " + data["antonym"].get<std::string>();
    }
    
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
    
    if (data.contains("pos"))
    {
      ret += "<i>  " + data["pos"].get<std::string>() + "</i>";
    }
    
    if (data.contains("ph"))
    {
      ret += " " + data["ph"].get<std::string>();
    }
    
    if (data.contains("usage"))
    {
      ret += "<br/>" + data["usage"].get<std::string>();
    }
    
    // Examples
    try_parse_examples(data, ret);
    
    // Sentential Patterns
    try_parse_patterns(data, ret);
    
    // Collocations
    try_parse_collocations(data, ret);
  }
  
  void parse_quiz_question(const nlohmann::json &data, std::string &ret)
  {
    // Source
    if (data.contains("source"))
    {
      ret += "[" + data["source"].get<std::string>() + "] ";
    }
    // Subject
    ret += data["subject"].get<std::string>();
    
    // Options
    if (data.contains("option_A"))
    {
      ret += "<br/>A. " + data["option_A"].get<std::string>();
    }
    
    if (data.contains("option_B"))
    {
      ret += "<br/>B. " + data["option_B"].get<std::string>();
    }
    
    if (data.contains("option_C"))
    {
      ret += "<br/>C. " + data["option_C"].get<std::string>();
    }
    
    if (data.contains("option_D"))
    {
      ret += "<br/>D. " + data["option_D"].get<std::string>();
    }
    
  }
  
  void parse_quiz_answer(const nlohmann::json &data, std::string &ret)
  {
    ret += data["answer"].get<std::string>() + "  ";
  }
  
  
  void parse_list(const nlohmann::json &data, std::string &ret, const std::string &list_type, const std::string &tag,
                  const std::function<void(const nlohmann::json &, std::string &)> &fn)
  {
    if (data.contains(tag))
    {
      ret += "<" + list_type + " class=\"mdui-list\">";
      for (auto &r: data[tag])
      {
        ret += "<li>";
        fn(r, ret);
        ret += "</li>";
      }
      ret += "</" + list_type + ">";
    }
  }
  
  void try_parse_examples(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("examples"))
    {
      for (auto &r: data["examples"])
      {
        parse_example(r, ret);
      }
    }
  }
  
  void try_parse_patterns(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("patterns"))
    {
      parse_list(data, ret, "ul", "patterns", parse_pattern);
    }
  }
  
  void try_parse_notes(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("notes"))
    {
      parse_list(data, ret, "ul", "notes", parse_note);
    }
  }
  
  void try_parse_discriminations(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("discriminations"))
    {
      parse_list(data, ret, "ul", "discriminations", parse_discrimination);
    }
  }
  
  void try_parse_collocations(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("collocations"))
    {
      parse_list(data, ret, "ul", "collocations", parse_collocation);
    }
  }
  
  void try_parse_explanations(const nlohmann::json &data, std::string &ret)
  {
    if (data.contains("explanations"))
    {
      ret += "<ol class=\"mdui-list\">";
      for (auto &r: data["explanations"])
      {
        ret += "<li><strong>";
        parse_explanation(r, ret, "</strong>");
        ret += "</li>";
      }
      ret += "</ol>";
    }
  }
  
  std::string VOC::get_explanation(size_t index) const
  {
    std::string ret;
    auto &detail = vocabulary[index].detail;
    // Word
    ret += "<h1>" + vocabulary[index].word;
    // part of speech
    if (detail.contains("pos"))
    {
      ret += "<i>  " + detail["pos"].get<std::string>() + "</i>";
    }
    // vital
    if (detail["vital"].get<bool>())
    {
      ret += " ★";
    }
    ret += "<button class=\"mdui-btn mdui-ripple\">\n"
           "  <i class=\"mdui-icon material-icons\" onclick=\"speak(&quot;" + vocabulary[index].word
           + "&quot;)\">play_arrow</i></button>";
    ret += "</h1>";
    // Pronunciation
    if (detail.contains("en_ph"))
    {
      ret += "英 /" + detail["en_ph"].get<std::string>() + "/  ";
    }
    if (detail.contains("usa_ph"))
    {
      ret += "美 /" + detail["usa_ph"].get<std::string>() + "/  ";
    }
    
    // Usage
    if (detail.contains("usage"))
    {
      ret += "<br/>" + detail["usage"].get<std::string>();
    }
    
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
      {
        min_distance = (std::min)((std::abs)(static_cast<int>(r.first - i)), min_distance);
      }
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
    {
      ret.emplace_back(WordRef{&vocabulary[r.first], r.first});
    }
    
    while(ret.size() < n)
      ret.emplace_back(at(utils::randnum<size_t>(0, vocabulary.size())));
    
    return ret;
  }
  
  WordRef VOC::at(size_t w) const
  {
    return {&vocabulary[w], w};
  }
  
  bool contains(size_t search_pos, const std::string& str, size_t target_size)
  {
    if (search_pos != 0
        && std::isalpha(str[search_pos - 1]))
      return false;
    if (search_pos + str.size() < str.size()
        && std::isalpha(str[search_pos + target_size]))
      return false;
    return true;
  }
  
  std::deque<size_t> VOC::search(const std::string &w) const
  {
    std::deque<size_t> ret;
    for (size_t i = 0; i < vocabulary.size(); ++i)
    {
      if (auto search_pos = vocabulary[i].word.find(w); search_pos != std::string::npos)
      {
        if (!contains(search_pos, vocabulary[i].word, w.size()))
          continue;
        ret.emplace_front(i);
      }
      else if (auto search_pos = vocabulary[i].meaning.find(w); search_pos != std::string::npos)
      {
        if (!contains(search_pos, vocabulary[i].word, w.size()))
          continue;
        ret.emplace_back(i);
      }
    }
    return ret;
  }
  
  size_t VOC::size() const { return vocabulary.size(); }
}