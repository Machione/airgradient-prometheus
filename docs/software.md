# Software

We're going to be flashing custom software onto the WEMOS D1 Mini microcontroller. I suggest following [AirGradient's guide](https://www.airgradient.com/documentation/basic-setup-skills-and-equipment-needed-to-build-our-airgradient-diy-sensor/) on how to do this for the first time if that is not something you're familiar with.

The only part in the instructions above that is ambiguous is what to set the board to in the Arduino IDE. I found that `LOLIN(WEMOS) D1 R2 & mini` works. Go to Tools, Boards, esp8266 and find the board listed above. Note that you will first need to have installed the ESP8266 Platform using the board manager; see the instructions listed above.

Indeed, if this is your first time building the AirGradient sensor, I recommend you follow all their instructions, including [flashing the official software](https://www.airgradient.com/documentation/diy-v4/#software) to begin with, so that you can test that the sensor is working as intended before deviating from the norm.

## Prerequisites

- AirGradient library. Tested with version 3.1.3. Follow the instructions above to install this.
- NTPClient (by Fabrice Weinberg). Tested with version 3.2.1. Install in the same way as the AirGradient library.

## Configuration

At the top of the .ino file in this project, you will find a number of variables that will allow you to configure the functionality of the device to your liking. Change these settings as you desire.

Variable                          | Default       | Description
----------------------------------|---------------|------------
`PROMETHEUS_DEVICE_ID`            | "AirGradient" | The ID that will be published to Prometheus. Set this to something unique to your device, probably describing its location. Useful if you have more than one device since this ID can be used in Grafana to separate the readings.
`TEMPERATURE_CORRECTION_OFFSET`   | -3.5          | The offset to apply to the temperature readings. Since the temperature sensor is closely located to other components on the PCB, the reading can be slightly incorrect compared to the true air temperature. This applies a correction to the reading taken from the sensor, in degrees (either Celsius or Fahrenheit depending on the setting below). It's advised that you calibrate the readings of the AirGradient device to another, known-good temperature reading, in the device's final installation location, and then set this offset accordingly.
`USE_US_AQI`                      | false         | If `true` then PM2.5 measurements will be converted to the [United States' Air Quality Index](https://en.wikipedia.org/wiki/Air_quality_index#United_States) (AQI). If `false`, the PM2.5 readings will be in µg/㎥ (micro gram per metre cubed).
`USE_FAHRENHEIT`                  | false         | If `true` then temperature measurements will be converted to degrees Fahrenheit. If `false`, the temperature readings will be in degrees Celsius (AKA centigrade).
`DISP_UPDATE_INTERVAL`            | 30            | How often the OLED display will update to show the latest sensor readings, in seconds.
`SENSOR_CO2_UPDATE_INTERVAL`      | 10            | How often the CO2 readings will be taken from the sensor, in seconds.
`SENSOR_PM_UPDATE_INTERVAL`       | 10            | How often the PM2.5 readings will be taken from the sensor, in seconds.
`SENSOR_TEMP_HUM_UPDATE_INTERVAL` | 10            | How often the temperature and relative humidity readings will be taken from the sensor, in seconds.
`UTC_OFFSET`                      | 0             | Number of hours between your time zone and UTC (Coordinated Universal Time). Use <https://www.timeanddate.com/time/map/> to help you find out what this value should be. This allow the device to calculate the accurate local time in your location, so it can turn on/off the display at the correct time of day (see below).
`DISPLAY_TURN_ON_HOUR`            | 6             | The hour of the day (in 24 hour format) when the OLED display should turn on and warning LED should operate. If you want the display and warning LED to be _off all day_ then set `DISPLAY_TURN_ON_HOUR` to a higher number than `DISPLAY_TURN_OFF_HOUR`. If you want the display and warning LED to be _active all day_, then set `DISPLAY_TURN_ON_HOUR` to -1 and `DISPLAY_TURN_OFF_HOUR` to 25.
`DISPLAY_TURN_OFF_HOUR`           | 22            | The hour of the day (in 24 hour format) when the OLED display should turn off and warning LED should deactivate. If you want the display and warning LED to be _off all day_ then set `DISPLAY_TURN_ON_HOUR` to a higher number than `DISPLAY_TURN_OFF_HOUR`. If you want the display and warning LED to be _active all day_, then set `DISPLAY_TURN_ON_HOUR` to -1 and `DISPLAY_TURN_OFF_HOUR` to 25.
`STATUS_LED`                      | true          | Set to `false` if you have not connected a status LED to the device, or you want to turn off this functionality. If `true` then the status LED will activate; turning on or blinking when the configured sensor's reading rises above the thresholds defined below.
`STATUS_LED_PIN`                  | D7            | Set to the pin of the WEMOS D1 Mini microcontroller that the positive terminal of the status LED has been connected to.
`STATUS_CHECK_SENSOR`             | "co2"         | The sensor whose readings are compared to the threshold values below to determine whether the LED should be off, on, or flashing. One of `"co2"`, `"pm"`, `"temp"`, or `"hum"`.
`STATUS_WARNING_THRESHOLD_VALUE`  | 1000          | When the `STATUS_CHECK_SENSOR` reading rises above this value, then the status LED will turn on.
`STATUS_DANGER_THRESHOLD_VALUE`   | 1500          | When the `STATUS_CHECK_SENSOR` reading rises above this value, then the status LED will flash.

## Prometheus

If you haven't already, get Prometheus installed and running. There is a [helpful guide one the Prometheus website](https://prometheus.io/docs/prometheus/latest/getting_started/).

You should assign a static IP address for your AirGradient device. I find it's best to do this in your router's settings. Exactly how this is done will depend on your router's software, so do some Googling. Replace `your_device_ip_address` in the instructions that follow with the IP address you have set.

A server will be running on the device and will be serving the metrics from all the sensors in a format that Prometheus understands at port 9100. After powering on the device and connecting it to your WiFi network (see [AirGradient's build instructions](https://www.airgradient.com/documentation/diy-v4/)), you can test this by trying to access <http://your_device_ip_address:9100/metrics> in your browser. You should see some text that looks something like the following.

```text
# HELP pm02 Particulate Matter PM2.5 value, in μg/m³
# TYPE pm02 gauge
pm02{sensor="airgradient",id="AirGradient",mac="48:e7:29:58:1e:a2"}1
# HELP rco2 CO2 value, in ppm
# TYPE rco2 gauge
rco2{sensor="airgradient",id="AirGradient",mac="48:e7:29:58:1e:a2"}442
# HELP atmp Temperature, in degrees Celsius
# TYPE atmp gauge
atmp{sensor="airgradient",id="AirGradient",mac="48:e7:29:58:1e:a2"}20.5
# HELP rhum Relative humidity, in percent
# TYPE rhum gauge
rhum{sensor="airgradient",id="AirGradient",mac="48:e7:29:58:1e:a2"}51.2
```

Next, you can add the following to your `prometheus.yml` file, which tells Prometheus to scrape from the IP address above.

```yaml
scrape_configs:
  - job_name: "airgradient"
    scrape_interval: 60s
    static_configs:
      - targets: ["your_device_ip_address:9100"]
```

You may change the `scrape_interval` to be anything you like, although I wouldn't recommend setting it lower than the sensor update intervals defined above (i.e. `SENSOR_CO2_UPDATE_INTERVAL`, `SENSOR_PM_UPDATE_INTERVAL`, and `SENSOR_TEMP_HUM_UPDATE_INTERVAL`) since the information scraped by Prometheus won't update any more regularly than that.

You can also add more sensors to the list by adding more targets under the `static_configs`.

After saving the updated `prometheus.yml` file, you should see the target appearing in the Prometheus Web UI. Navigate to it, then click Targets under the Status dropdown in the menu bar. You should see the IP address of your device there with the "UP" state. If not, check the device is powered on, booted correctly, that it has been assigned the desired IP address (a reboot of the device and/or router may be required), that you can access it through your firewall by trying to access <http://your_device_ip_address:9100/metrics> from your browser, and that the `prometheus.yml` settings are correct (check for errors in the Prometheus logs). If all that fails, connect to the WEMOS D1 Mini to your Arduino Serial Monitor and check the logs for issues.

## Grafana

If you haven't already, get Grafana installed and running. Follow [their official guide](https://grafana.com/docs/grafana/latest/getting-started/build-first-dashboard) for instructions if required.

Navigate to your Grafana Web UI, go to Dashboards -> New -> Import. Copy the contents of [grafana.json](./grafana.json) into the "Import via dashboard JSON model" box. Click Load. Select your Prometheus data source. Optionally, you can give your dashboard a different name.

The dashboard is designed to automatically expand with new rows whenever new devices are connected. Each row/device is identified by the `PROMETHEUS_DEVICE_ID` you have set. You can use the filter at the top-left of the dashboard to select which devices to display metrics from.

You may need to adjust the units of the temperature and PM2.5 visualisations depending on how you have configured your device. I have opted for degrees Celsius and µg/㎥ since those are the defaults from `USE_FAHRENHEIT` and `USE_US_AQI`. If you change either of these variables to `true`, then Grafana will render the values provided to it using the incorrect units. You can change this by hovering over the top-right of the respective visualisation, clicking the ... menu, and clicking Edit. Scroll through the settings on the right panel until you find the setting for Unit under the category of Standard options. Remove the currently selected unit and optionally replace it with your unit of preference for that measurement. You should only need to do that once for the top row of the dashboard, and the same change will be applied to all other rows (if there are any) after you refresh the dashboard.
