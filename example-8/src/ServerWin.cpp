#ifdef _WIN32
// _WIN32 marco is defined for both 32-bit and 64-bit environments
// windows code goes here

// pragma for visual C++
//#pragma comment(lib, "Ws2_32.lib")

#include <iostream>  // std::cout(...), std::cerr(...)
#include <string>    // std::to_string

// Windows system header files must be lower case.
#include <winsock2.h>  // The Winsock2.h header file internally includes core elements from the Windows.h header file, so there is not usually an #include line for the Windows.h header file in Winsock applications.
#include <ws2tcpip.h>  // getaddrinfo

#include "Server.h"
#include "Worker.h"

namespace ggolbik {
namespace cpp {
namespace tls {

Server::Server(unsigned short port) : Server(port, "") {}

Server::Server(unsigned short port, std::string interfaceAddress)
    : port{port},
      interfaceAddress{interfaceAddress},
      enabled{false},
      running{false},
      listenSocket{INVALID_SOCKET} {}

Server::~Server() { this->close(); }

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

/**
 * The WSAStartup function initiates use of the Winsock DLL by a process.
 * The WSAStartup function must be the first Windows Sockets function called by
 * an application or DLL. It allows an application or DLL to specify the version
 * of Windows Sockets required and retrieve details of the specific Windows
 * Sockets implementation. The application or DLL can only issue further Windows
 * Sockets functions after successfully calling WSAStartup.
 *
 * @return true if initialized, otherwise false
 */
static bool initializeWinsock() {
  // Create a WSADATA object
  WSADATA wsaData;
  // The WSAStartup function is called to initiate use of WS2_32.dll.
  // If successful, the WSAStartup function returns zero. Otherwise, it returns
  // one of the error codes.
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

  // check for errors.
  if (result != 0) {
    switch (result) {
      case WSASYSNOTREADY:
        std::cerr << "The underlying network subsystem is not ready for "
                     "network communication."
                  << std::endl;
        break;
      case WSAVERNOTSUPPORTED:
        std::cerr
            << "The version of Windows Sockets support requested is not "
               "provided by this particular Windows Sockets implementation."
            << std::endl;
        break;
      case WSAEINPROGRESS:
        std::cerr << "A blocking Windows Sockets 1.1 operation is in progress."
                  << std::endl;
        break;
      case WSAEPROCLIM:
        std::cerr << "A limit on the number of tasks supported by the Windows "
                     "Sockets implementation has been reached."
                  << std::endl;
        break;
      case WSAEFAULT:
        std::cerr << "The lpWSAData parameter is not a valid pointer."
                  << std::endl;
        break;
      default:
        std::cerr << "Unknown error during initialization." << std::endl;
        break;
    }
    return false;
  }
  return true;
}

/**
 * The method
 *   INT WSAAPI getaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA
 * *pHints, PADDRINFOA *ppResult); is defined in header <ws2tcpip.h>
 *
 * ::htons converts a u_short from host to TCP/IP network byte order (which is
 * big-endian).
 *
 * The method
 *   u_short htons(u_short hostshort);
 * is defined in header <winsock.h>
 */
static bool createSocketAddress(unsigned short port,
                                const std::string &interfaceAddress,
                                sockaddr_in &socketAddress) {
  // define the IPv4 address family.
  socketAddress.sin_family = AF_INET;

  // set port
  socketAddress.sin_port = ::htons(port);

  // check interface address. use any address if not specified.
  if (interfaceAddress.empty()) {
    // INADDR_ANY will bind the socket to all available interfaces.
    socketAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
  } else {
    // convert and set IP address
    socketAddress.sin_addr.s_addr = ::inet_addr(interfaceAddress.c_str());
  }

  // check if address is valid
  if (socketAddress.sin_addr.s_addr == ::htonl(INADDR_NONE)) {
    // failed to set address
    return false;
  }  // else address is defined

  return true;
}

static SOCKET createSocket(sockaddr_in &socketAddress) {
  // SOCK_STREAM is used to specify a stream socket.
  // IPPROTO_TCP is used to specify the TCP protocol.
  return ::socket(socketAddress.sin_family, SOCK_STREAM, IPPROTO_TCP);
}

bool Server::closeSocket() {
  // lock mutex to set listen socket appropriately
  std::unique_lock<std::mutex> lock(this->mutexCloseSocket);

  // check if socket is valid
  if (this->listenSocket != INVALID_SOCKET) {
    // invalid socket
    // socket seems to be already closed.
    return true;
  }

  // close socket
  bool closed = ::closesocket(this->listenSocket) == 0;

  // set socket to an invalid value.
  this->listenSocket = INVALID_SOCKET;

  // return whether socket has been closed successfully.
  return closed;
}

static bool bindSocket(SOCKET socket, sockaddr_in &address) {
  // Setup the TCP listening socket
  // bind the socket
  int rc = ::bind(socket, (sockaddr *)&address, sizeof(address));
  if (rc == SOCKET_ERROR) {
    return false;
  }
  return true;
}

static bool setSocketModeNonBlocking(SOCKET socket) {
  // If iMode = 0, blocking is enabled;
  // If iMode != 0, non-blocking mode is enabled.

  u_long iMode = 1;
  if (::ioctlsocket(socket, FIONBIO, &iMode) == NO_ERROR) {
    return true;
  }
  return false;
}

static bool listenOnSocket(SOCKET socket) {
  // After the socket is bound to an IP address and port on the system, the
  // server must then listen on that IP address and port for incoming connection
  // requests. SOMAXCONN; // max incoming connections
  if (::listen(socket, SOMAXCONN) == SOCKET_ERROR) {
    return false;
  }
  return true;
}

static int select(SOCKET socket) {
  timeval timeout = {};
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;  // 100000 := 100ms

  fd_set readfds;

  int nfds = FD_SETSIZE;

  // Initialize the file descriptor set.
  // FD_ZERO initializes the file descriptor set set to be the empty set.
  FD_ZERO(&readfds);
  // FD_SET adds file descriptor to the file descriptor set.
  FD_SET(socket, &readfds);
  // FD_SET(0, &readfds) would listen on stdin

  int result = ::select(nfds, &readfds, NULL /*&writefds*/, NULL /*&exceptfds*/,
                        &timeout);

  if (result == 0) {
    // timeout occurred
    return result;
  } else if (result == SOCKET_ERROR) {
    // error occurred
    return result;
  }  // else select returned number of file descriptors

  // FD_ISSET returns a nonzero value (true) if filedes/socket is a member of
  // the file descriptor set, and zero (false) otherwise.
  if (!FD_ISSET(socket, &readfds)) {
    // The socket is not a member of the readfds set
    return SOCKET_ERROR;
  }  // else socket is ready for reading data

  // you can now call accept() on the socket.
  return result;
}

static SOCKET accept(SOCKET socket, sockaddr_in &peerAddress) {
  int addrlen = sizeof(peerAddress);

  // If address is not a null pointer, the address of the peer for the accepted
  // connection shall be stored in the sockaddr structure pointed to by address,
  // and the length of this address shall be stored in the object pointed to by
  // address_len.
  return ::accept(socket, (sockaddr *)&peerAddress, (socklen_t *)&addrlen);
}

/**
 * All processes (applications or DLLs) that call Winsock functions must
 * initialize the use of the Windows Sockets DLL before making other Winsock
 * functions calls. This also makes certain that Winsock is supported on the
 * system.
 *
 * The WSADATA structure contains information about the Windows Sockets
 * implementation.
 *
 * WSAStartup() returns zero if successful. Otherwise, it returns one of the
 * error codes.
 *
 * The method
 *   int WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);
 * is defined in header <winsock.h>
 */
bool Server::open(const std::string& password) {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  // check if server is already listening
  if (this->enabled) {
    std::cerr << "Server is already listening." << std::endl;
    return false;
  }

  // Initialize Winsock.
  if (!initializeWinsock()) {
    std::cout << "WSAStartup failed." << std::endl;
    return false;
  }

  // create a socket address
  sockaddr_in address;
  if (!createSocketAddress(this->port, this->interfaceAddress, address)) {
    std::cerr << "Failed to create socket address." << std::endl;
    return false;
  }

  this->listenSocket = INVALID_SOCKET;
  // Create a SOCKET for the server to listen for client
  this->listenSocket = createSocket(address);
  // Check for errors to ensure that the socket is a valid socket.
  if (this->listenSocket == INVALID_SOCKET) {
    std::cerr << "Create socket failed." << std::endl;
    return false;
  }

  // set socket to be nonblocking.
  if (!setSocketModeNonBlocking(this->listenSocket)) {
    std::cerr << "Set socket to be non-blocking failed." << std::endl;
    if (!this->closeSocket()) {
      std::cerr << "Failed to close socket." << std::endl;
      printError();
    }
    return false;
  }

  // bind the socket
  if (!bindSocket(this->listenSocket, address)) {
    std::cerr << "Bind failed." << std::endl;
    if (!this->closeSocket()) {
      std::cerr << "Failed to close socket." << std::endl;
      printError();
    }
    return false;
  }

  // start listening
  if (!listenOnSocket(this->listenSocket)) {
    std::cerr << "Listen failed." << std::endl;
    if (!this->closeSocket()) {
      std::cerr << "Failed to close socket." << std::endl;
      printError();
    }
    return false;
  }

  // Update status
  this->enabled = true;
  this->running = true;

  // start accept thread
  this->serverThread = std::thread(&Server::run, this);
  std::cout << "Server thread ID: " << this->serverThread.get_id() << std::endl;

  return true;
}

bool Server::isOpen() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  // return whether server has been stopped or is still running
  return this->enabled || this->running;
}

void Server::close() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  if (this->enabled) {
    // signal stop
    this->enabled = false;

    std::cout << "Join server thread" << std::endl;

    // wait until thread stops
    if (this->serverThread.joinable()) {
      this->serverThread.join();
    }

    // should already be closed by server thread
    if (!this->closeSocket()) {
      std::cerr << "Failed to close socket." << std::endl;
      printError();
    }

    this->running = false;
  }
}

void Server::run() {
  std::cout << "Listening on port " << this->port << std::endl;

  std::vector<std::shared_ptr<Worker>> workers = {};

  while (this->enabled) {
    // select returns 0 if timeout or -1 if error
    int rc = select(this->listenSocket);
    if (rc < 0) {
      // select failed
      std::cerr << "Select failed." << std::endl;
      break;
    } else if (rc == 0) {
      // timeout occurred
      continue;
    } else if (!this->enabled) {
      // server shall stop listening
      break;
    }  // else socket is ready to read from

    // listen socket must be non blocking even if we use select().
    // The pending connection might be closed before accept is called.

    // Accept a client socket
    sockaddr_in peerAddress;
    rc = accept(this->listenSocket, peerAddress);
    if (rc < 0) {
      // an error occurred or the connection has been closed before accept could
      // be executed
      std::cerr << "Failed to accept." << std::endl;
      // continue. The select block will handle the termination if the socket in
      // not valid .
      continue;
    } else if (!this->enabled) {
      // server shall stop listening
      break;
    }  // else rc is the socket;

    int clientSocket = rc;

    // set non blocking mode
    if (!setSocketModeNonBlocking(clientSocket)) {
      std::cerr << "Failed to set client socket in non blocking mode"
                << std::endl;
      continue;
    }

    // pass the accepted client socket to a worker thread
    std::shared_ptr<Worker> worker(new Worker(clientSocket));
    // put worker in list to be able to stop all workers
    workers.push_back(worker);
    // start worker to read
    worker->start();
  }

  // stop all workers
  for (std::shared_ptr<Worker> worker : workers) {
    if (worker) {
      worker->close();
    }
  }

  // close socket
  if (!this->closeSocket()) {
    std::cerr << "Failed to close socket." << std::endl;
    printError();
  }

  this->running = false;
  std::cout << "Stopped listening on port " << this->port << std::endl;
}


bool Server::setKeyFileName(const std::string& fileName) {
  if (this->isOpen()) {
    return false;
  }
  this->keyFileName = fileName;
  return true;
}

const std::string& Server::getKeyFileName() { return this->keyFileName; }

bool Server::setCertFileName(const std::string& fileName) {
  if (this->isOpen()) {
    return false;
  }
  this->certFileName = fileName;
  return true;
}

const std::string& Server::getCertFileName() { return this->certFileName; }

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
#endif
