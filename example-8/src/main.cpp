#include <csignal>
#include <fstream>
#include <iostream>
#include <string>

#include "Algorithm.h"
#include "Client.h"
#include "OpenSslWrapper.h"
#include "Server.h"

static bool fileExists(const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}

static int runServer(const std::string& serverAddress = "",
                     unsigned short port = 5044, const std::string& key = "",
                     const std::string& cert = "", std::string password = "") {
  std::cout << "Starting server..." << std::endl;
  ggolbik::cpp::tls::Server server(port, serverAddress);
  if (key.empty() && cert.empty()) {
    if (!fileExists(server.getKeyFileName()) ||
        !fileExists(server.getCertFileName())) {
      std::cout << "Generate self signed certificate." << std::endl;
      if (!ggolbik::cpp::tls::OpenSslWrapper::createSelfSignedCert(
              server.getKeyFileName(), server.getCertFileName(), password)) {
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

static int runClient(const std::string& serverAddress = "127.0.0.1",
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

static int runAlgorithm(const std::string& task, const std::string& input,
                        const std::string& signature,
                        const std::string fileName, std::string key = "",
                        std::string cert = "", std::string password = "") {
  if (task == "base64-encode") {
    std::string encoded;
    if (!input.empty()) {
      if (ggolbik::cpp::tls::Algorithm::encodeBase64(input, encoded)) {
        std::cout << "base64-encoded (input): " << encoded << std::endl;
        return 0;
      }
    }
  } else if (task == "base64-decode") {
    std::string decoded;
    if (!input.empty()) {
      if (ggolbik::cpp::tls::Algorithm::decodeBase64(input, decoded)) {
        std::cout << "base64-decoded (input): " << decoded << std::endl;
        return 0;
      }
    }
  } else if (task == "base64url-encode") {
    std::string encoded;
    if (!input.empty()) {
      if (ggolbik::cpp::tls::Algorithm::encodeBase64Url(input, encoded)) {
        std::cout << "base64url-encoded (input): " << encoded << std::endl;
        return 0;
      }
    }
  } else if (task == "base64url-decode") {
    std::string decoded;
    if (!input.empty()) {
      if (ggolbik::cpp::tls::Algorithm::decodeBase64Url(input, decoded)) {
        std::cout << "base64url-decoded (input): " << decoded << std::endl;
        return 0;
      }
    }
  } else if (task == "sha256") {
    std::string hashString;
    if (!input.empty()) {
      if (ggolbik::cpp::tls::Algorithm::calcSha256String(input, hashString)) {
        std::cout << "sha256 (input): " << hashString << std::endl;
        return 0;
      }
    } else if (!fileName.empty() && fileExists(fileName)) {
      if (ggolbik::cpp::tls::Algorithm::calcSha256File(fileName, hashString)) {
        std::cout << "sha256 (file): " << hashString << std::endl;
        return 0;
      }
    }
  } else if (task == "sign") {
    if (key.empty() && cert.empty()) {
      key = "key.pem";
      cert = "cert.pem";
      if (!fileExists(key) || !fileExists(cert)) {
        std::cout << "Generate self signed certificate." << std::endl;
        if (!ggolbik::cpp::tls::OpenSslWrapper::createSelfSignedCert(key, cert,
                                                                     "")) {
          std::cerr << "Failed to create self signed certificate." << std::endl;
          return -1;
        }
      }
      std::cout << "Using self signed certificate." << std::endl;
    }

    ggolbik::cpp::tls::OpenSslWrapper::TlsKey privateKey = {};
    if (!ggolbik::cpp::tls::OpenSslWrapper::readKeyFile(key, privateKey,
                                                        password)) {
      std::cerr << "Failed to read key." << std::endl;
      return -1;
    }

    if (!input.empty()) {
      std::string signatureStr;
      if (!ggolbik::cpp::tls::OpenSslWrapper::signData(privateKey, input,
                                                       signatureStr)) {
        std::cerr << "Failed to sign data." << std::endl;
        return -1;
      }

      std::string encodedSignature;
      if (!ggolbik::cpp::tls::Algorithm::encodeBase64(signatureStr,
                                                      encodedSignature)) {
        std::cerr << "Failed to encode signed data." << std::endl;
        return -1;
      }

      std::cout << "signature (base64): " << encodedSignature << std::endl;
    }
  } else if (task == "verify") {
    if (key.empty() && cert.empty()) {
      key = "key.pem";
      cert = "cert.pem";
      if (!fileExists(key) || !fileExists(cert)) {
        std::cout << "Generate self signed certificate." << std::endl;
        if (!ggolbik::cpp::tls::OpenSslWrapper::createSelfSignedCert(key, cert,
                                                                     "")) {
          std::cerr << "Failed to create self signed certificate." << std::endl;
          return -1;
        }
      }
      std::cout << "Using self signed certificate." << std::endl;
    }

    ggolbik::cpp::tls::OpenSslWrapper::TlsX509Cert x509Cert = {};
    if (!ggolbik::cpp::tls::OpenSslWrapper::readCertFile(cert, x509Cert, "")) {
      std::cerr << "Failed to read cert." << std::endl;
      return -1;
    }
    // ggolbik::cpp::tls::OpenSslWrapper::displayCert(x509Cert);
    ggolbik::cpp::tls::OpenSslWrapper::TlsKey pubKey = {};
    if (!ggolbik::cpp::tls::OpenSslWrapper::readCertKey(x509Cert, pubKey)) {
      std::cerr << "Failed to read key." << std::endl;
    }

    if (!input.empty()) {
      std::string encodedSignature;
      ggolbik::cpp::tls::Algorithm::decodeBase64(signature, encodedSignature);
      if (ggolbik::cpp::tls::OpenSslWrapper::verifySignedData(
              pubKey, input, encodedSignature, "")) {
        std::cout << "Data is valid." << std::endl;
      } else {
        std::cout << "Data is not valid." << std::endl;
      }
    }
  }
  return -1;
}

static void printHelp() {
  std::cout << "Usage:" << std::endl;
  std::cout << "\tActions:" << std::endl;
  std::cout << "\t\tserver" << std::endl;
  std::cout << "\t\tclient" << std::endl;
  std::cout << "\t\talgorithm" << std::endl;
  std::cout << "\tParameters:" << std::endl;
  std::cout << "\t\thost=<IP-Address>" << std::endl;
  std::cout << "\t\tport=<Port Number>" << std::endl;
  std::cout << "\t\tkey=<path to key file>" << std::endl;
  std::cout << "\t\tcert=<path to cert file>" << std::endl;
  std::cout << "\t\ttask=<base64-encode|base64-decode|base64url-"
               "encode|base64url-decode|sha256|sign|verify>"
            << std::endl;
  std::cout << "\t\tinput=<data to consume>" << std::endl;
  std::cout << "\t\tsignature=<base64 signature of input>" << std::endl;
  std::cout << "\t\tfile=<file which contains the data to consume>"
            << std::endl;
  std::cout << "\tExample:" << std::endl;
  std::cout << "\t\tproject_cpp_binary client host=127.0.0.1 port=5044"
            << std::endl;
}

class Configuration {
 public:
  bool isServer;
  bool isClient;
  bool isAlgorithm;
  std::string serverAddress = "127.0.0.1";
  int port = 5044;
  std::string key = "";
  std::string cert = "";
  std::string algorithmTask;
  std::string algorithmInput;
  std::string algorithmFile;
  std::string algorithmSignature;
};

static bool parseArguments(Configuration& configuration, int argc,
                           char* argv[]) {
  configuration = {};

  std::cout << "Input Arguments:" << std::endl;
  for (int i = 0; i < argc; i++) {
    std::cout << "\t" << std::to_string(i) << ": " << argv[i] << std::endl;
    if (std::string("client").compare(argv[i]) == 0) {
      configuration.isClient = true;
    }
    if (std::string("server").compare(argv[i]) == 0) {
      configuration.isServer = true;
    }
    if (std::string("algorithm").compare(argv[i]) == 0) {
      configuration.isAlgorithm = true;
    }
    if (std::string(argv[i]).rfind("host=", 0) == 0) {
      configuration.serverAddress = std::string(argv[i]);
      std::string delimiter = "host=";
      configuration.serverAddress = configuration.serverAddress.substr(
          5, configuration.serverAddress.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("key=", 0) == 0) {
      configuration.key = std::string(argv[i]);
      std::string delimiter = "key=";
      configuration.key = configuration.key.substr(
          delimiter.size(), configuration.key.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("cert=", 0) == 0) {
      configuration.cert = std::string(argv[i]);
      std::string delimiter = "cert=";
      configuration.cert = configuration.cert.substr(
          delimiter.size(), configuration.cert.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("port=", 0) == 0) {
      std::string strPort = std::string(argv[i]);
      std::string delimiter = "port=";
      strPort =
          strPort.substr(delimiter.size(), strPort.size() - delimiter.size());
      try {
        unsigned long inputPort = stoul(strPort);
        if (inputPort > UINT16_MAX) {
          throw std::invalid_argument(
              "Port number is out of range. Value must between 0 and 65535.");
        }
        configuration.port = (unsigned short)inputPort;
      } catch (std::invalid_argument& e) {
        std::cerr << "Failed to parse port '" << strPort << "'. " << e.what()
                  << std::endl;
      }
    }
    if (std::string(argv[i]).rfind("task=", 0) == 0) {
      configuration.algorithmTask = std::string(argv[i]);
      std::string delimiter = "task=";
      configuration.algorithmTask = configuration.algorithmTask.substr(
          delimiter.size(),
          configuration.algorithmTask.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("input=", 0) == 0) {
      configuration.algorithmInput = std::string(argv[i]);
      std::string delimiter = "input=";
      configuration.algorithmInput = configuration.algorithmInput.substr(
          delimiter.size(),
          configuration.algorithmInput.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("signature=", 0) == 0) {
      configuration.algorithmSignature = std::string(argv[i]);
      std::string delimiter = "signature=";
      configuration.algorithmSignature =
          configuration.algorithmSignature.substr(
              delimiter.size(),
              configuration.algorithmSignature.size() - delimiter.size());
    }
    if (std::string(argv[i]).rfind("file=", 0) == 0) {
      configuration.algorithmFile = std::string(argv[i]);
      std::string delimiter = "file=";
      configuration.algorithmFile = configuration.algorithmFile.substr(
          delimiter.size(),
          configuration.algorithmFile.size() - delimiter.size());
    }
  }

  if (!configuration.isServer && !configuration.isClient &&
      !configuration.isAlgorithm) {
    std::cerr << "Action must be selected." << std::endl;
    return false;
  } else if ((configuration.isServer && configuration.isClient) ||
             (configuration.isServer && configuration.isAlgorithm) ||
             (configuration.isClient && configuration.isAlgorithm)) {
    std::cerr << "You can not select multiple actions." << std::endl;
    return false;
  }

  // print parameters
  std::cout << "Host: " << configuration.serverAddress << std::endl;
  std::cout << "Port: " << configuration.port << std::endl;
  std::cout << "Key: " << configuration.key << std::endl;
  std::cout << "Cert: " << configuration.cert << std::endl;
  std::cout << "Task: " << configuration.algorithmTask << std::endl;
  std::cout << "Input: " << configuration.algorithmInput << std::endl;
  std::cout << "Signature: " << configuration.algorithmSignature << std::endl;
  std::cout << "File: " << configuration.algorithmFile << std::endl;

  return true;
}

int main(int argc, char* argv[]) {
  Configuration configuration;
  if (!parseArguments(configuration, argc, argv)) {
    printHelp();
    return -1;
  }

#ifndef _WIN32
  // writing to a broken socket will cause a SIGPIPE and make the program crash.
  // ignore the SIGPIPE and handle the error directly in your code.
  // set the SIGPIPE handler to SIG_IGN.
  // This will prevent any socket or pipe write from causing a SIGPIPE signal.
  std::signal(SIGPIPE, SIG_IGN);
#endif

  if (configuration.isServer) {
    return runServer(configuration.serverAddress, configuration.port);
  } else if (configuration.isClient) {
    return runClient(configuration.serverAddress, configuration.port);
  } else if (configuration.isAlgorithm) {
    return runAlgorithm(
        configuration.algorithmTask, configuration.algorithmInput,
        configuration.algorithmSignature, configuration.algorithmFile);
  }
}