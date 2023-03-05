#include "gbj_appwifi.h"

void gbj_appwifi::connect()
{
  // Already connected
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  // Waiting for reconnection
  if (millis() - status_.tsEvent > status_.timeWait)
  {
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
    SERIAL_VALUE("Connecting to", wifi_.ssid)
    WiFi.begin(wifi_.ssid, wifi_.pass);
    status_.flLoop = true; // Reset in the disconnect handler
    while (WiFi.waitForConnectResult(Timing::PERIOD_TIMEOUT) != WL_CONNECTED &&
           status_.flLoop)
    {
      SERIAL_DOT
    }
  }
}
