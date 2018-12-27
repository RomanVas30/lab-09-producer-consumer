#include <krauler.hpp>

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  unsigned depth = 4;
  std::string url = "";
  std::string output = "";
  unsigned network_threads = 2;
  unsigned parser_threads = 2;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "depth", po::value<unsigned>(&depth), "set compression level")(
      "url", po::value<std::string>(&url), "URL start page")(
	  "network_threads", po::value<unsigned>(&network_threads), "")(
      "parser_threads", po::value<unsigned>(&parser_threads), "")(
	  "output", po::value<std::string>(&output), "");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm); 
  std::cout << depth << std::endl;
  std::cout << url << std::endl;
  std::cout << network_threads << std::endl;
  std::cout << parser_threads << std::endl;
  std::cout << output << std::endl;
  Krauler k(depth, network_threads, parser_threads, output);
  k.download(url);
}