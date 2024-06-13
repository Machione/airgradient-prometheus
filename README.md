# AirGradient Basic/DIY - Prometheus

I, like many others, mostly work from home, spending several hours in a small spare room of my house. In my part of the world, it is very uncommon to air condition your home, and for much of the year it is too cold to keep a window open for long periods of time. I was interested to see what impact that had on my mental performance and measure the affect of different ventilation approaches. Additionally, having woodworking as a hobby means that I'm also concerned about the amount of fine particulate matter that I inadvertently may be breathing in, even after the visible wood dust has dissipated.

Inspired by [Jeff Geerling's air quality monitoring setup](https://www.jeffgeerling.com/blog/2021/airgradient-diy-air-quality-monitor-co2-pm25), and being a sucker for DIY and soldering, I decided to invest in a couple of AirGradient's [DIY air sensors](https://www.airgradient.com/diy/). However, much like Jeff, I'd prefer not to send my data to AirGradient indefinitely, and would like to store that data locally using Prometheus and visualising it using Grafana.

This repository describes that process. It borrows heavily from [Jeff's own repository](https://github.com/geerlingguy/airgradient-prometheus), of which this is a fork. However, when trying to follow his approach directly, the code refused to compile. I suspect because the AirGradient Arduino Library that Jeff uses is now outdated. Jeff used 2.4.15 but 3.1.3 is the latest at time of writing, and it seems that the [move from v2 to v3](https://forum.airgradient.com/t/new-airgradient-arduino-library-version-3/1639) has, as expected, removed some backwards compatibility. Therefore, this repository also borrows heavily from the [official AirGradient BASIC example Arduino code](https://github.com/airgradienthq/arduino/tree/master/examples/BASIC). Essentially is is the official AirGradient code with all the phoning-home removed, and with a server instantiated and able to serve metrics to Prometheus when requested.

In addition to bringing Jeff's code up-to-date, this project aims to add a few enhanced features.

- Turn off the display during night time hours to extend its life. Air quality measurement and the Prometheus server remain active even when the screen is off.
- Add a LED which blinks when PM2.5 or CO2 levels go above a configurable threshold. This is to act as a visual warning/reminder when I'm in the workshop to put on/continue wearing a dust mask.
- Add an offset adjustment to the temperature sensor. Due to the proximity of the temperature and humidity module to other components, it can be affected by surrounding heat, usually causing it to read a degree or two higher than the true air temperature. This offset allows for this correction to be made depending on your own setup.
- Improved documentation.

## Hardware

### Initial Assembly

I bought version 4 of the [DIY Basic Kit](https://www.airgradient.com/shop/#!/DIY-Basic-Kit/p/447361353/category=166777529) directly from AirGradient. They offer other versions and kits, and there is an option to source the parts yourself. However I have not tested any of these alternatives with this project.

I cannot do better than [AirGradient's own build instructions](https://www.airgradient.com/documentation/diy-v4/) when it comes to instructions on assembling the hardware. Indeed, I would recommend you follow them and even go so far as to [connect your kit to their dashboard](https://app.airgradient.com/onboarding/welcome) to ensure that all is functioning as intended before trying to modify away from the standard software.

The only gotcha when following the official instructions that I found is that during the connection of the kit to the dashboard, you are asked to provide its serial number. The instructions tell you that when the kit starts up for the first time it will create a wireless access point with SSID in the format `airgradient-xxxxx` where `xxxxx` is the serial number you need. This is incorrect. Instead, the full serial number that you need will be displayed briefly while the kit is booting, after you have connected it to your WiFi. The OLED display will show something like this.

```
Warm Up
Serial#
'yyyyyyyyy
yyy'
```

This twelve-character `yyyyyyyyyyyy` serial number is really what is required to connect your sensor to the AirGradient dashboard.

### Case

Instead of the official case, to protect the sensor from knocks and dust (especially since I intend to deploy one of them into a workshop) I decided to buy [a project enclosure from AliExpress](https://www.aliexpress.com/item/1005001304761174.html), in size 115-90-55, with ears. This allows for mounting the sensors wherever I wish, and the clear plastic cover means the OLED display can still be seen. Drilling a few ventilation holes and one for the USB-C power cable is easy.

### LED

Upon examining the [schematics from AirGradient](https://www.airgradient.com/documentation/diy-v4/#schematics), along with the pinout of the [WEMOS LOLIN D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) and the [WEMOS 0.66" OLED shield](https://www.wemos.cc/en/latest/d1_mini_shield/oled_0_66.html), we can see that pins D0, D7, and D8 are unpopulated. Conveniently they are also exposed on the left hand side of the AirGradient PCB. Therefore, we can connect any of these to LED(s) to act as a visual queue to improve ventilation and/or don a dust mask.

You will need a LED of your choice and a corresponding resistor. The WEMOS D1 Mini operates at 3.3V, meaning that a 75Ω resistor should be sufficient for most LEDs, however I recommend you look up the specifications of your LED and calculate the resistor required yourself (there are plenty of free online calculators available for this purpose). I also used a LED panel mount to fit the LED neatly to the side of my project enclosure and some JST connectors to be able to disconnect the mounted LED easily from the PCB for assembly and disassembly.

Connect the positive side of the LED (the longer leg) to either D0, D7 or D8 on the AirGradient PCB. Connect the resistor to the negative side of the LED and then to ground (labelled G) on the Airgradient PCB.

The LED will illuminate whenever a given sensor's reading rises above a certain value (configured below). It will start to flash if the reading rises even further.

## Software

We're going to be flashing custom software onto the WEMOS D1 Mini microcontroller. I suggest following [AirGradient's guide](https://www.airgradient.com/documentation/basic-setup-skills-and-equipment-needed-to-build-our-airgradient-diy-sensor/) on how to do this for the first time if that is not something you're familiar with.

The only part in the instructions above that is ambiguous is what to set the board to in the Arduino IDE. I found that `LOLIN(WEMOS) D1 R2 & mini` works. Go to Tools, Boards, esp8266 and find the board listed above. Note that you will first need to have installed the ESP8266 Platform using the board manager; see the instructions listed above.

Indeed, if this is your first time building the AirGradient sensor, I recommend you follow all their instructions, including [flashing the official software](https://www.airgradient.com/documentation/diy-v4/#software) to begin with, so that you can test that the sensor is working as intended before deviating from the norm.

### Prerequisites

- AirGradient Library. Tested with 3.1.3. In the Arduino IDE, go to Tools, Manage Libraries, and then search for AirGradient and install the AirGradient library.
- NTPClient (by Fabrice Weinberg). Tested with 3.2.1.

### Configuration

At the top of the .ino file in this project, you will find a number of variables that will allow you to configure the functionality of the device to your liking. Change these settings as you desire.

Variable                          | Default       | Description
----------------------------------|---------------|------------
`PROMETHEUS_DEVICE_ID`            | "airgradient" | The ID that will be published to Prometheus. Set this to something unique to your device, probably describing its location. Useful if you have more than one device since this ID can be used in Grafana to separate the readings.
`TEMPERATURE_CORRECTION_OFFSET`   | -1.5          | The offset to apply to the temperature readings. Since the temperature sensor is closely located to other components on the PCB, the reading can be slightly incorrect compared to the true air temperature. This applies a correction to the reading taken from the sensor, in degrees (either Celsius or Fahrenheit depending on the setting below). It's advised that you calibrate the readings of the AirGradient device to another, known-good temperature reading, in the device's final installation location, and then set this offset accordingly.
`USE_US_AQI`                      | false         | If `true` then PM2.5 measurements will be converted to the [United States' Air Quality Index](https://en.wikipedia.org/wiki/Air_quality_index#United_States) (AQI). If `false`, the PM2.5 readings will be in µg/㎥ (micro gram per metre cubed).
`USE_FAHRENHEIT`                  | false         | If `true` then temperature measurements will be converted to degrees Fahrenheit. If `false`, the temperature readings will be in degrees Celsius (AKA centigrade).
`DISP_UPDATE_INTERVAL`            | 30            | How often the OLED display will update to show the latest sensor readings, in seconds.
`SENSOR_CO2_UPDATE_INTERVAL`      | 10            | How often the CO2 readings will be taken from the sensor, in seconds.
`SENSOR_PM_UPDATE_INTERVAL`       | 10            | How often the PM readings will be taken from the sensor, in seconds.
`SENSOR_TEMP_HUM_UPDATE_INTERVAL` | 10            | How often the temperature and humidity readings will be taken from the sensor, in seconds.
`UTC_OFFSET`                      | 0             | Number of hours offset between your time zone and UTC (Coordinated Universal Time). Use <https://www.timeanddate.com/time/map/> to help you find out what this value should be. This allow the device to calculate the accurate local time in your location, so it can turn on/off the display at the correct time of day (see below).
`DISPLAY_TURN_ON_HOUR`            | 6             | The hour of the day (in 24 hour format) when the OLED display should turn on and warning LED should operate. I.e. 6 -> turn on at 6 a.m. in the morning, 21 -> turn on at 9 p.m. at night. If you always want the display and warning LED to be _off_ then set `DISPLAY_TURN_ON_HOUR` to a higher number than `DISPLAY_TURN_OFF_HOUR`. If you always want the display and warning LED to be _active_, then set `DISPLAY_TURN_ON_HOUR` to -1 and `DISPLAY_TURN_OFF_HOUR` to 25.
`DISPLAY_TURN_OFF_HOUR`           | 22            | The hour of the day (in 24 hour format) when the OLED display should turn off and warning LED should deactivate. I.e. 6 -> turn off at 6 a.m. in the morning, 21 -> turn off at 9 p.m. at night. If you always want the display and warning LED to be _off_ then set `DISPLAY_TURN_ON_HOUR` to a higher number than `DISPLAY_TURN_OFF_HOUR`. If you always want the display and warning LED to be _active_, then set `DISPLAY_TURN_ON_HOUR` to -1 and `DISPLAY_TURN_OFF_HOUR` to 25.
`STATUS_LED`                      | true          | Set to `false` if you have not connected a status LED to the device, or you want to turn off this functionality. If `true` then the status LED will activate; turning on or blinking when the configured sensor's reading rises above the thresholds defined below.
`STATUS_LED_PIN`                  | D7            | Set to the pin of the WEMOS D1 Mini microcontroller that the positive terminal of the status LED has been connected to.
`STATUS_CHECK_SENSOR`             | "co2"         | The sensor whose readings are alerted using the status LED when values rise above the thresholds defined below. One of `"co2"`, `"pm"`, `"temp"`, or `"hum"`.
`STATUS_WARNING_THRESHOLD_VALUE`  | 1001          | When the `STATUS_CHECK_SENSOR` reading rises above this value, then the status LED will turn on.
`STATUS_DANGER_THRESHOLD_VALUE`   | 1501          | When the `STATUS_CHECK_SENSOR` reading rises above this value, then the status LED will flash.

## Prometheus

TODO: Add instructions

## Grafana

TODO: Add instructions