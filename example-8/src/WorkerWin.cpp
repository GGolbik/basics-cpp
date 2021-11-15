#ifdef _WIN32

#include <iostream>
// Windows system header files must be lower case.
#include <winsock2.h>  // The Winsock2.h header file internally includes core elements from the Windows.h header file, so there is not usually an #include line for the Windows.h header file in Winsock applications.

#include "Worker.h"

namespace ggolbik {
namespace cpp {
namespace tls {

Worker::Worker(SOCKET socket)
    : clientSocket{socket}, enabled{false}, running{false} {}

Worker::~Worker() { this->close(); }

/**
 *
 *
 * @param error
 * @param message
 */
static void printError() {
  LPWSTR pBuffer = NULL;
  if (FormatMessageW(
          FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
          (LPWSTR)&pBuffer, 0, NULL) == 0) {
    // failed to get message
    std::cerr << "Failed to get error message." << std::endl;
  } else {
    std::wcerr << std::wstring(pBuffer) << std::endl;
  }
  LocalFree(pBuffer);
}

bool Worker::start() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  if (this->clientSocket == INVALID_SOCKET) {
    std::cerr << "Invalid socket" << std::endl;
    return false;
  }

  this->enabled = true;
  this->running = true;

  this->workerThread = std::thread(&Worker::run, this);

  std::cout << "Worker thread ID: " << this->workerThread.get_id() << std::endl;

  return true;
}

bool Worker::isRunning() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);
  // return whether server has been stopped or is still running
  return this->enabled || this->running;
}

static bool closeSocket(SOCKET socket) {
  if (socket == INVALID_SOCKET) {
    // invalid socket
    // socket already closed
    return true;
  }

  if (::closesocket(socket) != 0) {
    return false;
  } else {
    return true;
  }
}

void Worker::close() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  if (this->enabled) {
    this->enabled = false;

    std::cout << "Join worker thread" << std::endl;

    if (this->workerThread.joinable()) {
      this->workerThread.join();
    }

    std::cout << "Close connection." << std::endl;

    // close socket
    if (!closeSocket(this->clientSocket)) {
      std::cerr << "Shutdown socket failed." << std::endl;
      printError();
    } else {
      std::cout << "Connection closed." << std::endl;
    }

    // set socket to an invalid value.
    this->clientSocket = INVALID_SOCKET;
    this->running = false;
  }
}

void Worker::run() {
  // Receive until the peer shuts down the connection
  while (this->enabled) {
    std::string message;
    if (this->readString(message)) {
      std::cout << "Data: " << message << std::endl;
      if (!this->write(message.c_str(), message.size())) {
        std::cerr << "Failed to send data to client" << std::endl;
      }
    } else {
      // failed to read
      break;
    }
  }

  if (!closeSocket(this->clientSocket)) {
    std::cout << "Shutdown socket failed." << std::endl;
    printError();
  }
  this->clientSocket = INVALID_SOCKET;

  this->running = false;
  std::cout << "Stopped Worker thread ID: " << this->workerThread.get_id()
            << std::endl;
}

bool Worker::readString(std::string &message) {
  byte recvbuf[Worker::MAX_BUFFER_SIZE];

  message = "";

  ssize_t rc;
  while (this->enabled) {
    rc = ::recv(this->clientSocket, recvbuf, Worker::MAX_BUFFER_SIZE, 0);
    if (rc < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
      // The file descriptor fd refers to a file other than a socket and has
      // been marked nonblocking (O_NONBLOCK), and the read would block.
      if (this->enabled) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
      continue;
    } else if (rc < 0) {
      // an error occurred
      std::cerr << "Failed to read" << std::endl;
      break;
    } else if (rc == 0) {
      // reached end of file
      break;
    } else if (!this->enabled) {
      // server shall stop listening
      break;
    } else {
      // data has been read
      message = std::string(recvbuf, static_cast<size_t>(rc));
      return true;
    }
  }

  return false;
}

bool Worker::write(const byte data[], size_t length) {
  if (length == 0) {
    return true;
  }

  size_t position = 0;
  int remaining = length;

  ssize_t rc;
  while (this->enabled && remaining > 0) {
    rc = ::send(this->clientSocket, data + (sizeof(byte) * position), remaining,
                0);

    if (rc < 0 && WSAGetLastError() == WSAEWOULDBLOCK) {
      std::cout << "Worker write AGAIN" << std::endl;
      // The file descriptor fd refers to a file other than a socket and has
      // been marked nonblocking (O_NONBLOCK), and the read would block.
      if (this->enabled) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      continue;
    } else if (rc < 0) {
      // an error occurred
      std::cerr << "Failed to write" << std::endl;
      break;
    } else if (rc == 0) {
      // reached end of file
      break;
    } else if (!this->enabled) {
      break;
    }

    remaining -= rc;
    position += rc;
  }

  if (remaining == 0) {
    return true;
  }

  return false;
}

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
#endif