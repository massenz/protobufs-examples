// Copyright (c) 2021 AlertAvert.com. All rights reserved.
// Created by M. Massenzio (marco@alertavert.com)

#include <iostream>
#include <regex>
#include <string>
#include <glog/logging.h>

#include <google/protobuf/util/json_util.h>

#include <json.hpp>

#include "distlib/utils/ParseArgs.hpp"
#include "envelope.pb.h"
#include "payload.pb.h"

#include "version.h"


using namespace std;
using namespace string_literals;
using namespace io::kapsules::clients;


std::ostream &operator<<(std::ostream &out, const Server &server) {
  out << server.server() << " ("s << server.server_id() << "): "s << server.location();
  return out;
}

std::ostream &operator<<(std::ostream &out, const Sources &sources) {
  for(const auto &source : sources.sources()) {

    std::string package = sources.package();
    std::replace(package.begin(), package.end(), '.', '/');
    out << package << "/"s << source << endl;
  }
  return out;
}

template<typename P>
inline std::ostream &Print(
    const io::kapsules::Envelope &envelope,
    std::ostream &out = std::cout
) {
  out << "From: " << envelope.sender() << endl;
  out << "Subject: " << envelope.message() << endl;

  if (envelope.has_payload()) {
    out << "Type: " << envelope.payload().type_url()
        << " (" << envelope.payload_type_id() << ")" << endl;
    out << "---- Payload ----" << endl;
    const auto &payload = envelope.payload();
    if (payload.Is<P>()) {
      P content;
      payload.UnpackTo(&content);
      cout << content << endl;
    }
    out << "---- Payload Ends ----" << endl;
  }
  return out;
}

void fromJson(const nlohmann::json &jsonData) {
  io::kapsules::Envelope fromJson;
  google::protobuf::util::Status status =
      google::protobuf::util::JsonStringToMessage(jsonData.dump(), &fromJson);
  if (status.ok()) {
    cout << "\n----\nProtobuf extracted from JSON:" << endl;
    if (fromJson.has_payload() && fromJson.payload().Is<Server>()) {
      Print<Server>(fromJson);
    } else {
      Print<Sources>(fromJson);
    }
  } else {
    LOG(ERROR) << "Could not convert server from JSON: " << status.error_message();
  }
}


void usage(const std::string &arg) {
  regex progname{"/?(\\w+)$"};
  smatch matches;
  string prog{arg};

  if (regex_search(arg, matches, progname)) {
    prog = matches[0];
  }
  cout << "Usage: " << prog << " --server | --source\n\n"
       << "--server       Use a Server type payload\n"
       << "--source       Use source code payload" << endl;
}

void headline() {
  cout << PROJECT_NAME << " - " << RELEASE_STR << endl << endl;
}

int main(int argc, const char *argv[]) {

  GOOGLE_PROTOBUF_VERIFY_VERSION;
  google::InitGoogleLogging(argv[0]);

  ::utils::ParseArgs parser(argv, argc);
  if (parser.Enabled("debug") || parser.Enabled("trace")) {
    FLAGS_logtostderr = true;
    FLAGS_v = parser.Enabled("debug") ? 2 : 3;
  }

  headline();
  if (parser.has("version")) {
    return EXIT_SUCCESS;
  }

  if (parser.has("help")) {
    usage(parser.progname());
    return EXIT_SUCCESS;
  }

  bool hasServerPayload = parser.Enabled("server");
  bool hasSourcePayload = parser.Enabled("source");

  if (!(hasServerPayload || hasSourcePayload)) {
    usage(parser.progname());
    LOG(ERROR) << "Missing required payload type" << endl;
    return EXIT_FAILURE;
  }

  using namespace io::kapsules;
  Envelope envelope;
  envelope.set_sender("demo-prog");
  envelope.set_message("This is a demo");

  if (hasServerPayload) {
    Server server;
    server.set_server("host.kapsules.io");
    server.set_server_id(9999);
    server.set_location("us-west-a2");

    // Use the Any PackFrom to encode an arbitrary PB payload.
    envelope.mutable_payload()->PackFrom(server);
    envelope.set_payload_type_id(PayloadType::SERVER_TYPE);
  } else {
    Sources sources;
    sources.set_package("com.example.data.container");
    sources.add_sources("one.java");
    sources.add_sources("two.java");
    sources.add_sources("three.java");

    // Use the Any PackFrom to encode an arbitrary PB payload.
    envelope.mutable_payload()->PackFrom(sources);
    envelope.set_payload_type_id(PayloadType::SOURCES_TYPE);
  }

  // After "receiving" the Envelope message, check its payload type:
  if (envelope.payload_type_id() == PayloadType::SERVER_TYPE) {
    Print<Server>(envelope);
  } else if (envelope.payload_type_id() == PayloadType::SOURCES_TYPE){
    Print<Sources>(envelope);
  } else {
    LOG(ERROR) << "Unknown payload type (" << envelope.payload_type_id() << ")";
    return EXIT_FAILURE;
  }

  std::string jsonOut;
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = true;

  auto status = google::protobuf::util::MessageToJsonString(envelope, &jsonOut, options);
  if (status.ok()) {
    using nlohmann::json;
    auto data = json::parse(jsonOut);

    // The entire envelope, including payload, can be serialized as JSON:
    cout << data.dump(2) << endl;

    // Or you can extract specific fields:
    if (data["payload"].contains("server")) {
      cout << "Server: " << data["payload"]["server"] << endl;
    }

    // From the JSON, we can reconstruct the Protobufs.
    // This just extracts the whole Envelope from JSON:
    fromJson(data);

    // Finally, from the JSON, one can just reconstruct the Payload
    auto payload = data["payload"];

    // Here we would validate the type as one known, before dropping it.
    cout << "Received a JSON payload of type: " << payload["@type"] << endl;
    if (payload["@type"].is_string()) {
      auto type = payload["@type"].get<std::string>();
      if (type.find("io.kapsules.clients.Server") != std::string::npos) {
        Server fromJson;

        // We need to remove the @type annotation, or Google Protobuf parser will complain
        // The Server PB has no knowledge of the Any configured in Envelope.
        payload.erase("@type");
        status = google::protobuf::util::JsonStringToMessage(payload.dump(), &fromJson);
        if (status.ok()) {
          cout << "Protobuf extracted from JSON payload:" << endl;
          cout << fromJson;
        } else {
          LOG(ERROR) << "Could not convert server from JSON: " << status.error_message();
        }
      }
    }
  } else {
    LOG(ERROR) << "Could not convert Envelope Protobuf to JSON: " << status.error_message();
  }

  google::protobuf::ShutdownProtobufLibrary();
  return EXIT_SUCCESS;
}
