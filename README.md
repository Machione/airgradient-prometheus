# AirGradient Basic/DIY - Prometheus

I, like many others, mostly work from home, spending several hours in a small spare room of my house. In my part of the world, it is very uncommon to air condition your home, and for much of the year it is too cold to keep a window open for long periods of time. I was interested to see what impact that had on my mental performance and measure the affect of different ventilation approaches. Additionally, having woodworking as a hobby means that I'm also concerned about the amount of fine particulate matter that I inadvertently may be breathing in, even after the visible wood dust has dissipated.

Inspired by [Jeff Geerling's air quality monitoring setup](https://www.jeffgeerling.com/blog/2021/airgradient-diy-air-quality-monitor-co2-pm25), and being a sucker for DIY and soldering, I decided to invest in a couple of AirGradient's [DIY air sensors](https://www.airgradient.com/diy/). However, much like Jeff, I'd prefer not to send my data to AirGradient indefinitely, and would like to store that data locally, visualising it using Grafana.

This repository describes that process. It borrows heavily from [Jeff's own repository](https://github.com/geerlingguy/airgradient-prometheus), of which this is a fork. However, when trying to follow his approach directly, I noticed that his code allowed for more complex monitoring (including TVOC and NOx) which I have no need for. But perhaps more importantly, the code refused to compile, I suspect because the AirGradient Arduino Library that Jeff uses is now outdated. Jeff used 2.4.15 but 3.1.0 is the latest at time of writing, and it seems that the [move from v2 to v3](https://forum.airgradient.com/t/new-airgradient-arduino-library-version-3/1639) has, as expected, removed some backwards compatibility. Therefore, this repository also borrows from the [official AirGradient BASIC example Arduino code](https://github.com/airgradienthq/arduino/tree/master/examples/BASIC).

In addition to bringing Jeff's code up-to-date, this project aims to add a few enhanced features.

- Turn off the display during night time hours to extend its life. Measurement and the Prometheus server remain active even when the screen is off.
- Add a LED which blinks when PM2.5 levels go above a configurable threshold. This is to act as a visual warning/reminder when I'm in the workshop to put on/continue wearing a dust mask.
- Improved documentation.

## Hardware

I bought version 4 of the [DIY Basic Kit](https://www.airgradient.com/shop/#!/DIY-Basic-Kit/p/447361353/category=166777529) directly from AirGradient. They offer other versions, kits, and there is an option to source the parts yourself. However I have not tested any of these alternatives with this project.

I cannot do better than [AirGradient's own build instructions](https://www.airgradient.com/documentation/diy-v4/) when it comes to instructions on assembling the hardware. Indeed, I would recommend you follow them and even go so far as to [connect your kit to their dashboard](https://app.airgradient.com/onboarding/welcome) to ensure that all is functioning as intended before trying to modify aware from the standard software.

The only gotcha when following the official instructions that I found is that during the connection of the kit to the dashboard, you are asked to provide its serial number. The instructions tell you that when the kit starts up for the first time it will create a wireless access point with SSID in the format `airgradient-xxxxx` where `xxxxx` is the serial number you need. This is incorrect. Instead, the full serial number that you need will be displayed briefly while the kit is booting after you have connected it to your WiFi. The OLED display will show something like this.

```
Warm Up
Serial#
'yyyyyyyyy
yyy'
```

This twelve-character `yyyyyyyyyyyy` serial number is really what is required to connect your sensor to the AirGradient dashboard.

Additionally, instead of the official case, to protect the sensor from knocks and dust (especially since I intend to deploy one of the m into a workshop) I decided to buy [a project enclosure from AliExpress](https://www.aliexpress.com/item/1005001304761174.html), in size 115-90-55, with ears. This allows for mounting the sensors wherever I wish, and the clear plastic cover means the OLED display can still be seen. Drilling a few ventilation holes and one for the USB-C power cable should be easy.

## How it Works

If you're using the official AirGradient Arduino sketch (`C02_PM_SHT_OLED_WIFI`), you can configure it to enable WiFi and send data to a remote server every 9 seconds (as it cycles through the display of PM2.5, CO2, temperature, and humidity values).

By default, it sends a small JSON payload to AirGradient's servers, and you can monitor the data via their service.

This project configures the AirGradient sensor for local access (instead of delivering data to AirGradient's servers), and includes two configurations:

  1. [`AirGradient-DIY`](AirGradient-DIY/README.md): This is an Arduino sketch with all the code needed to set up an AirGradient sensor as a Prometheus endpoint on a WiFi network, suitable for scraping from any Prometheus instance (e.g. [geerlingguy/internet-pi](https://github.com/geerlingguy/internet-pi))
  2. [`AirGradient-ESPHome`](AirGradient-ESPHome/README.md): This is an ESPHome configuration which integrates the AirGradient sensor with Home Assistant using ESPHome.

Please see the README file in the respective configuration folder for more information about how to set up your AirGradient sensor.
