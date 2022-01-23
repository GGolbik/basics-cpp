
#include "Algorithm.h"

#include <openssl/bio.h>
#include <openssl/evp.h>

#include <algorithm>  // std::replace
#include <fstream>
#include <functional>
#include <iomanip>  // std::setfill and std::setw
#include <iostream>
#include <limits>
#include <sstream>

namespace ggolbik {
namespace cpp {
namespace tls {

static bool fileExists(const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}

/// <summary>
/// Decodes a base64 string.
///
/// https://www.openssl.org/docs/man1.0.2/man3/BIO_new.html
/// https://www.openssl.org/docs/man1.0.2/man3/BIO_f_base64.html
/// https://www.openssl.org/docs/man1.0.2/man3/BIO_new_mem_buf.html
/// https://linux.die.net/man/3/bio_new_mem_buf
/// https://www.openssl.org/docs/man1.1.0/man3/BIO_push.html
/// https://www.openssl.org/docs/man1.0.2/man3/BIO_read.html
/// </summary>
/// <param name="encodedBase64String">The encoded base64 string</param>
/// <param name = "decodedString">The decoded string</param>
/// <returns>true if decode was successful, false otherwise</returns>
bool Algorithm::decodeBase64(const std::string& encodedBase64String,
                             std::vector<unsigned char>& decoded) {
  if ((encodedBase64String.size() < 1) ||
      ((encodedBase64String.size() % Algorithm::Base64Chars) != 0)) {
    std::cerr << "The length of the base64 string is invalid. The length must "
                 "be multiple of four but is "
              << encodedBase64String.size() << "." << std::endl;
    return false;
  }

  // A BIO is an I/O stream abstraction
  // The BIO_new() function returns a new BIO using method type.
  // BIO_f_base64() returns the base64 BIO method
  // This is a filter BIO that base64 encodes any data written through it and
  // decodes any data read through it. create a unquie ptr with custom deletion
  // to free up BIO.
  std::unique_ptr<::BIO, std::function<void(::BIO * p)>> b64(
      ::BIO_new(::BIO_f_base64()), [](::BIO* p)  //->void
      {
        // BIO_free_all() frees up an entire BIO chain.
        ::BIO_free_all(p);
      });

  if (!b64) {
    std::cerr << "Failed to create BIO object." << std::endl;
    return false;
  }

  // The flag BIO_FLAGS_BASE64_NO_NL can be set with BIO_set_flags() to encode
  // the data all on one line or expect the data to be all on one line.
  ::BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);

  // create read only buffer for the base64 string
  // BIO_new_mem_buf() creates a memory BIO using len bytes of data at buf if
  // len is -1 then the buf is assumed to be nul terminated and its length is
  // determined by strlen. The BIO is set to a read only state and as a result
  // cannot be written to. This is useful when some data needs to be made
  // available from a static area of memory in the form of a BIO. The supplied
  // data is read directly from the supplied buffer : it is not copied first, so
  // the supplied area of memory must be unchanged until the BIO is freed.
  // Create a read only memory BIO. it's fine that encodedBase64String is const.
  BIO* source = ::BIO_new_mem_buf(
      static_cast<const void*>(encodedBase64String.c_str()), -1);

  if (source == nullptr) {
    std::cerr << "Failed to create BIO memory buffer for data." << std::endl;
    return false;
  }

  // The names of these functions are perhaps a little misleading.BIO_push()
  // joins two BIO chains. Therefore BIO_free must not be called on source
  // because the b64 will free the bio chain.
  ::BIO_push(b64.get(), source);

  // create buffer to store decoded data
  const size_t maxlen = ((encodedBase64String.size() / 4) * 3) + 1;
  decoded = std::vector<unsigned char>(maxlen);
  if (maxlen > static_cast<size_t>(std::numeric_limits<int>::max())) {
    std::cerr << "base64 string length exceeds supported limit." << std::endl;
    return false;
  }

  // BIO_read() attempts to read len bytes from BIO b and places the data in
  // buf.
  const int len =
      ::BIO_read(b64.get(), decoded.data(), static_cast<int>(maxlen));
  if (len > 0) {
    decoded.resize(static_cast<size_t>(len));
    return true;
  } else {
    // if len 0 or -1 : no data was successfully read or written
    // if len is -2 : the operation is not implemented in the specific BIO type.
    std::cerr << "The base64 string could not be decoded. Error: " << len
              << std::endl;
    return false;
  }
}

bool Algorithm::decodeBase64(const std::string& encodedBase64String,
                             std::string& decodedString) {
  std::vector<unsigned char> decoded;
  if (Algorithm::decodeBase64(encodedBase64String, decoded)) {
    decodedString = std::string(std::string(decoded.begin(), decoded.end()));
    return true;
  }
  return false;
}

bool Algorithm::decodeBase64Url(const std::string& encodedBase64UrlString,
                                std::vector<unsigned char>& decoded) {
  // Convert base64url to base64
  // "base64url" differs from the standard Base64 encoding in two aspects :
  // 1. different characters are used for index 62 and 63 ( - and _ instead of +
  // and / )
  // 2. no mandatory padding with = characters to make the string length a
  // multiple of four.

  // base64_url_alphabet:
  //'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  //'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  //'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  //'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  //'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'

  // base64_alphabet:
  //'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
  //'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  //'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  //'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  //'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'

  std::string encodedBase64String = encodedBase64UrlString;
  // replace all '-' with '+'
  std::replace(encodedBase64String.begin(), encodedBase64String.end(), '-',
               '+');
  // replace all '_' with '/'
  std::replace(encodedBase64String.begin(), encodedBase64String.end(), '_',
               '/');
  // add padding
  while ((encodedBase64String.size() % Algorithm::Base64Chars) != 0) {
    encodedBase64String += "=";
  }
  // decode base64
  return Algorithm::decodeBase64(encodedBase64String, decoded);
}

bool Algorithm::decodeBase64Url(const std::string& encodedBase64UrlString,
                                std::string& decodedString) {
  std::vector<unsigned char> decoded;
  bool result = Algorithm::decodeBase64Url(encodedBase64UrlString, decoded);
  if (result) {
    decodedString = std::string(std::string(decoded.begin(), decoded.end()));
  }
  return result;
}

bool Algorithm::initSha256() {
  // create sha256 context. SHA256_Init returns 1 for success, 0 otherwise.
  if (::SHA256_Init(&this->sha256Context) == Algorithm::Sha256Error) {
    // failed to create context
    std::cerr << "Failed to create context for hash." << std::endl;
    return false;
  }
  return true;
}

bool Algorithm::updateSha256(const void* data, size_t len) {
  // append data to the current context. SHA256_Update returns 1 for success, 0
  // otherwise.
  if (::SHA256_Update(&this->sha256Context, data, len) ==
      Algorithm::Sha256Error) {
    // failed to append data
    std::cerr << "Failed to update hash." << std::endl;
    return false;
  }
  return true;
}

bool Algorithm::finalSha256(std::vector<unsigned char>& hash) {
  hash.resize(SHA256_DIGEST_LENGTH, 0);
  // pass final hash in array. SHA256_Final returns 1 for success, 0 otherwise.
  if (::SHA256_Final(hash.data(), &this->sha256Context) ==
      Algorithm::Sha256Error) {
    // failed to calc hash
    std::cerr << "Failed to set hash." << std::endl;
    return false;
  }
  return true;
}

bool Algorithm::calcSha256File(const std::string& filename,
                               std::vector<unsigned char>& hash) {
  Algorithm algorithm = {};
  if (!algorithm.initSha256()) {
    // Failed to init sha256 class
    return false;
  }
  // else continue

  // check if file exists
  if (fileExists(filename)) {
    char buffer[Algorithm::HashFileBufferSize];
    std::ifstream infile(filename.c_str(), std::ifstream::binary);
    // read from file and pass data to calc hash
    while (infile.good() && !infile.eof()) {
      infile.read(buffer, Algorithm::HashFileBufferSize);
      // gcount returns the number of characters successfully read and stored by
      // read function.
      if (!algorithm.updateSha256(static_cast<const void*>(buffer),
                                  static_cast<std::size_t>(infile.gcount()))) {
        return false;
      }
    }

    // check for bad and not fail. failbit will be set if EOF has been reached,
    // too.
    if (infile.bad()) {
      std::cerr << "Failed to read complete file to calculate hash."
                << std::endl;
      return false;
    }
    // else everything is okay

    // pass final hash in array
    if (!algorithm.finalSha256(hash)) {
      return false;
    } else {
      // else calculation of hash was successful
      return true;
    }
  } else {
    std::cerr << "Hash can not be calculated of non existing file."
              << std::endl;
  }
  return false;
}

bool Algorithm::calcSha256File(const std::string& filename,
                               std::string& hashString) {
  std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH, 0);
  if (Algorithm::calcSha256File(filename, hash)) {
    // SHA-256 produces a 256-bit (32 bytes) hash value. It's usually
    // represented as a hexadecimal number of 64 digits.
    std::stringstream hashStream;
    hashStream << std::hex << std::setfill('0');
    for (size_t i = 0; i < hash.size(); ++i) {
      hashStream << std::setw(2) << (int)hash[i];
    }
    hashString = std::string(hashStream.str());
    return true;
  }
  return false;
}

bool Algorithm::calcSha256String(const std::string& str,
                                 std::vector<unsigned char>& hash) {
  Algorithm algorithm = {};
  if (!algorithm.initSha256()) {
    return false;
  }
  if (!algorithm.updateSha256(str.c_str(), str.size())) {
    return false;
  }
  if (!algorithm.finalSha256(hash)) {
    std::cerr << "Failed to create hash." << std::endl;
    return false;
  }
  return true;
}

bool Algorithm::calcSha256String(const std::string& str,
                                 std::string& hashString) {
  std::vector<unsigned char> hash(SHA256_DIGEST_LENGTH, 0);
  if (Algorithm::calcSha256String(str, hash)) {
    // SHA-256 produces a 256-bit (32 bytes) hash value. It's usually
    // represented as a hexadecimal number of 64 digits.
    std::stringstream hashStream;
    hashStream << std::hex << std::setfill('0');
    for (size_t i = 0; i < hash.size(); ++i) {
      hashStream << std::setw(2) << (int)hash[i];
    }
    hashString = std::string(hashStream.str());
    return true;
  }
  return false;
}

bool Algorithm::encodeBase64(const std::string& decodedString,
                             std::string& encodedBase64String) {
  if (decodedString.size() < 1) {
    encodedBase64String = "";
    return true;
  }

  // A BIO is an I/O stream abstraction
  // The BIO_new() function returns a new BIO using method type.
  // BIO_f_base64() returns the base64 BIO method
  // This is a filter BIO that base64 encodes any data written through it and
  // decodes any data read through it. create a unquie ptr with custom deletion
  // to free up BIO.
  std::unique_ptr<::BIO, std::function<void(::BIO * p)>> b64(
      ::BIO_new(::BIO_f_base64()), [](::BIO* p) -> void {
        // BIO_free_all() frees up an entire BIO chain.
        ::BIO_free_all(p);
      });

  if (!b64) {
    std::cerr << "Failed to create BIO object." << std::endl;
    return false;
  }

  // The flag BIO_FLAGS_BASE64_NO_NL can be set with BIO_set_flags() to encode
  // the data all on one line or expect the data to be all on one line.
  ::BIO_set_flags(b64.get(), BIO_FLAGS_BASE64_NO_NL);

  // create BIO that holds the result
  ::BIO* sink = ::BIO_new(
      ::BIO_s_mem());  // BIO_new_mem_buf(encodedBase64String.CStr(), -1);

  if (!sink) {
    std::cerr << "Failed to create BIO memory buffer for data." << std::endl;
    return false;
  }

  // The names of these functions are perhaps a little misleading.BIO_push()
  // joins two BIO chains. Therefore BIO_free must not be called on source
  // because the b64 will free the bio chain. The BIO *BIO_push(BIO *b, BIO
  // *append); function appends the BIO append to b, it returns b.
  ::BIO_push(b64.get(), sink);

  int written = ::BIO_write(b64.get(), decodedString.c_str(),
                            static_cast<int>(decodedString.size()));
  if (written <= 0) {
    std::cerr << "Failed to write data to BIO memory." << std::endl;
    return false;
  }
  // BIO_flush() returns 1 for success and 0 or -1 for failure.
  if (BIO_flush(b64.get()) != 1) {
    std::cerr << "Failed to flush data to BIO memory." << std::endl;
    return false;
  }

  const char* encoded;
  const long len = BIO_get_mem_data(sink, &encoded);
  if (len < 1) {
    std::cerr << "Failed to get encoded data from BIO memory." << std::endl;
    return false;
  } else {
    encodedBase64String = std::string(std::string(encoded, len));
    return true;
  }
}

bool Algorithm::encodeBase64(const std::string& decodedString,
                             std::vector<unsigned char>& encodedBase64) {
  std::string encodedBase64String;
  if (Algorithm::encodeBase64(decodedString, encodedBase64String)) {
    encodedBase64 = std::vector<unsigned char>(encodedBase64String.begin(),
                                               encodedBase64String.end());
    return true;
  }
  return false;
}

bool Algorithm::encodeBase64Url(const std::string& decodedString,
                                std::string& encodedBase64UrlString) {
  if (Algorithm::encodeBase64(decodedString, encodedBase64UrlString)) {
    // Convert base64 to base64url
    // "base64url" differs from the standard Base64 encoding in two aspects :
    // 1. different characters are used for index 62 and 63 ( - and _ instead of
    // + and / )
    // 2. no mandatory padding with = characters to make the string length a
    // multiple of four.

    // const char base64_url_alphabet[] = {
    //'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    //'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    //'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    //'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    //'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'
    //};

    // const char base64_alphabet[] = {
    //'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    //'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    //'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    //'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    //'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    //};

    // replace all '+' with '-'
    std::replace(encodedBase64UrlString.begin(), encodedBase64UrlString.end(),
                 '+', '-');
    // replace all '/' with '_'
    std::replace(encodedBase64UrlString.begin(), encodedBase64UrlString.end(),
                 '/', '_');
    // remove padding
    while (encodedBase64UrlString.back() == '=') {
      encodedBase64UrlString.pop_back();
    }
    return true;
  }
  return false;
}

bool Algorithm::encodeBase64Url(const std::string& decodedString,
                                std::vector<unsigned char>& encodedBase64Url) {
  std::string encodedBase64UrlString;
  if (Algorithm::encodeBase64(decodedString, encodedBase64UrlString)) {
    encodedBase64Url = std::vector<unsigned char>(
        encodedBase64UrlString.begin(), encodedBase64UrlString.end());
    return true;
  }
  return false;
}

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
