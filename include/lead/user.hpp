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
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <map>

namespace lead
{
  struct WordRecord
  {
    size_t points;
  };
  
  void to_json(nlohmann::json &j, const lead::WordRecord &p)
  {
    j = nlohmann::json{p.points};
  }
  
  void from_json(const nlohmann::json &j, lead::WordRecord &p)
  {
    j.get_to(p.points);
  }
  
  class UserManager;
  
  class UserRef
  {
    friend void to_json(nlohmann::json &j, const lead::UserRef &p);
    
    friend class UserManager;
  
  private:
    bool valid;
    std::string user_id;
    std::string password;
    VOC *current_voc;
    size_t voc_pos;
    std::vector<WordRecord> word_records;
    leveldb::DB *db;
  public:
    UserRef() : valid(false), db(nullptr) {}
    
    UserRef(const std::string &user_info, leveldb::DB *db_, std::vector<VOC> *vocs)
        : db(db_), valid(true)
    {
      nlohmann::json info{user_info};
      info["user_id"].get_to<std::string>(user_id);
      info["password"].get_to<std::string>(password);
      info["current_voc"].get_to<size_t>(voc_pos);
      current_voc = &(*vocs)[voc_pos];
      for (auto &r: info["words_points"])
      {
        word_records.emplace_back(r.get<WordRecord>());
      }
    }
    
    UserRef(std::string userid, std::string passwd, leveldb::DB *db_, std::vector<VOC> *vocs)
        : user_id(std::move(userid)), password(std::move(passwd)), db(db_), valid(true), voc_pos(0),
          current_voc(&(*vocs)[0])
    {
      word_records.insert(word_records.begin(), current_voc->size(), WordRecord{10});
    }
    
    ~UserRef()
    {
      if (user_id != "__lead_guest__")
      {
        leveldb::Status s = db->Put(leveldb::WriteOptions(), user_id, nlohmann::json{*this}.dump());
      }
    }
    
    bool is_valid() const
    {
      return valid;
    }
    
    WordRef get_word(size_t w)
    {
      return current_voc->at(w);
    }
    
    WordRef get_random_word()
    {
      std::vector<size_t> candidate;
      for (size_t i = 0; i < word_records.size(); ++i)
      {
        if (word_records[i].points != 0)
        {
          candidate.emplace_back(i);
        }
      }
      size_t pos = candidate[utils::randnum<size_t>(0, candidate.size())];
      return current_voc->at(pos);
    }
    
    WordRecord &word_record(size_t w)
    {
      return word_records[w];
    }
  
    nlohmann::json get_quiz(WordRef wr) const
    {
      auto words = current_voc->get_similiar_words(wr, 3);
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
            {"answer",   opt[3]}
        };
      }
      return {};
    }
    nlohmann::json get_examples(size_t pos) const
    {
      auto wr = current_voc->at(pos);
      return wr.word->examples;
    }
    
    nlohmann::json search(const std::string &word)
    {
      WordRef wr = current_voc->search(word);
      if (wr.is_valid())
      {
        return {{"status",  "success"},
                {"pos",     wr.pos},
                {"message", wr.word->word}};
      }
      return {{"status",  "failed"},
              {"message", "没有找到" + word}};
    }
  };
  
  void to_json(nlohmann::json &j, const lead::UserRef &p)
  {
    j = nlohmann::json{
        {"user_id",     p.user_id},
        {"password",    p.password},
        {"current_voc", p.voc_pos},
        {"word_points", p.word_records}
    };
  }
  
  enum class UserManagerStatus
  {
    success,
    user_not_found,
    incorrect_userid_or_password,
    db_error
  };
  
  std::string to_string(UserManagerStatus s)
  {
    switch (s)
    {
      case UserManagerStatus::success:
        return "Success";
      case UserManagerStatus::user_not_found:
        return "User not found";
      case UserManagerStatus::incorrect_userid_or_password:
        return "Incorrect user id or password";
      case UserManagerStatus::db_error:
        return "DB error";
    }
    return "Unknown";
  }
  
  class UserManager
  {
  private:
    std::vector<VOC> vocabularies;
    leveldb::DB *db;
  public:
    UserManager(const std::string &db_path, const std::string &voc_dir_path)
    {
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, db_path, &db);
      assert(status.ok());
      
      for (auto &dir: std::filesystem::directory_iterator(voc_dir_path))
      {
        vocabularies.emplace_back(VOC{});
        if (!dir.is_directory())
        {
          vocabularies.back().set_name(dir.path().filename());
          vocabularies.back().load(dir.path());
        }
      }
    }
    
    ~UserManager()
    {
      delete db;
    }
    
    std::tuple<UserManagerStatus, UserRef> create_user(const std::string &userid, const std::string &passwd)
    {
      UserRef ur{userid, passwd, db, &vocabularies};
      leveldb::Status s = db->Put(leveldb::WriteOptions(), userid, nlohmann::json{ur}.dump());
      if (s.ok()) return {UserManagerStatus::success, ur};
      return {UserManagerStatus::db_error, {}};
    }
    
    std::tuple<UserManagerStatus, UserRef> get_user(const std::string &userid, const std::string &passwd)
    {
      if (userid == "__lead_guest__" && passwd == "__lead_guest__")
      {
        return {UserManagerStatus::success, {"__lead_guest__", "__lead_guest__", db, &vocabularies}};
      }
      std::string value;
      leveldb::Status s = db->Get(leveldb::ReadOptions(), userid, &value);
      UserRef ur{value, db, &vocabularies};
      if (s.ok())
      {
        if (passwd == ur.password)
        {
          return {UserManagerStatus::success, ur};
        }
        else
        {
          return {UserManagerStatus::incorrect_userid_or_password, {}};
        }
      }
      else if (s.IsNotFound())
      {
        return {UserManagerStatus::user_not_found, {}};
      }
      else
      {
        return {UserManagerStatus::db_error, {}};
      }
    }
  };
}
#endif