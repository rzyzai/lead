<h2 align="center">
Lead
</h2>

<p align="center">
<strong>一个简约的背单词应用</strong>
</p>

首页: https://rzyzai.tech/lead/  
DEMO: http://lead.rzyzai.tech/

### 示例
| ![memorize](https://pic.imgdb.cn/item/6573363ac458853aef3b36b5.png) | ![quiz](https://pic.imgdb.cn/item/65733639c458853aef3b319e.png) |
|---------------------------------------------------------|---------------------------------------------------------|
| ![search](https://pic.imgdb.cn/item/65733638c458853aef3b29f4.png) | ![monitor](https://pic.imgdb.cn/item/65733639c458853aef3b2dc1.png)   |

### 部署
```shell
git clone https://github.com/rzyzai/lead.git
cd lead
mkdir build && cd build
cmake .. && make 
(或 g++ ../src/*.cpp -I ../include -I ../include/bundled -lpthread -lleveldb -std=c++17 -O2 -o lead) 
./lead ../res/config/config.json
打开 "localhost:8080"
```

### config.json
```json
{
  "admin_password": "admin",
  "listen_address": "0.0.0.0",
  "listen_port": 8080,
  "resource_path": "../res",
  "smtp_email": "",
  "smtp_password": "",
  "smtp_server": "",
  "smtp_username": ""
}
```
- `admin_password`为管理员密码，用于在网页端查看服务器状态和修改配置。
- `listen_address`、`listen_address`为服务器监听地址
- `resource_path`是资源目录
- `smtp_**` 用于注册时发送验证邮件(选填)

### 依赖

- [nlohmann/json](https://github.com/nlohmann/json)
- [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- [leveldb](https://github.com/google/leveldb)
- [cppcodec](https://github.com/tplgy/cppcodec)
- [libcurl](https://curl.se/libcurl/)
- [MDUI v1](https://www.mdui.org/docs/)


### 日照一中AI社
lead 来自[日照一中AI社](https://github.com/rzyzai)