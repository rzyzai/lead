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
#include "lead/server.hpp"
#include "lead/utils.hpp"
#include "lead/user.hpp"
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"
#include <string>
#include <chrono>
#include <vector>
#include <filesystem>

namespace lead
{
  Server::Server(const std::string addr, int port, const std::string &res_path_)
      : listen_addr(addr), listen_port(port), res_path(res_path_), user(res_path / "voc", res_path / "records") {}
  
  void Server::run()
  {
    httplib::Server svr;
    svr.set_mount_point("/", res_path / "index.html");
    svr.set_mount_point("/favicon.ico", res_path / "icon" / "favicon.ico");
    svr.set_mount_point("/", res_path / "html");
    svr.set_mount_point("/css", res_path / "css");
    svr.set_mount_point("/fonts", res_path / "fonts");
    svr.set_mount_point("/icons", res_path / "icons");
    svr.set_mount_point("/js", res_path / "js");
    svr.Get("/api/get_quiz", [this](const httplib::Request &req, httplib::Response &res)
    {
      WordRef wr;
      if (req.has_param("word_index"))
        wr = user.get_word(std::stoi(req.get_param_value("word_index")));
      auto quiz = user.get_quiz(wr);
      wr = user.vocabulary.at(quiz["indexes"][quiz["answer"]].get<int>());
      res.set_content(nlohmann::json{
          {"status", "success"},
          {"word",   wr},
          {"quiz",   quiz},
      }.dump(), "application/json");
      user.write_records();
    });
    svr.Get("/api/pass", [this](const httplib::Request &req, httplib::Response &res)
    {
      user.word_record(std::stoi(req.get_param_value("word_index")))->points = 0;
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
    svr.Get("/api/renew", [this](const httplib::Request &req, httplib::Response &res)
    {
      user.word_record(std::stoi(req.get_param_value("word_index")))->points = planned_review_times;
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
    
    svr.Get("/api/quiz_passed", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto &p = user.word_record(std::stoi(req.get_param_value("word_index")))->points;
      if (p != 0) --p;
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
    svr.Get("/api/quiz_failed", [this](const httplib::Request &req, httplib::Response &res)
    {
      user.word_record(std::stoi(req.get_param_value("word_index")))->points += 10;
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
    svr.Get("/api/quiz_prompt", [this](const httplib::Request &req, httplib::Response &res)
    {
      size_t index = std::stoi(req.get_param_value("word_index"));
      auto &p = user.word_record(index)->points;
      ++p;
      auto a = std::stoi(req.get_param_value("A_index"));
      auto b = std::stoi(req.get_param_value("B_index"));
      auto c = std::stoi(req.get_param_value("C_index"));
      auto d = std::stoi(req.get_param_value("D_index"));
      res.set_content(nlohmann::json{{"status", "success"},
                                     {"A",  {{"is_marked", user.is_marked(a)}, {"explanation", user.get_explanation(a)}}},
                                     {"B",  {{"is_marked", user.is_marked(b)}, {"explanation", user.get_explanation(b)}}},
                                     {"C",  {{"is_marked", user.is_marked(c)}, {"explanation", user.get_explanation(c)}}},
                                     {"D",  {{"is_marked", user.is_marked(d)}, {"explanation", user.get_explanation(d)}}}
      }.dump(), "application/json");
    });
    svr.Get("/api/search", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user.search(req.get_param_value("word")).dump(), "application/json");
    });
  
    svr.Get("/api/get_marked", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user.get_marked().dump(), "application/json");
    });
  
    svr.Get("/api/get_passed", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user.get_passed().dump(), "application/json");
    });
  
    svr.Get("/api/get_explanation", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto i = std::stoi(req.get_param_value("word_index"));
      res.set_content(nlohmann::json{{"status", "success"}, {"explanation", user.get_explanation(i)}}.dump(), "application/json");
    });
  
    svr.Get("/api/get_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto i = std::stoi(req.get_param_value("word_index"));
      res.set_content(nlohmann::json{{"status", "success"}, {"word", user.get_word(i).word->word}}.dump(), "application/json");
    });
    
    svr.Get("/api/get_plan", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user.get_plan().dump(), "application/json");
    });
    
    svr.Get("/api/mark_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      int ret = user.mark_word(std::stoi(req.get_param_value("word_index")));
      if(ret == 0)
      {
        user.write_records();
        res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
      }
      else
      {
        user.write_records();
        res.set_content(nlohmann::json{{"status", "failed"}, {"message", "已经收藏过了"}}.dump(), "application/json");
      }
    });
  
    svr.Get("/api/unmark_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      int ret = user.unmark_word(std::stoi(req.get_param_value("word_index")));
      if(ret == 0)
      {
        user.write_records();
        res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
      }
      else
      {
        user.write_records();
        res.set_content(nlohmann::json{{"status", "failed"}, {"message", "没有收藏过该单词"}}.dump(), "application/json");
      }
    });
  
    svr.Get("/api/clear_records", [this](const httplib::Request &req, httplib::Response &res)
    {
      user.clear_records();
      user.write_records();
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
  
    svr.Get("/api/clear_marks", [this](const httplib::Request &req, httplib::Response &res)
    {
      user.clear_marks();
      user.write_records();
      res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
    });
  
    svr.Get("/api/prev_memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      WordRef wr = user.prev_memorize_word();
      if(wr.is_valid())
      {
        res.set_content(nlohmann::json{
            {"status",     "success"},
            {"word",       wr},
            {"content",    user.get_explanation(wr.index)},
        }.dump(), "application/json");
      }
      else
      {
        res.set_content(nlohmann::json{
            {"status",     "failed"},
            {"message",  "没有上一个了"}
        }.dump(), "application/json");
      }
    });
  
    svr.Get("/api/set_memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto i = std::stoi(req.get_param_value("word_index"));
      WordRef wr = user.set_memorize_word(i);
      if(wr.is_valid())
      {
        user.write_records();
        res.set_content(nlohmann::json{
            {"status",     "success"},
            {"word",       wr},
            {"content",    user.get_explanation(wr.index)},
        }.dump(), "application/json");
      }
      else
      {
        res.set_content(nlohmann::json{
            {"status",     "failed"},
            {"message",     "错误的 word_index"}
        }.dump(), "application/json");
      }
    });
  
  
    svr.Get("/api/memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      WordRef wr;
      if(req.has_param("next") && req.get_param_value("next") == "true")
      {
        if(auto &p = user.word_record(user.curr_memorize_word().index)->points; p != 0)
          --p;
        wr = user.get_memorize_word();
        if(!wr.is_valid())
        {
          res.set_content(nlohmann::json{
              {"status",     "failed"},
              {"message",    "没有下一个了"},
          }.dump(), "application/json");
          return;
        }
        user.write_records();
      }
      else
      {
        wr = user.curr_memorize_word();
      }
      res.set_content(nlohmann::json{
          {"status",     "success"},
          {"word",       wr},
          {"content",    user.get_explanation(wr.index)},
      }.dump(), "application/json");
    });
    
    svr.set_exception_handler([](const auto &req, auto &res, std::exception_ptr ep)
                              {
                                auto fmt = "<h1>Error 500</h1><p>%s</p>";
                                char buf[BUFSIZ];
                                std::string ew;
                                try
                                {
                                  std::rethrow_exception(ep);
                                }
                                catch (std::exception &e)
                                {
                                  ew = e.what();
                                  snprintf(buf, sizeof(buf), fmt, e.what());
                                }
                                catch (...)
                                { // See the following NOTE
                                  snprintf(buf, sizeof(buf), fmt, "Unknown Exception");
                                }
                                std::cerr << utils::red("Exception: ") << ew << std::endl;
                                res.set_content(buf, "text/html");
                                res.status = 500;
                              });
    svr.set_error_handler([](const auto &req, auto &res)
                          {
                            auto fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
                            char buf[BUFSIZ];
                            snprintf(buf, sizeof(buf), fmt, res.status);
                            std::cerr << utils::red("Error: ") << res.status << std::endl;
                            res.set_content(buf, "text/html");
                          });
    svr.set_logger([](const httplib::Request &req, const httplib::Response &res)
                   {
                     std::cout << "Request:\n" << "  Method: " << req.method << "\n  Path: " << req.path
                               << "\n  Body: "
                               << req.body
                               << "\n  Params: \n";
                     for (auto &r: req.params)
                     {
                       std::cout << "    " << r.first << ": " << r.second << "\n";
                     }
      
                     if (utils::begin_with(req.path, "/css") || utils::begin_with(req.path, "/fonts")
                         || utils::begin_with(req.path, "/html") || utils::begin_with(req.path, "/icons")
                         || utils::begin_with(req.path, "/js") || req.path == "/")
                     {
                       std::cout << "Response: \n";
                     }
                     else
                     {
                       std::cout << "Response: \n" << "  Body: " << res.body << "\n";
                     }
      
                     auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                     struct tm *ptm = localtime(&tt);
                     char date[60] = {0};
                     sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
                             (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
                             (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);
                     std::cout << utils::green("^^^^^^^^^^") << date << utils::green("^^^^^^^^^^") << std::endl;
                   });
    
    std::cout << "Server started at '" << listen_addr << ":" << listen_port << "'." << std::endl;
    svr.listen(listen_addr, listen_port);
  }
}