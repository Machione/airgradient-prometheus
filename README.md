# AirGradient Basic/DIY - Prometheus

I, like many others, mostly work from home, spending several hours in a small spare room of my house. In my part of the world, it is very uncommon to air condition your home, and for much of the year it is too cold to keep a window open for long periods of time. I was interested to see what impact that had on my mental performance and measure the affect of different ventilation approaches. Additionally, having woodworking as a hobby means that I'm also concerned about the amount of fine particulate matter that I inadvertently may be breathing in, even after the visible wood dust has dissipated.

Inspired by [Jeff Geerling's air quality monitoring setup](https://www.jeffgeerling.com/blog/2021/airgradient-diy-air-quality-monitor-co2-pm25), and being a sucker for DIY and soldering, I decided to invest in a couple of AirGradient's [DIY air sensors](https://www.airgradient.com/diy/). However, much like Jeff, I'd prefer not to send my data to AirGradient indefinitely, and would like to store that data locally, visualising it using Grafana.

This repository describes that process. It borrows heavily from [Jeff's own repository](https://github.com/geerlingguy/airgradient-prometheus), of which this is a fork. However, when trying to follow his approach directly, I noticed that his code allowed for more complex monitoring (including TVOC and NOx) which I have no need for. But perhaps more importantly, the code refused to compile, I suspect because the AirGradient Arduino Library that Jeff uses is now outdated. Jeff used 2.4.15 but 3.1.0 is the latest at time of writing, and it seems that the [move from v2 to v3](https://forum.airgradient.com/t/new-airgradient-arduino-library-version-3/1639) has, as expected, removed some backwards compatibility. Therefore, this repository also borrows from the [official AirGradient BASIC example Arduino code](https://github.com/airgradienthq/arduino/tree/master/examples/BASIC).

In addition to bringing Jeff's code up-to-date, this project aims to add a few enhanced features.

- Turn off the display during night time hours to extend its life. Measurement and the Prometheus server remain active even when the screen is off.
- Add a LED which blinks when PM2.5 levels go above a configurable threshold. This is to act as a visual warning/reminder when I'm in the workshop to put on/continue wearing a dust mask.
- Improved documentation.

## How it Works

If you're using the official AirGradient Arduino sketch (`C02_PM_SHT_OLED_WIFI`), you can configure it to enable WiFi and send data to a remote server every 9 seconds (as it cycles through the display of PM2.5, CO2, temperature, and humidity values).

By default, it sends a small JSON payload to AirGradient's servers, and you can monitor the data via their service.

This project configures the AirGradient sensor for local access (instead of delivering data to AirGradient's servers), and includes two configurations:

  1. [`AirGradient-DIY`](AirGradient-DIY/README.md): This is an Arduino sketch with all the code needed to set up an AirGradient sensor as a Prometheus endpoint on a WiFi network, suitable for scraping from any Prometheus instance (e.g. [geerlingguy/internet-pi](https://github.com/geerlingguy/internet-pi))
  2. [`AirGradient-ESPHome`](AirGradient-ESPHome/README.md): This is an ESPHome configuration which integrates the AirGradient sensor with Home Assistant using ESPHome.

Please see the README file in the respective configuration folder for more information about how to set up your AirGradient sensor.
