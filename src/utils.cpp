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

#include "lead/globals.hpp"
#include "lead/utils.hpp"
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <unordered_set>
#include <cstring>

#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <dirent.h>

#define IPV6_ADDR_GLOBAL        0x0000U
#define IPV6_ADDR_LOOPBACK      0x0010U
#define IPV6_ADDR_LINKLOCAL     0x0020U
#define IPV6_ADDR_SITELOCAL     0x0040U
#define IPV6_ADDR_COMPATv4      0x0080U

namespace lead::utils
{
  SystemStatus get_system_status()
  {
    SystemStatus status;
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    struct tm *tmNow = localtime(&tt);
    char date[20] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d", (int) tmNow->tm_year + 1900, (int) tmNow->tm_mon + 1,
            (int) tmNow->tm_mday, (int) tmNow->tm_hour, (int) tmNow->tm_min, (int) tmNow->tm_sec);
    status.time = std::string{date};
    auto n = std::chrono::steady_clock::now();
    double duration_second = std::chrono::duration<double>(n - lead_start_time).count();
    int hour = duration_second / 3600;
    int min = (duration_second - hour * 3600) / 60;
    int sec = duration_second - hour * 3600 - min * 60;
    status.running_time = std::to_string(hour) + ":" + std::to_string(min) + ":" + std::to_string(sec);
  
    status.time_since_epoch = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>
                                                 (std::chrono::system_clock::now().time_since_epoch()).count());
  
    struct sysinfo info;
    if (sysinfo(&info) == 0)
    {
      FILE *fd;
      fd = fopen("/proc/meminfo", "r");
      if (fd == NULL) {
        perror("open /proc/meminfo failed\n");
        exit(0);
      }
      size_t bytes_read;
      size_t read;
      char *line = NULL;
      int index = 0;
      int avimem = 0;
      while ((read = getline(&line, &bytes_read, fd)) != -1) {
        if (++index <= 2) {
          continue;
        }
        if (strstr(line, "MemAvailable") != NULL) {
          sscanf(line, "%*s%d%*s", &avimem);
          break;
        }
      }
      int t = info.totalram / 1024.0;
      status.used_memory = std::to_string((t - avimem) * 1.0 / 1024);
      status.total_memory = std::to_string(info.totalram * 1.0 / 1024 / 1024);
  
      struct sysinfo sysinf;
      memset(&sysinf, 0, sizeof sysinf);
      if (!sysinfo(&sysinf))
      {
        float f_load = 1.f / (1 << SI_LOAD_SHIFT);
        status.load =
            std::to_string(info.loads[0] * f_load) + " "
            + std::to_string(info.loads[1] * f_load)
            + " " + std::to_string(info.loads[2] * f_load);
      }
    }
    return status;
  }

// https://blog.csdn.net/W1107101310/article/details/109708783
  void parse_inet6(const char *ifname, NetCardInfo& info)
  {
    FILE *f;
    int ret, scope, prefix;
    unsigned char ipv6[16];
    char dname[IFNAMSIZ];
    char address[INET6_ADDRSTRLEN];
    
    f = fopen("/proc/net/if_inet6", "r");
    if (f == NULL)
    {
      return;
    }
    while (19 == fscanf(f,
                        " %2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx %*x %x %x %*x %s",
                        &ipv6[0],
                        &ipv6[1],
                        &ipv6[2],
                        &ipv6[3],
                        &ipv6[4],
                        &ipv6[5],
                        &ipv6[6],
                        &ipv6[7],
                        &ipv6[8],
                        &ipv6[9],
                        &ipv6[10],
                        &ipv6[11],
                        &ipv6[12],
                        &ipv6[13],
                        &ipv6[14],
                        &ipv6[15],
                        &prefix,
                        &scope,
                        dname))
    {
      
      if (strcmp(ifname, dname) != 0)
      {
        continue;
      }
      
      if (inet_ntop(AF_INET6, ipv6, address, sizeof(address)) == NULL)
      {
        continue;
      }
      info.ipv6 = address;
    }
    fclose(f);
  }
  
  void parse_ioctl(const char *ifname, NetCardInfo& info)
  {
    int sock;
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    char address[INET_ADDRSTRLEN];
    size_t ifnamelen;
    
    ifnamelen = strlen(ifname);
    if (ifnamelen >= sizeof(ifr.ifr_name)) {
      return ;
    }
    memcpy(ifr.ifr_name, ifname, ifnamelen);
    ifr.ifr_name[ifnamelen] = '\0';
    
    sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
      return;
    }
    
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) != -1) {
      char m[32];
      sprintf(m, "%02x:%02x:%02x:%02x:%02x:%02x",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
      info.mac = m;
    }
    
    if (ioctl(sock, SIOCGIFADDR, &ifr) == -1) {
      close(sock);
      return;
    }
    
    ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
    if (inet_ntop(AF_INET, &ipaddr->sin_addr, address, sizeof(address)) != NULL) {
     info.ipv4 = address;
    }
    close(sock);
  }
  
  std::tuple<std::string, SystemInfo> get_system_info()
  {
    std::string message;
    SystemInfo info;
    char hostbuffer[256];
    if(int ret = gethostname(hostbuffer, sizeof(hostbuffer)); ret == -1)
    {
      info.hostname = "-";
      message += strerror(errno);
    }
    else
      info.hostname = hostbuffer;
  
    struct utsname unamebuffer;
    if(int ret = uname(&unamebuffer); ret < 0)
    {
      info.sysname = "-";
      info.version = "-";
      info.release = "-";
      info.machine = "-";
      message += strerror(errno);
    }
    else
    {
      info.sysname = unamebuffer.sysname;
      info.version = unamebuffer.version;
      info.release = unamebuffer.release;
      info.machine = unamebuffer.machine;
    }
  
    DIR *d;
    struct dirent *de;
    d = opendir("/sys/class/net/");
    if (d == NULL) return {message, info};
  
    while (NULL != (de = readdir(d))) {
      if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
        continue;
      }
      NetCardInfo i;
      i.name = de->d_name;
      parse_ioctl(de->d_name, i);
      parse_inet6(de->d_name, i);
      if(!i.mac.empty() || !i.ipv4.empty() || !i.ipv6.empty())
        info.network.emplace_back(i);
    }
    closedir(d);
    return {message, info};
  }
  
  void to_json(nlohmann::json &j, const NetCardInfo &p)
  {
    j = {{"name", p.name}, {"mac", p.mac}, {"ipv4", p.ipv4}, {"ipv6", p.ipv6}};
  }
  
  std::string get_string_from_file(const std::string &path)
  {
    std::ifstream file{path, std::ios::binary};
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
  }
  
  bool begin_with(const std::string &a, const std::string &b)
  {
    if (a.size() < b.size()) return false;
    for (size_t i = 0; i < b.size(); ++i)
    {
      if (a[i] != b[i])
      {
        return false;
      }
    }
    return true;
  }
  
  std::string effect(const std::string &str, Effect effect_)
  {
    if (str.empty()) return "";
    if (effect_ == utils::Effect::bg_shadow)
    {
      return "\033[48;5;7m" + str + "\033[49m";
    }
    else if (effect_ == utils::Effect::bg_strong_shadow)
    {
      return "\033[48;5;8m" + str + "\033[49m";
    }
    
    int effect = static_cast<int>(effect_);
    int end = 0;
    if (effect >= 1 && effect <= 7)
    {
      end = 0;
    }
    else if (effect >= 30 && effect <= 37)
    {
      end = 39;
    }
    else if (effect >= 40 && effect <= 47)
    {
      end = 49;
    }
    return "\033[" + std::to_string(effect) + "m" + str + "\033[" + std::to_string(end) + "m";
  }
  
  std::string red(const std::string &str)
  {
    return effect(str, Effect::fg_red);
  }
  
  std::string green(const std::string &str)
  {
    return effect(str, Effect::fg_green);
  }
  
  std::string yellow(const std::string &str)
  {
    return effect(str, Effect::fg_yellow);
  }
  
  std::string blue(const std::string &str)
  {
    return effect(str, Effect::fg_blue);
  }
  
  std::string magenta(const std::string &str)
  {
    return effect(str, Effect::fg_magenta);
  }
  
  std::string cyan(const std::string &str)
  {
    return effect(str, Effect::fg_cyan);
  }
  
  std::string white(const std::string &str)
  {
    return effect(str, Effect::fg_white);
  }
  
  int get_edit_distance(const std::string &s1, const std::string &s2)
  {
    std::size_t n = s1.size();
    std::size_t m = s2.size();
    if (n * m == 0) return static_cast<int>(n + m);
    std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1));
    for (int i = 0; i < n + 1; i++)
    {
      D[i][0] = i;
    }
    for (int j = 0; j < m + 1; j++)
    {
      D[0][j] = j;
    }
    
    for (int i = 1; i < n + 1; i++)
    {
      for (int j = 1; j < m + 1; j++)
      {
        int left = D[i - 1][j] + 1;
        int down = D[i][j - 1] + 1;
        int left_down = D[i - 1][j - 1];
        if (s1[i - 1] != s2[j - 1]) left_down += 1;
        D[i][j] = (std::min)(left, (std::min)(down, left_down));
      }
    }
    return D[n][m];
  }
  
}