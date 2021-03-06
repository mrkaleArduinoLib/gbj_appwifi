/*
  NAME:
  gbj_appwifi

  DESCRIPTION:
  Application library for processing connection to an access point over WiFi.
  - The library activates multicast DNS with the provided wifi hostname.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the license GNU GPL v3
  http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
  License (MIT) for added code.

  CREDENTIALS:
  Author: Libor Gabaj
 */
#ifndef GBJ_APPWIFI_H
#define GBJ_APPWIFI_H

#if defined(ESP8266)
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <Arduino.h>
  #include <WiFi.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#else
  #error !!! Only platforms with WiFi are suppored !!!
#endif
#include "gbj_appbase.h"
#include "gbj_serial_debug.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appwifi"

class gbj_appwifi : public gbj_appcore
{
public:
  const char *VERSION = "GBJ_APPWIFI 1.5.1";

  typedef void Handler();

  struct Handlers
  {
    Handler *onConnectStart;
    Handler *onConnectTry;
    Handler *onConnectSuccess;
    Handler *onConnectFail;
    Handler *onDisconnect;
    Handler *onRestart;
  };

  /*
    Constructor

    DESCRIPTION:
    Constructor creates the class instance object and sets credentials for wifi.

    PARAMETERS:
    ssid - The name of a wifi network to connect to.
      - Data type: constant string
      - Default value: none
      - Limited range: none
    pass - The passphrase for a wifi network.
      - Data type: constant string
      - Default value: none
      - Limited range: none
    hostname - The hostname for a device on the network.
      - Data type: constant string
      - Default value: none
      - Limited range: none
    handlers - A structure with pointers to various callback handler functions.
      - Data type: Handlers
      - Default value: structure with zeroed all handlers
      - Limited range: system address range

    RETURN: object
  */
  inline gbj_appwifi(const char *ssid,
                     const char *pass,
                     const char *hostname,
                     Handlers handlers = Handlers())
  {
    wifi_.ssid = ssid;
    wifi_.pass = pass;
    wifi_.hostname = hostname;
    handlers_ = handlers;
  }
  inline gbj_appwifi(const char *ssid,
                     const char *pass,
                     const char *hostname,
                     const IPAddress staticIp,
                     const IPAddress gateway,
                     const IPAddress subnet,
                     const IPAddress primaryDns = IPAddress(),
                     const IPAddress secondaryDns = IPAddress(),
                     Handlers handlers = Handlers())
  {
    wifi_.ssid = ssid;
    wifi_.pass = pass;
    wifi_.hostname = hostname;
    wifi_.staticIp = staticIp;
    wifi_.gateway = gateway;
    wifi_.subnet = subnet;
    wifi_.primaryDns = primaryDns;
    wifi_.secondaryDns = secondaryDns;
    handlers_ = handlers;
  }

  /*
    Processing

    DESCRIPTION:
    The method should be called in an application sketch loop.
    It processes main functionality and is control by the internal timer.

    PARAMETERS: None

    RETURN: none
  */
  inline void run()
  {
    if (!isConnected())
    {
      connect();
    }
  }

  // Getters
  inline bool isConnected() { return WiFi.isConnected(); }
  inline byte getFails() { return status_.fails; }
  inline int getRssi() { return WiFi.RSSI(); }
  inline const char *getAddressIp() { return addressIp_; }
  inline const char *getAddressMac() { return addressMac_; }
  inline const char *getHostname()
  {
    return isConnected() ? WiFi.getHostname() : wifi_.hostname;
  };

private:
  enum Timing : unsigned long
  {
    PERIOD_FAIL = 500,
    PERIOD_SET = 1 * 60 * 1000,
  };
  enum Params : byte
  {
    PARAM_TRIES = 20,
    PARAM_FAILS = 6,
  };
  struct Wifi
  {
    const char *ssid;
    const char *pass;
    const char *hostname;
    IPAddress staticIp;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress primaryDns;
    IPAddress secondaryDns;
  } wifi_;
  struct Status
  {
    byte fails;
    unsigned long tsRetry;
    bool flConnGain;
    void reset()
    {
      tsRetry = 0;
      flConnGain = false;
    }
    void init()
    {
      reset();
      fails = 0;
      flConnGain = true;
    }
  } status_;
  char addressIp_[16];
  char addressMac_[18];
  Handlers handlers_;
  ResultCodes connect();
  inline void setAddressIp()
  {
    strcpy(addressIp_, WiFi.localIP().toString().c_str());
  }
  inline void setAddressMac()
  {
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    sprintf(addressMac_,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0],
            mac[1],
            mac[2],
            mac[3],
            mac[4],
            mac[5]);
  }
};

#endif
