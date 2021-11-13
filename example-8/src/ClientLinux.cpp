#ifdef __linux__
// linux code goes here
// Implementation of Berkeley sockets

#include <arpa/inet.h>   // ::htons(...) ; ::inet_addr(...)
#include <fcntl.h>       // ::fcntl(...)
#include <netdb.h>       // ::gethostbyname(...) ; hostent
#include <netinet/in.h>  // sockaddr_in
#include <openssl/ssl.h>
#include <sys/socket.h>  // ::socket(...) ; SOCK_STREAM ; AF_INET ; connect(...)
#include <unistd.h>      // ::close(int), ::read(int, void*, size_t), write(...)

#include <cerrno>    // errno
#include <cstring>   // strerror_r(...) , std::memcpy(...)
#include <iostream>  // std::cout(...), std::cerr(...)

#include "Client.h"

namespace ggolbik {
namespace cpp {
namespace tls {

Client::Client(std::string serverAddress, unsigned short port)
    : enabled{false}, serverAddress{serverAddress}, port{port} {}

Client::~Client() { this->close(); }

/**
 * errno is thread safe. On Linux, the global errno variable is thread-specific.
 * POSIX requires that errno be threadsafe. If the value of errno should be
 * preserved across a library call, it must be saved.
 *
 * strerror_r is thread safe.
 *
 * errno is defined in <cerrno> and is an integer value.
 *
 * The method call
 *   char *::strerror_r(int errnum, char *buf, size_t buflen);
 * is defined in <cstring>
 */
static void printError() {
  size_t length = 1024;
  char buffer[length];
  std::cerr << "(" << errno << ") " << ::strerror_r(errno, buffer, length)
            << std::endl;
}

/**
 * Creates the IP socket address.
 *
 * struct hostent *gethostbyname(const char *name); // Returns a pointer to a
 resultant struct hostent or success, or NULL on error. On error, the h_errno
 variable holds an error number.
 *
 *
 * ::inet_addr() interprets a string that represents numbers in the Internet
 standard dotted decimal notation and returns a corresponding Internet address.
 * If ::inet_addr() is successful, it returns the address in network byte order.
 Otherwise, it returns INADDR_NONE ( 0xFFFFFFFF ), and sets errno to indicate
 the type of error.
 *
 * ::htons() converts the unsigned short integer hostshort from host byte order
 to network byte order.
 *
 * ::htonl() converts the unsigned integer hostlong from host byte order to
 network byte order. Required to convert the macros INADDR_ANY, INADDR_LOOPBACK,
 INADDR_ANY.
 *
 * ::gethostbyname(): If name is an IPv4 address, no lookup is performed and
 gethostbyname() simply copies name into the h_name field.
 * Returns the hostent structure or a null pointer if an error occurs.
 *
 * :: bcopy() copies n bytes from src to dest.  The result is correct, even when
 both areas overlap.
 *
 * The method call
 *   unsigned long inet_addr(const char *cp);
 * as well as the method call
 *   uint16_t htons(uint16_t hostshort);
 *   uint32_t htonl(uint32_t hostlong);
 * are defined in header <arpa/inet.h>
 *
 * The method
 *   struct hostent *gethostbyname(const char *name);
 * is defined in header <netdb.h>
 *
 * The <sys/socket.h> header defines the following macros, with distinct integer
 values:
 * - AF_INET
 * - AF_INET6
 *
 * The <netinet/in.h> header defines the following macros:
 * - INADDR_ANY (0.0.0.0)
 * - INADDR_LOOPBACK (127.0.0.1)
 * - INADDR_BROADCAST (255.255.255.255)
 * - INADDR_NONE
 * as well as the sockaddr_in type:
 *   struct sockaddr_in {
 *     sa_family_t    sin_family; // address family: AF_INET
 *     in_port_t      sin_port;   // port in network byte order
 *     struct in_addr sin_addr;   // internet address
 *   };
 *
 * The method
 *   void bcopy(const void *src, void *dest, size_t n);
 * is defined in header <strings.h>
 *
 * Notice: errno is thread safe. On Linux, the global errno variable is
 thread-specific. POSIX requires that errno be threadsafe.
 *
 * @param port the port number
 * @param serverAddress the IP address of the interface.
 * @param socketAddress the socket address object.
 * @return true if socket address has been created. false if the method failed
 to create the socket address. On error you should check the errno value.
 */
static bool createSocketAddress(const std::string &serverAddress,
                                unsigned short port,
                                sockaddr_in &socketAddress) {
  // define the IPv4 address family.
  socketAddress.sin_family = AF_INET;

  // set port
  socketAddress.sin_port = ::htons(port);

  // check interface address. use any address if not specified.
  if (serverAddress.empty()) {
    // host must be specified
    return false;
  } else {
    // convert and set IP address
    hostent *server = ::gethostbyname(serverAddress.c_str());
    if (server == nullptr) {
      // see h_errno;
      return false;
    }

    // h_addr: This is a synonym for h_addr_list[0]; in other words, it is the
    // first host address. h_length: This is the length, in bytes, of each
    // address.
    ::memcpy(&socketAddress.sin_addr.s_addr, server->h_addr, server->h_length);
  }

  // check if address is valid
  if (socketAddress.sin_addr.s_addr == ::htonl(INADDR_NONE)) {
    // failed to set address
    return false;
  }  // else address is defined

  return true;
}

/**
 * Creates a TCP socket for the given address.
 *
 * A socket is a generalized interprocess communication channel.
 * Like a pipe, a socket is represented as a file descriptor.
 * Unlike pipes sockets support communication between unrelated processes, and
 * even between processes running on different machines that communicate over a
 * network. Sockets are the primary means of communicating with other machines;
 * telnet, rlogin, ftp, talk and the other familiar network programs use
 * sockets.
 *
 * When you create a socket, you must specify the style of communication you
 * want to use and the type of protocol that should implement it. The
 * communication style of a socket defines the user-level semantics of sending
 * and receiving data on the socket.
 *
 * There are two types of Internet Sockets
 *   SOCK_STREAM : Stream Sockets - Stream sockets are reliable two-way
 * connected communication streams. SOCK_DGRAM : Datagram Sockets - Datagram
 * sockets are sometimes called "connectionless sockets" There are a few other
 * options like SOCK_SEQPACKET : Provides a sequenced, reliable, two-way
 * connection-based data transmission path for datagrams of fixed maximum
 * length; a consumer is required to read an entire packet with each input
 * system call. SOCK_RAW : Provides raw network protocol access. SOCK_RDM :
 * Provides a reliable datagram layer that does not guarantee ordering.
 *
 * You must also choose a namespace for naming the socket.
 * a namespace is sometimes called a protocol family.
 * - AF_INET
 *
 * Finally you must choose the protocol to carry out the communication.
 * The protocol determines what low-level mechanism is used to transmit and
 * receive data. Each protocol is valid for a particular namespace and
 * communication style.
 *
 * For each combination of style and namespace there is a default protocol,
 * which you can request by specifying 0 as the protocol number. So you would
 * see sometime
 *   ::socket(socketAddress.sin_family, SOCK_STREAM, 0);
 * to create a TCP socket.
 *
 * The method call
 *   int socket(int domain, int type, int protocol);
 * is defined in header <sys/socket.h>
 *
 * In <netinet/in.h> you find the sockaddr_in type:
 *   struct sockaddr_in {
 *     sa_family_t    sin_family; // address family: AF_INET
 *     in_port_t      sin_port;   // port in network byte order
 *     struct in_addr sin_addr;   // internet address
 *   };
 *
 * The <netinet/in.h> header defines the following macros:
 * - IPPROTO_TCP
 * The <sys/socket.h> header defines the following macros, with distinct integer
 * values:
 * - SOCK_STREAM
 * - AF_INET
 *
 * @param socketAddress The address with the specified address family /
 * namespace.
 * @return On success, a file descriptor for the new socket is returned. On
 * error, -1 is returned, and errno is set appropriately.
 */
static int createSocket(sockaddr_in &socketAddress) {
  // SOCK_STREAM is used to specify a stream socket.
  // IPPROTO_TCP is used to specify the TCP protocol.
  return ::socket(socketAddress.sin_family, SOCK_STREAM, IPPROTO_TCP);
}

/**
 * The connect() system call connects the socket referred to by the file
 * descriptor sockfd to the address specified by addr. The addrlen argument
 * specifies the size of addr. The format of the address in addr is determined
 * by the address space of the socket sockfd; If the socket sockfd is of type
 * SOCK_DGRAM, then addr is the address to which datagrams are sent by default,
 * and the only address from which datagrams are received.
 * If the socket is of type SOCK_STREAM or SOCK_SEQPACKET, this call attempts to
 * make a connection to the socket that is bound to the address specified by
 * addr.
 *
 * The method call
 *    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
 * is defined in header <sys/socket.h>
 *
 * @param socket
 * @param socketAddress
 * @return true
 * @return false
 */
static bool connect(int socket, sockaddr_in &socketAddress) {
  // If the connection or binding succeeds, zero is returned. On error, -1 is
  // returned, and errno is set appropriately.
  return ::connect(socket, reinterpret_cast<sockaddr *>(&socketAddress),
                   sizeof(sockaddr_in)) == 0;
}

bool Client::isOpen() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  return this->enabled;
}

/**
 * Sets the socket / file descriptor in non-blocking mode.
 *
 * By default, TCP sockets are in "blocking" mode. For example, when you call
 * ::read() to read from a stream, control isn't returned to your program until
 * at least one byte of data is read from the remote site. This process of
 * waiting for data to appear is referred to as "blocking". The same is true for
 * the write() API, the connect() API, etc. When you run them, the connection
 * "blocks" until the operation is complete.
 *
 * Its possible to set a descriptor so that it is placed in "non-blocking" mode.
 * When placed in non-blocking mode, you never wait for an operation to
 * complete. This is an invaluable tool if you need to switch between many
 * different connected sockets, and want to ensure that none of them cause the
 * program to "lock up."
 *
 * If you call ::read() in non-blocking mode, it will return any data that the
 * system has in it's read buffer for that socket. But, it won't wait for that
 * data. If the read buffer is empty, the system will return from ::read()
 * immediately saying "Operation Would Block!". To be more precise ::read()
 * returns -1 to indicate that an error occurred. And errno is set to EAGAIN or
 * EWOULDBLOCK. errno as well as the macros EAGAIN and EWOULDBLOCK are defined
 * in header <cerrno>
 *
 * The "non-blocking" mode is set by changing one of the socket's "flags". The
 * flags are a series of bits, each one representing a different capability of
 * the socket. So, to turn on non-blocking mode requires three steps:
 * 1. Call the ::fcntl() API to retrieve the socket descriptor's current flag
 * settings into a local variable.
 * 2. In our local variable, set the O_NONBLOCK (non-blocking) flag on. (being
 * careful, of course, not to tamper with the other flags)
 * 3. Call the ::fcntl() API to set the flags for the descriptor to the value in
 * our local variable.
 *
 * The method call
 *   int ::fcntl(int fd, int cmd, ...arg);
 * and the macro O_NONBLOCK are defined in header <fcntl.h>
 * On error, ::fctnl() returns -1 and errno is set appropriately.
 *
 * Notice: errno is thread safe. On Linux, the global errno variable is
 * thread-specific. POSIX requires that errno be threadsafe.
 *
 * @param socket the file descriptor to set in non-blocking mode.
 * @return true if mode could be set. false if the method failed to set the
 * mode. On error you should check the errno value.
 */
static bool setSocketModeNonBlocking(int socket) {
  // get the current flags
  int flags = ::fcntl(socket, F_GETFL, 0);
  if (flags == -1) {
    // failed to get flags
    return false;
  }  // else get flags was successful.

  // set flag for non-blocking mode
  flags |= O_NONBLOCK;
  if (::fcntl(socket, F_SETFL, flags) == -1) {
    // failed to set flags
    return false;
  }  // else socket / file descriptor has been set in non-blocking mode.

  return true;
}

bool Client::open() {
  // lock mutex
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  // check if client is already running
  if (this->enabled) {
    std::cerr << "Client is already running." << std::endl;
    return false;
  }

  this->enabled = true;

  // create TLS context
  this->tlsContextPtr =
      OpenSslWrapper::TlsContextPtr(OpenSslWrapper::createTlsContextClient());
  if (!tlsContextPtr) {
    std::cerr << "Failed to create TLS context." << std::endl;
    this->enabled = false;
    return false;
  }

  // create a socket address
  sockaddr_in address;
  if (!createSocketAddress(this->serverAddress, this->port, address)) {
    std::cerr << "Failed to create socket address." << std::endl;
    printError();
    this->enabled = false;
    return false;
  }

  // Create a SOCKET for the client to listen
  this->clientSocket = createSocket(address);
  // Check for errors to ensure that the socket is a valid socket.
  if (this->clientSocket < 0) {
    std::cerr << "Create socket failed." << std::endl;
    printError();
    this->closeSocket();
    this->enabled = false;
    return false;
  }

  // connect to host
  // Check for errors to ensure that the socket is a valid socket.
  if (!connect(this->clientSocket, address)) {
    std::cerr << "Connect socket failed." << std::endl;
    printError();
    this->closeSocket();
    this->enabled = false;
    return false;
  }

  // set socket to be nonblocking.
  if (!setSocketModeNonBlocking(this->clientSocket)) {
    std::cerr << "Set socket to be non-blocking failed." << std::endl;
    printError();
    this->closeSocket();
    this->enabled = false;
    return false;
  }

  // check TLS
  this->tlsPtr = OpenSslWrapper::TlsPtr(
      OpenSslWrapper::connectTls(this->tlsContextPtr.get(), clientSocket));
  if (!this->tlsPtr) {
    std::cerr << "Failed to establish TLS connection." << std::endl;
    printError();
    this->closeSocket();
    this->enabled = false;
    return false;
  }

  // connection established
  OpenSslWrapper::displayCerts(this->tlsPtr.get());

  return true;
}

void Client::close() {
  // lock mutex to set listen socket appropriately
  std::unique_lock<std::mutex> lock(this->mutexPublicMethods);

  this->enabled = false;

  if (!this->closeSocket()) {
    std::cerr << "Close socket failed." << std::endl;
    printError();
  }
}

bool Client::closeSocket() {
  // check if socket is valid
  if (this->clientSocket < 0) {
    // invalid socket
    // socket seems to be already closed.
    return true;
  }

  // close socket
  if (::close(this->clientSocket) != 0) {
    return false;
  }

  // set socket to an invalid value.
  this->clientSocket = -1;

  return true;
}

bool Client::write(const byte data[], size_t length) {
  if (this->tlsPtr) {
    return this->writeTls(data, length);
  }

  if (length == 0) {
    return true;
  }

  size_t position = 0;
  int remaining = length;

  ssize_t rc;
  try {
    while (this->enabled && remaining > 0) {
      rc = ::write(this->clientSocket, data + (sizeof(byte) * position),
                   remaining);

      if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        std::cout << "Worker read AGAIN" << std::endl;
        // The file descriptor fd refers to a file other than a socket and has
        // been marked nonblocking (O_NONBLOCK), and the read would block.
        if (this->enabled) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        continue;
      } else if (rc < 0) {
        // an error occurred
        printError();
        std::cerr << "Failed to write" << std::endl;
        break;
      } else if (rc == 0) {
        // reached end of file
        break;
      }

      remaining -= rc;
      position += rc;
    }

    if (remaining == 0) {
      return true;
    }
  } catch (...) {
  }

  return false;
}

/**
 * ssize_t write(int fd, const void *buf, size_t count);
 */
bool Client::writeTls(const byte data[], size_t length) {
  if (length == 0) {
    return true;
  }

  size_t position = 0;
  int remaining = length;

  int rc;
  try {
    while (this->enabled && remaining > 0) {
      rc = ::SSL_write(this->tlsPtr.get(), data + (sizeof(byte) * position),
                       remaining);
      if (rc <= 0) {
        // failed to read data
        int error = ::SSL_get_error(this->tlsPtr.get(), rc);
        if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_WRITE) {
          if (this->enabled) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
          }
          continue;
        } else {
          // an error occurred
          printError();
          std::cerr << "Failed to write" << std::endl;
          break;
        }
      }

      remaining -= rc;
      position += rc;
    }

    if (remaining == 0) {
      return true;
    }
  } catch (...) {
  }

  return false;
}

bool Client::readString(std::string &message) {
  if (this->tlsPtr) {
    return this->readStringTls(message);
  }
  int result = -1;
  do {
    result = this->tryReadString(message);
    if (result > 0) {
      return true;
    } else if (result == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  } while (this->enabled && result == 0);
  return false;
}

int Client::tryReadString(std::string &message) {
  if (this->tlsPtr) {
    return this->tryReadStringTls(message);
  }
  byte recvbuf[Client::MAX_BUFFER_SIZE];

  message = "";

  ssize_t rc;
  while (this->enabled) {
    rc = ::read(this->clientSocket, recvbuf, Client::MAX_BUFFER_SIZE);
    if (rc < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      return 0;
    } else if (rc < 0) {
      // an error occurred
      printError();
      std::cerr << "Failed to read" << std::endl;
      return -1;
    } else if (rc == 0) {
      // reached end of file
      return 1;
    } else {
      // data has been read
      message = std::string(recvbuf, static_cast<size_t>(rc));
      return rc;
    }
  }

  return false;
}

bool Client::readStringTls(std::string &message) {
  int result = -1;
  do {
    result = this->tryReadStringTls(message);
    if (result > 0) {
      return true;
    } else if (result == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  } while (this->enabled && result == 0);
  return false;
}

int Client::tryReadStringTls(std::string &message) {
  byte recvbuf[Client::MAX_BUFFER_SIZE];

  message = "";

  int rc;
  if (this->enabled) {
    rc = ::SSL_read(this->tlsPtr.get(), recvbuf, Client::MAX_BUFFER_SIZE);
    if (rc > 0) {
      // data has been read
      message = std::string(recvbuf, static_cast<size_t>(rc));
      return rc;
    }
    int error = ::SSL_get_error(this->tlsPtr.get(), rc);
    switch (error) {
      case SSL_ERROR_WANT_READ:
      case SSL_ERROR_WANT_WRITE:
        return 0;
      default:
        break;
    }
  }
  return -1;
}

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
#endif