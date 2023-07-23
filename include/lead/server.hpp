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
    User user;
  public:
    Server(const std::string &res_path_)
        : res_path(res_path_), user(res_path / "voc") {}
    
    void run()
    {
      httplib::Server svr;
      std::string index_html = utils::get_string_from_file(res_path / "html" / "index.html");
      std::string record_html = utils::get_string_from_file(res_path / "html" / "record.html");
      std::string about_html = utils::get_string_from_file(res_path / "html" / "about.html");
      std::string lead_js = utils::get_string_from_file(res_path / "js" / "lead.js");
      std::string lead_css = utils::get_string_from_file(res_path / "css" / "lead.css");
      svr.Get("/", [&index_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(index_html, "text/html");
      });
      svr.Get("/about.html", [&about_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(about_html, "text/html");
      });
      svr.Get("/record.html", [&record_html](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(record_html, "text/html");
      });
      svr.Get("/lead.js", [&lead_js](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(lead_js, "text/javascript");
      });
      svr.Get("/lead.css", [&lead_css](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(lead_css, "text/css");
      });
      svr.Get("/api/get_quiz", [this](const httplib::Request &req, httplib::Response &res)
      {
          WordRef wr;
          if (req.has_param("word_index"))
            wr = user.get_word(std::stoi(req.get_param_value("word_index")));
          else
            wr = user.get_random_word();
          auto quiz = user.get_quiz(wr);
          res.set_content(nlohmann::json{
              {"status", "success"},
              {"word",   wr.word->word},
              {"quiz",   quiz},
              {"index",  wr.index}
              }.dump(), "application/json");
      });
      svr.Get("/api/pass", [this](const httplib::Request &req, httplib::Response &res)
      {
          user.word_record(std::stoi(req.get_param_value("word_index"))).points = 0;
          res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
      });
      svr.Get("/api/quiz_passed", [this](const httplib::Request &req, httplib::Response &res)
      {
          auto &p = user.word_record(std::stoi(req.get_param_value("word_index"))).points;
          if (p != 0) --p;
          res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
      });
      svr.Get("/api/quiz_failed", [this](const httplib::Request &req, httplib::Response &res)
      {
          auto &p = user.word_record(std::stoi(req.get_param_value("word_index"))).points;
          p += 3;
          res.set_content(nlohmann::json{{"status", "success"}}.dump(), "application/json");
      });
      svr.Get("/api/quiz_prompt", [this](const httplib::Request &req, httplib::Response &res)
      {
          size_t index = std::stoi(req.get_param_value("word_index"));
          auto &p = user.word_record(index).points;
          ++p;
          res.set_content(nlohmann::json{{"status", "success"},
                  {"A", user.get_explanation(std::stoi(req.get_param_value("A_index")))},
                  {"B", user.get_explanation(std::stoi(req.get_param_value("B_index")))},
                  {"C", user.get_explanation(std::stoi(req.get_param_value("C_index")))},
                  {"D", user.get_explanation(std::stoi(req.get_param_value("D_index")))}}.dump(), "application/json");
      });
      svr.Get("/api/search", [this](const httplib::Request &req, httplib::Response &res)
      {
          res.set_content(user.search(req.get_param_value("word")).dump(), "application/json");
      });
      svr.Get("/api/get_progress", [this](const httplib::Request &req, httplib::Response &res)
      {
        res.set_content(user.get_progress().dump(), "application/json");
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
                       else if (req.path == "/record.html")
                       {
                         std::cout << "record.html\n";
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
  };
}
#endif