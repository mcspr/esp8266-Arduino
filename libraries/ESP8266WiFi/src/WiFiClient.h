/*
  WiFiClient.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino.  All right reserved.

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

  Modified by Ivan Grokhotkov, December 2014 - esp8266 support
*/

#ifndef wificlient_h
#define wificlient_h
#include <memory>
#include "Arduino.h"
#include "Print.h"
#include "Client.h"
#include "IPAddress.h"
#include "include/slist.h"
#include "include/internal.h"

class ClientContext;
class WiFiServer;

class WiFiClient : public Client, public SList<WiFiClient> {
protected:
  WiFiClient(ClientContext* client);

public:
  WiFiClient();
  virtual ~WiFiClient();
  WiFiClient(const WiFiClient&);
  WiFiClient& operator=(const WiFiClient&);

  // b/c this is both a real class and a virtual parent of the secure client, make sure
  // there's a safe way to copy from the pointer without 'slicing' it; i.e. only the base
  // portion of a derived object will be copied, and the polymorphic behavior will be corrupted. 
  //
  // this class still implements the copy and assignment though, so this is not yet enforced
  // (but, *should* be inside the Core itself, see httpclient & server)
  //
  // ref.
  // - https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual
  // - https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rh-copy
  virtual std::unique_ptr<WiFiClient> clone() const;

  virtual uint8_t status();
  int connect(IPAddress ip, uint16_t port) override;
  int connect(const char *host, uint16_t port) override;
  virtual int connect(const String& host, uint16_t port);

  using Print::write;
  size_t write(uint8_t) override;
  size_t write(const uint8_t *buf, size_t size) override;

  int available() override;
  int read() override;
  int read(uint8_t* buf, size_t size) override;
  int read(char* buf, size_t size);

  int peek() override;
  virtual size_t peekBytes(uint8_t *buffer, size_t length);
  size_t peekBytes(char *buffer, size_t length) {
    return peekBytes((uint8_t *) buffer, length);
  }
  void flush() override { (void)flush(0); } // wait for all outgoing characters to be sent, output buffer should be empty after this call
  void stop() override { (void)stop(0); }
  bool flush(unsigned int maxWaitMs);
  bool stop(unsigned int maxWaitMs);
  uint8_t connected() override;
  operator bool() override;

  virtual IPAddress remoteIP();
  virtual uint16_t  remotePort();
  virtual IPAddress localIP();
  virtual uint16_t  localPort();

  static void setLocalPortStart(uint16_t port) { _localPort = port; }

  int availableForWrite() override;

  friend class WiFiServer;

  static void stopAll();
  static void stopAllExcept(WiFiClient * c);

  virtual void     keepAlive (uint16_t idle_sec = TCP_DEFAULT_KEEPALIVE_IDLE_SEC, uint16_t intv_sec = TCP_DEFAULT_KEEPALIVE_INTERVAL_SEC, uint8_t count = TCP_DEFAULT_KEEPALIVE_COUNT);
  virtual bool     isKeepAliveEnabled () const;
  virtual uint16_t getKeepAliveIdle () const;
  virtual uint16_t getKeepAliveInterval () const;
  virtual uint8_t  getKeepAliveCount () const;
  virtual void     disableKeepAlive () { keepAlive(0, 0, 0); }

  // default NoDelay=False (Nagle=True=!NoDelay)
  // Nagle is for shortly delaying outgoing data, to send less/bigger packets
  // Nagle should be disabled for telnet-like/interactive streams
  // Nagle is meaningless/ignored when Sync=true
  static void setDefaultNoDelay (bool noDelay);
  static bool getDefaultNoDelay ();
  bool getNoDelay() const;
  void setNoDelay(bool nodelay);

  // default Sync=false
  // When sync is true, all writes are automatically flushed.
  // This is slower but also does not allocate
  // temporary memory for sending data
  static void setDefaultSync (bool sync);
  static bool getDefaultSync ();
  bool getSync() const;
  void setSync(bool sync);

  // peek buffer API is present
  bool hasPeekBufferAPI () const override { return true; }

  // return number of byte accessible by peekBuffer()
  size_t peekAvailable () override;

  // return a pointer to available data buffer (size = peekAvailable())
  // semantic forbids any kind of read() before calling peekConsume()
  const void* peekBuffer () override;

  // consume bytes after use (see peekBuffer)
  void peekConsume (size_t consume) override;

  bool outputCanTimeout () override { return connected(); }
  bool inputCanTimeout () override { return connected(); }

  // Immediately stops this client instance.
  // Unlike stop(), does not wait to gracefuly shutdown the connection.
  void abort();

protected:
  static int8_t _s_connected(void* arg, void* tpcb, int8_t err);
  static void _s_err(void* arg, int8_t err);

  int8_t _connected(void* tpcb, int8_t err);
  void _err(int8_t err);

  ClientContext* _client;
  WiFiClient* _owned;
  static uint16_t _localPort;
};

#endif
