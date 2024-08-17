/*
  NAME:
  gbj_appwifi

  DESCRIPTION:
  Application library for processing connection to an access point over WiFi.

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

#include <Arduino.h>
#if defined(ESP8266)
  #include <ESP8266Ping.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <ESP32Ping.h>
  #include <WiFi.h>
#else
  #error !!! Only platforms with WiFi are supported !!!
#endif
#include "gbj_appcore.h"
#include "gbj_appsmooth.h"
#include "gbj_running.h"
#include "gbj_serial_debug.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appwifi"
class gbj_appwifi : public gbj_appcore
{
public:
  // Callback procedures templates
#if defined(ESP8266)
  typedef void (*cbEvent_t)(WiFiEvent_t);
#elif defined(ESP32)
  typedef void (*cbEvent_t)(arduino_event_id_t, arduino_event_info_t);
#endif
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
    smooth_ = new gbj_appsmooth<gbj_running, int>();
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

  inline void begin(cbEvent_t cbGotIp, cbEvent_t cbDisconnected)
  {
#if defined(ESP32)
    WiFi.mode(WIFI_STA);
#endif
#if defined(ESP8266)
    WiFi.onEvent(cbGotIp, WiFiEvent_t::WIFI_EVENT_STAMODE_GOT_IP);
    WiFi.onEvent(cbDisconnected, WiFiEvent_t::WIFI_EVENT_STAMODE_DISCONNECTED);
#elif defined(ESP32)
    WiFi.onEvent(cbGotIp, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(cbDisconnected,
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
#endif
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
    smooth_->begin();
    smooth_->getMeasurePtr()->setMedian();
    status_.tsEvent = millis();
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
    status_.tsEvent = millis();
#if defined(ESP8266)
    WiFi.mode(WIFI_OFF);
#endif
  }

  /*
    Simple ping to the gateway

    DESCRIPTION:
    The method executes ping to the current gateway IP, only if there is
    connection to a wifi access point.
    - The ping should detect false wifi status as connected, while the real
    connection has been broken.

    PARAMETERS:
    pingCnt - The number of pings executed.
      - Data type: byte
      - Default value: 2
      - Limited range: 0 ~ 255

    RETURN:
    Boolean flag about pinging to wifi gateway.
  */
  bool pingGW(byte pingCnt = 2)
  {
    return isConnected() ? Ping.ping(WiFi.gatewayIP(), pingCnt) : false;
  }

  // Getters
  inline bool isConnected() { return WiFi.isConnected(); }
  inline int getRssi() { return WiFi.RSSI(); }
  inline int getRssiSmooth()
  {
    return isConnected() ? smooth_->getValue() : getRssi();
  }
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

      case WL_DISCONNECTED:
        return SERIAL_F("Disconnected");
        break;

      case WL_NO_SHIELD:
        return SERIAL_F("Wifi shield not present");
        break;
#if defined(ESP8266)
      case WL_WRONG_PASSWORD:
        return SERIAL_F("Incorrect password");
        break;
#endif
      default:
        return SERIAL_F("Uknown");
        break;
    }
    return statusText_;
  }

  // Setters
  inline int setRssiSmooth()
  {
    if (isConnected())
    {
      smooth_->setValue(getRssi());
    }
    return getRssiSmooth();
  }

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
    PERIOD_CONNECT_DFT = 5 * 1000,
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
  gbj_appsmooth<gbj_running, int> *smooth_;
  void connect();
  inline void setAddressIp()
  {
    strcpy(addressIp_, WiFi.localIP().toString().c_str());
  }
  inline void setAddressMac()
  {
    byte mac[6];
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
