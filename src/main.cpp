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

void detect_file(const std::filesystem::path& path)
{
  if(!std::filesystem::is_regular_file(path))
  {
    std::cerr << "Missing file: " << path << "." << std::endl;
    std::exit(-1);
  }
}

int main(int argc, char* argv[])
{
  if (argc != 4)
  {
    std::cerr << "Usage: lead addr port resource_path" << std::endl;
    return -1;
  }
  for(auto& r : std::string(argv[2]))
  {
    if(!std::isdigit(r))
    {
      std::cerr << "Invalid port." << std::endl;
      std::cerr << "Usage: lead addr port resource_path" << std::endl;
      return -1;
    }
  }
  std::filesystem::path res(argv[3]);
  detect_file(res / "css" / "lead.css");
  detect_file(res / "html" / "about.html");
  detect_file(res / "html" / "index.html");
  detect_file(res / "html" / "record.html");
  detect_file(res / "js" / "lead.js");
  detect_file(res / "voc" / "data.json");
  detect_file(res / "voc" / "index.json");
  
  lead::Server svr(argv[1], std::stoi(argv[2]), res);
  svr.run();
  return 0;
}
