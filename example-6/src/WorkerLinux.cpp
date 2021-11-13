#ifdef __linux__
// linux code goes here

#include <sys/socket.h> // includes a number of definitions of structures needed for sockets.
#include <netinet/in.h> // contains constants and structures needed for internet domain address.
#include <unistd.h>     // ::close(int), ::read(int, void*, size_t)
#include <cstring>
#include <string>
#include <iostream>
#include <errno.h> // errno - is thread safe. On Linux, the global errno variable is thread-specific. POSIX requires that errno be threadsafe.
#include <limits>  // for numeric_limits
#include <memory>

#include "Worker.h"

namespace ggolbik
{
namespace cplusplus
{
namespace socket
{

Worker::Worker(int socket) : clientSocket{socket}, enabled{false}, running{false} {}

Worker::~Worker()
{
  this->close();
}

/**
 * errno is thread safe. On Linux, the global errno variable is thread-specific. POSIX requires that errno be threadsafe.
 * If the value of errno should be preserved across a library call, it must be saved.
 * 
 * strerror_r is thread safe.
 * 
 * errno is defined in <cerrno> and is an integer value.
 * 
 * The method call
 *   char *::strerror_r(int errnum, char *buf, size_t buflen);
 * is defined in <cstring>
 */
static void printError()
{
  size_t length = 1024;
  char buffer[length];
  std::cerr << "(" << errno << ") " << ::strerror_r(errno, buffer, length) << std::endl;
}

bool Worker::start()
{
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);
  
  if (this->clientSocket == -1)
  {
    std::cerr << "Invalid socket" << std::endl;
    return false;
  }

  this->enabled = true;
  this->running = true;

  this->workerThread = std::thread(&Worker::run, this);

  std::cout << "Worker thread ID: " << this->workerThread.get_id() << std::endl;

  return true;
}

bool Worker::isRunning()
{
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);
  // return whether server has been stopped or is still running
  return this->enabled || this->running;
}

static bool closeSocket(int socket)
{
  if (socket == -1)
  {
    // invalid socket
    // socket already closed
    return true;
  }

  if (::close(socket) != 0)
  {
    return false;
  }
  else
  {
    return true;
  }
}

void Worker::close()
{
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  if (this->enabled)
  {
    this->enabled = false;

    std::cout << "Join worker thread" << std::endl;

    if (this->workerThread.joinable())
    {
      this->workerThread.join();
    }

    std::cout << "Close connection." << std::endl;

    // With shutdown, you will still be able to receive pending data the peer already sent.
    // We use close because this is not required.
    // close socket
    if (!closeSocket(this->clientSocket))
    {
      printError();
      std::cout << "Shutdown socket failed." << std::endl;
    }
    else
    {
      std::cout << "Connection closed." << std::endl;
    }
    this->clientSocket = -1;
    this->running = false;
  }
}

bool Worker::readString(std::string &message)
{
  byte recvbuf[Worker::MAX_BUFFER_SIZE];

  message = "";

  ssize_t rc;
  while (this->enabled)
  {
    rc = ::read(this->clientSocket, recvbuf, Worker::MAX_BUFFER_SIZE);
    if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
      // The file descriptor fd refers to a file other than a socket and has been marked nonblocking (O_NONBLOCK), and the read would block.
      if (this->enabled)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      continue;
    }
    else if (rc < 0)
    {
      // an error occurred
      printError();
      std::cerr << "Failed to read" << std::endl;
      break;
    }
    else if (rc == 0)
    {
      // reached end of file
      break;
    }
    else if (!this->enabled)
    {
      // server shall stop listening
      break;
    }
    else
    {
      // data has been read
      message = std::string(recvbuf, static_cast<size_t>(rc));
      return true;
    }
  }

  return false;
}

void Worker::run()
{
  // Receive until the peer shuts down the connection
  while (this->enabled)
  {
    std::string message;
    if (this->readString(message))
    {
      std::cout << "Data: " << message << std::endl;
      if (!this->write(message.c_str(), message.size()))
      {
        std::cerr << "Failed to send data to client" << std::endl;
      }
    }
    else
    {
      // failed to read
      break;
    }
  }

  if (!closeSocket(this->clientSocket))
  {
    printError();
    std::cout << "Shutdown socket failed." << std::endl;
  }
  else
  {
    this->clientSocket = -1;
  }

  this->running = false;
  std::cout << "Stopped Worker thread ID: " << this->workerThread.get_id() << std::endl;
}

/**
 * ssize_t write(int fd, const void *buf, size_t count);
 */
bool Worker::write(const byte data[], size_t length)
{
  if (length == 0)
  {
    return true;
  }

  size_t position = 0;
  int remaining = length;

  ssize_t rc;
  while (this->enabled && remaining > 0)
  {
    rc = ::write(this->clientSocket, data + (sizeof(byte) * position), remaining);

    if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
      std::cout << "Worker write AGAIN" << std::endl;
      // The file descriptor fd refers to a file other than a socket and has been marked nonblocking (O_NONBLOCK), and the read would block.
      if (this->enabled)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      continue;
    }
    else if (rc < 0)
    {
      // an error occurred
      printError();
      std::cerr << "Failed to write" << std::endl;
      break;
    }
    else if (rc == 0)
    {
      // reached end of file
      break;
    }
    else if (!this->enabled)
    {
      break;
    }

    remaining -= rc;
    position += rc;
  }

  if (remaining == 0)
  {
    return true;
  }

  return false;
}

} // namespace socket
} // namespace cplusplus
} // namespace ggolbik
#endif