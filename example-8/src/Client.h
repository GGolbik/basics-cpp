#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#ifndef _WIN32
#include "OpenSslWrapper.h"
#endif

namespace ggolbik {
namespace cpp {
namespace tls {

class Client {
 private:  // type definitions
  typedef char byte;

 public:  // const
  // 64KiByte
  static const unsigned int MAX_BUFFER_SIZE = 65535;

 public:  // construction/destruction/operators
  /**
   * @brief Construct a new Client object
   *
   * @param serverAddress the interface
   * @param port the port
   */
  Client(std::string serverAddress, unsigned short port);
  /**
   * Move constructor
   */
  Client(Client &&) = delete;
  /**
   * Move assignment operator
   */
  Client &operator=(Client &&) = delete;
  /**
   * Copy constructor
   */
  Client(const Client &) = delete;
  /**
   * Copy assignment operator
   */
  Client &operator=(Client &) = delete;
  /**
   * Destructor
   */
  virtual ~Client();

 public:  // methods
  bool open();
  bool isOpen();
  void close();
  /**
   * @brief Reads a string from the stream.
   *
   * @param message The read message if return value is > 0
   * @return the size of the read data, if 0 there was no data, if -1 an error
   * occured.
   */
  int tryReadString(std::string &message);
  /**
   * @brief Reads a string from the stream. Blocks until data is available or an
   * error occured.
   *
   * @param message
   * @return true if data has been read. false if an error occured.
   */
  bool readString(std::string &message);
  /**
   * @brief Writes data to the stream.
   *
   * @param data the data to write
   * @param length the length of the array
   * @return true if data has been written. false if an error occured.
   */
  bool write(const byte data[], size_t length);

 private:
  bool closeSocket();

 private:
  std::mutex mutexPublicMethods;
  int clientSocket;
  bool enabled;
  std::string serverAddress;
  unsigned short port;

 public:  // TLS methods
  int tryReadStringTls(std::string &message);
  // blocks until data is available
  bool readStringTls(std::string &message);
  bool writeTls(const byte data[], size_t length);

 private:  // TLS fields
  std::string keyFileName;
  std::string certFileName;
#ifndef _WIN32
  OpenSslWrapper::TlsContextPtr tlsContextPtr;
  OpenSslWrapper::TlsPtr tlsPtr;
#endif
};

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
