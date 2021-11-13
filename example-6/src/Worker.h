#ifndef GGOLBIK_CPLUSPLUS_SOCKET_WORKER_H
#define GGOLBIK_CPLUSPLUS_SOCKET_WORKER_H

#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <memory>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace ggolbik
{
namespace cplusplus
{
namespace socket
{

/**
 * 
 */
class Worker
{

private: // type definitions
  typedef char byte;

private: // const
  // 64KiByte
  static const unsigned int MAX_BUFFER_SIZE = 65535;

public: // construction/destruction/operators
  /**
   * Default constructor
   */
  #ifdef _WIN32
  Worker(SOCKET socket);  
  #else
  Worker(int socket);  
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
  Worker &operator=(Worker &) = delete;
  /**
   * Destructor
   */
  virtual ~Worker();

public: // methods
  bool start();
  bool isRunning();
  /**
   * Stops the worker and closes the socket. 
   */
  void close();

private: // helper methods
  void run();
  bool readString(std::string& message);
  bool write(const byte data[], size_t length);

private: // fields
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
};

} // namespace socket
} // namespace cplusplus
} // namespace ggolbik

#endif
