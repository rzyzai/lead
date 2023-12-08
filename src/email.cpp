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

#include <string>
#include <cstring>
#include <curl/curl.h>
#include "bundled/cppcodec/base64_default_url.hpp"

#include "lead/email.hpp"
#include "lead/utils.hpp"

namespace lead
{
  void EmailSender::init(const std::string &server_, const std::string &username_, const std::string &password)
  {
    server = server_;
    username = username_;
    passwd = password;
  }
  
  static size_t payload_source(char *ptr, size_t size, size_t nmemb, void *userp)
  {
    struct email_upload_status *upload_ctx = (struct email_upload_status *) userp;
    const char *data;
    size_t room = size * nmemb;
    if ((size == 0) || (nmemb == 0) || ((size * nmemb) < 1)) return 0;
    data = &(*upload_ctx->payload_text)[upload_ctx->bytes_read];
    if (data)
    {
      size_t len = strlen(data);
      if (room < len)
      {
        len = room;
      }
      memcpy(ptr, data, len);
      upload_ctx->bytes_read += len;
      return len;
    }
    return 0;
  }
  
  int EmailSender::send(const Email &email) const
  {
    const std::string payload_text =
        "content-type:text/plain;charset=utf-8\r\n"
        "Date: " + utils::get_time() + "\r\n"
                                       "To: =?UTF-8?B?" + cppcodec::base64_url::encode(email.to_name) + "?= <" +
        email.to_addr + ">\r\n"
                        "From: =?UTF-8?B?" + cppcodec::base64_url::encode(email.from_name) + "?= <" + email.from_addr +
        ">\r\n"
        "Subject: =?UTF-8?B?" + cppcodec::base64_url::encode(email.subject) + "?=\r\n\r\n"
        + email.body + "\r\n";
    
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct email_upload_status upload_ctx = {0, &payload_text};
    curl = curl_easy_init();
    if (curl)
    {
      curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
      curl_easy_setopt(curl, CURLOPT_PASSWORD, passwd.c_str());
      curl_easy_setopt(curl, CURLOPT_URL, server.c_str());
      curl_easy_setopt(curl, CURLOPT_MAIL_FROM, email.from_addr.c_str());
      recipients = curl_slist_append(recipients, email.to_addr.c_str());
      curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
      curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
      curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      res = curl_easy_perform(curl);
      if (res != CURLE_OK)
      {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
      }
      curl_slist_free_all(recipients);
      curl_easy_cleanup(curl);
    }
    return (int) res;
  }
}