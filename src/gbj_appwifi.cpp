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
  // Wait for connection
  if (status_.tsRetry && (millis() - status_.tsRetry) < Timing::PERIOD_CONN)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // Start connection
  WiFi.mode(WIFI_STA);
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
  SERIAL_VALUE("Connecting to", wifi_.ssid)
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
    SERIAL_LINE
    SERIAL_VALUE("Connection", "Success")
    params();
    SERIAL_VALUE("tries", Params::PARAM_TRIES - counter + 1)
    SERIAL_VALUE("fails", status_.fails)
    WiFi.setAutoReconnect(false);
    WiFi.persistent(true);
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
    SERIAL_LINE
    SERIAL_VALUE("Connection", "Fail")
    SERIAL_VALUE("fails", status_.fails)
    if (handlers_.onConnectFail)
    {
      handlers_.onConnectFail();
    }
    setLastResult(ResultCodes::ERROR_CONNECT);
  }
  return getLastResult();
}
