/** gbj_appwifi
 *
 * @brief Application library for processing connection to an access point over
 * WiFi.
 *
 * @copyright This program is free software; you can redistribute it and/or
 * modify it under the terms of the license GNU GPL v3
 * http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
 * License (MIT) for added code.
 *
 * @author Libor Gabaj
 *
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
#include "gbj_appbase.h"
#include "gbj_serial_debug.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appwifi"

// Class definition for wifi processing
class gbj_appwifi : public gbj_appbase
{
public:
  // Callback procedures templates
#if defined(ESP8266)
  typedef void (*cbGotIP_t)(const WiFiEventStationModeGotIP &);
  typedef void (*cbDisconnect_t)(const WiFiEventStationModeDisconnected &);
#elif defined(ESP32)
  typedef void (*cbEvent_t)(arduino_event_id_t, arduino_event_info_t);
#endif

  /** Constructor.
   *
   * @brief Constructor creates the class instance object and sets credentials
   * for wifi.
   *
   * @param ssid Name of a wifi network to connect to.
   * @param pass Passphrase for a wifi network.
   * @param hostname Hostname for a device on the network.
   * @param staticIp Static IP address of the MCU if defined.
   * @param gateway IP address of the wifi router (access point).
   * @param subnet IP address of the wifi subnet.
   * @param primaryDns IP address of the primary DNS server.
   * @param secondaryDns IP address of the secondary DNS server.
   *
   * @returns object
   *
   */
  inline gbj_appwifi(const char *ssid, const char *pass, const char *hostname)
  {
    wifi_.ssid = ssid;
    wifi_.pass = pass;
    wifi_.hostname = hostname;
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

  /** Processing */
  inline void run()
  {
    if (WiFi.isConnected())
    {
      check();
    }
    else
    {
      connect();
    }
  }

  /** Activity at connection success.
   *
   * @warning The method should be called in handler for event GotIP.
   *
   */
  void connectSuccess()
  {
    SERIAL_VALUE("connectSuccess()", getStatus())
    connection_.init();
    setAddressIp();
    setAddressMac();
    SERIAL_VALUE("IP", WiFi.localIP())
    SERIAL_VALUE("MAC", getAddressMac())
    SERIAL_VALUE("SSID", wifi_.ssid)
    SERIAL_VALUE("Hostname", wifi_.hostname)
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI())
    // Enable immediate pinging
    ping_.tsPing = 0;
  }

  /** Activity at connection failure.
   *
   * @warning The method should be called in handler for event Disconnected.
   *
   */
  void connectFail()
  {
    SERIAL_VALUE("connectFail()", getStatus())
    connection_.reset();
    connection_.flDisconnect = true;
#if defined(ESP8266)
    WiFi.mode(WIFI_OFF);
#endif
  }

  /** Simple ping to the gateway.

   * @brief The method executes ping to the current gateway IP, only if there is
   * connection to a wifi access point. The ping detects false wifi status as
   * connected, while the real connection has been broken.
   *
   * @param pingCnt The byte number of pings executed.
   *
   * @returns Boolean success flag about pinging to wifi gateway.
   *
   */
  bool pingGW(byte pingCnt = 2)
  {
    bool flResult =
      WiFi.isConnected() ? Ping.ping(WiFi.gatewayIP(), pingCnt) : false;
    SERIAL_VALUE("Ping GW", flResult ? "SUCCESS" : "ERROR")
    return flResult;
  }

  /** Set pinging period.
   *
   * @brief The method is overloaded and sets time period for pinging to DNS
   * servers.
   *
   * @note If the input time period is of type integer, it is considered to be
   * in milliseconds.
   * @note If the input time period is of type string, it is considered to be in
   * seconds.
   * @note If no input time period is provided, the setter sets the default
   * period hardcoded in the library.
   *
   * @param period Numerical time period for generating heartbeat pulses in
   * milliseconds.
   * @param periodSec Textual time period for generating heartbeat pulses in
   * seconds.
   *
   */
  inline void setPeriod(unsigned long period = 0)
  {
    ping_.period = (period == 0 ? Timing::PERIOD_PING : period);
  }
  inline void setPeriod(String periodSec)
  {
    setPeriod(1000 * (unsigned long)periodSec.toInt());
  }

  // Current pinging period
  inline unsigned long getPeriod() { return ping_.period; }

  // Enable pinging to the gateway immediately
  inline void enablePing()
  {
    ping_.flEnabled = true;
    ping_.tsPing = 0;
  }

  // Disable pinging to the gateway
  inline void disablePing() { ping_.flEnabled = false; }

  // Flag about enabled pinging to the DNS
  inline bool isPingEnabled() { return ping_.flEnabled; }

  // Flag about disabled pinging to the DNS
  inline bool isPingDisabled() { return !ping_.flEnabled; }

  // Success flag about wifi connection to AP
  inline bool isConnected() { return WiFi.isConnected(); }

  // Success flag about wifi connection and pinging to AP
  inline bool isContact() { return WiFi.isConnected() && ping_.flSuccess; }

  // Current RSSI value
  inline int getRssi() { return WiFi.RSSI(); }

  inline const char *getAddressIp() { return addressIp_; }
  inline const char *getAddressMac() { return addressMac_; }
  inline unsigned int getIdMac() { return idMac_; }
  inline const char *getHostname()
  {
    return isConnected() ? WiFi.getHostname() : wifi_.hostname;
  };
  inline String getStatus(int status = WiFi.status())
  {
    switch (status)
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

      case -1:
        return SERIAL_F("Timeout");
        break;

      default:
        return SERIAL_F("Uknown");
        break;
    }
    return statusText_;
  }

private:
  // Time periods
  enum Timing : unsigned long
  {
    PERIOD_TIMEOUT = 1 * 1000,
    PERIOD_WAIT = 5 * 1000,
    PERIOD_RECONNECT = 15 * 1000,
    PERIOD_PING = 1 * 60 * 1000,
  };

  // General parameters
  enum Params : byte
  {
    PARAM_WAITS = 20,
  };

  // Parameters of wifi
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

  // Parameters of the current connection phase
  struct Connection
  {
    // Number of remaining waits for connecting
    byte waits;

    // Timestamp of the last event
    unsigned long tsEvent;

    // Flag about waiting for the next connection phase
    bool flWait;

    // Flag about disconnecting
    bool flDisconnect;

    // Reset waiting data
    void reset()
    {
      waits = 0;
      flWait = false;
    }

    // Initialize the structure
    void init()
    {
      reset();
      tsEvent = 0;
    }

  } connection_;

  // Structure for pinging parameters
  struct Pinging
  {
    // Flag about enabled pinging to the gateway
    bool flEnabled = true;

    // Success flag about pinging to the gateway
    bool flSuccess;

    // Timestamp of last ping to the gateway
    unsigned long tsPing;

    // Period for pinging in milliseconds
    unsigned long period = Timing::PERIOD_PING;

  } ping_;

  char addressIp_[16];
  char addressMac_[18];
  char statusText_[30];
  unsigned int idMac_;
#if defined(ESP8266)
  WiFiEventHandler onGotIpHandler_, onDisconnectHandler_;
#endif

  // Connect to wifi procedure
  void connect();

  // Check connection to gateways
  void check();

  /**
   * Set the IP address of the MCU as a string.
   */
  inline void setAddressIp()
  {
    strcpy(addressIp_, WiFi.localIP().toString().c_str());
  }

  /**
   * Set the MAC address of the MCU as a string.
   */
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
