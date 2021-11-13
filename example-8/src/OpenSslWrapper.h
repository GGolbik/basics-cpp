#ifdef __linux__
#pragma once

#include <openssl/ssl.h>

#include <cstdio>
#include <memory>
#include <string>

namespace ggolbik {
namespace cpp {
namespace tls {

struct TlsContextDeleterFunctor {
  void operator()(::SSL_CTX *p) const {
    if (p != nullptr) {
      ::SSL_CTX_free(p);
    }
  }
};

struct TlsDeleterFunctor {
  void operator()(::SSL *p) const {
    if (p != nullptr) {
      // It is not possible to call SSL_write() after calling SSL_shutdown().
      ::SSL_shutdown(p);
      ::SSL_free(p);
    }
  }
};

class OpenSslWrapper {
 public:
  /**
   * @brief Using custom deleter with unique_ptr
   */
  using TlsContextPtr = std::unique_ptr<::SSL_CTX, TlsContextDeleterFunctor>;
  /**
   * @brief Using custom deleter with unique_ptr
   */
  using TlsPtr = std::unique_ptr<::SSL, TlsDeleterFunctor>;

  /**
   * @brief Create a Tls Context object
   *
   * @return new SSL_CTX object or NULL if the creation of a new SSL_CTX object
   * failed.
   */
  static ::SSL_CTX *createTlsContextServer();

  static ::SSL_CTX *createTlsContextClient();

  /**
   * @brief Set the key and cert
   *
   * @param ctx
   * @param keyFileName
   * @param certFileName
   * @return
   */
  static bool configureTlsContext(::SSL_CTX *ctx,
                                  const std::string &keyFileName,
                                  const std::string &certFileName);

  static ::SSL *acceptTls(::SSL_CTX *ctx, int socket);

  static ::SSL *connectTls(::SSL_CTX *ctx, int socket);

  static void displayCerts(::SSL *ssl);
  static void displayCertsSimple(::SSL *ssl);

  static bool createSelfSignedCert(const std::string &keyFileName,
                                   const std::string &certFileName,
                                   const std::string &password);
};
}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik

#endif