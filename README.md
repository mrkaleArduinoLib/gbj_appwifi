<a id="library"></a>

# gbj\_appwifi
This is an application library, which is used as a project specific library for particular PlatformIO project. It encapsulates the functionality of `connection to WiFi network`. The encapsulation provides following advantages:

* Functionality is hidden from the main sketch.
* The library follows the principle `separation of concerns`.
* The library is reusable for various projects without need to code connection management.
* Update in library is valid for all involved projects.
* It specifies (inherits from) the parent application library `gbj_appcore`.
* It utilizes funcionality and error handling from the parent class.


## Fundamental functionality
* The library enables to set a static (fixed) IP address of the microcontroller.
* The connection to wifi is checked at every loop of a main sketch.
* The library does not support multicast DNS on purpose, because it appeared as unreliable in praxis.
* If no connection to wifi is detected or has been lost, the library starts a new attempt to connect and tries to recover wifi connection.
* The library is designed for blocking the MCU at wifi connection establishment as little as possible, so that it utilizes a main sketch loop for it.
* The project library relies on the system library for wifi management as for reconnection repetation, event handlers firing, etc.
* The project library counts on handlers of type _WiFiEventHandler_ for events  _WiFiEventStationModeGotIP_ and _WiFiEventStationModeDisconnected_ at least.
* If _WiFiEventStationModeGotIP_ handler is not implemented in the main sketch or from some reasons it does not fire after successful connection, the library simulates its activation.
* If _WiFiEventStationModeDisconnected_ handler is not implemented in the main sketch or from some reasons it does not fire during repeating waiting for a connection result, the library simulates its activation by a safety counter after safety number of failed connection attempts.


<a id="internals"></a>

## Internal parameters
Internal parameters are hard-coded in the library as enumerations and none of them have setters or getters associated.

* **Timeout of waiting for connection result** (`1 second`): It is a time interval injected to the system wifi library method called in a loop, which is waiting for connection result.
* **Default waiting period for next connection attempt** (`15 seconds`): It is a time period since recent failed connection attempt, during which the system is waiting in non-blocking mode for next connection attempt. This time period does not have effect at permanent failures like wrong password or wifi network name. In that cases the wifi management and timeouts are under control of the system library.
* **Minimal waiting period for next connection attempt** (`5 seconds`): Minimal reasonable value, to which the input value is limited.
* **Maximal waiting period for next connection attempt** (`60 seconds`): Maximal reasonable value, to which the input value is limited.
* **Safety number of connection result waits** ('30'): Maximal number of waitings for connection result used by the safety counter for simulating the _WiFiEventStationModeDisconnected_ handler.


<a id="dependency"></a>

## Dependency

* **gbj\_appcore**: Parent library loaded from the file `gbj_appcore.h`.
* **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.

#### Espressif ESP8266 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **ESP8266WiFi.h**: Main include file for the wifi connection.

#### Espressif ESP32 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **WiFi.h**: Main include file for the wifi connection.

#### Particle platform
* **Particle.h**: Includes alternative (C++) data type definitions.

> Library is not intended to be utilized on platforms without WiFi capabality.


<a id="constants"></a>

## Constants
* **VERSION**: Name and semantic version of the library.

Other constants, enumerations, result codes, and error codes are inherited from the parent library.


<a id="interface"></a>

## Interface
* [gbj_appwifi()](#gbj_appwifi)
* [run()](#run)
* [connectSuccess()](#connectSuccess)
* [connectFail()](#connectFail)
* [getStatus()](#getStatus)
* [getEventMillis()](#getEventMillis)
* [getHostname()](#getHostname)
* [getAddressIp()](#getAddressIp)
* [getAddressMac()](#getAddressMac)
* [getRssi()](#getRssi)
* [getBeginMillis()](#getBeginMillis)
* [getPeriod()](#getPeriod)
* [setPeriod()](#setPeriod)
* [isConnected()](#isConnected)


<a id="gbj_appwifi"></a>
## gbj_appwifi()

#### Description
Overloaded constructor creates the class instance object and initiates internal resources.
* It inputs credentials for a wifi network.
* It enables set network parameters for setting the static IP address.

#### Syntax
    gbj_appwifi(const char *ssid, const char *pass, const char *hostname)
    gbj_appwifi(const char *ssid, const char *pass, const char *hostname,
      const IPAddress staticIp, const IPAddress gateway, const IPAddress subnet,
      const IPAddress primaryDns, const IPAddress secondaryDns)

#### Parameters
* **ssid**: Pointer to the name of the wifi network to connect to.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **pass**: Pointer to the passphrase for the wifi network.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **hostname**: Pointer to the hostname for a device on the network.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **staticIp**: IP address to be set as static (fixed) one for the microcontroller. It consists from 4 octets in case of IPv4 addressing and is usually defined as compiler macro in a main sketch.
  * *Data type*: IPAddress
  * *Default value*: none


* **gateway**: IP address of the gateway (router) in the network.
  * *Data type*: IPAddress
  * *Default value*: none


* **subnet**: Subnet mask of the network. It consists from 4 octets in case of IPv4 addressing.
  * *Data type*: IPAddress
  * *Default value*: none


* **primaryDns**: Optional IP address of the primary DNS server used. It can be the address of the gateway, if it ensures DNS connectivity.
  * *Data type*: IPAddress
  * *Default value*: empty address


* **secondaryDns**: Optional IP address of the secondary DNS server used. It is empty, if the gateway ensures DNS connectivity.
  * *Data type*: IPAddress
  * *Default value*: empty address

#### Returns
Object performing connection and reconnection to the wifi network.

#### Example
For case with dynamic IP address assigned by a DHCP server:
```cpp
gbj_appwifi wifi = gbj_appwifi(WIFI_SSID, WIFI_PASS, WIFI_HOSTNAME);
```

For case with static IP address assigned in the firmware and defined usually by compiler macros:
```cpp
gbj_appwifi wifi = gbj_appwifi(WIFI_SSID,
                               WIFI_PASS,
                               WIFI_HOSTNAME,
                               IPAddress(IP_STATIC),
                               IPAddress(IP_GATEWAY),
                               IPAddress(IP_SUBNET),
                               IPAddress(IP_DNS_PRIMARY),
                               IPAddress(IP_DNS_SECONDARY));
```

For case with static IP address assigned in the firmware and defined directly in a main sketch without DNS servers and handlers:
```cpp
gbj_appwifi wifi = gbj_appwifi(WIFI_SSID,
                               WIFI_PASS,
                               WIFI_HOSTNAME,
                               IPAddress(192, 168, 0, 20),
                               IPAddress(192, 168, 0, 1),
                               IPAddress(255, 255, 255, 0));
```

[Back to interface](#interface)


<a id="run"></a>

## run()

#### Description
The execution method should be called frequently, usually in the loop function of a main sketch.
* The method connects to the wifi network at the very first calling it and reconnects to it if neccesary.
* If the serial connection is active, the library outputs flow of the connection and at success lists basic parameters of the connection to wifi.

#### Syntax
    void run()

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="connectSuccess"></a>

## connectSuccess()

#### Description
The method processes the successful connection to wifi network.
* The method should be called in handler for the event _WiFiEventStationModeGotIP_.
* If the serial connection is active, the method outputs basic parameters of the connection to wifi.

#### Syntax
    void connectSuccess()

#### Parameters
None

#### Returns
None

#### Example
For handler as a lambda function.
```cpp
WiFiEventHandler wifiConnectHandler;
void setup()
{
  wifiConnectHandler = WiFi.onStationModeGotIP(
    [](const WiFiEventStationModeGotIP &event)
    {
      wifi.connectSuccess();
    });
}
```
For handler as a separate function.
```cpp
WiFiEventHandler wifiConnectHandler;
void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  wifi.connectSuccess();
}
void setup()
{
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
}
```

#### See also
[connectFail()](#connectFail)

[Back to interface](#interface)


<a id="connectFail"></a>

## connectFail()

#### Description
The method processes the failed connection to wifi network.
* The method should be called in handler for the event _WiFiEventStationModeDisconnected_.
* If the serial connection is active, the method outputs the reason status of failed connection to wifi.

#### Syntax
    void connectFail()

#### Parameters
None

#### Returns
None

#### Example
For handler as a lambda function.
```cpp
WiFiEventHandler wifiDisconnectHandler;
void setup()
{
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(
    [](const WiFiEventStationModeDisconnected &event)
    {
      wifi.connectFail();
    });
}
```
For handler as a separate function.
```cpp
WiFiEventHandler wifiDisconnectHandler;
void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
  wifi.connectFail();
}
void setup()
{
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
}
```

#### See also
[connectSuccess()](#connectSuccess)

[Back to interface](#interface)


<a id="getEventMillis"></a>

## getEventMillis()

#### Description
The method returns the timestamp of the recent event, either successfull connection, sudden disconnection, or failed connection attempt to wifi.

#### Syntax
    unsigned long getEventMillis()

#### Parameters
None

#### Returns
A timestamp of the recent wifi event.

[Back to interface](#interface)


<a id="getStatus"></a>

## getStatus()

#### Description
The method returns recent status of the connection to wifi in textual form.

#### Syntax
    String getStatus()

#### Parameters
None

#### Returns
A textual wifi status desription.

[Back to interface](#interface)


<a id="getHostname"></a>

## getHostname()

#### Description
The method returns the hostname of the microcontroller connected to the wifi network.
* It might be used as a microcontroller's identifier on a network.

#### Syntax
    const char* getHostname()

#### Parameters
None

#### Returns
A pointer to the constant string with hostname of the microcontroller on the network.

#### See also
[getAddressIp()](#getAddressIp)

[getAddressMac()](#getAddressMac)

[Back to interface](#interface)


<a id="getAddressIp"></a>

## getAddressIp()

#### Description
The method returns the IPv4 address of the microcontroller connected to the wifi network.
* It may changed at wifi reconnection, if no static address is set.

#### Syntax
    const char* getAddressIp()

#### Parameters
None

#### Returns
A pointer to the constant string with IPv4 address of the microcontroller on the network.

#### See also
[getAddressMac()](#getAddressMac)

[getHostname()](#getHostname)

[Back to interface](#interface)


<a id="getAddressMac"></a>

## getAddressMac()

#### Description
The method returns the MAC address of the microcontroller connected to the wifi network.
* It might be used as a microcontroller's hardware identifier or for setting a static IP address on a network.

#### Syntax
    const char* getAddressMac()

#### Parameters
None

#### Returns
A pointer to the constant string with MAC address of the microcontroller.

#### See also
[getAddressMac()](#getAddressMac)

[getHostname()](#getHostname)

[Back to interface](#interface)


<a id="getRssi"></a>

## getRssi()

#### Description
The method returns the current WiFi _Received Signal Strength Indicator_ (RSSI) of the microcontroller connected to the wifi network. Values are negative and in _decibel milliwats_ (dBm).

#### Syntax
    int getRssi()

#### Parameters
None

#### Returns
Current wifi signal strength of the microcontroller in _dBm_.

[Back to interface](#interface)


<a id="getBeginMillis"></a>

## getBeginMillis()

#### Description
The method returns timestamp of the recent begining of connection to a wifi network in milliseconds.

#### Syntax
    unsigned long getBeginMillis()

#### Parameters
None

#### Returns
Recent timestamp of the wifi connection in milliseconds.

[Back to interface](#interface)


<a id="getPeriod"></a>

## getPeriod()

#### Description
The method returns current waiting period.

#### Syntax
    unsigned long getPeriod()

#### Parameters
None

#### Returns
Current waiting period in milliseconds.

#### See also
[setPeriod()](#setPeriod)

[Back to interface](#interface)


<a id="setPeriod"></a>

## setPeriod()

#### Description
The overloaded method sets a new waiting period for a next reconnection attempt to a wifi network after failed previous one. The period can set in milliseconds or seconds.
* The method with numerical input argument is aimed for input in milliseconds.
* The method with textual input argument is aimed for input in seconds. It is useful with conjunction with a project data hub, which data has always string data type.
* If input period is zero or not numerical (leading to zero), the library sets the [internal default period](#internals).
* The input period is sanitized and constraint for [minimal and maximal value](#internals).


#### Syntax
    void setPeriod(unsigned long period)
    void setPeriod(String periodSec)

#### Parameters
* **period**: Duration of the waiting period in milliseconds.
  * *Valid values*: 0 ~ 2^32 - 1
  * *Default value*: none


* **periodSec**: Duration of the waiting period in seconds declared as string.
  * *Valid values*: String
  * *Default value*: none

#### Returns
None

#### See also
[getPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="isConnected"></a>

## isConnected()

#### Description
The method returns a flag whether the microcontroller is connected to the wifi.

#### Syntax
    bool isConnected()

#### Parameters
None

#### Returns
Flag about connecting status to wifi.

[Back to interface](#interface)
