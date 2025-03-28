#include "gbj_appwifi.h"

void gbj_appwifi::connect()
{
  // Wait for reconnection after a while
  if (connection_.flDisconnect &&
      millis() - connection_.tsEvent < Timing::PERIOD_RECONNECT)
  {
    return;
  }

  // Wait after timout
  if (connection_.flWait &&
      millis() - connection_.tsEvent < Timing::PERIOD_WAIT)
  {
    return;
  }

  // Initiate connection
  if (connection_.waits == 0)
  {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(wifi_.hostname);
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
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
  }
  connection_.tsEvent = millis();
  connection_.flWait = true;
  connection_.flDisconnect = false;
  connection_.waits++;
  SERIAL_VALUE("Waitings", connection_.waits)
  int status = WiFi.waitForConnectResult(Timing::PERIOD_TIMEOUT);
  SERIAL_VALUE("Status", getStatus(status))

  // Connected successfully
  if (status == WL_CONNECTED)
  {
    return;
  }

  // All waitings exhausted
  if (connection_.waits >= Params::PARAM_WAITS)
  {
    connectFail();
  }
}

void gbj_appwifi::check()
{
  // Ping to the AP gateway
  if (ping_.flEnabled && millis() - ping_.tsPing > ping_.period)
  {
    ping_.flSuccess = pingGW();
    ping_.tsPing = millis();
    // Disconnect from wifi at false connection
    if (WiFi.isConnected() && !ping_.flSuccess)
    {
      SERIAL_TITLE("Disconnect at false connection")
      WiFi.disconnect();
    }
  }
}
