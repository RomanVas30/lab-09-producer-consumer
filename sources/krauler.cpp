#include <krauler.hpp>

// std::condition_variable cv;

void Krauler::download(const std::string& host) {
  // std::cout << "THERE" << std::endl;
  const std::string target = "/echo";
  //  boost::asio::io_context ioc;
  boost::asio::ip::tcp::resolver resolver(ioc);
  boost::asio::ip::tcp::socket socket(ioc);
  boost::asio::connect(socket, resolver.resolve(host, "80"));
  http::request<http::string_body> req(http::verb::get, target, 11);
  std::lock_guard<std::recursive_mutex> lock(download_mutex);
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  http::write(socket, req);
  boost::beast::flat_buffer buffer;
  http::response<http::string_body> res;
  http::read(socket, buffer, res);
  std::string msg = res.body();
  socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
  parse_queue.push_back(msg);
  std::cout << "New href" << std::endl;
  // cv.notify_one();
  parse();
}

void Krauler::search_for_links(GumboNode* node, ThreadPool& pool) {
  if (node->type != GUMBO_NODE_ELEMENT) {
    return;
  }
  GumboAttribute* href;
  if (node->v.element.tag == GUMBO_TAG_A &&
      (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
     std::cout <<converting (href->value) << std::endl;
    pool.enqueue(&Krauler::download, this, converting(href->value));
  }
  GumboVector* children = &node->v.element.children;
  for (unsigned int i = 0; i < children->length; ++i) {
    search_for_links(static_cast<GumboNode*>(children->data[i]), pool);
  }
}

void Krauler::parse() {
  GumboOutput* output = gumbo_parse(parse_queue.front().c_str());
  ThreadPool pool(4);
  search_for_links(output->root, pool);
  std::cout << parse_queue.size() << std::endl;
  /*for (unsigned i = 0; i < parse_queue.size(); i++) {
    std::cout << parse_queue[i] << std::endl;
    std::cout << std::endl;
  }*/
  gumbo_destroy_output(&kGumboDefaultOptions, output);
}

std::string Krauler::converting(std::string url) {
  if ((url[0] == '/') && (url[1] == '/')) url = url.substr(2);
  std::size_t pos = url.find("https:");
  if (pos == 0) url = url.substr(8);
  return url;
}
