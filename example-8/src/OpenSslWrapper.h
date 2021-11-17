#pragma once

#include <openssl/ssl.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <functional>

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

struct TlsX509CertDeleterFunctor
{
  void operator()(::X509* p) const
  {
    if(p != nullptr)
    {
      ::X509_free(p);
    }
  }
};

struct TlsKeyDeleterFunctor
{
  void operator()(::EVP_PKEY* p) const
  {
    if(p != nullptr)
    {
      ::EVP_PKEY_free(p);
    }
  }
};

struct TlsMessageDigestContextDeleterFunctor
{
  void operator()(::EVP_MD_CTX* p) const
  {
    if(p != nullptr)
    {
      ::EVP_MD_CTX_free(p);
    }
  }
};

class OpenSslWrapper {
 public:
  using TlsX509Cert = std::unique_ptr<::X509, TlsX509CertDeleterFunctor>;
  using TlsKey = std::unique_ptr<::EVP_PKEY, TlsKeyDeleterFunctor>;
  using TlsMessageDigestContext = std::unique_ptr<::EVP_MD_CTX, TlsMessageDigestContextDeleterFunctor>;

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
  static void displayCert(const TlsX509Cert& cert);
  static void displayCertsSimple(::SSL *ssl);

  static bool createSelfSignedCert(const std::string &keyFileName,
                                   const std::string &certFileName,
                                   const std::string &password);

  static bool readKeyFile(const std::string& fileName, TlsKey& key, const std::string& password);

  static bool readCertFile(const std::string& fileName, TlsX509Cert& cert, const std::string& password);

  static bool readCertKey(const std::string& fileName, TlsKey& key);
  static bool readCertKey(const TlsX509Cert& cert, TlsKey& key);

  static bool signData(TlsKey& key, const std::string& msg, std::string& signature);

  static bool verifySignedData(TlsKey& key, const std::string& msg, const std::string& signature, const std::string& algorithm);
};
}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
