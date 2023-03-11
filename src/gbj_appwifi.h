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
#include "gbj_appcore.h"
#include "gbj_serial_debug.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appwifi"
class gbj_appwifi : public gbj_appcore
{
public:
  /*
    Constructor

    DESCRIPTION:
    Constructor creates the class instance object and sets credentials for
    wifi.

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

    RETURN: object
  */
  inline gbj_appwifi(const char *ssid, const char *pass, const char *hostname)
  {
    wifi_.ssid = ssid;
    wifi_.pass = pass;
    wifi_.hostname = hostname;
    status_.init();
  }
  inline gbj_appwifi(const char *ssid,
                     const char *pass,
                     const char *hostname,
                     const IPAddress staticIp,
                     const IPAddress gateway,
                     const IPAddress subnet,
                     const IPAddress primaryDns = IPAddress(),
                     const IPAddress secondaryDns = IPAddress())
    : gbj_appwifi(ssid, pass, hostname)
  {
    wifi_.staticIp = staticIp;
    wifi_.gateway = gateway;
    wifi_.subnet = subnet;
    wifi_.primaryDns = primaryDns;
    wifi_.secondaryDns = secondaryDns;
  }

  /*
    Processing

    DESCRIPTION:
    The method should be called in an application sketch loop.
    It processes main functionality and is control by the internal timer.

    PARAMETERS: None

    RETURN: none
  */
  inline void run() { connect(); }

  /*
    Activity at connection success

    DESCRIPTION:
    The method should be called in "WiFiEventHandler" for event
    "WiFiEventStationModeGotIP".

    PARAMETERS: None

    RETURN: none
  */
  void connectSuccess()
  {
    SERIAL_VALUE("connectSuccess()", getStatus())
    setAddressIp();
    setAddressMac();
    SERIAL_VALUE("IP", WiFi.localIP())
    SERIAL_VALUE("MAC", getAddressMac())
    SERIAL_VALUE("SSID", wifi_.ssid)
    SERIAL_VALUE("Hostname", wifi_.hostname)
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI())
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    status_.init();
    status_.flHandlerSuccess = true;
    status_.tsEvent = millis();
    SERIAL_DELIM
  }

  /*
    Activity at connection failure

    DESCRIPTION:
    The method should be called in "WiFiEventHandler" for event
    "WiFiEventStationModeDisconnected".

    PARAMETERS: None

    RETURN: none
  */
  void connectFail()
  {
    SERIAL_VALUE("connectFail()", getStatus())
    SERIAL_VALUE("delay", millis() - status_.tsEvent)
    status_.flBegin = true;
    status_.flHandlerSuccess = false;
    status_.timeWait = status_.timePeriod;
    status_.waits = 0;
    WiFi.mode(WIFI_OFF);
    status_.tsEvent = millis();
    SERIAL_DELIM
  }

  // Getters
  inline bool isConnected() { return WiFi.isConnected(); }
  inline int getRssi() { return WiFi.RSSI(); }
  inline unsigned long getPeriod() { return status_.timePeriod; }
  inline unsigned long getEventMillis() { return status_.tsEvent; }
  inline const char *getAddressIp() { return addressIp_; }
  inline const char *getAddressMac() { return addressMac_; }
  inline const char *getHostname()
  {
    return isConnected() ? WiFi.getHostname() : wifi_.hostname;
  };
  inline String getStatus()
  {
    switch (WiFi.status())
    {
      case WL_IDLE_STATUS:
        return SERIAL_F("Changing between statuses");
        break;

      case WL_NO_SSID_AVAIL:
        return SERIAL_F("No SSID available");
        break;

      case WL_SCAN_COMPLETED:
        return SERIAL_F("Wifi networks scanning completed");
        break;

      case WL_CONNECTED:
        return SERIAL_F("Connected successfully");
        break;

      case WL_CONNECT_FAILED:
        return SERIAL_F("Connection failed");
        break;

      case WL_CONNECTION_LOST:
        return SERIAL_F("Connection lost");
        break;

      case WL_WRONG_PASSWORD:
        return SERIAL_F("Incorrect password");
        break;

      case WL_DISCONNECTED:
        return SERIAL_F("Disconnected");
        break;

      case WL_NO_SHIELD:
        return SERIAL_F("Wifi shield not present");
        break;

      default:
        return SERIAL_F("Uknown");
        break;
    }
    return statusText_;
  }

  // Setters

  // Set reconnect period inputed as unsigned long in milliseconds
  inline void setPeriod(unsigned long period = Timing::PERIOD_CONNECT_DFT)
  {
    status_.timePeriod =
      constrain(period, Timing::PERIOD_CONNECT_MIN, Timing::PERIOD_CONNECT_MAX);
  }
  // Set timer period inputed as String in seconds
  inline void setPeriod(String period)
  {
    setPeriod(1000 * (unsigned long)period.toInt());
  }

private:
  enum Timing : unsigned long
  {
    PERIOD_TIMEOUT = 1 * 1000,
    PERIOD_CONNECT_DFT = 15 * 1000,
    PERIOD_CONNECT_MIN = 0 * 1000,
    PERIOD_CONNECT_MAX = 60 * 1000,
  };
  enum Params : byte
  {
    PARAM_SAFETY_WAITS = 30,
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
    unsigned long tsEvent, timeWait;
    unsigned long timePeriod = Timing::PERIOD_CONNECT_DFT;
    bool flBegin, flHandlerSuccess;
    byte waits;
    void init()
    {
      flBegin = true;
      flHandlerSuccess = false;
      timeWait = 0;
      waits = 0;
    }
  } status_;
  char addressIp_[16];
  char addressMac_[18];
  char statusText_[30];
  void connect();
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
