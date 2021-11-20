#include "gbj_appwifi.h"
const String gbj_appwifi::VERSION = "GBJ_APPWIFI 1.0.0";

gbj_appwifi::ResultCodes gbj_appwifi::connect()
{
  if (isConnected())
  {
    SERIAL_TITLE("Connected");
    return setLastResult();
  }
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_OFF); // Try this only if device cannot connect to AP
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname_);
  WiFi.begin(ssid_, pass_);
  uint8_t counter = Params::PARAM_ATTEMPS;
  SERIAL_ACTION("Connecting to AP...");
  if (fails_)
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      if (counter--)
      {
        delay(Timing::PERIOD_CONNECT);
      }
      else
      {
        SERIAL_ACTION_END("Timeout");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        fails_--;
        SERIAL_VALUE("fails", Params::PARAM_FAILS - fails_);
        return setLastResult(ResultCodes::ERROR_CONNECT);
      }
      SERIAL_DOT;
    }
    SERIAL_ACTION_END("Connected");
    SERIAL_VALUE("SSID", ssid_);
    SERIAL_VALUE("IP", WiFi.localIP());
    SERIAL_VALUE("Hostname", hostname_);
    SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI());
    SERIAL_VALUE("fails", Params::PARAM_FAILS - fails_);
    fails_ = Params::PARAM_FAILS;
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    setLastResult();
  }
  else
  {
    SERIAL_ACTION_END("Restart MCU");
    setLastResult(ResultCodes::ERROR_CONNECT);
    ESP.restart();
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