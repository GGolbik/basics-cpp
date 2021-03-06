#include "OpenSslWrapper.h"

#include <openssl/err.h>

#include <cstdio>  // fopen fclose
#include <functional>
#include <iostream>
#include <thread>  // std::this_thread
#include <vector>
namespace ggolbik {
namespace cpp {
namespace tls {

/**
 * @brief Create a Tls Context object
 *
 * @return new SSL_CTX object or NULL if the creation of a new SSL_CTX object
 * failed.
 */
::SSL_CTX *OpenSslWrapper::createTlsContextServer() {
  // The SSL_CTX object uses method as the connection method.
  // Three method variants are available:
  // - a generic method (for either client or server use),
  // - a server-only method,
  // - and a client-only method.
  const ::SSL_METHOD *method = TLS_server_method();

  // SSL_CTX_new() creates a new SSL_CTX object, which holds various
  // configuration and data relevant to SSL/TLS or DTLS session establishment.
  // These are later inherited by the SSL object representing an active session.
  // The method parameter specifies whether the context will be used for the
  // client or server side or both.
  SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    ::perror("Unable to create SSL context.");
    // prints the error strings for all errors that OpenSSL has recorded to
    // file, thus emptying the error queue.
    ::ERR_print_errors_fp(stderr);
  }

  return ctx;
}

::SSL_CTX *OpenSslWrapper::createTlsContextClient() {
  // The SSL_CTX object uses method as the connection method.
  // Three method variants are available:
  // - a generic method (for either client or server use),
  // - a server-only method,
  // - and a client-only method.
  const ::SSL_METHOD *method = TLS_client_method();

  // SSL_CTX_new() creates a new SSL_CTX object, which holds various
  // configuration and data relevant to SSL/TLS or DTLS session establishment.
  // These are later inherited by the SSL object representing an active session.
  // The method parameter specifies whether the context will be used for the
  // client or server side or both.
  ::SSL_CTX *ctx = SSL_CTX_new(method);
  if (!ctx) {
    ::perror("Unable to create SSL context.");
    // prints the error strings for all errors that OpenSSL has recorded to
    // file, thus emptying the error queue.
    ::ERR_print_errors_fp(stderr);
  }

  return ctx;
}

/**
 * @brief Set the key and cert
 *
 * @param ctx
 * @param keyFileName
 * @param certFileName
 * @return
 */
bool OpenSslWrapper::configureTlsContext(::SSL_CTX *ctx,
                                         const std::string &keyFileName,
                                         const std::string &certFileName) {
  if (ctx == nullptr) {
    return false;
  }

  // set key
  if (::SSL_CTX_use_PrivateKey_file(ctx, keyFileName.c_str(),
                                    SSL_FILETYPE_PEM) <= 0) {
    ::perror("Unable to set private key.");
    // prints the error strings for all errors that OpenSSL has recorded to
    // file, thus emptying the error queue.
    ::ERR_print_errors_fp(stderr);
    return false;
  }

  // set cert
  if (::SSL_CTX_use_certificate_file(ctx, certFileName.c_str(),
                                     SSL_FILETYPE_PEM) <= 0) {
    ::perror("Unable to set certifacte.");
    // prints the error strings for all errors that OpenSSL has recorded to
    // file, thus emptying the error queue.
    ::ERR_print_errors_fp(stderr);
    return false;
  }

  return true;
}

::SSL *OpenSslWrapper::acceptTls(::SSL_CTX *ctx, int socket) {
  ::SSL *ssl = ::SSL_new(ctx);
  // SSL_set_fd() sets the file descriptor fd as the input/output facility for
  // the TLS/SSL (encrypted) side of ssl. fd will typically be the socket file
  // descriptor of a network connection.
  if (::SSL_set_fd(ssl, socket) != 1) {
    ::SSL_free(ssl);
    return nullptr;
  }
  int rc = 0;
  do {
    rc = ::SSL_accept(ssl);
    if (rc != 1) {
      int error = ::SSL_get_error(ssl, rc);
      switch (error) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          break;
        default:
          ::perror("Unable to accept tls connection.");
          // prints the error strings for all errors that OpenSSL has recorded
          // to file, thus emptying the error queue.
          ::ERR_print_errors_fp(stderr);
          ::SSL_free(ssl);
          return nullptr;
      }
    }
  } while (rc != 1);
  return ssl;
}

::SSL *OpenSslWrapper::connectTls(::SSL_CTX *ctx, int socket) {
  ::SSL *ssl = ::SSL_new(ctx);
  // SSL_set_fd() sets the file descriptor fd as the input/output facility for
  // the TLS/SSL (encrypted) side of ssl. fd will typically be the socket file
  // descriptor of a network connection.
  if (::SSL_set_fd(ssl, socket) != 1) {
    ::SSL_free(ssl);
    return nullptr;
  }
  int rc = 0;
  do {
    rc = ::SSL_connect(ssl);
    if (rc != 1) {
      int error = ::SSL_get_error(ssl, rc);
      switch (error) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          break;
        default:
          ::perror("Unable to establish tls connection.");
          // prints the error strings for all errors that OpenSSL has recorded
          // to file, thus emptying the error queue.
          ::ERR_print_errors_fp(stderr);
          ::SSL_free(ssl);
          return nullptr;
      }
    }
  } while (rc != 1);
  return ssl;
}

void OpenSslWrapper::displayCert(const TlsX509Cert &cert) {
  if (cert) {
    ::BIO *bio_out = ::BIO_new_fp(stdout, BIO_NOCLOSE);

    ::X509_print(bio_out, cert.get());

    ::BIO_free(bio_out);
  } else {
    std::cout << "Info: No certificate found." << std::endl;
  }
}

void OpenSslWrapper::displayCerts(::SSL *ssl) {
  TlsX509Cert cert = TlsX509Cert(::SSL_get_peer_certificate(ssl));
  // get the server's certificate
  displayCert(cert);
}

void OpenSslWrapper::displayCertsSimple(::SSL *ssl) {
  // get the server's certificate
  ::X509 *cert = ::SSL_get_peer_certificate(ssl);
  if (cert != nullptr) {
    std::cout << "Server certificate:" << std::endl;

    char *line = ::X509_NAME_oneline(::X509_get_subject_name(cert), 0, 0);
    ::printf("Subject: %s\n", line);
    delete line;

    line = ::X509_NAME_oneline(::X509_get_issuer_name(cert), 0, 0);
    ::printf("Issuer: %s\n", line);
    delete line;

    ::BIO *bio_out = ::BIO_new_fp(stdout, BIO_NOCLOSE);
    ::BIO_printf(bio_out, "Valid From: ");
    ::ASN1_TIME_print(bio_out, X509_get_notBefore(cert));
    ::BIO_printf(bio_out, "\n");

    ::BIO_printf(bio_out, "Valid Until: ");
    ::ASN1_TIME_print(bio_out, X509_get_notAfter(cert));
    ::BIO_printf(bio_out, "\n");

    ::BIO_free(bio_out);

    ::X509_free(cert);
  } else {
    std::cout << "Info: No certificate configured." << std::endl;
  }
}

bool OpenSslWrapper::createSelfSignedCert(const std::string &keyFileName,
                                          const std::string &certFileName,
                                          const std::string &password) {
  // Before we can actually create a certificate, we need to create a private
  // key. OpenSSL provides the EVP_PKEY structure for storing an
  // algorithm-independent private key in memory.
  // In order to allocate an EVP_PKEY structure, we use EVP_PKEY_new
  // There is also a corresponding function for freeing the structure -
  // EVP_PKEY_free
  ::EVP_PKEY *pkey = ::EVP_PKEY_new();
  if (pkey == nullptr) {
    return false;
  }

  // An exponent for the key is also needed, which will require allocating a
  // BIGNUM with BN_new and then assigning with BN_set_word:
  ::BIGNUM *bn;
  bn = ::BN_new();
  ::BN_set_word(bn, RSA_F4);

  // Now we need to generate a key.
  // This is done with the RSA_generate_key
  ::RSA *rsa = RSA_new();
  if (rsa == nullptr) {
    ::EVP_PKEY_free(pkey);
    return false;
  }
  if (::RSA_generate_key_ex(
          rsa,  /* pointer to the RSA structure */
          2048, /* number of bits for the key - 2048 is a good value */
          bn,   /* exponent allocated earlier */
          NULL  /* callback - can be NULL if progress isn't needed */
          ) != 1) {
    ::RSA_free(rsa);
    ::EVP_PKEY_free(pkey);
    return false;
  }

  // now we have an RSA key, and we can assign it to our EVP_PKEY structure from
  // earlier The RSA structure will be automatically freed when the EVP_PKEY
  // structure is freed.
  EVP_PKEY_assign_RSA(pkey, rsa);

  // Now for the certificate itself.
  // OpenSSL uses the X509 structure to represent an x509 certificate in memory.
  // there is a corresponding function for freeing the structure - X509_free.
  X509 *x509 = ::X509_new();

  // Now we need to set a few properties of the certificate using some X509_*
  // functions:

  // This sets the serial number of our certificate to '1'.
  // Some open-source HTTP servers refuse to accept a certificate with a serial
  // number of '0', which is the default
  ::ASN1_INTEGER_set(::X509_get_serialNumber(x509), 1);

  // The next step is to specify the span of time during which the certificate
  // is actually valid. sets the certificate's notBefore property to the current
  // time. (The X509_gmtime_adj function adds the specified number of seconds to
  // the current time - in this case none.)
  ::X509_gmtime_adj(X509_get_notBefore(x509), 0);
  // sets the certificate's notAfter property to 365 days from now (60 seconds *
  // 60 minutes * 24 hours * 365 days).
  ::X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

  // Now we need to set the public key for our certificate using the key we
  // generated earlier
  ::X509_set_pubkey(x509, pkey);

  // Since this is a self-signed certificate, we set the name of the issuer to
  // the name of the subject. The first step in that process is to get the
  // subject name:
  X509_NAME *name = ::X509_get_subject_name(x509);
  // country code
  ::X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"CA",
                               -1, -1, 0);
  // organization
  ::X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                               (unsigned char *)"GGolbik.", -1, -1, 0);
  // common name
  ::X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (unsigned char *)"localhost", -1, -1, 0);
  // Now we can actually set the issuer name:
  ::X509_set_issuer_name(x509, name);

  // And finally we are ready to perform the signing process.
  // We call X509_sign with the key we generated earlier.
  ::X509_sign(x509, pkey, EVP_sha256());

  // We now have a self-signed certificate! But we're not done yet - we need to
  // write these files out to disk.

  //  The first one we will need is PEM_write_PrivateKey for saving our private
  //  key.
  ::FILE *keyFile = ::fopen(keyFileName.c_str(), "wb");
  if (keyFile == nullptr) {
    ::X509_free(x509);
    ::EVP_PKEY_free(pkey);
    return false;
  }

  // If you don't want to encrypt the private key, then simply pass NULL for the
  // cipher and passphrase parameter.
  unsigned char *passphrase = NULL;
  int passphraseLen = -1;
  const EVP_CIPHER *cipher = NULL;
  if (!password.empty()) {
    passphrase = new unsigned char[password.size()];
    password.empty() ? NULL : password.c_str();
    passphraseLen = password.size();
    cipher = EVP_des_ede3_cbc();
  }
  if (::PEM_write_PrivateKey(
          keyFile,    /* write the key to the file we've opened */
          pkey,       /* our key from earlier */
          cipher,     /* default cipher for encrypting the key on disk */
          passphrase, /* passphrase required for decrypting the key on disk */
          passphraseLen, /* length of the passphrase string */
          NULL,          /* callback for requesting a password */
          NULL           /* data to pass to the callback */
          ) != 1) {
    ::fclose(keyFile);
    std::remove(keyFileName.c_str());
    ::X509_free(x509);
    ::EVP_PKEY_free(pkey);
    return false;
  }
  ::fclose(keyFile);
  keyFile = NULL;
  if (passphrase != NULL) {
    delete[] passphrase;
  }

  // we need to write the certificate out to disk. The function we need for this
  // is PEM_write_X509:
  FILE *certFile = ::fopen(certFileName.c_str(), "wb");
  if (certFile == nullptr) {
    ::X509_free(x509);
    ::EVP_PKEY_free(pkey);
    return false;
  }
  if (::PEM_write_X509(
          certFile, /* write the certificate to the file we've opened */
          x509      /* our certificate */
          ) != 1) {
    ::fclose(certFile);
    std::remove(keyFileName.c_str());
    std::remove(certFileName.c_str());
    ::X509_free(x509);
    ::EVP_PKEY_free(pkey);
    return false;
  }
  ::fclose(certFile);

  ::X509_free(x509);
  ::EVP_PKEY_free(pkey);
  return true;
}

bool OpenSslWrapper::readKeyFile(const std::string &fileName, TlsKey &key,
                                 const std::string &password) {
  ::FILE *fp = ::fopen(fileName.c_str(), "r");
  if (!fp) {
    std::cerr << "Failed to open file " << fileName << std::endl;
    return false;
  }
  key.reset(PEM_read_PrivateKey(
      fp, NULL, NULL, password.empty() ? NULL : (void *)password.c_str()));
  ::fclose(fp);
  return key.get() != nullptr;
}

bool OpenSslWrapper::readCertKey(const std::string &fileName, TlsKey &key) {
  TlsX509Cert x509Cert = {};
  if (!OpenSslWrapper::readCertFile(fileName, x509Cert, "")) {
    return false;
  }
  return readCertKey(x509Cert, key);
}

bool OpenSslWrapper::readCertKey(const TlsX509Cert &cert, TlsKey &key) {
  key.reset(X509_get_pubkey(cert.get()));
  return key.get() != nullptr;
}

bool OpenSslWrapper::readCertFile(const std::string &fileName,
                                  TlsX509Cert &cert,
                                  const std::string &password) {
  ::FILE *fp = ::fopen(fileName.c_str(), "r");
  if (!fp) {
    std::cerr << "Failed to open file " << fileName << std::endl;
    return false;
  }
  cert.reset(PEM_read_X509(fp, NULL, NULL,
                           password.empty() ? NULL : (void *)password.c_str()));
  ::fclose(fp);
  return cert.get() != nullptr;
}
/*
bool OpenSslWrapper::readCertFile(const std::string& fileName,
std::unique_ptr<::RSA, std::function<void(::RSA* p)>>& key, const std::string&
password)
{
    ::FILE *fp = ::fopen(fileName.c_str(), "r");
    if (!fp)
    {
        std::cerr << "Failed to open file " << fileName << std::endl;
        return false;
    }
    // PEM_read_RSA_PUBKEY() reads the PEM format. PEM_read_RSAPublicKey() reads
the PKCS#1 format. key.reset(PEM_read_RSA_PUBKEY(fp, NULL, NULL,
password.empty() ? NULL : (void*)password.c_str()));
    ::fclose(fp);
    return key.get() != nullptr;
}
*/

// Asymmetric
bool OpenSslWrapper::signData(TlsKey &key, const std::string &msg,
                              std::string &signature) {
  std::size_t slen;

  /* Create the Message Digest Context */
  TlsMessageDigestContext mdctx = TlsMessageDigestContext(::EVP_MD_CTX_new());
  if (!mdctx) {
    return false;
  }

  /* Initialise the DigestSign operation - SHA-256 has been selected
   * as the message digest function in this example */
  if (1 != ::EVP_DigestSignInit(mdctx.get(), NULL, ::EVP_sha256(), NULL,
                                key.get())) {
    return false;
  }

  /* Call update with the message */
  if (1 != ::EVP_DigestUpdate(mdctx.get(), msg.c_str(), msg.size())) {
    return false;
  }

  /* Finalise the DigestSign operation */
  /* First call EVP_DigestSignFinal with a NULL sig parameter to
   * obtain the length of the signature. Length is returned in slen */
  if (1 != ::EVP_DigestSignFinal(mdctx.get(), NULL, &slen)) {
    return false;
  }
  /* Allocate memory for the signature based on size in slen */
  std::vector<char> sig(slen);
  /* Obtain the signature */
  if (1 !=
      ::EVP_DigestSignFinal(mdctx.get(), (unsigned char *)sig.data(), &slen)) {
    return false;
  }
  // copy data
  signature = std::string(sig.data(), slen);

  return true;
}

/**
 * @brief
 *
 * @param key the public key
 * @param msg input data
 * @param signature signed data
 * @param algorithm the used algorithm e.g. RS256, RS512, HS256, HS512
 * @return
 */
bool OpenSslWrapper::verifySignedData(TlsKey &key, const std::string &msg,
                                      const std::string &signature,
                                      const std::string &algorithm) {
  // Get openssl impl
  // EVP_MD see https://www.openssl.org/docs/manmaster/man3/EVP_DigestInit.html
  // EVP_get_digestbyname see
  // https://www.openssl.org/docs/man1.0.2/man3/EVP_md5.html
  const ::EVP_MD *messageDigest = ::EVP_sha256();
  if (algorithm == "RS256" || algorithm == "HS256") {
    // RS256 (RSA Signature with SHA-256)
    // HS256 (HMAC with SHA-256)
    messageDigest = ::EVP_get_digestbyname("SHA256");
  } else if (algorithm == "RS512" || algorithm == "HS512") {
    // RS512 (RSA Signature with SHA-512)
    // HS512 (HMAC with SHA-512)
    messageDigest = ::EVP_get_digestbyname("SHA512");
  } else if (!algorithm.empty()) {
    messageDigest = EVP_get_digestbyname(algorithm.c_str());
  }
  if (messageDigest == nullptr) {
    return false;
  }

  /* Create the Message Digest Context */
  TlsMessageDigestContext mdctx = TlsMessageDigestContext(EVP_MD_CTX_new());
  if (!mdctx) {
    return false;
  }

  // Initialise the DigestVerify operation - SHA-256 has been selected as the
  // message digest function in this example
  if (1 != ::EVP_DigestVerifyInit(mdctx.get(), NULL, messageDigest, NULL,
                                  key.get())) {
    return false;
  }

  /* Call update with the message */
  if (1 != ::EVP_DigestVerifyUpdate(mdctx.get(), msg.c_str(), msg.size())) {
    return false;
  }

  /* Finalise the _DigestVerify operation */
  if (1 != ::EVP_DigestVerifyFinal(mdctx.get(),
                                   (unsigned char *)signature.c_str(),
                                   signature.size())) {
    return false;
  }

  // ::EVP_DigestVerifyInit
  // ::EVP_DigestVerifyUpdate
  // ::EVP_DigestVerifyFinal
  return true;
}

}  // namespace tls
}  // namespace cpp
}  // namespace ggolbik
