#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <openssl/ssl.h>

#include "OpenSslWrapper.h"
#endif

namespace ggolbik {
namespace cpp {
namespace tls {

/**
 *
 */
class Worker {
 private:  // type definitions
  typedef char byte;

 private:  // const
  // 64KiByte
  static const unsigned int MAX_BUFFER_SIZE = 65535;

 public:  // construction/destruction/operators
/**
 * Default constructor
 */
#ifdef _WIN32
  Worker(SOCKET socket);
#else
  Worker(int socket, ::SSL *ssl);
#endif
  /**
   * Move constructor
   */
  Worker(Worker &&) = delete;
  /**
   * Move assignment operator
   */
  Worker &operator=(Worker &&) = delete;
  /**
   * Copy constructor
   */
  Worker(const Worker &) = delete;
  /**
   * Copy assignment operator
   */
  Worker &operator=(const Worker &) = delete;
  /**
   * Destructor
   */
  virtual ~Worker();

 public:  // methods
  bool start();
  bool isRunning();
  /**
   * Stops the worker and closes the socket.
   */
  void close();

 private:  // helper methods
  void run();
  bool readString(std::string &message);
  bool write(const byte data[], size_t length);

 private:  // fields
  std::mutex mutexPublicMethods;
  std::mutex mutexCloseSocket;
#ifdef _WIN32
  SOCKET clientSocket;
#else
  int clientSocket;
#endif
  std::atomic_bool enabled;
  std::atomic_bool running;
  std::thread workerThread;

 public:  // TLS methods
  bool readStringTls(std::string &message);
  bool writeTls(const byte data[], size_t length);

 private:  // TLS fields
#ifndef _WIN32
  OpenSslWrapper::TlsPtr tlsPtr;
#endif
};

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
