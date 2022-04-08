/*
  WiFiClientBearSSL- SSL client/server for esp8266 using BearSSL libraries
  - Mostly compatible with Arduino WiFi shield library and standard
    WiFiClient/ServerSecure (except for certificate handling).

  Copyright (c) 2018 Earle F. Philhower, III

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _BEARSSLHELPERS_H
#define _BEARSSLHELPERS_H

#include <bearssl/bearssl.h>
#include <StackThunk.h>
#include <Updater.h>

#include <memory>
#include <utility>

// Internal opaque structures, not needed by user applications
namespace brssl {
  class public_key;
  class private_key;
};

namespace BearSSL {
namespace Detail {

template <typename T>
struct DeleteHelper {
  void operator()(T*) const;
};

template <typename T>
using Pointer = std::unique_ptr<T, DeleteHelper<T>>;

  // to avoid dealing with unique_ptr and it's inability to not throw exceptions,
  // explicitly manage both certificate list and it's trust anchors parsed result

template <typename T, typename Value>
class PointerHolder {
  public:
    using value_type = Value;
    static constexpr size_t value_size = sizeof(value_type);
    
    PointerHolder() = default;

    PointerHolder(const PointerHolder &) = delete;
    PointerHolder& operator=(const PointerHolder &) = delete;

    PointerHolder(PointerHolder &&other) noexcept {
      move(std::move(other));
    }

    PointerHolder &operator=(PointerHolder &&other) noexcept {
      move(std::move(other));
      return *this;
    }

    explicit operator bool() const {
      return _ptr && _count;
    }

    void reset(value_type *ptr, size_t count) {
      _ptr = ptr;
      _count = count;
    }

    void move(T &&other) noexcept {
      clear();
      reset(other._ptr, other._count);
      other._ptr = nullptr;
      other._count = 0;
    }

    size_t count() const {
      return _count;
    }

    value_type *get() const {
      return _ptr;
    }

    value_type *get(size_t index) const {
      return &_ptr[index];
    }

    value_type *operator[](size_t index) const {
      return get(index);
    }

    bool update(T &other);

    void clear() {
      T::clear(_ptr, _count);
      _ptr = nullptr;
      _count = 0;
    }

  private:
    size_t _count = 0;
    value_type *_ptr = nullptr;
};

struct Certificates : public PointerHolder<Certificates, br_x509_certificate> {
  public:
    using value_type = br_x509_certificate;
    static constexpr size_t value_size = sizeof(value_type);

    using PointerHolder<Certificates, value_type>::clear;

    Certificates() = default;
    ~Certificates() = default;

    Certificates(const char *cert, size_t length);

    Certificates(const Certificates &) = delete;
    Certificates(Certificates &&) = default;

    Certificates& operator=(const Certificates &) = delete;
    Certificates& operator=(Certificates &&) = default;

    Certificates(const uint8_t *cert, size_t length) :
      Certificates((const char *)cert, length)
    {}

    static void clear(value_type *ptr, size_t count);
};

struct TrustAnchors : public PointerHolder<TrustAnchors, br_x509_trust_anchor> {
  using value_type = br_x509_trust_anchor;
  static constexpr size_t value_size = sizeof(value_type);

  using PointerHolder<TrustAnchors, value_type>::clear;

  TrustAnchors() = default;
  ~TrustAnchors() = default;

  TrustAnchors(const TrustAnchors &) = delete;
  TrustAnchors(TrustAnchors &&) = default;

  TrustAnchors& operator=(const TrustAnchors &) = delete;
  TrustAnchors& operator=(TrustAnchors &&) = default;

  TrustAnchors(const Certificates &certs) {
    update(certs);
  }

  bool update(const Certificates &certs);

  static void clear(value_type *ptr, size_t count);
};

} // namespace Detail

// Holds either a single public RSA or EC key for use when BearSSL wants a pubkey.
// Copies all associated data so no need to keep input PEM/DER keys.
// All inputs can be either in RAM or PROGMEM.
class PublicKey {
  public:
    // Disable copies, we're pointer based
    PublicKey(const PublicKey &) = delete;
    PublicKey(PublicKey &&) = default;

    PublicKey& operator=(const PublicKey &) = delete;
    PublicKey& operator=(PublicKey &&) = default;

    PublicKey(const char *pemKey);
    PublicKey(const uint8_t *derKey, size_t derLen);
    PublicKey(Stream& stream, size_t size);
    PublicKey(Stream& stream) : PublicKey(stream, stream.available()) { };

    bool parse(const char *pemKey);
    bool parse(const uint8_t *derKey, size_t derLen);

    // Accessors for internal use, not needed by apps
    bool isRSA() const;
    bool isEC() const;
    const br_rsa_public_key *getRSA() const;
    const br_ec_public_key *getEC() const;

    void clear();

  private:
    Detail::Pointer<brssl::public_key> _key;
};

// Holds either a single private RSA or EC key for use when BearSSL wants a secretkey.
// Copies all associated data so no need to keep input PEM/DER keys.
// All inputs can be either in RAM or PROGMEM.
class PrivateKey {
  public:
    // Disable copies, we're pointer based
    PrivateKey(const PrivateKey&) = delete;
    PrivateKey(PrivateKey &&) = default;

    PrivateKey& operator=(const PrivateKey&) = delete;
    PrivateKey& operator=(PrivateKey &&) = default;

    PrivateKey(const char *pemKey);
    PrivateKey(const uint8_t *derKey, size_t derLen);
    PrivateKey(Stream& stream, size_t size);
    PrivateKey(Stream& stream) : PrivateKey(stream, stream.available()) { };

    bool parse(const char *pemKey);
    bool parse(const uint8_t *derKey, size_t derLen);

    // Accessors for internal use, not needed by apps
    bool isRSA() const;
    bool isEC() const;
    const br_rsa_private_key *getRSA() const;
    const br_ec_private_key *getEC() const;

  private:
    Detail::Pointer<brssl::private_key> _key;
};

// Holds one or more X.509 certificates and associated trust anchors for
// use whenever BearSSL needs a cert or TA.  May want to have multiple
// certs for things like a series of trusted CAs (but check the CertStore class
// for a more memory efficient way).
// Copies all associated data so no need to keep input PEM/DER certs.
// All inputs can be either in RAM or PROGMEM.
class X509List {
  public:
    X509List() = default;
    ~X509List() = default;

    // Disable the copies, we're pointer based
    X509List(const X509List &) = delete;
    X509List(X509List &&) = default;

    X509List& operator=(const X509List &) = delete;
    X509List& operator=(X509List &&) = default;

    X509List(const char *pemCert);
    X509List(const uint8_t *derCert, size_t derLen);

    X509List(Stream& stream, size_t size);
    X509List(Stream& stream) : X509List(stream, stream.available()) { };

    bool append(const char *pemCert);
    bool append(const uint8_t *derCert, size_t derLen);

    void clear();

    // Accessors
    size_t getCount() const {
      return _certs.count();
    }
    const br_x509_certificate *getX509Certs() const {
      return _certs.get();
    }
    const br_x509_trust_anchor *getTrustAnchors() const {
      return _tas.get();
    }

  private:
    Detail::Certificates _certs;
    Detail::TrustAnchors _tas;
};

// Opaque object which wraps the BearSSL SSL session to make repeated connections
// significantly faster.  Completely optional.
class WiFiClientSecure;

// Cache for a TLS session with a server
// Use with BearSSL::WiFiClientSecure::setSession
// to accelerate the TLS handshake
class Session {
  friend class WiFiClientSecureCtx;

  public:
    Session() { memset(&_session, 0, sizeof(_session)); }
  private:
    br_ssl_session_parameters *getSession() { return &_session; }
    // The actual BearSSL session information
    br_ssl_session_parameters _session;
};

// Represents a single server session.
// Use with BearSSL::ServerSessions.
using ServerSession = uint8_t[100];

// Cache for the TLS sessions of multiple clients.
// Use with BearSSL::WiFiServerSecure::setCache
class ServerSessions {
  friend class WiFiClientSecureCtx;

  public:
    ServerSessions() = default;

    // Uses the given buffer to cache the given number of sessions and initializes it.
    ServerSessions(ServerSession *sessions, size_t size) {
      reset(sessions, size);
    }

    // Update the internal buffer pointer and it's size
    void reset(ServerSession *sessions, size_t size);

    // Returns the number of sessions the cache can hold.
    size_t size() { return _size; }

  private:
    // Returns the cache's vtable or null if the cache has no capacity.
    const br_ssl_session_cache_class **getCache();

    // Size of the store in sessions.
    size_t _size = 0;
    // Store where the information for the sessions are stored.
    ServerSession *_store = nullptr;

    // Cache of the server using the _store.
    br_ssl_session_cache_lru _cache;
};

// Dynamically allocates a cache for the given number of sessions and initializes it.
// If the allocation of the buffer wasn't successful, the value
// returned by size() will be 0.
class DynamicServerSessions : public ServerSessions {
  public:
    DynamicServerSessions(uint32_t size) {
      if (size) {
        _ptr = std::make_unique<ServerSession[]>(size);
        reset(_ptr.get(), size);
      }
    }

  private:
    std::unique_ptr<ServerSession[]> _ptr;
};


// Updater SHA256 hash and signature verification
class HashSHA256 : public UpdaterHashClass {
  public:
    virtual void begin() override;
    virtual void add(const void *data, uint32_t len) override;
    virtual void end() override;
    virtual int len() override;
    virtual const void *hash() override;
    virtual const unsigned char *oid() override;
  private:
    br_sha256_context _cc;
    unsigned char _sha256[32];
};

class SigningVerifier : public UpdaterVerifyClass {
  public:
    virtual uint32_t length() override;
    virtual bool verify(UpdaterHashClass *hash, const void *signature, uint32_t signatureLen) override;

  public:
    SigningVerifier(PublicKey *pubKey) { _pubKey = pubKey; stack_thunk_add_ref(); }
    ~SigningVerifier() { stack_thunk_del_ref(); }

  private:
    PublicKey *_pubKey;
};

// Stack thunked versions of calls
extern "C" {
extern unsigned char *thunk_br_ssl_engine_recvapp_buf( const br_ssl_engine_context *cc, size_t *len);
extern void thunk_br_ssl_engine_recvapp_ack(br_ssl_engine_context *cc, size_t len);
extern unsigned char *thunk_br_ssl_engine_recvrec_buf( const br_ssl_engine_context *cc, size_t *len);
extern void thunk_br_ssl_engine_recvrec_ack(br_ssl_engine_context *cc, size_t len);
extern unsigned char *thunk_br_ssl_engine_sendapp_buf( const br_ssl_engine_context *cc, size_t *len);
extern void thunk_br_ssl_engine_sendapp_ack(br_ssl_engine_context *cc, size_t len);
extern unsigned char *thunk_br_ssl_engine_sendrec_buf( const br_ssl_engine_context *cc, size_t *len);
extern void thunk_br_ssl_engine_sendrec_ack(br_ssl_engine_context *cc, size_t len);
};

};

#endif
