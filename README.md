<a id="library"></a>
# gbj\_appwifi
This is an application library, wich is used usually as a project library for particular PlatformIO project. However; in every project utilizing the wifi connection should be copied the same library, so that it is located in central library storage.

- Library specifies (inherits from) the system `gbj_appbase` library.
- Library utilizes error handling from the parent class.


<a id="dependency"></a>
## Dependency

- **gbj\_appbase**: Parent library for all application libraries loaded from the file `gbj_appbase.h`.
- **gbj\_timer**: Library for executing internal timer within an instance object loaded from the file `gbj_timer.h`.
- **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`.

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

#### Arduino platform
- **Arduino.h**: Main include file for the Arduino SDK.
- **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.

#### Espressif ESP8266 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **ESP8266WiFi.h**: Main include file for the wifi connection.

#### Espressif ESP32 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **WiFi.h**: Main include file for the wifi connection.


<a id="constants"></a>
## Constants

- **gbj\_appwifi::VERSION**: Name and semantic version of the library.

Other constants and enumerations are inherited from the parent library.


<a id="interface"></a>
## Interface

- [gbj_appwifi()](#gbj_appwifi)
- [run()](#run)
- [setPeriod()](#setPeriod)
- [getPeriod()](#getPeriod)
- [isConnected()](#isConnected)


<a id="gbj_appwifi"></a>
## gbj_appwifi()

#### Description
Constructor creates the class instance object and initiates internal resources.
- Constructor creates one internal timer without a timer handler.

#### Syntax
    gbj_appwifi(const char *ssid, const char *pass);

#### Parameters
- **ssid**: Pointer to the name of the wifi network to connect to.
  - *Valid values*: Constant pointer to string
  - *Default value*: none

- **pass**: Pointer to the password for the wifi network.
  - *Valid values*: Constant pointer to string
  - *Default value*: none

#### Returns
Object performing connection and reconnection to the wifi network.

[Back to interface](#interface)


<a id="run"></a>
## run()

#### Description
The execution method, which should be called frequently, usually in the loop function of a sketch.
- The method connects to the wifi network at the very first calling it.
- At the end of each timer period the method checks the connection to the wifi network and reconnects to it if neccesary.
- If the serial connection is active the library outputs flow of the connection and at success lists basic parameters of the connection to wifi.

#### Syntax
	void run();

#### Parameters
None

#### Returns
None

#### Example
Calling methods in the sketch loop.
```cpp
gbj_appwifi wifi = gbj_wifi("MySSID", "MyPassword");

void loop()
{
  wifi.run();
}
```

[Back to interface](#interface)


<a id="setPeriod"></a>
## setPeriod()

#### Description
The method sets a new interna timer period. It allows to dynamically change a frequency of wifi connection checking.

#### Syntax
    void setPeriod(unsigned long period)

#### Parameters
* **period**: Duration of a repeating interval in milliseconds.
  * *Valid values*: 0 ~ 2^32 * 1
  * *Default value*: none

#### Returns
None

#### See also
[getPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="getPeriod"></a>
## getPeriod()

#### Description
The method returns current internal timer period, i.e., frequency of the wifi connection check.

#### Syntax
    unsigned long getPeriod()

#### Parameters
None

#### Returns
Current timer period in milliseconds.

#### See also
[setPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="isConnected"></a>
## isConnected()

#### Description
The method returns a flag whether the device is connected to the wifi.

#### Syntax
    bool isConnected();

#### Parameters
None

#### Returns
Flag about connecting status to wifi.

[Back to interface](#interface)
