#pragma once
#include <openssl/sha.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ggolbik {
namespace cpp {
namespace tls {

class Algorithm {
 public:  // base64
  static bool decodeBase64(const std::string& encodedBase64String,
                           std::string& decodedString);
  static bool decodeBase64(const std::string& encodedBase64String,
                           std::vector<unsigned char>& decoded);
  static bool decodeBase64Url(const std::string& encodedBase64UrlString,
                              std::string& decodedString);
  static bool decodeBase64Url(const std::string& encodedBase64UrlString,
                              std::vector<unsigned char>& decoded);

  static bool encodeBase64(const std::string& decodedString,
                           std::string& encodedBase64String);
  static bool encodeBase64(const std::string& decodedString,
                           std::vector<unsigned char>& encodedBase64);
  static bool encodeBase64Url(const std::string& decodedString,
                              std::string& encodedBase64UrlString);
  static bool encodeBase64Url(const std::string& decodedString,
                              std::vector<unsigned char>& encodedBase64Url);

  static bool calcSha256File(const std::string& filename,
                             std::vector<unsigned char>& hash);
  static bool calcSha256File(const std::string& filename,
                             std::string& hashString);
  static bool calcSha256String(const std::string& str,
                               std::vector<unsigned char>& hash);
  static bool calcSha256String(const std::string& str, std::string& hashString);

 private:
  // Files should be read in 4 KiB chunks.
  static constexpr int HashFileBufferSize = 4 * 1024;
  // Each character is used to represent 6 bits (log2(64) = 6).
  // Therefore 4 chars are used to represent 4 * 6 = 24 bits = 3 bytes
  // So you need 4*(n/3) chars to represent n bytes, and this needs to be
  // rounded up to a multiple of 4.
  static constexpr const std::size_t Base64Chars = 4;
  // The OpenSSL methods return 1 for error, otherwise 0.
  static constexpr const int Sha256Error = 0;

 private:
  /// <summary>Constructs an <see cref="Algorithm" /> instance.</summary>
  Algorithm() = default;
  /// <summary>Copy constructor.</summary>
  Algorithm(const Algorithm&) = default;
  /// <summary>Copy assignment operator</summary>
  Algorithm& operator=(const Algorithm&) = default;
  /// <summary>Destructs this instance and frees all resources.</summary>
  virtual ~Algorithm() = default;

 private:  // sha256
  bool initSha256();
  bool updateSha256(const void* data, std::size_t len);
  bool finalSha256(std::vector<unsigned char>& hash);

 private:
  ::SHA256_CTX sha256Context;
};

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
