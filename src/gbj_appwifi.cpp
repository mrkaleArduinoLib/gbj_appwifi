#include "gbj_appwifi.h"
const String gbj_appwifi::VERSION = "GBJ_APPWIFI 1.0.0";

gbj_appwifi::ResultCodes gbj_appwifi::connect()
{
  if (isConnected())
  {
    return setLastResult();
  }
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  // WiFi.persistent(false);
  // WiFi.mode(WIFI_OFF); // Try this only if device cannot connect to AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _pass);
  SERIAL_DELIM
  SERIAL_ACTION("gbj_appwifi::Connecting to AP...");
  uint8_t counter = Timing::PERIOD_ATTEMPS;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (counter--)
    {
      delay(Timing::PERIOD_CONNECT);
    }
    else
    {
      SERIAL_TITLE("Timeout");
      return setLastResult(ResultCodes::ERROR_CONNECT);
    }
    SERIAL_DOT;
  }
  SERIAL_TITLE("Connected");
  SERIAL_VALUE("gbj_appwifi::SSID", _ssid);
  SERIAL_VALUE("gbj_appwifi::IP", WiFi.localIP());
  SERIAL_VALUE("gbj_appwifi::RSSI (dBm)", WiFi.RSSI());
  SERIAL_DELIM;
  return setLastResult();
}
