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
#include "email.hpp"
#include "user.hpp"
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"
#include <string>
#include <chrono>
#include <vector>

namespace lead
{
  class Server
  {
  public:
    std::string config_path;
    std::string res_path;
    VOC vocabulary;
    EmailSender email_sender;
    UserManager user_manager;
    std::string listen_addr;
    int listen_port;
    std::string admin_passwd;
    nlohmann::json config;
  public:
    Server(const std::string &config_path_);
    
    void run();
  
  private:
    void auth_do(const httplib::Request &req, httplib::Response &res,
                 const std::function<nlohmann::json(std::unique_ptr<UserRef>, const httplib::Request &)> &func);
    
    void admin_do(const httplib::Request &req, httplib::Response &res,
                  const std::function<nlohmann::json(const httplib::Request &)> &func);
  };
}
#endif