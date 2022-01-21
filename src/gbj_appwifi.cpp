#include "gbj_appwifi.h"
const String gbj_appwifi::VERSION = "GBJ_APPWIFI 1.1.0";

gbj_appwifi::ResultCodes gbj_appwifi::connect()
{
  // Call callback just once since connection lost
  if (status_.flConnGain)
  {
    SERIAL_TITLE("Connection lost")
    status_.flConnGain = false;
    if (handlers_.onDisconnect)
    {
      handlers_.onDisconnect();
    }
  }
  // Wait for mcu restart recovery period.
  // Expected restoring an access point after external wifi failure.
  // Ignore period, if the mcu has not been reset by software, e.g., by reset
  // button or firmware upload.
  if (getResetReason() == gbj_appwifi::BOOT_SOFT_RESTART &&
      status_.restarts >= Params::PARAM_RESTARTS &&
      millis() < Timing::PERIOD_RESTART)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // Wait for recovery period after failed connection
  if (status_.tsRetry && millis() - status_.tsRetry < Timing::PERIOD_CYCLE)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_OFF); // Try this only if device cannot connect to AP
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname_);
  WiFi.begin(ssid_, pass_);
  if (handlers_.onConnectStart)
  {
    handlers_.onConnectStart();
  }
  SERIAL_ACTION("Connection to AP...")
  byte counter = Params::PARAM_TRIES;
  while (WiFi.status() != WL_CONNECTED && counter--)
  {
    SERIAL_DOT
    if (handlers_.onConnectTry)
    {
      handlers_.onConnectTry();
    }
    delay(Timing::PERIOD_FAIL);
  }
  // Successful connection
  if (WiFi.status() == WL_CONNECTED)
  {
    setAddressIp();
    setAddressMac();
    SERIAL_ACTION_END("Success")
    SERIAL_VALUE("tries", Params::PARAM_TRIES - counter + 1)
    SERIAL_VALUE("fails", status_.fails)
    SERIAL_VALUE("SSID", ssid_)
    SERIAL_VALUE("Hostname", hostname_)
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI())
    SERIAL_VALUE("IP", WiFi.localIP())
    SERIAL_VALUE("MAC", getAddressMac())
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    status_.reset();
    status_.flConnGain = true;
    if (handlers_.onConnectSuccess)
    {
      handlers_.onConnectSuccess();
    }
    setLastResult(ResultCodes::SUCCESS);
  }
  // Failed connection
  else
  {
    status_.tsRetry = millis();
    status_.fails++;
    SERIAL_ACTION_END("Fail")
    SERIAL_VALUE("fails", status_.fails)
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    if (handlers_.onConnectFail)
    {
      handlers_.onConnectFail();
    }
    // Restart MCU
    if (status_.fails >= Params::PARAM_FAILS)
    {
      status_.restarts++;
      SERIAL_VALUE("Restart", status_.restarts)
      if (handlers_.onRestart)
      {
        handlers_.onRestart();
      }
      ESP.restart();
    }
    setLastResult(ResultCodes::ERROR_CONNECT);
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
    SERIAL_TITLE("mDNS started")
    if (handlers_.onMdnsSuccess)
    {
      handlers_.onMdnsSuccess();
    }
    return setLastResult();
  }
  else
  {
    SERIAL_TITLE("mDNS failed")
    if (handlers_.onMdnsFail)
    {
      handlers_.onMdnsFail();
    }
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
}