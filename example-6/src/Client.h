#ifndef GGOLBIK_CPLUSPLUS_SOCKET_CLIENT_H
#define GGOLBIK_CPLUSPLUS_SOCKET_CLIENT_H

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

namespace ggolbik
{
namespace cplusplus
{
namespace socket
{

class Client
{
private: // type definitions
  typedef char byte;

public: // const
  // 64KiByte
  static const unsigned int MAX_BUFFER_SIZE = 65535;

public: // construction/destruction/operators
  /**
   * @param serverAddress the interface
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

public: // methods
  bool open();
  bool isOpen();
  void close();
  bool tryReadString(std::string &message);
  // blocks until data is available
  bool readString(std::string &message);
  bool write(const byte data[], size_t length);

private:
  bool closeSocket();

private:
  std::mutex mutexPublicMethods;
  int clientSocket;
  bool enabled;
  std::string serverAddress;
  unsigned short port;
};

} // namespace socket
} // namespace cplusplus
} // namespace ggolbik

#endif