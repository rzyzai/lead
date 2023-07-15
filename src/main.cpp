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
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"
#include <iostream>

int main()
{
  lead::VOC voc;
  voc.load("../src/voc.json");
  httplib::Server svr;
  std::string html = lead::utils::get_string_from_file("../src/index.html");
  std::string js = lead::utils::get_string_from_file("../src/lead.js");
  svr.Get("/", [&html](const httplib::Request &req, httplib::Response &res)
  {
    res.set_content(html, "text/html");
  });
  svr.Get("/lead.js", [&js](const httplib::Request &req, httplib::Response &res)
  {
    res.set_content(js, "text/javascript");
  });
  svr.Get("/get_quiz", [&voc](const httplib::Request &req, httplib::Response &res)
  {
    auto wr = voc.get_a_word();
    auto quiz = voc.generate_a_quiz(wr);
    res.set_content(nlohmann::json{{"word", wr.word->word},
                                   {"pronunciation", wr.word->pronunciation},
                                   {"quiz", quiz},
                                   {"pos",  wr.pos}}.dump(), "application/json");
  });
  svr.Get("/pass", [&voc](const httplib::Request &req, httplib::Response &res)
  {
    auto pos = req.get_param_value("pos");
    voc.pass(std::stoi(pos));
  });
  svr.Get("/quiz_passed", [&voc](const httplib::Request &req, httplib::Response &res)
  {
    auto pos = req.get_param_value("pos");
    auto wr = voc.at(std::stoi(pos));
    wr.word->reviewed_times++;
  });
  svr.Get("/quiz_failed", [&voc](const httplib::Request &req, httplib::Response &res)
  {
    auto pos = req.get_param_value("pos");
    auto wr = voc.at(std::stoi(pos));
    wr.word->planned_times += 3;
  });
  svr.Get("/words", [&voc](const httplib::Request &req, httplib::Response &res)
  {
    const auto &v = voc.get_voc();
    res.set_content(nlohmann::json{v}.dump(), "application/json");
  });
  svr.set_logger([](const httplib::Request &req, const httplib::Response &res)
  {
    std::cout << "Request:\n " << "  Method: " << req.method << "\n  Path: " << req.path << "\n  Body: " << req.body
    << "\nResponse: \n" << "  Body: " << res.body << std::endl;
  });
  svr.listen("0.0.0.0", 8080);
  return 0;
}
