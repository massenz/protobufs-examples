// Copyright (c) 2020 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com)

#include <iostream>
#include <regex>
#include <string>
#include <glog/logging.h>

#include "version.h"
#include "Sample.hpp"
#include "demo.pb.h"

using namespace std;

namespace demo {

inline std::ostream &operator<<(std::ostream &out, const demo::Server &server) {
  out << "'" << server.hostname() << ":" << server.port() << "'";

  if (server.has_ip_addr() && ! server.ip_addr().empty()) {
    out << " [" << server.ip_addr() << "]";
  }
  return out;
}

void usage(const std::string &arg) {
  regex progname{"/?(\\w+)$"};
  smatch matches;
  string prog{arg};

  if (regex_search(arg, matches, progname)) {
    prog = matches[0];
  }
  cout << "Usage: " << prog << " greeting" << endl;
}

void headline() {
  cout << "Template Project Demo - " << RELEASE_STR << endl << endl;
}
} // namespace demo

int main(int argc, char *argv[]) {

  demo::headline();
  if (argc < 2) {
    demo::usage(argv[0]);
    std::cerr << "[ERROR] Missing required argument `greeting`" << endl;
    exit(EXIT_FAILURE);
  }

  std::string mesg { argv[1] };
  ::Sample s(mesg);
  demo::Server self;

  // If importing the `distutils` library, you can use:
  //    server.set_hostname(utils::Hostname());
  //    server.set_ip_addr(utils::InetAddress());
  self.set_hostname("gondor");
  self.set_ip_addr("192.168.1.51");
  self.set_port(8080);

  cout << "Hello, " << s.name() << "!\nModify this template with your own fabulous code!"
       << "\nRunning on: " << self << endl;

  return EXIT_SUCCESS;
}
