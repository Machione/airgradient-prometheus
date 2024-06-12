# AirGradient Basic/DIY - Prometheus

I, like many others, mostly work from home, spending several hours in a small spare room of my house. In my part of the world, it is very uncommon to air condition your home, and for much of the year it is too cold to keep a window open for long periods of time. I was interested to see what impact that had on my mental performance and measure the affect of different ventilation approaches. Additionally, having woodworking as a hobby means that I'm also concerned about the amount of fine particulate matter that I inadvertently may be breathing in, even after the visible wood dust has dissipated.

Inspired by [Jeff Geerling's air quality monitoring setup](https://www.jeffgeerling.com/blog/2021/airgradient-diy-air-quality-monitor-co2-pm25), and being a sucker for DIY and soldering, I decided to invest in a couple of AirGradient's [DIY air sensors](https://www.airgradient.com/diy/). However, much like Jeff, I'd prefer not to send my data to AirGradient indefinitely, and would like to store that data locally using Prometheus and visualising it using Grafana.

This repository describes that process. It borrows heavily from [Jeff's own repository](https://github.com/geerlingguy/airgradient-prometheus), of which this is a fork. However, when trying to follow his approach directly, the code refused to compile. I suspect because the AirGradient Arduino Library that Jeff uses is now outdated. Jeff used 2.4.15 but 3.1.3 is the latest at time of writing, and it seems that the [move from v2 to v3](https://forum.airgradient.com/t/new-airgradient-arduino-library-version-3/1639) has, as expected, removed some backwards compatibility. Therefore, this repository also borrows heavily from the [official AirGradient BASIC example Arduino code](https://github.com/airgradienthq/arduino/tree/master/examples/BASIC).

In addition to bringing Jeff's code up-to-date, this project aims to add a few enhanced features.

- Turn off the display during night time hours to extend its life. Air quality measurement and the Prometheus server remain active even when the screen is off.
- Add a LED which blinks when PM2.5 or CO2 levels go above a configurable threshold. This is to act as a visual warning/reminder when I'm in the workshop to put on/continue wearing a dust mask.
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

Upon examining the [schematics from AirGradient](https://www.airgradient.com/documentation/diy-v4/#schematics), along with the pinout of the [WEMOS LOLIN D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) and the [WEMOS 0.66" OLED shield](https://www.wemos.cc/en/latest/d1_mini_shield/oled_0_66.html), we can see that pins D0 (GPIO 16), D7 (GPIO 13), and D8 (GPIO 15) are unpopulated. Conveniently they are also exposed on the left hand side of the AirGradient PCB.

Therefore, we can connect any/all of these to LEDs to act as a visual queue to improve ventilation and/or don a dust mask.

## Software

We're going to be flashing custom software onto the WEMOS D1 Mini microcontroller. I suggest following [AirGradient's guide](https://www.airgradient.com/documentation/basic-setup-skills-and-equipment-needed-to-build-our-airgradient-diy-sensor/) on how to do this for the first time if that is not something you're familiar with.

The only part in the instructions above that is ambiguous is what to set the board to in the Arduino IDE. I found that `LOLIN(WEMOS) D1 R2 & mini` works. Go to Tools, Boards, esp8266 and find the board listed above. Note that you will first need to have installed the ESP8266 Platform using the board manager; see the instructions listed above.

Indeed, if this is your first time building the AirGradient sensor, I recommend you follow all their instructions, including [flashing the official software](https://www.airgradient.com/documentation/diy-v4/#software) to begin with, so that you can test that the sensor is working as intended before deviating from the norm.

### Prerequisites

AirGradient Library. Tested with 3.1.3. In the Arduino IDE, go to Tools, Manage Libraries, and then search for AirGradient and install the AirGradient library.

NTPClient (by Fabrice Weinberg). Tested with 3.2.1.

### Configuration

At the top of the .ino file in this project, you will find a number of variables that will allow you to configure the functionality of the sensor to your liking.

Variable                          | Default     | Description
----------------------------------|-------------|------------
`WIFI_SSID`                       | N/A         |
`WIFI_PASSWORD`                   | N/A         |
`DEVICE_ID`                       | airgradient |
`DISP_UPDATE_INTERVAL`            | 30000       | How often the OLED display updates. In milliseconds.
`UTC_HOUR_OFFSET`                 | 1           |
`DISPLAY_START_HOUR`              | 7           | 
`DISPLAY_END_HOUR`                | 23          | 
`SENSOR_PM_UPDATE_INTERVAL`       | 10000       | How often to get a reading from the PM sensor. In milliseconds.
`SENSOR_CO2_UPDATE_INTERVAL`      | 10000       | How often to get a reading from the CO2 sensor. In milliseconds.
`SENSOR_TEMP_HUM_UPDATE_INTERVAL` | 10000       | How often to get a reading from the temperature and humidity sensor. In milliseconds.

## Prometheus

TODO: Add instructions

## Grafana

TODO: Add instructions