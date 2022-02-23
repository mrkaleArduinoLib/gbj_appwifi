#include "gbj_appwifi.h"

gbj_appwifi::ResultCodes gbj_appwifi::connect()
{
  // Call callback just once since connection lost
  if (status_.flConnGain)
  {
    SERIAL_TITLE("Connection lost")
    status_.reset();
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
      millis() < Timing::PERIOD_CYCLE)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // Wait for recovery period after failed connection
  if (status_.tsRetry && millis() - status_.tsRetry < Timing::PERIOD_SET)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_OFF); // Try this only if device cannot connect to AP
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.hostname(wifi_.hostname);
  if (wifi_.staticIp)
  {
    WiFi.config(wifi_.staticIp,
                wifi_.gateway,
                wifi_.subnet,
                wifi_.primaryDns,
                wifi_.secondaryDns);
  }
  WiFi.begin(wifi_.ssid, wifi_.pass);
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
    SERIAL_VALUE("SSID", wifi_.ssid)
    SERIAL_VALUE("Hostname", wifi_.hostname)
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI())
    SERIAL_VALUE("IP", WiFi.localIP())
    SERIAL_VALUE("MAC", getAddressMac())
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    status_.init();
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
  SERIAL_ACTION("mDNS starting...")
  if (MDNS.begin(getHostname()))
  {
    SERIAL_ACTION_END("Success")
    if (handlers_.onMdnsSuccess)
    {
      handlers_.onMdnsSuccess();
    }
    return setLastResult();
  }
  else
  {
    SERIAL_ACTION_END("Fail")
    if (handlers_.onMdnsFail)
    {
      handlers_.onMdnsFail();
    }
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
}