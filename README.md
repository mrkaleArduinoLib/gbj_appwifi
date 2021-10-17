<a id="library"></a>

# gbj\_appwifi
This is an application library, which is used usually as a project library for particular PlatformIO project. However; in every project utilizing the wifi connection should be copied the same library, so that it is located in central library storage.

- Library specifies (inherits from) the application `gbj_appbase` library.
- Library utilizes error handling from the parent class.
- Library activates multicast DNS right after wifi connection.
- If the connection to Wifi network fails for 5 subsequent attempts with timeout, the library restarts the microcontroller.


<a id="dependency"></a>

## Dependency

- **gbj\_appbase**: Parent library loaded from the file `gbj_appbase.h`.
- **gbj\_timer**: Library for executing internal timer within an instance object loaded from the file `gbj_timer.h`.
- **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.

#### Espressif ESP8266 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **ESP8266WiFi.h**: Main include file for the wifi connection.
- **ESP8266mDNS.h**: Main include file for the mDNS.

#### Espressif ESP32 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **WiFi.h**: Main include file for the wifi connection.
- **ESPmDNS.h**: Main include file for the mDNS.

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

> Library is not intended to be utilized on platforms without WiFi capabality.


<a id="constants"></a>

## Constants

- **gbj\_appwifi::VERSION**: Name and semantic version of the library.

Other constants and enumerations are inherited from the parent library.


<a id="interface"></a>

## Interface

- [gbj_appwifi()](#gbj_appwifi)
- [run()](#run)
- [setPeriod()](#period)
- [getPeriod()](#period)
- [getHostname()](#getHostname)
- [isConnected()](#isConnected)


<a id="gbj_appwifi"></a>
## gbj_appwifi()

#### Description
Constructor creates the class instance object and initiates internal resources.
- It inputs credentials for a wifi network.
- It creates one internal timer without a timer handler.

#### Syntax
    gbj_appwifi(const char *ssid, const char *pass, const char *hostname)

#### Parameters

- **ssid**: Pointer to the name of the wifi network to connect to.
  - *Valid values*: Constant pointer to string
  - *Default value*: none


- **pass**: Pointer to the passphrase for the wifi network.
  - *Valid values*: Constant pointer to string
  - *Default value*: none


- **hostname**: Pointer to the hostname for a device on the network as well as for the mDNS domain.
  - *Valid values*: Constant pointer to string
  - *Default value*: none

#### Returns
Object performing connection and reconnection to the wifi network.

[Back to interface](#interface)


<a id="run"></a>

## run()

#### Description
The execution method as the implementation of the virtual method from parent class, which should be called frequently, usually in the loop function of a sketch.
- The method connects to the wifi network at the very first calling it.
- At the end of each timer period the method checks the connection to the wifi network and reconnects to it if neccesary.
- If the serial connection is active, the library outputs flow of the connection and at success lists basic parameters of the connection to wifi.

[Back to interface](#interface)


<a id="period"></a>

## setPeriod(), getPeriod()

#### Description
The methods are just straitforward implementation of the virual methods from the parent class.

[Back to interface](#interface)


<a id="getHostname"></a>

## getHostname()

#### Description
The method returns the hostname of the device connected to the wifi network. It might be used as a device's attribute for the IoT platform.

#### Syntax
    const char* getHostname()

#### Parameters
None

#### Returns
A pointer to the constant string with hostname of the device.

[Back to interface](#interface)


<a id="isConnected"></a>

## isConnected()

#### Description
The method returns a flag whether the device is connected to the wifi.

#### Syntax
    bool isConnected()

#### Parameters
None

#### Returns
Flag about connecting status to wifi.

[Back to interface](#interface)
