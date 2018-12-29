#include <krauler.hpp>

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

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
  size_t depth = 4;
  std::string url = "yandex.ru";
  std::string output = "Log.txt";
  size_t network_threads = 2;
  size_t parser_threads = 2;
  po::options_description desc("Allowed options");
  desc.add_options()("help", "produce help message")(
      "depth", po::value<size_t>(&depth), "the depth of the search page")(
      "url", po::value<std::string>(&url), "URL start page")(
	  "network_threads", po::value<size_t>(&network_threads), "number of threads to download pages")(
      "parser_threads", po::value<size_t>(&parser_threads), "number of threads to process pages")(
	  "output", po::value<std::string>(&output), "the path to the output file");
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
