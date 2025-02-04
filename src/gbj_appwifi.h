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

// Class definition for wifi processing
class gbj_appwifi : public gbj_appcore
{
public:
  // Callback procedures templates
#if defined(ESP8266)
  // typedef void (*cbEvent_t)(WiFiEvent_t);
  typedef void (*cbGotIP_t)(const WiFiEventStationModeGotIP &);
  typedef void (*cbDisconnect_t)(const WiFiEventStationModeDisconnected &);
#elif defined(ESP32)
  typedef void (*cbEvent_t)(arduino_event_id_t, arduino_event_info_t);
#endif

  /** Constructor.

  Constructor creates the class instance object and sets credentials for
  wifi.

  @param ssid Name of a wifi network to connect to.
  @param pass Passphrase for a wifi network.
  @param hostname Hostname for a device on the network.
  @param staticIp Static IP address of the MCU if defined.
  @param gateway IP address of the wifi router (access point).
  @param subnet IP address of the wifi subnet.
  @param primaryDns IP address of the primary DNS server.
  @param secondaryDns IP address of the secondary DNS server.

  @returns object
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

#if defined(ESP8266)
  inline void begin(cbGotIP_t cbGotIp, cbDisconnect_t cbDisconnected)
  {
    onGotIpHandler_ = WiFi.onStationModeGotIP(cbGotIp);
    onDisconnectHandler_ = WiFi.onStationModeDisconnected(cbDisconnected);
  }
#elif defined(ESP32)
  inline void begin(cbEvent_t cbGotIp, cbEvent_t cbDisconnected)
  {
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(cbGotIp, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(cbDisconnected,
                 WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  }
#endif

  // Processing
  inline void run() { connect(); }

  /** Activity at connection success.

  The method should be called in hander for event GotIP.
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

  /**  Activity at connection failure.

  The method should be called in hander for event Disconnected.
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
    WiFi.mode(WIFI_OFF); // Calls event 'cbDisconnected'
#endif
  }

  /** Simple ping to the gateway.

  The method executes ping to the current gateway IP, only if there is
  a connection to a wifi access point. The ping detects false wifi status as
  connected, while the real connection has been broken.

  @param pingCnt The byte number of pings executed.

  @returns Boolean success flag about pinging to wifi gateway.
  */
  bool pingGW(byte pingCnt = 2)
  {
    bool flResult =
      WiFi.isConnected() ? Ping.ping(WiFi.gatewayIP(), pingCnt) : false;
    SERIAL_VALUE("Ping GW", flResult ? "SUCCESS" : "ERROR")
    return flResult;
  }

  /** Simple ping to a DNS server.

  The method executes ping to the current DNS server IP, only if there is
  connection to a wifi access point. The ping detects disconnection from
  internet.

  @param dnsIP The IP address used for pinging.
  @param pingCnt The byte number of pings executed.

  @returns Boolean success flag about pinging to wifi gateway.
  */
  bool pingDNS(const IPAddress dnsIP, byte pingCnt = 2)
  {
    bool flResult = WiFi.isConnected() ? Ping.ping(dnsIP, pingCnt) : false;
    SERIAL_VALUE_VALUE(
      "Ping DNS", dnsIP, "Result", flResult ? "SUCCESS" : "ERROR")
    return flResult;
  }

  // Return success flag about wifi connection to AP
  inline bool isConnected() { return WiFi.isConnected(); }

  // Return success flag about wifi connection and pinging to AP
  inline bool isContact() { return isConnected() && pingGW(); }

  // Return current RSSI value
  inline int getRssi() { return WiFi.RSSI(); }

  // Return statistically smoothed RSSI value
  inline int getRssiSmooth()
  {
    return isConnected() ? smooth_->getValue() : getRssi();
  }

  inline unsigned long getPeriod() { return status_.timePeriod; }
  inline unsigned long getEventMillis() { return status_.tsEvent; }
  inline const char *getAddressIp() { return addressIp_; }
  inline const char *getAddressMac() { return addressMac_; }
  inline unsigned int getIdMac() { return idMac_; }
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
  unsigned int idMac_;
  gbj_appsmooth<gbj_running, int> *smooth_;
#if defined(ESP8266)
  WiFiEventHandler onGotIpHandler_, onDisconnectHandler_;
#endif
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
    idMac_ = (mac[4] << 8) + mac[5];
  }
};

#endif
