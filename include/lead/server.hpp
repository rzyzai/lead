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
#ifndef LEAD_SERVER_HPP
#define LEAD_SERVER_HPP
#pragma once

#include "utils.hpp"
#include "user.hpp"
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"
#include <string>
#include <chrono>
#include <vector>
#include <filesystem>

namespace lead
{
  class Server
  {
  private:
    std::filesystem::path res_path;
    UserManager user_manager;
  public:
    Server(const std::string &res_path_)
        : res_path(res_path_), user_manager(res_path / "db" / "lead", res_path / "voc") {}
    
    void run()
    {
      httplib::Server svr;
      std::string index_html = utils::get_string_from_file(res_path / "html" / "index.html");
      std::string account_html = utils::get_string_from_file(res_path / "html" / "account.html");
      std::string about_html = utils::get_string_from_file(res_path / "html" / "about.html");
      std::string lead_js = utils::get_string_from_file(res_path / "js" / "lead.js");
      std::string lead_css = utils::get_string_from_file(res_path / "css" / "lead.css");
      std::string jpg1 = utils::get_string_from_file(res_path / "img" / "1.jpg");
      std::string jpg2 = utils::get_string_from_file(res_path / "img" / "2.jpg");
      std::string jpg3 = utils::get_string_from_file(res_path / "img" / "3.jpg");
      std::string jpg4 = utils::get_string_from_file(res_path / "img" / "4.jpg");
      svr.Get("/", [&index_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(index_html, "text/html");
      });
      svr.Get("/about.html", [&about_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(about_html, "text/html");
      });
      svr.Get("/account.html", [&account_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(account_html, "text/html");
      });
      svr.Get("/lead.js", [&lead_js](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(lead_js, "text/javascript");
      });
      svr.Get("/lead.css", [&lead_css](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(lead_css, "text/css");
      });
      svr.Get("/1.jpg", [&jpg1](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(jpg1, "image/jpeg");
      });
      svr.Get("/2.jpg", [&jpg2](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(jpg2, "image/jpeg");
      });
      svr.Get("/3.jpg", [&jpg3](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(jpg3, "image/jpeg");
      });
      svr.Get("/4.jpg", [&jpg4](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(jpg4, "image/jpeg");
      });
      svr.Get("/api/get_quiz", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          WordRef wr;
          if (req.has_param("word_index"))
          {
            wr = ur.get_word(std::stoi(req.get_param_value("word_index")));
          }
          else
          {
            wr = ur.get_random_word();
          }
          auto quiz = ur.get_quiz(wr);
          return {
              {"status", "success"},
              {"word",   wr.word->word},
              {"quiz",   quiz},
              {"index",    wr.index}};
        });
      });
      svr.Get("/api/pass", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          ur.word_record(std::stoi(req.get_param_value("word_index"))).points = 0;
          return {{"status", "success"}};
        });
      });
      svr.Get("/api/quiz_passed", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          auto &p = ur.word_record(std::stoi(req.get_param_value("word_index"))).points;
          if (p != 0) --p;
          return {{"status", "success"}};
        });
      });
      svr.Get("/api/quiz_failed", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          auto &p = ur.word_record(std::stoi(req.get_param_value("word_index"))).points;
          p += 3;
          return {{"status", "success"}};
        });
      });
      svr.Get("/api/quiz_prompt", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          size_t index = std::stoi(req.get_param_value("word_index"));
          auto &p = ur.word_record(index).points;
          ++p;
          return {{"status", "success"}};
        });
      });
      svr.Get("/api/explanation", [this](const httplib::Request &req, httplib::Response &res)
      {
          res.set_content(nlohmann::json{{"status", "success"},
                  {"explanation",
                   utils::get_string_from_file(res_path / "voc" / "entries"
                   / ("explanation-" + req.get_param_value("word_index")))}}.dump(), "application/json");
      });
      svr.Get("/api/search", [this](const httplib::Request &req, httplib::Response &res)
      {
        auth_do(req, res, [](UserRef ur, const httplib::Request &req) -> nlohmann::json
        {
          return ur.search(req.get_param_value("search_word"));
        });
      });
      svr.Get("/api/login", [this](const httplib::Request &req, httplib::Response &res)
      {
        nlohmann::json body{req.body};
        auto[status, ur] = user_manager.get_user(body["user_id"], body["password"]);
        if (status == UserManagerStatus::success)
        {
          res.set_content(nlohmann::json{{"status", "success"},
                                         {"type",   "login"}}.dump(), "application/json");
        }
        else if (status == UserManagerStatus::user_not_found)
        {
          auto[status, new_ur] = user_manager.create_user(body["user_id"], body["password"]);
          if (status == UserManagerStatus::success)
          {
            res.set_content(nlohmann::json{{"status", "success"},
                                           {"type",   "register"}}.dump(), "application/json");
          }
          else
          {
            res.set_content(nlohmann::json{{"status",  "failed"},
                                           {"type",    "register"},
                                           {"message", to_string(status)}}.dump(), "application/json");
          }
        }
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
                       std::cout << "Response: \n" << "  Body: ";
                       if (req.path == "/")
                       {
                         std::cout << "index.html\n";
                       }
                       else if (req.path == "/account.html")
                       {
                         std::cout << "account.html\n";
                       }
                       else if (req.path == "/about.html")
                       {
                         std::cout << "about.html\n";
                       }
                       else if (req.path == "/lead.js")
                       {
                         std::cout << "lead.js\n";
                       }
                       else if (req.path == "/lead.css")
                       {
                         std::cout << "lead.css\n";
                       }
                       else if (req.path == "/1.jpg")
                       {
                         std::cout << "1.jpg\n";
                       }
                       else if (req.path == "/2.jpg")
                       {
                         std::cout << "2.jpg\n";
                       }
                       else if (req.path == "/3.jpg")
                       {
                         std::cout << "3.jpg\n";
                       }
                       else if (req.path == "/4.jpg")
                       {
                         std::cout << "4.jpg\n";
                       }
                       
                       else
                       {
                         std::cout << res.body << "\n";
                       }
        
                       auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                       struct tm *ptm = localtime(&tt);
                       char date[60] = {0};
                       sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
                               (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
                               (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);
                       std::cout << utils::green("^^^^^^^^^^") << date << utils::green("^^^^^^^^^^") << std::endl;
                     });
      svr.listen("0.0.0.0", 8080);
    }
  
  private:
    void auth_do(const httplib::Request &req, httplib::Response &res,
                 const std::function<nlohmann::json(UserRef, const httplib::Request &)> &func)
    {
      auto[status, ur] = user_manager.get_user(req.get_param_value("user_id"), req.get_param_value("password"));
      if (status == UserManagerStatus::success)
      {
        auto res_json = func(ur, req);
        res.set_content(res_json.dump(), "application/json");
      }
      else
      {
        res.set_content(nlohmann::json
                            {
                                {"status",  "failed"},
                                {"message", to_string(status)}}.dump(), "application/json");
      }
    }
  };
}
#endif