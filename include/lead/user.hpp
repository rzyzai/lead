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
#include "meta.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <memory>

namespace lead
{
  constexpr size_t planned_review_times = 30;
  const std::vector<std::string> settings_list
      {
          "memorize_autoplay"
      };
  
  struct WordRecord
  {
    size_t points;
    
    WordRecord() : points(planned_review_times) {}
  };
  
  void to_json(nlohmann::json &j, const lead::WordRecord &p);
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p);
  
  bool check_settings(const nlohmann::json &s);
  
  enum class UserManagerStatus
  {
    success,
    user_not_found,
    user_already_exists,
    incorrect_username_or_password,
    db_error
  };
  
  std::string to_string(UserManagerStatus s);
  
  class UserManager;
  
  class UserRef
  {
    friend UserManager;
  public:
    size_t userid;
    std::string username;
    std::string email;
    std::string profile_picture;
  private:
    std::string passwd;
    std::vector<WordRecord> word_records;
    std::vector<size_t> marked_words;
    size_t plan_pos;
    nlohmann::json settings;
    VOC *vocabulary;
    leveldb::DB *user_db;
  public:
    UserRef(size_t userid_, std::string username_, std::string email_, std::string passwd_, VOC *voc, leveldb::DB *db);
    
    UserRef(const nlohmann::json &config, VOC *voc, leveldb::DB *db);
    
    ~UserRef();
    
    WordRef get_memorize_word();
    
    WordRef prev_memorize_word();
    
    WordRef set_memorize_word(size_t index);
    
    WordRef curr_memorize_word() const;
    
    WordRecord *word_record(size_t w);
    
    nlohmann::json get_quiz(WordRef wr) const;
    
    void clear_word_records();
    
    void clear_marks();
    
    int mark_word(size_t index);
    
    int unmark_word(size_t index);
    
    bool is_marked(size_t index) const;
    
    nlohmann::json get_marked() const;
    
    nlohmann::json get_passed() const;
    
    nlohmann::json get_plan() const;
    
    nlohmann::json get_settings() const;
    
    nlohmann::json update_settings(const nlohmann::json &settings_);
  
    leveldb::Status write_records();
  };
  
  class UserManager
  {
  private:
    leveldb::DB *db;
  public:
    Meta meta;
    VOC* vocabulary;
    std::string record_path;
  public:
    UserManager(const std::string &record_path, VOC* voc);
    
    ~UserManager();
    
    std::tuple<UserManagerStatus, std::unique_ptr<UserRef>>
    create_user(const std::string &username, const std::string &email, const std::string &passwd);
    
    std::tuple<UserManagerStatus, std::unique_ptr<UserRef>>
    get_user(const std::string &username, const std::string &passwd);
  
    void update_meta() const;
  };
}
#endif