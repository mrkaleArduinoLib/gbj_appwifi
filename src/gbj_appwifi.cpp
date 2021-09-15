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
  WiFi.begin(_ssid, _pass);
  SERIAL_DELIM
  SERIAL_ACTION("Connecting to AP...");
  uint8_t counter = Timing::PERIOD_ATTEMPS;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (counter--)
    {
      delay(Timing::PERIOD_CONNECT);
    }
    else
    {
      SERIAL_ACTION_END("Timeout");
      return setLastResult(ResultCodes::ERROR_CONNECT);
    }
    SERIAL_DOT;
  }
  SERIAL_ACTION_END("Connected");
  SERIAL_VALUE("SSID", _ssid);
  SERIAL_VALUE("IP", WiFi.localIP());
  SERIAL_VALUE("RSSI(dBm)", WiFi.RSSI());
  SERIAL_DELIM;
  return setLastResult();
}
