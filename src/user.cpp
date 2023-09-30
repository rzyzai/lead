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
#include "lead/user.hpp"
#include "lead/voc.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>

namespace lead
{
  void to_json(nlohmann::json &j, const lead::WordRecord &p)
  {
    j = p.points;
  }
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p)
  {
    j.get_to(p.points);
  }
  
  bool check_settings(const nlohmann::json &s)
  {
    for (auto &r: settings_list) { if (!s.contains(r)) return false; }
    return true;
  }
  
  std::string to_string(UserManagerStatus s)
  {
    switch (s)
    {
      case UserManagerStatus::success:
        return "Success";
      case UserManagerStatus::user_not_found:
        return "User not found";
      case UserManagerStatus::user_already_exists:
        return "User already exists";
      case UserManagerStatus::incorrect_username_or_password:
        return "Incorrect user id or password";
      case UserManagerStatus::db_error:
        return "DB error";
    }
    return "Unknown";
  }
  
  UserRef::UserRef(size_t userid_, std::string username_, std::string email_, std::string passwd_, VOC* voc, leveldb::DB* db)
  : userid(userid_), username(std::move(username_)), email(std::move(email_)), passwd(std::move(passwd_)),
  vocabulary(voc), user_db(db),
    word_records(voc->size()), plan_pos(0)
  {
    std::map<std::string, bool> settings_;
    for (auto &r: settings_list)
      settings_[r] = false;
    settings = settings_;
  }
  
  UserRef::UserRef(const nlohmann::json &config, VOC *voc, leveldb::DB *db)
  : vocabulary(voc), user_db(db)
  {
    word_records = config["record"].get<std::vector<WordRecord>>();
    marked_words = config["marked_words"].get<std::vector<size_t>>();
    plan_pos = config["plan_pos"].get<size_t>();
    userid = config["userid"].get<size_t>();
    username = config["username"].get<std::string>();
    profile_picture = config["profile_picture"].get<std::string>();
    email = config["email"].get<std::string>();
    passwd = config["passwd"].get<std::string>();
    settings = config["settings"];
    if (!check_settings(settings))
      throw std::runtime_error("Invalid settings.");
  }
  
  UserRef::~UserRef() {write_records();}
  
  void UserRef::clear_word_records()
  {
    word_records.clear();
    word_records.insert(word_records.end(), vocabulary->size(), {});
    plan_pos = 0;
  }
  
  void UserRef::clear_marks()
  {
    marked_words.clear();
  }
  
  WordRecord *UserRef::word_record(size_t w)
  {
    return &word_records[w];
  }
  
  WordRef UserRef::curr_memorize_word() const
  {
    return vocabulary->at(plan_pos);
  }
  
  WordRef UserRef::get_memorize_word()
  {
    if (plan_pos + 1 >= vocabulary->size()) return {};
    ++plan_pos;
    while (word_records[plan_pos].points == 0 && plan_pos < word_records.size()) ++plan_pos;
    return vocabulary->at(plan_pos);
  }
  
  WordRef UserRef::prev_memorize_word()
  {
    if (plan_pos != 0)
    {
      --plan_pos;
      while (word_records[plan_pos].points == 0 && plan_pos != 0) --plan_pos;
      if (word_records[plan_pos].points == 0) return {};
      return vocabulary->at(plan_pos);
    }
    return {};
  }
  
  WordRef UserRef::set_memorize_word(size_t index)
  {
    auto wr = vocabulary->at(index);
    if (wr.is_valid())
    {
      plan_pos = index;
    }
    return wr;
  }
  
  nlohmann::json UserRef::get_quiz(WordRef wr) const
  {
    if (!wr.is_valid())
      wr = vocabulary->at(utils::randnum<size_t>(0, vocabulary->size()));
    
    auto words = vocabulary->get_similiar_words(wr, 3, [](WordRef wr) -> bool { return true; });
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
  
  int UserRef::mark_word(size_t index)
  {
    if (std::find(marked_words.begin(), marked_words.end(), index) != marked_words.end())
      return -1;
    marked_words.emplace_back(index);
    return 0;
  }
  
  int UserRef::unmark_word(size_t index)
  {
    if (auto it = std::find(marked_words.begin(), marked_words.end(), index); it != marked_words.end())
      marked_words.erase(it);
    else
      return -1;
    return 0;
  }
  
  bool UserRef::is_marked(size_t index) const
  {
    return std::find(marked_words.begin(), marked_words.end(), index) != marked_words.end();
  }
  
  nlohmann::json UserRef::get_marked() const
  {
    std::vector<nlohmann::json> ret_marked_words;
    for (auto &r: marked_words)
    {
      WordRef word = vocabulary->at(r);
      ret_marked_words.emplace_back(nlohmann::json{{"word", word}});
    }
    
    return {{"status",       "success"},
            {"marked_words", ret_marked_words}};
  }
  
  nlohmann::json UserRef::get_passed() const
  {
    std::vector<nlohmann::json> ret_passed_words;
    for (size_t i = 0; i < word_records.size(); ++i)
    {
      if (word_records[i].points == 0)
      {
        auto word = vocabulary->at(i);
        ret_passed_words.emplace_back(nlohmann::json{{"word", word}});
      }
    }
    
    return {{"status",            "success"},
            {"passed_word_count", ret_passed_words.size()},
            {"word_count",        vocabulary->size()},
            {"passed_words",      ret_passed_words}};
  }
  
  nlohmann::json UserRef::get_plan() const
  {
    size_t passed = 0;
    for (size_t i = 0; i < vocabulary->size(); ++i)
    {
      if (word_records[i].points == 0)
        ++passed;
    }
    return {{"status",              "success"},
            {"finished_word_count", passed},
            {"planned_word_count",  vocabulary->size()}};
  }
  
  nlohmann::json UserRef::get_settings() const
  {
    return {{"status",   "success"},
            {"settings", settings}
    };
  }
  
  nlohmann::json UserRef::update_settings(const nlohmann::json &settings_)
  {
    if (!check_settings(settings_))
      return {{"status",  "failed"},
              {"message", "Invalid settings"}};
    settings = settings_;
    return {{"status", "success"}};
  }
  
  leveldb::Status UserRef::write_records()
  {
    return user_db->Put(leveldb::WriteOptions(), username,
                        nlohmann::json{
                            {"userid",          userid},
                            {"username",        username},
                            {"passwd",          passwd},
                            {"email",           email},
                            {"profile_picture", profile_picture},
                            {"record",          word_records},
                            {"marked_words",    marked_words},
                            {"plan_pos",        plan_pos},
                            {"settings",        settings}}.dump());
  
  }
  
  UserManager::UserManager(const std::string &record_path_, VOC* voc)
  : vocabulary(voc), record_path(record_path_)
  {
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(options, record_path + "/userdb", &db);
    assert(status.ok());
    auto ur = std::make_unique<UserRef>(0, "__lead_guest__", "lead@rzyzai.tech",  "__lead_guest__", vocabulary, db);
    assert(ur->write_records().ok());
    std::ifstream meta_file(record_path + "/meta.json");
    nlohmann::json::parse(meta_file).get_to(meta);
  }
  
  UserManager::~UserManager()
  {
    delete db;
  }
  
  std::tuple<UserManagerStatus, std::unique_ptr<UserRef>>
  UserManager::create_user(const std::string &username, const std::string &email, const std::string &passwd)
  {
    std::string value;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), username, &value);
    if(s.ok())
      return {UserManagerStatus::user_already_exists, nullptr};
    if(s.IsNotFound())
    {
      auto ur = std::make_unique<UserRef>(meta.user_count, username, email, passwd, vocabulary, db);
      if (ur->write_records().ok())
      {
        meta.user_count++;
        update_meta();
        return {UserManagerStatus::success, std::move(ur)};
      }
      return {UserManagerStatus::db_error, nullptr};
    }
    else
      return {UserManagerStatus::db_error, nullptr};
  }
  
  std::tuple<UserManagerStatus, std::unique_ptr<UserRef>>
  UserManager::get_user(const std::string &username, const std::string &passwd)
  {
    std::string value;
    leveldb::Status s = db->Get(leveldb::ReadOptions(), username, &value);
    if (s.ok())
    {
      auto ur = std::make_unique<UserRef>(nlohmann::json::parse(value), vocabulary, db);
      if (passwd == ur->passwd)
      {
        return {UserManagerStatus::success, std::move(ur)};
      }
      else
      {
        return {UserManagerStatus::incorrect_username_or_password, nullptr};
      }
    }
    else if (s.IsNotFound())
    {
      return {UserManagerStatus::user_not_found, nullptr};
    }
    else
    {
      return {UserManagerStatus::db_error, nullptr};
    }
  }
  
  void UserManager::update_meta() const
  {
    std::fstream meta_file(record_path + "/meta.json", std::ios::out | std::ios::trunc);
    nlohmann::json json;
    to_json(json, meta);
    meta_file << json.dump();
    meta_file.close();
  }
}
