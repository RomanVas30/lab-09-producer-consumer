// Copyright 2018 Roman Vasyutin romanvas3008@gmail.com

#ifndef INCLUDE_KRAULER_HPP_
#define INCLUDE_KRAULER_HPP_

#include <ThreadPool.h>
#include <gumbo.h>
#include <string_buffer.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/config.hpp>
#include <boost/program_options/environment_iterator.hpp>
#include <boost/program_options/eof_iterator.hpp>
#include <boost/program_options/errors.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/value_semantic.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/version.hpp>
#include <boost/thread.hpp>

#include "sertificate.hpp"
#include <iostream>
#include <deque>
#include <string>
#include <stdexcept>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <thread>

using namespace std::chrono_literals;
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class Krauler {
public:
  Krauler() : url_("yandex.ru"),
              depth_(4),
              network_threads_(2),
              parser_threads_(2),
              output_("output.txt") {}
  Krauler(const std::string& url, unsigned depth, unsigned network_threads,
          unsigned parser_threads, const std::string& output)
     : url_(url),
       depth_(depth),
       network_threads_(network_threads),
       parser_threads_(parser_threads),
       output_(output) {}
  void download(const std::string, const std::string);
  void parse_main_url();
  void parse_url();
  void search_for_links(GumboNode*);
  void search_for_pictures(GumboNode*, unsigned);
  std::string convert_url_host (std::string);
  std::string convert_url_target (std::string);
  void make();
  void filing();

 private:
  std::string url_;
  const size_t depth_;
  size_t network_threads_;
  size_t parser_threads_;
  std::string output_;
  unsigned count_not_download = 1;
  unsigned count_unparsed = 0;
  bool done = false;
  bool notified = false;
  std::vector<std::string> links;
  std::deque<std::string> parse_queue;
  std::deque<std::string> file_queue;
  boost::recursive_mutex file_mutex;
  boost::recursive_mutex download_mutex;
  std::condition_variable_any cv;
  std::condition_variable_any cv_file;
};

#endif // INCLUDE_KRAULER_HPP_
