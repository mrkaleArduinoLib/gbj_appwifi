#include "gbj_appwifi.h"
const String gbj_appwifi::VERSION = "GBJ_APPWIFI 1.1.0";

gbj_appwifi::ResultCodes gbj_appwifi::connect()
{
  // Call callback just once since connection lost
  if (status_.flConnGain)
  {
    SERIAL_TITLE("Connection lost");
    status_.flConnGain = false;
    if (handlers_.onDisconnect)
    {
      handlers_.onDisconnect();
    }
  }
  // Wait for restoring an access point at external wifi failure
  // for a time period since mcu boot and try to connect
  if (status_.restarts >= Params::PARAM_RESTARTS)
  {
    if (millis() > Timing::PERIOD_RESTART)
    {
      SERIAL_VALUE("New cycle after restarts", status_.restarts);
      status_.reset();
    }
    else
    {
      return setLastResult(ResultCodes::ERROR_NOINIT);
    }
  }
  // The first connection atempt immediately, next one after the retry period
  if (status_.tsRetry > 0 && millis() - status_.tsRetry < Timing::PERIOD_RETRY)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_OFF); // Try this only if device cannot connect to AP
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname_);
  WiFi.begin(ssid_, pass_);
  if (status_.fails < Params::PARAM_FAILS)
  {
    if (handlers_.onConnectStart)
    {
      handlers_.onConnectStart();
    }
    SERIAL_ACTION("Connecting to AP...");
    status_.tries = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (++status_.tries < Params::PARAM_TRIES)
      {
        if (handlers_.onConnectTry)
        {
          handlers_.onConnectTry();
        }
        delay(Timing::PERIOD_CONNECT);
      }
      else
      {
        SERIAL_ACTION_END("Timeout");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        status_.tsRetry = millis();
        status_.fails++;
        SERIAL_VALUE("tries", status_.tries);
        SERIAL_VALUE("fails", status_.fails);
        if (handlers_.onConnectFail)
        {
          handlers_.onConnectFail();
        }
        // Restart MCU
        if (status_.fails >= Params::PARAM_FAILS)
        {
          SERIAL_TITLE("Restart MCU");
          status_.restarts++;
          if (handlers_.onRestart)
          {
            handlers_.onRestart();
          }
          ESP.restart();
        }
        return setLastResult(ResultCodes::ERROR_CONNECT);
      }
      SERIAL_DOT;
    }
    setAddressIp();
    setAddressMac();
    SERIAL_ACTION_END("Connected");
    SERIAL_VALUE("tries", status_.tries);
    SERIAL_VALUE("fails", status_.fails);
    SERIAL_VALUE("SSID", ssid_);
    SERIAL_VALUE("Hostname", hostname_);
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI());
    SERIAL_VALUE("IP", WiFi.localIP());
    SERIAL_VALUE("MAC", getAddressMac());
    status_.reset();
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    mdns();
    if (handlers_.onConnectSuccess)
    {
      handlers_.onConnectSuccess();
    }
    status_.flConnGain = true;
  }
  return getLastResult();
}

gbj_appwifi::ResultCodes gbj_appwifi::mdns()
{
  if (!isConnected() || MDNS.isRunning())
  {
    return setLastResult();
  }
  // Start multicast DNS
  if (MDNS.begin(getHostname()))
  {
    SERIAL_TITLE("mDNS started");
    return setLastResult();
  }
  else
  {
    SERIAL_TITLE("mDNS failed");
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
}