
# Hardware

## Initial Assembly

I bought version 4 of the [DIY Basic Kit](https://www.airgradient.com/shop/#!/DIY-Basic-Kit/p/447361353/category=166777529) directly from AirGradient. They offer other versions and kits, and there is an option to source the parts yourself.

I cannot do better than [AirGradient's own build instructions](https://www.airgradient.com/documentation/diy-v4/) when it comes to instructions on assembling the hardware. Indeed, I would recommend you follow them and even go so far as to [connect your kit to their dashboard](https://app.airgradient.com/onboarding/welcome) to ensure that all is functioning as intended before trying to modify away from the standard software.

The only gotcha when following the official instructions is, during the connection of the kit to the dashboard, you are asked to provide its serial number. The instructions tell you that when the kit starts up for the first time it will create a wireless access point with SSID in the format `airgradient-xxxxx` where `xxxxx` is the serial number you need. This is incorrect. Instead, the full serial number that you need will be displayed briefly while the kit is booting, after you have connected it to your WiFi. The OLED display will show something like this.

```text
Warm Up
Serial#
'yyyyyyyyy
yyy'
```

This twelve-character `yyyyyyyyyyyy` serial number is really what is required to connect your sensor to the AirGradient dashboard.

## Case

Instead of the official case, to protect the sensor from knocks and dust (especially since I intend to deploy one of them into a workshop) I decided to buy [a project enclosure from AliExpress](https://www.aliexpress.com/item/1005001304761174.html), in size 115-90-55, with ears. This allows for mounting the sensors wherever I wish, and the clear plastic cover means the OLED display can still be seen.

Drilling a few ventilation holes in the top and bottom for both airflow and the USB-C power cable is easy. Make sure you locate the ventilation holes close to where the fan of the PM sensor. For increased protection against dust and debris, I glued added some fine 120 mesh over the holes, with a small slit cut in one part to allow the USB cable through. Additionally, due to space constraints of my case, I opted for a right-angled USB-C cable.

## LED

Upon examining the [schematics from AirGradient](https://www.airgradient.com/documentation/diy-v4/#schematics), along with the pinout of the [WEMOS LOLIN D1 Mini](https://www.wemos.cc/en/latest/d1/d1_mini.html) and the [WEMOS 0.66" OLED shield](https://www.wemos.cc/en/latest/d1_mini_shield/oled_0_66.html), we can see that pins D0, D7, and D8 are unpopulated. Conveniently they are also exposed as open pads (holes) on the left hand side of the AirGradient PCB. Therefore, we can connect any of these to LED(s) to act as a visual queue to improve ventilation and/or don a dust mask.

You will need a LED and a corresponding resistor. The WEMOS D1 Mini operates at 3.3V, meaning that a 75Ω resistor should be sufficient for most LEDs, however I recommend you look up the specifications of your LED and calculate the resistor required yourself (there are plenty of free online calculators available for this purpose). I also used a LED panel mount to fit the LED neatly to the front of my project enclosure and some JST connectors to be able to disconnect the mounted LED easily from the PCB for assembly and disassembly.

Connect the positive side of the LED (the longer leg) to either D0, D7 or D8 on the AirGradient PCB. Connect the resistor to the negative side of the LED and then to ground (labelled G) on the AirGradient PCB.

The LED will illuminate whenever a given sensor's reading rises above a certain value (configured in [software settings](./software.md)). It will start to flash if the reading rises even further.
