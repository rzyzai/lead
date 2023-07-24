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
#include "lead/server.hpp"
#include <filesystem>

template<typename T, typename... Ts>
void detect_file(T&& arg1, Ts&& ...args)
{
  if (!std::filesystem::is_regular_file(arg1))
    std::cerr << "Warning: Missing file: " << arg1 << "." << std::endl;
  if constexpr(sizeof...(args) > 0)
    detect_file(std::forward<Ts>(args)...);
}

template<typename... Ts>
void detect_res(const std::filesystem::path& res, Ts&& ...args)
{
  detect_file((res / args)...);
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    std::cerr << "Usage: lead addr port resource_path" << std::endl;
    return -1;
  }
  for (auto &r: std::string(argv[2]))
  {
    if (!std::isdigit(r))
    {
      std::cerr << "Invalid port." << std::endl;
      std::cerr << "Usage: lead addr port resource_path" << std::endl;
      return -1;
    }
  }
  std::filesystem::path res(argv[3]);
  std::cout << "Scanning resource files.\n";
  detect_res(res, "css/googlefont.css", "css/lead.css", "css/mdui.css", "css/mdui.css.map", "css/mdui.min.css",
             "css/mdui.min.css.map", "fonts/4UasrENHsxJlGDuGo1OIlJfC6l_24rlCK1Yo_Iqcsih3SAyH6cAwhX9RPjIUvQ.woff2",
             "fonts/roboto/LICENSE.txt", "fonts/roboto/Roboto-Black.woff", "fonts/roboto/Roboto-Black.woff2",
             "fonts/roboto/Roboto-BlackItalic.woff", "fonts/roboto/Roboto-BlackItalic.woff2",
             "fonts/roboto/Roboto-Bold.woff", "fonts/roboto/Roboto-Bold.woff2", "fonts/roboto/Roboto-BoldItalic.woff",
             "fonts/roboto/Roboto-BoldItalic.woff2", "fonts/roboto/Roboto-Light.woff",
             "fonts/roboto/Roboto-Light.woff2", "fonts/roboto/Roboto-LightItalic.woff",
             "fonts/roboto/Roboto-LightItalic.woff2", "fonts/roboto/Roboto-Medium.woff",
             "fonts/roboto/Roboto-Medium.woff2", "fonts/roboto/Roboto-MediumItalic.woff",
             "fonts/roboto/Roboto-MediumItalic.woff2", "fonts/roboto/Roboto-Regular.woff",
             "fonts/roboto/Roboto-Regular.woff2", "fonts/roboto/Roboto-RegularItalic.woff",
             "fonts/roboto/Roboto-RegularItalic.woff2", "fonts/roboto/Roboto-Thin.woff",
             "fonts/roboto/Roboto-Thin.woff2", "fonts/roboto/Roboto-ThinItalic.woff",
             "fonts/roboto/Roboto-ThinItalic.woff2", "html/about.html", "html/index.html", "html/record.html",
             "icons/material-icons/LICENSE.txt", "icons/material-icons/MaterialIcons-Regular.ijmap",
             "icons/material-icons/MaterialIcons-Regular.woff", "icons/material-icons/MaterialIcons-Regular.woff2",
             "js/jquery.min.js", "js/lead.js", "js/mdui.esm.js", "js/mdui.esm.js.map", "js/mdui.js", "js/mdui.js.map",
             "js/mdui.min.js", "js/mdui.min.js.map", "records/record.json", "voc/data.json", "voc/index.json");
  lead::Server svr(argv[1], std::stoi(argv[2]), res);
  svr.run();
  return 0;
}