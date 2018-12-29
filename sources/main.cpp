#include <krauler.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  size_t depth = 4;
  std::string url = "yandex.ru";
  std::string output = "Log.txt";
  size_t network_threads = 2;
  size_t parser_threads = 2;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "depth", po::value<size_t>(&depth), "set compression level")(
      "url", po::value<std::string>(&url), "URL start page")(
	  "network_threads", po::value<size_t>(&network_threads), "")(
      "parser_threads", po::value<size_t>(&parser_threads), "")(
	  "output", po::value<std::string>(&output), "");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  try {
    Krauler k(url, depth, network_threads, parser_threads, output);
    k.make();
  }
  catch (std::exception& e) {
        e.what();
  }
}
