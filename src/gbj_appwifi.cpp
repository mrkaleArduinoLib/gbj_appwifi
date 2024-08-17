#include "gbj_appwifi.h"

void gbj_appwifi::connect()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  /*
  Right after boot or success handler the wating time is zero, which enables
  waiting for connect result in the main sketch loop as frequently as determined
  by timeout.
  Failure handler sets the wating time, so that mcu waits a while for next
  attempt to connection after previous attempt failed.
  */
  // Waiting for reconnection
  if (millis() - status_.tsEvent > status_.timeWait)
  {
    /*
    Zeroing waiting time right after delay determined by the failure handler
    enables waiting for connect result in the main sketch loop as frequently as
    determined by timeout again.
    */
    status_.timeWait = 0;
    // Start connection
    if (status_.flBegin)
    {
      /*
      Resetting begin flag ensures one-time start of wifi connection until
      succesful connection or failure handler. It enables waiting for connect
      result in the main sketch loop as frequently as determined by timeout.
      */
      status_.flBegin = false;
      status_.tsEvent = millis();
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
    }
    status_.waits++;
    SERIAL_VALUE("Waiting", status_.waits)
    WiFi.waitForConnectResult(Timing::PERIOD_TIMEOUT);
    if (WiFi.status() == WL_CONNECTED)
    {
      /*
       If there is no success handler implemented in the main sketch, it is
       simulated.
      */
      if (!status_.flHandlerSuccess)
      {
        connectSuccess();
      }
    }
    /*
     If there is no failure handler implemented in the main sketch, the safety
     counter simulates that handler.
    */
    else if (status_.waits >= Params::PARAM_SAFETY_WAITS)
    {
      connectFail();
    }
  }
}
