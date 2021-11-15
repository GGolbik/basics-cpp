#include <csignal>
#include <fstream>
#include <iostream>
#include <string>

#include "Client.h"
#include "OpenSslWrapper.h"
#include "Server.h"

static bool fileExists(const std::string &name) {
  std::ifstream f(name.c_str());
  return f.good();
}

static int runServer(std::string serverAddress = "", unsigned short port = 5044,
                     std::string key = "", std::string cert = "") {
  std::cout << "Starting server..." << std::endl;
  ggolbik::cpp::tls::Server server(port, serverAddress);
  if (key.empty() && cert.empty()) {
    if (!fileExists(server.getKeyFileName()) ||
        !fileExists(server.getCertFileName())) {
      std::cout << "Generate self signed certificate." << std::endl;
      if (!ggolbik::cpp::tls::OpenSslWrapper::createSelfSignedCert(
              server.getKeyFileName(), server.getCertFileName(), "")) {
        std::cerr << "Failed to create self signed certificate." << std::endl;
        return -1;
      }
    }
    std::cout << "Using self signed certificate." << std::endl;
  }
  if (!key.empty()) {
    server.setKeyFileName(key);
  }
  if (!cert.empty()) {
    server.setCertFileName(cert);
  }
  server.open();

  if (!server.isOpen()) {
    std::cout << "Failed to open server." << std::endl;
    return -1;
  }

  std::cout << "Started server." << std::endl;

  std::cout << ">>> Type any key and press return to stop." << std::endl;
  char k;
  std::cin >> k;

  std::cout << "Stopping server..." << std::endl;

  server.close();

  if (server.isOpen()) {
    std::cout << "Failed to close server." << std::endl;
    return -1;
  }
  std::cout << "Stopped server." << std::endl;
  return 0;
}

static int runClient(std::string serverAddress = "127.0.0.1",
                     unsigned short port = 5044) {
  std::cout << "Starting client..." << std::endl;
  ggolbik::cpp::tls::Client client(serverAddress, port);

  std::cout << "Connect to server." << std::endl;
  if (!client.open()) {
    std::cout << "Failed to connect to server." << std::endl;
  } else {
    std::cout << "Client is connected to server." << std::endl;

    std::cout << ">>> Enter 'quit', 'q' or 'exit' to stop program."
              << std::endl;
    std::string message;
    do {
      if (message != "") {
        if (client.write(message.c_str(), message.size())) {
          std::cout << "> Data has been sent." << std::endl;
        } else {
          std::cerr << "> Failed to send data." << std::endl;
          break;
        }
      }

      std::string response;
      if (client.tryReadString(response)) {
        std::cout << "Response: " << response << std::endl;
      }

      std::cout << ">>> Enter a message to send and press return." << std::endl;
      std::getline(std::cin, message);
    } while (message != "quit" && message != "q" && message != "exit");
  }

  std::cout << "Stopping client..." << std::endl;

  client.close();

  std::cout << "Stopped client." << std::endl;
  return 0;
}

static void printHelp() {
  std::cout << "Usage:" << std::endl;
  std::cout << "\tActions:" << std::endl;
  std::cout << "\t\tserver" << std::endl;
  std::cout << "\t\tclient" << std::endl;
  std::cout << "\tParameters:" << std::endl;
  std::cout << "\t\thost=<IP-Address>" << std::endl;
  std::cout << "\t\tport=<Port Number>" << std::endl;
  std::cout << "\t\tkey=<path to key file>" << std::endl;
  std::cout << "\t\tcert=<path to cert file>" << std::endl;
  std::cout << "\tExample:" << std::endl;
  std::cout << "\t\tproject_cpp_binary client host=127.0.0.1 port=5044"
            << std::endl;
}

int main(int argc, char *argv[]) {
  bool isServer = false;
  bool isClient = false;
  std::string serverAddress = "127.0.0.1";
  int port = 5044;
  std::string key = "";
  std::string cert = "";

#ifndef _WIN32
  // writing to a broken socket will cause a SIGPIPE and make the program crash.
  // ignore the SIGPIPE and handle the error directly in your code.
  // set the SIGPIPE handler to SIG_IGN.
  // This will prevent any socket or pipe write from causing a SIGPIPE signal.
  std::signal(SIGPIPE, SIG_IGN);
#endif

  std::cout << "Input Arguments:" << std::endl;
  for (int i = 0; i < argc; i++) {
    std::cout << "\t" << std::to_string(i) << ": " << argv[i] << std::endl;
    if (std::string("client").compare(argv[i]) == 0) {
      isClient = true;
    }
    if (std::string("server").compare(argv[i]) == 0) {
      isServer = true;
    }
    if (std::string(argv[i]).rfind("host=", 0) == 0) {
      serverAddress = std::string(argv[i]);
      std::string delimiter = "host=";
      serverAddress = serverAddress.substr(5, serverAddress.size() - 5);
    }
    if (std::string(argv[i]).rfind("key=", 0) == 0) {
      key = std::string(argv[i]);
      std::string delimiter = "key=";
      key = key.substr(4, key.size() - 4);
    }
    if (std::string(argv[i]).rfind("cert=", 0) == 0) {
      cert = std::string(argv[i]);
      std::string delimiter = "cert=";
      cert = cert.substr(5, cert.size() - 5);
    }
    if (std::string(argv[i]).rfind("port=", 0) == 0) {
      std::string strPort = std::string(argv[i]);
      std::string delimiter = "port=";
      strPort = strPort.substr(5, strPort.size() - 5);
      try {
        unsigned long inputPort = stoul(strPort);
        if (inputPort > UINT16_MAX) {
          throw std::invalid_argument(
              "Port number is out of range. Value must between 0 and 65535.");
        }
        port = (unsigned short)inputPort;
      } catch (std::invalid_argument &e) {
        std::cerr << "Failed to parse port '" << strPort << "'. " << e.what()
                  << std::endl;
      }
    }
  }

  if (!isServer && !isClient) {
    std::cerr << "client and/or server must be started." << std::endl;
    printHelp();
    return -1;
  } else if (isServer && isClient) {
    std::cerr << "Just client or server can be started and not both."
              << std::endl;
    printHelp();
    return -1;
  }

  // print parameters
  std::cout << "Host: " << serverAddress << std::endl;
  std::cout << "Port: " << port << std::endl;
  std::cout << "Key: " << key << std::endl;
  std::cout << "Cert: " << cert << std::endl;

  if (isServer) {
    runServer(serverAddress, port);
  } else if (isClient) {
    runClient(serverAddress, port);
  }
}