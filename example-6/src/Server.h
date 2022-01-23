#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace ggolbik {
namespace cpp {
namespace socket {

/**
 * Set up a listening socket
 */
class Server {
 private:  // type definitions
  typedef char byte;

 public:  // construction/destruction/operators
  /**
   * @param port define port of server
   */
  Server(unsigned short port);
  /**
   * @param port define port of server
   * @param interfaceAddress the interface
   */
  Server(unsigned short port, std::string interfaceAddress);
  /**
   * Move constructor
   */
  Server(Server &&) = delete;
  /**
   * Move assignment operator
   */
  Server &operator=(Server &&) = delete;
  /**
   * Copy constructor
   */
  Server(const Server &) = delete;
  /**
   * Copy assignment operator
   */
  Server &operator=(const Server &) = delete;
  /**
   * Destructor
   */
  virtual ~Server();

 public:  // methods
  /**
   * Opens the socket. Return false if server failed to open socket or is
   * already open.
   */
  bool open();
  /**
   * Returns true if the server is enabled or running
   */
  bool isOpen();
  /**
   * Close the server socket.
   */
  void close();

 private:  // helper methods
  /**
   * Close the socket and sets the socket member to an invalid value.
   *
   * @return true if socket could be closed, otherwise false.
   */
  bool closeSocket();
  /**
   * The method executed by the server thread.
   */
  void run();

 private:  // fields
  std::mutex mutexPublicMethods;
  std::mutex mutexCloseSocket;
  /**
   * The port to bind the listen socket.
   */
  unsigned short port;
  /**
   * The address to bind the listen socket.
   */
  std::string interfaceAddress;
  /**
   * Whether the server has been enabled.
   */
  std::atomic_bool enabled;
  /**
   * Whether the server is running.
   */
  std::atomic_bool running;
  /**
   * The thread used to listen for connections.
   */
  std::thread serverThread;
/**
 * The current listen socket.
 */
#ifdef _WIN32
  SOCKET listenSocket;
#else
  int listenSocket;
#endif
};

}  // namespace socket
}  // namespace cpp
}  // namespace ggolbik
