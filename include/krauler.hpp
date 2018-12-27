// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_KRAULER_HPP_
#define INCLUDE_KRAULER_HPP_

#include <ThreadPool.h>
#include <gumbo.h>
#include <string_buffer.h>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
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

#include <deque>
#include <string>
#include <vector>
#include <condition_variable>
#include <mutex>

namespace http = boost::beast::http;

class Krauler {
public:
  Krauler(unsigned depth, unsigned network_threads,
          unsigned parser_threads, const std::string output)
     : depth_(depth),
       network_threads_(network_threads),
       parser_threads_(parser_threads),
       output_(output) {}
  void download(const std::string& host);
  void parse();
  void search_for_links(GumboNode* node, ThreadPool& pool);
  std::string converting(std::string url);

 private:
  std::string url_;
  unsigned depth_;
  unsigned network_threads_;
  unsigned parser_threads_;
  std::string output_;

  boost::asio::io_context ioc;
  std::deque<std::string> parse_queue;
  std::recursive_mutex download_mutex;
};

#endif // INCLUDE_KRAULER_HPP_
