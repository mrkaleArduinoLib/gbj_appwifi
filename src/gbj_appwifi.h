/*
  NAME:
  gbj_appwifi

  DESCRIPTION:
  Application library for processing connection to an access point over WiFi.
  It is a substitution for project libraries, which should be same for the wifi.

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

#if defined(__AVR__)
  #include <Arduino.h>
  #include <inttypes.h>
#elif defined(ESP8266)
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <Arduino.h>
  #include <WiFi.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif
#include "gbj_appbase.h"
#include "gbj_serial_debug.h"
#include "gbj_timer.h"

class gbj_appwifi : gbj_appbase
{
public:
  static const String VERSION;

  /*
    Constructor.

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

    RETURN: object
  */
  inline gbj_appwifi(const char *ssid, const char *pass)
  {
    _ssid = ssid;
    _pass = pass;
    _timer = new gbj_timer(Timing::PERIOD_CHECK, 0, true);
  }

  /*
    Processing.

    DESCRIPTION:
    The method should be called in an application sketch loop.
    It processes main functionality and is control by the internal timer.

    PARAMETERS: None

    RETURN: none
  */
  inline void run()
  {
    if (_timer->run())
    {
      connect();
    }
  }

  // Setters
  inline void setPeriod(unsigned long period) { _timer->setPeriod(period); };

  // Getters
  inline unsigned long getPeriod() { return _timer->getPeriod(); };
  inline bool isConnected() { return WiFi.isConnected(); }

private:
  enum Timing : unsigned int
  {
    PERIOD_ATTEMPS = 20,
    PERIOD_CONNECT = 500,
    PERIOD_CHECK = 7345, // Avoid useless collisions
  };
  const char *_ssid;
  const char *_pass;
  gbj_timer *_timer;
  ResultCodes connect();
};

#endif
