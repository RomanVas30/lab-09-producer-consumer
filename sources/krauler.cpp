#include <krauler.hpp>

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

void Krauler::download(std::string host, std::string target) {
  boost::recursive_mutex::scoped_lock lk(download_mutex);
  --count_not_download;
  try {
    net::io_context ioc;
    ssl::context ctx{ssl::context::sslv23_client};
    load_root_certificates(ctx);
    ctx.set_verify_mode(ssl::verify_peer);
    tcp::resolver resolver{ioc};
    ssl::stream<tcp::socket> stream{ioc, ctx};
    auto const results = resolver.resolve(
      boost::asio::ip::tcp::resolver::query{host, "https"});
    net::connect(stream.next_layer(),
                 results.begin(),
                 results.end());
    stream.handshake(ssl::stream_base::client);
    http::request<http::string_body> req(http::verb::get,
                                         target,
                                         11);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::accept, "text/html");
    req.set(http::field::connection, "close");
    http::write(stream, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);
    std::string msg = res.body();
    beast::error_code ec;
    stream.shutdown(ec);
    parse_queue.push_back(msg);
  }
  catch (std::exception& e) {
    e.what();
  }
  notified = true;
  cv.notify_one();
}

void Krauler::search_for_links(GumboNode* node) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
    std::string curr_str = href->value;
    bool valid_href = false;
    if (curr_str.find("https:") == 0)
      valid_href = true;
    if ((curr_str.size() > 5) && (valid_href)){
      links.push_back(href->value);
    }
  }
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]));
  }
}

void Krauler::search_for_pictures(GumboNode* node, unsigned depth) {
  if ((node->type != GUMBO_NODE_ELEMENT) || (depth == 0)) {
    return;
  }
  GumboAttribute* src;
  if (node->v.element.tag == GUMBO_TAG_IMG &&
      (src = gumbo_get_attribute(&node->v.element.attributes, "src"))) {
           file_queue.push_back(src->value);
  }
  GumboVector* children = &node->v.element.children;
  --depth;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_pictures(static_cast<GumboNode*> (children->data[i]), depth);
  }
}

void Krauler::parse_main_url() {
  GumboOutput* output = gumbo_parse(parse_queue.front().c_str());
  search_for_links(output->root);
  gumbo_destroy_output(&kGumboDefaultOptions, output);
  parse_queue.pop_front();
  notified = false;
}

void Krauler::parse_url() {
  boost::recursive_mutex::scoped_lock lk(download_mutex);
  --count_unparsed;
  while ((!notified) && (count_not_download != 0))
    cv.wait(lk);
  if (parse_queue.size() != 0){
    GumboOutput* output = gumbo_parse(parse_queue.front().c_str());
    boost::recursive_mutex::scoped_lock look(download_mutex);
    search_for_pictures(output->root, depth_);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    parse_queue.pop_front();
  }
  done = true;
  notified = false;
  cv_file.notify_one();
}

std::string Krauler::convert_url_host (
  std::string url) {
  if (url.find("https:") == 0)
    url = url.substr(8);
  std::string result_host = "";
  for (unsigned i = 0; i < url.size(); i++) {
    if ((url[i] == '/') || (url[i] == '?')) break;
    result_host+=url[i];
  }
    return result_host;
}

std::string Krauler::convert_url_target (std::string url) {
  if (url.find("https:") == 0)
    url = url.substr(8);
  std::string result_target = "";
  unsigned pos = 0;
  while (url[pos] != '/') { ++pos; }
  for (unsigned i = pos; i < url.size(); i++) {
    result_target += url[i];
  }
  return result_target;
}

void Krauler::filing() {
  boost::recursive_mutex::scoped_lock lock(download_mutex);
  while ((!done) && (count_unparsed != 0))
    cv_file.wait(lock);
  done = false;
  while (file_queue.size() != 0) {
    BOOST_LOG_TRIVIAL(trace) << file_queue.front() << std::endl;
    file_queue.pop_front();
  }
}

void Krauler::make(){
  try {
    boost::log::add_file_log(output_);
    download(convert_url_host(url_), convert_url_target(url_));
    parse_main_url();
    ThreadPool pool_writer(1);
    ThreadPool pool_downloads(network_threads_);
    ThreadPool pool_parsers(parser_threads_);
    count_not_download = count_unparsed = links.size();
    for (unsigned i = 0; i < links.size(); ++i){
      pool_downloads.enqueue(&Krauler::download, this,
                             convert_url_host(links[i]),
                             convert_url_target(links[i]));
      pool_parsers.enqueue(&Krauler::parse_url, this);
      pool_writer.enqueue(&Krauler::filing, this);
    }
  }
  catch (std::exception& e) {
    e.what();
  }
}
