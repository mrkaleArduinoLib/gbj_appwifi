<a id="library"></a>

# gbj\_appwifi
This is an application library, which is used usually as a project library for particular PlatformIO project. It encapsulates the functionality of `connection to WiFi network`. The encapsulation provides following advantages:

* Functionality is hidden from the main sketch.
* The library follows the principle `separation of concern`.
* The library is reusable for various projects without need to code connection management.
* Update in library is valid for all involved projects.
* It specifies (inherits from) the parent application library `gbj_appcore`.
* It utilizes funcionality and error handling from the parent class.
* The library activates multicast DNS right after wifi connection.


## Fundamental functionality
* The connection to wifi is checked at every loop of a main sketch.
* If no connection to wifi is detected, the library starts the [connection process](#connection).


<a id="internals"></a>

## Internal parameters
Internal parameters are hard-coded in the library as enumerations and none of them have setters or getters associated.

* **Period of waiting for next connection attempt** (0.5 second): It is a time period, during which the system is waiting in blocking mode for next attempt to connect to wifi.
* **Number of failed connection attempts in the connection set** (20): It is a countdown for failed connections to wifi at blocking waiting. After reaching this number of connection fails, which represents a connection set, the library starts waiting for next set, but without blocking the system.
* **Period of waiting for next connection set** (1 minute): It is a time period since recent failed connection attempt of recent connection set, during which the system is waiting in non-blocking mode for next connection set of attempts.
* **Number of failed connection sets** (3): It is a countdown for failed connection sets to wifi at non-blocking waiting. After reaching this number of connection sets, which represents a connection cycle, the library restarts the microcontroller.
* **Number of MCU restarts** (3): It is number of restarts of the microcontroller, after which the library starts waiting for next connection cycle.
* **Period of waiting for next connection cycle** (1 hour): It is a time period since recent microcontroller restart of recent failed connection cycle, during which the system is waiting in non-blocking mode for next cycle of connections.


<a id="connection"></a>

## Connection process
The connection process is composed of 3 level aiming to be robust. It gives the chance either the microcontroller itself or the WiFi Access Point to recover from failure and when connect automatically. The connection process is controlled by [internal parameters](#internals).

1. **Set of connection attemps**. It is a number of subsequent failed connection attemps. The library tries to connect to AP. If it fails, it starts waiting in blocking mode for next attempt. If predefined number of connection attemps fails, the library starts waiting for next connection set. The connection set with waiting periods among connection attempts allow the microcontroller to consolidate its internals to establish connection. If a connection attemp is successful, the library breaks entire connection process and goes to connection checking mode.

2. **Cycle of connection sets**. It is a number of subsequent failed connection sets. After failed connection set the library restart the microcontroller and starts new connection cycle. If predefined number of connection cycles (microcontroller restarts) fails, the library starts waiting for next connection cycle. The connection cycle with waiting periods among connection sets allow the microcontroller to wait for a network WiFi router or access point to consolidate, restart, or so.

3. **Reccurent connection cycles**. It is a repeating processing of previous two levels of connection process. If a connection cycle fails, the library starts waiting for repeating connection process described before. The waiting period among connection cycles allow to manually resolve potential problems with a WiFi router or access point, its configuration, restarting, or so.


<a id="dependency"></a>

## Dependency

* **gbj\_appcore**: Parent library loaded from the file `gbj_appcore.h`.
* **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.

#### Espressif ESP8266 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **ESP8266WiFi.h**: Main include file for the wifi connection.
* **ESP8266mDNS.h**: Main include file for the mDNS.

#### Espressif ESP32 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **WiFi.h**: Main include file for the wifi connection.
* **ESPmDNS.h**: Main include file for the mDNS.

#### Particle platform
* **Particle.h**: Includes alternative (C++) data type definitions.

> Library is not intended to be utilized on platforms without WiFi capabality.


<a id="constants"></a>

## Constants
* **VERSION**: Name and semantic version of the library.

Other constants, enumerations, result codes, and error codes are inherited from the parent library.


<a id="interface"></a>

## Custom data types
* [Handler](#handler)
* [Handlers](#handlers)

## Interface
* [gbj_appwifi()](#gbj_appwifi)
* [run()](#run)
* [setRestarts()](#setRestarts)
* [getRestarts()](#getRestarts)
* [getHostname()](#getHostname)
* [getAddressIp()](#getAddressIp)
* [getAddressMac()](#getAddressMac)
* [getRssi()](#getRssi)
* [getFails()](#getFails)
* [isConnected()](#isConnected)
* [isMdns()](#isMdns)


<a id="handler"></a>

## Handler

#### Description
The template or the signature of a callback function, which is called at particular event in the processing. It can be utilized for instant communicating with other modules of the application (project).
* A handler is just a bare function without any input parameters and returning nothing.
* A handler can be declared just as `void` type.

#### Syntax
    typedef void Handler()

#### Parameters
None

#### Returns
None

#### See also
[Handlers](#handlers)

[Back to interface](#interface)


<a id="handlers"></a>

## Handlers

#### Description
Structure of pointers to handlers each for particular event in processing.
* Individual or all handlers do not need to be defined in the main sketch, just those that are useful.

#### Syntax
    struct Handlers
    {
      Handler *onConnectStart;
      Handler *onConnectTry;
      Handler *onConnectSuccess;
      Handler *onConnectFail;
      Handler *onDisconnect;
      Handler *onRestart;
      Handler *onMdnsSuccess;
      Handler *onMdnsFail;
    }

#### Parameters
* **onConnectStart**: Pointer to a callback function, which is called right before a new connection set.
* **onConnectTry**: Pointer to a callback function, which is called after every failed connection attempt. It allows to observe the pending connection set.
* **onConnectSuccess**: Pointer to a callback function, which is called right after successful connection to wifi.
* **onConnectFail**: Pointer to a callback function, which is called right after failed connection set.
* **onDisconnect**: Pointer to a callback function, which is called at lost of connection to wifi. It allows to create an alarm or a signal about it.
* **onRestart**: Pointer to a callback function, which is called right before microcontroller restart. It allows to do some actions related to it, e.g., increment and save number of restarts to the EEPROM.
* **onMdnsSuccess**: Pointer to a callback function, which is called right after successful starting mDNS service.
* **onMdnsFail**: Pointer to a callback function, which is called right after failed starting mDNS service.

#### Example
```cpp
void onWifiSuccess()
{
  ...
}
gbj_appwifi::Handlers handlersWifi = { .onConnectSuccess = onWifiSuccess };
gbj_appwifi wifi = gbj_appwifi(..., handlersDevice);
```

#### See also
[Handler](#handler)

[gbj_appwifi](#gbj_appwifi)

[Back to interface](#interface)


<a id="gbj_appwifi"></a>
## gbj_appwifi()

#### Description
Constructor creates the class instance object and initiates internal resources.
* It inputs credentials for a wifi network.

#### Syntax
    gbj_appwifi(const char *ssid, const char *pass, const char *hostname, Handlers handlers)

#### Parameters
* **ssid**: Pointer to the name of the wifi network to connect to.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **pass**: Pointer to the passphrase for the wifi network.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **hostname**: Pointer to the hostname for a device on the network as well as for the mDNS domain.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **handlers**: Pointer to a structure of callback functions. This structure as well as handlers should be defined in the main sketch.
  * *Data type*: Handlers
  * *Default value*: empty structure

#### Returns
Object performing connection and reconnection to the wifi network.

[Back to interface](#interface)


<a id="run"></a>

## run()

#### Description
The execution method should be called frequently, usually in the loop function of a sketch.
* The method connects to the wifi network at the very first calling it and reconnects to it if neccesary.
* After successful connection the method activates multicast DNS.
* If the serial connection is active, the library outputs flow of the connection and at success lists basic parameters of the connection to wifi.

[Back to interface](#interface)


<a id="setRestarts"></a>

## setRestarts()

#### Description
The method receiving the number of the previous microcontroller restarts in a pending failed connection process.
* It is internally used for evaluating in connection process and incremented or reset, if needed.
* It is usually read from the EEPROM.

#### Syntax
    void setRestarts(byte restarts)

#### Parameters
* **restarts**: Number of microcontroller restarts in a pending failed connection process.
  * *Valid values*: 0 ~ 255
  * *Default value*: none

#### Returns
None

#### See also
[getRestarts()](#getRestarts)

[Back to interface](#interface)


<a id="getRestarts"></a>

## getRestarts()

#### Description
The method returns the number of the microcontroller restarts in a pending failed connection process.
* It might be used in appropriate handler(s) for statistics or storing in the EEPROM.

#### Syntax
    byte getRestarts()

#### Parameters
None

#### Returns
Number of microcontroller restarts in pending failed connection process.

#### See also
[setRestarts()](#setRestarts)

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


<a id="getFails"></a>

## getFails()

#### Description
The method returns the number of failed connection sets during a failed connection cycle until the successful connection.
* It might be usef for some statistics in appropriate handler(s).

#### Syntax
    byte getFails()

#### Parameters
None

#### Returns
Number of failed connection cycles in pending failed connection process.

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


<a id="isMdns"></a>

## isMdns()

#### Description
The method returns a flag whether the microcontroller has successfully start the mDNS service.

#### Syntax
    bool isMdns()

#### Parameters
None

#### Returns
Flag about starting mDNS service on a network.

[Back to interface](#interface)
