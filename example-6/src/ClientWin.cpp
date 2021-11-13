#ifdef _WIN32
// _WIN32 marco is defined for both 32-bit and 64-bit environments
// windows code goes here

#include "Client.h"
#include <iostream>

namespace ggolbik
{
namespace cplusplus
{
namespace socket
{

Client::Client(std::string serverAddress, unsigned short port) : serverAddress{serverAddress}, port{port} {}

Client::~Client()
{
  this->close();
}

bool Client::open()
{
  std::cerr << "NOT IMPLEMENTED" << std::endl;
  return false;
}

void Client::close()
{
}

bool Client::closeSocket()
{
  return false;
}

/**
 * ssize_t write(int fd, const void *buf, size_t count);
 */
bool Client::write(const byte data[], size_t length)
{
  return false;
}

bool Client::readString(std::string &message)
{
  return false;
}

bool Client::tryReadString(std::string &message)
{
  return false;
}

} // namespace socket
} // namespace cplusplus
} // namespace ggolbik
#endif