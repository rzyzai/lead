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
#include "lead/globals.hpp"

#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"

#include <string>
#include <chrono>
#include <vector>

namespace lead
{
  Server::Server(const std::string &config_path_)
  {
    config_path = config_path_;
    std::ifstream config_file(config_path);
    config = nlohmann::json::parse(config_file);
    config_file.close();
    
    config["resource_path"].get_to(res_path);
    config["admin_password"].get_to(admin_passwd);
    config["listen_address"].get_to(listen_addr);
    config["listen_port"].get_to(listen_port);
    
    std::string smtp_server, smtp_username, smtp_password, smtp_email;
    config["smtp_server"].get_to(smtp_server);
    config["smtp_username"].get_to(smtp_username);
    config["smtp_password"].get_to(smtp_password);
    config["smtp_email"].get_to(smtp_email);
    
    vocabulary.load(res_path + "/voc/voc.json");
    email_sender.init(smtp_server, smtp_username, smtp_password);
    user_manager.init(res_path + "/records", &vocabulary, &email_sender, smtp_email);
  }
  
  void Server::auth_do(const httplib::Request &req, httplib::Response &res,
                       const std::function<nlohmann::json(std::unique_ptr<UserRef>, const httplib::Request &)> &func)
  {
    auto[status, ur] = user_manager.get_user(req.get_param_value("username"), req.get_param_value("passwd"));
    if (status == UserManagerStatus::success)
    {
      auto res_json = func(std::move(ur), req);
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
  
  void Server::admin_do(const httplib::Request &req, httplib::Response &res,
                        const std::function<nlohmann::json(const httplib::Request &)> &func)
  {
    if (req.has_param("admin_password") && req.get_param_value("admin_password") == admin_passwd)
    {
      auto res_json = func(req);
      res.set_content(res_json.dump(), "application/json");
    }
    else
    {
      if (req.has_param("admin_password"))
      {
        res.set_content(nlohmann::json
                            {
                                {"status",  "failed"},
                                {"message", "管理员密码错误"}}.dump(), "application/json");
      }
      else
      {
        res.set_content(nlohmann::json
                            {
                                {"status",  "failed"},
                                {"message", "权限不足"}}.dump(), "application/json");
      }
    }
  }
  
  void Server::run()
  {
    httplib::Server svr;
    svr.set_mount_point("/", res_path + "/html");
    svr.set_mount_point("/", res_path + "/icons");
    svr.set_mount_point("/css", res_path + "/css");
    svr.set_mount_point("/fonts", res_path + "/fonts");
    svr.set_mount_point("/icons", res_path + "/icons");
    svr.set_mount_point("/js", res_path + "/js");
    svr.set_mount_point("/userpic", res_path + "/records/userpic");
    svr.Get("/api/send_verification_code", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user_manager.send_verification_code(req.get_param_value("email"),
                                                          req.get_param_value("username")).dump(), "application/json");
    });
    svr.Get("/api/register", [this](const httplib::Request &req, httplib::Response &res)
    {
      if (user_manager.verify(req.get_param_value("email"), req.get_param_value("verification_code")))
      {
        auto[status, ur] =
        user_manager.create_user(req.get_param_value("username"),
                                 req.get_param_value("email"),
                                 req.get_param_value("passwd"));
        if (status == UserManagerStatus::success)
        {
          res.set_content(nlohmann::json{{"status",          "success"},
                                         {"userid",          ur->userid},
                                         {"username",        ur->username},
                                         {"email",           ur->email},
                                         {"profile_picture", ur->profile_picture}}.dump(), "application/json");
        }
        else
        {
          res.set_content(nlohmann::json
                              {
                                  {"status",  "failed"},
                                  {"message", to_string(status)}}.dump(), "application/json");
        }
      }
      else
      {
        res.set_content(nlohmann::json
                            {
                                {"status",  "failed"},
                                {"message", "验证码错误"}}.dump(), "application/json");
      }
    });
    svr.Post("/api/upload_profile_picture", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        if (req.body.size() > 10485760)
        {
          return {{"status",  "failed"},
                  {"message", "The picture is too big."}};
        }
        auto file_name = "pic-" + std::to_string(ur->userid);
        std::ofstream ofs(res_path + "/records/userpic/" + file_name, std::ios::binary);
        ofs << req.body;
        ur->profile_picture = file_name;
        ofs.close();
        return {{"status",          "success"},
                {"profile_picture", file_name}};
      });
    });
    svr.Get("/api/login", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return {{"status",          "success"},
                {"userid",          ur->userid},
                {"username",        ur->username},
                {"email",           ur->email},
                {"profile_picture", ur->profile_picture}};
      });
    });
    svr.Get("/api/get_account_info", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return {{"status",          "success"},
                {"userid",          ur->userid},
                {"username",        ur->username},
                {"email",           ur->email},
                {"profile_picture", ur->profile_picture}};
      });
    });
    svr.Get("/api/get_quiz", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        WordRef wr;
        if (req.has_param("word_index"))
        {
          wr = vocabulary.at(std::stoi(req.get_param_value("word_index")));
        }
        auto quiz = ur->get_quiz(wr);
        wr = vocabulary.at(quiz["indexes"][quiz["answer"]].get<int>());
        return {
            {"status", "success"},
            {"word",   wr},
            {"quiz",   quiz},
        };
      });
    });
    svr.Get("/api/pass", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        ur->word_record(std::stoi(req.get_param_value("word_index")))->points = 0;
        return {{"status", "success"}};
      });
    });
    svr.Get("/api/renew", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        ur->word_record(std::stoi(req.get_param_value("word_index")))->points = planned_review_times;
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/quiz_passed", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        auto &p = ur->word_record(std::stoi(req.get_param_value("word_index")))->points;
        if (p != 0) --p;
        return {{"status", "success"}};
      });
    });
    svr.Get("/api/quiz_failed", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        ur->word_record(std::stoi(req.get_param_value("word_index")))->points += 10;
        return {{"status", "success"}};
      });
    });
    svr.Get("/api/quiz_prompt", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        size_t index = std::stoi(req.get_param_value("word_index"));
        auto &p = ur->word_record(index)->points;
        ++p;
        auto a = std::stoi(req.get_param_value("A_index"));
        auto b = std::stoi(req.get_param_value("B_index"));
        auto c = std::stoi(req.get_param_value("C_index"));
        auto d = std::stoi(req.get_param_value("D_index"));
        return {{"status", "success"},
                {"A",      {{"is_marked", ur->is_marked(a)},
                               {"explanation", vocabulary.get_explanation(a)}}},
                {"B",      {{"is_marked", ur->is_marked(b)},
                               {"explanation", vocabulary.get_explanation(b)}}},
                {"C",      {{"is_marked", ur->is_marked(c)},
                               {"explanation", vocabulary.get_explanation(c)}}},
                {"D",      {{"is_marked", ur->is_marked(d)},
                               {"explanation", vocabulary.get_explanation(d)}}}
        };
      });
    });
    
    svr.Get("/api/search", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        auto word = req.get_param_value("word");
        auto wr = vocabulary.search(word);
        std::vector<nlohmann::json> words;
        for (auto &r: wr)
        {
          WordRef word = vocabulary.at(r);
          words.emplace_back(nlohmann::json{{"word",      word},
                                            {"is_marked", ur->is_marked(word.index)}});
        }
        if (!wr.empty())
        {
          return {{"status",  "success"},
                  {"words",   words},
                  {"message", "找到了" + std::to_string(wr.size()) + "个结果"}};
        }
        return {{"status",  "failed"},
                {"message", "没有找到" + word}};
      });
    });
    
    svr.Get("/api/get_marked", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return ur->get_marked();
      });
    });
    
    svr.Get("/api/get_passed", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return ur->get_passed();
      });
    });
    
    svr.Get("/api/get_explanation", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto i = std::stoi(req.get_param_value("word_index"));
      res.set_content(nlohmann::json{{"status",      "success"},
                                     {"explanation", vocabulary.get_explanation(i)}}.dump(), "application/json");
    });
    
    svr.Get("/api/get_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auto i = std::stoi(req.get_param_value("word_index"));
      res.set_content(nlohmann::json{{"status", "success"},
                                     {"word",   vocabulary.at(i).word->word}}.dump(), "application/json");
    });
    
    svr.Get("/api/get_plan", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return ur->get_plan();
      });
    });
    
    svr.Get("/api/version", [this](const httplib::Request &req, httplib::Response &res)
    {
      res.set_content(user_manager.get_version().dump(), "application/json");
    });
    
    svr.Get("/api/shutdown", [this, &svr](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [&svr](const httplib::Request &req) -> nlohmann::json
      {
        lead_running = false;
        svr.stop();
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/reboot", [this, &svr](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [&svr](const httplib::Request &req) -> nlohmann::json
      {
        svr.stop();
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/login_admin", [this](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [](const httplib::Request &req) -> nlohmann::json
      {
        auto status = utils::get_system_status();
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/get_serverstatus", [this](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [](const httplib::Request &req) -> nlohmann::json
      {
        auto status = utils::get_system_status();
        return {
            {"status",           "success"},
            {"load",             status.load},
            {"total_memory",     status.total_memory},
            {"used_memory",      status.used_memory},
            {"time",             status.time},
            {"running_time",     status.running_time},
            {"time_since_epoch", status.time_since_epoch}
        };
      });
    });
    
    svr.Get("/api/get_serverinfo", [this](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [this](const httplib::Request &req) -> nlohmann::json
      {
        auto[msg, info] = utils::get_system_info();
        return {
            {"status",   "success"},
            {"message",  msg},
            {"hostname", info.hostname},
            {"sysname",  info.sysname},
            {"release",  info.release},
            {"version",  info.version},
            {"machine",  info.machine},
            {"network",  info.network},
            {"config",
                         {
                             {"listen_address", config["listen_address"].get<std::string>()},
                             {"listen_port", config["listen_port"].get<int>()},
                             {"resource_path", config["resource_path"].get<std::string>()},
                             {"admin_password", config["admin_password"].get<std::string>()},
                             {"smtp_server", config["smtp_server"].get<std::string>()},
                             {"smtp_username", config["smtp_username"].get<std::string>()},
                             {"smtp_password", config["smtp_password"].get<std::string>()},
                             {"smtp_email", config["smtp_email"].get<std::string>()},
              
                         }}
        };
      });
    });
    
    svr.Get("/api/update_config", [this](const httplib::Request &req, httplib::Response &res)
    {
      admin_do(req, res, [this](const httplib::Request &req) -> nlohmann::json
      {
        std::string message = "{";
        if (req.has_param("listen_address") && config["listen_address"] != req.get_param_value("listen_address"))
        {
          config["listen_address"] = req.get_param_value("listen_address");
          message += "listen_address, ";
        }
        if (req.has_param("listen_port")
            && std::to_string(config["listen_port"].get<int>()) != req.get_param_value("listen_port"))
        {
          config["listen_port"] = std::stoi(req.get_param_value("listen_port"));
          message += "listen_port, ";
        }
        if (req.has_param("resource_path") && config["resource_path"] != req.get_param_value("resource_path"))
        {
          config["resource_path"] = req.get_param_value("resource_path");
          message += "resource_path, ";
        }
        if (req.has_param("new_admin_password") &&
            config["admin_password"] != req.get_param_value("new_admin_password"))
        {
          admin_passwd = req.get_param_value("new_admin_password");
          config["admin_password"] = req.get_param_value("new_admin_password");
          message += "admin_password, ";
        }
        if (req.has_param("smtp_server") && config["smtp_server"] != req.get_param_value("smtp_server"))
        {
          config["smtp_server"] = req.get_param_value("smtp_server");
          message += "smtp_server, ";
        }
        if (req.has_param("smtp_username") && config["smtp_username"] != req.get_param_value("smtp_username"))
        {
          config["smtp_username"] = req.get_param_value("smtp_username");
          message += "smtp_username, ";
        }
        if (req.has_param("smtp_password") && config["smtp_password"] != req.get_param_value("smtp_password"))
        {
          config["smtp_password"] = req.get_param_value("smtp_password");
          message += "smtp_password, ";
        }
        if (req.has_param("smtp_email") && config["smtp_email"] != req.get_param_value("smtp_email"))
        {
          config["smtp_email"] = req.get_param_value("smtp_email");
          message += "smtp_email, ";
        }
        
        std::ofstream config_file(config_path);
        config_file << config.dump();
        config_file.flush();
        config_file.close();
        
        if (message != "{")
        {
          message.pop_back();
          message.pop_back();
          message += "} 已修改, 请重启服务器";
        }
        else
        {
          message = "配置没有改动";
        }
        return {{"status",  "success"},
                {"message", message}};
      });
    });
    
    
    svr.Get("/api/get_settings", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return ur->get_settings();
      });
    });
    
    svr.Post("/api/update_settings", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        return ur->update_settings(nlohmann::json::parse(req.body));
      });
    });
    
    svr.Get("/api/mark_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        int ret = ur->mark_word(std::stoi(req.get_param_value("word_index")));
        if (ret == 0)
        {
          return {{"status", "success"}};
        }
        else
        {
          return {{"status",  "failed"},
                  {"message", "已经收藏过了"}};
        }
      });
    });
    
    svr.Get("/api/unmark_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        int ret = ur->unmark_word(std::stoi(req.get_param_value("word_index")));
        if (ret == 0)
        {
          return {{"status", "success"}};
        }
        else
        {
          return {{"status",  "failed"},
                  {"message", "没有收藏过该单词"}};
        }
      });
    });
    
    svr.Get("/api/clear_word_records", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        ur->clear_word_records();
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/clear_marks", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        ur->clear_marks();
        return {{"status", "success"}};
      });
    });
    
    svr.Get("/api/prev_memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        WordRef wr = ur->prev_memorize_word();
        if (wr.is_valid())
        {
          return {{"status",  "success"},
                  {"word",    wr},
                  {"content", vocabulary.get_explanation(wr.index)}};
        }
        else
        {
          return {{"status",  "failed"},
                  {"message", "没有上一个了"}};
        }
      });
    });
    
    svr.Get("/api/set_memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        auto i = std::stoi(req.get_param_value("word_index"));
        auto wr = ur->set_memorize_word(i);
        if (wr.is_valid())
        {
          return {{"status",  "success"},
                  {"word",    wr},
                  {"content", vocabulary.get_explanation(wr.index)}};
        }
        else
        {
          return {{"status",  "failed"},
                  {"message", "错误的 word_index"}};
        }
      });
    });
    
    
    svr.Get("/api/memorize_word", [this](const httplib::Request &req, httplib::Response &res)
    {
      auth_do(req, res, [this](std::unique_ptr<UserRef> ur, const httplib::Request &req) -> nlohmann::json
      {
        WordRef wr;
        if (req.has_param("next") && req.get_param_value("next") == "true")
        {
          wr = ur->get_memorize_word();
          if (!wr.is_valid())
          {
            return {{"status",  "failed"},
                    {"message", "没有下一个了"}};
          }
        }
        else
        {
          wr = ur->curr_memorize_word();
        }
        return {{"status",  "success"},
                {"word",    wr},
                {"content", vocabulary.get_explanation(wr.index)}};
      });
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
                     if (utils::begin_with(req.path, "/api"))
                     {
                       std::cout << "Request:\n" << "  Method: " << req.method << "\n  Path: " << req.path
                                 << "\n  Body: "
                                 << req.body
                                 << "\n  Params: \n";
        
                       for (auto &r: req.params)
                       {
                         std::cout << "    " << r.first << ": " << r.second << "\n";
                       }
        
                       std::cout << "Response: \n" << "  Body: " << res.body << "\n";
                       auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                       struct tm *ptm = localtime(&tt);
                       char date[60] = {0};
                       sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
                               (int) ptm->tm_year + 1900, (int) ptm->tm_mon + 1, (int) ptm->tm_mday,
                               (int) ptm->tm_hour, (int) ptm->tm_min, (int) ptm->tm_sec);
                       std::cout << utils::green("^^^^^^^^^^") << date << utils::green("^^^^^^^^^^") << std::endl;
                     }
                   });
    
    std::cout << "Server started at '" << listen_addr << ":" << listen_port << "'." << std::endl;
    svr.listen(listen_addr, listen_port);
  }
}