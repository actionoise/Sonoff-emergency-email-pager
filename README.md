# Sonoff Emergency Email Pager

Wi-Fi emergency pager made by recycling an old SONOFF device with a faulty internal relay.

Instead of throwing it away, the hardware was reused as an emergency button system with a web configuration portal and automatic email alerts.

When the button is pressed, the device sends an email notification to the configured recipient.

---

## Features

* ESP8266-based emergency button system
* Sends an email when the emergency button is pressed
* Configurable Wi-Fi settings
* Configurable SMTP email settings
* Web configuration portal
* Automatic fallback Access Point mode
* Configuration page available at `192.168.4.1` if Wi-Fi connection fails
* Device IP shown in the Serial Monitor when connected to Wi-Fi
* Device IP can also be sent by email
* Optional repeated emergency email notifications
* EEPROM storage for saved settings
* LED status indication

---

## How It Works

At startup, the ESP8266 tries to connect to the saved Wi-Fi network.

If the connection is successful, the device starts a web server on its local network IP address.

The IP address is printed in the Serial Monitor, for example:



The configuration page can then be opened from a phone or computer connected to the same Wi-Fi network.

If the ESP8266 cannot connect to the saved Wi-Fi network, it automatically starts its own Wi-Fi Access Point.

Default Access Point name:

```text
EmergencyEmail001
```

Fallback configuration page:

```text
http://192.168.4.1/
```

---

## Emergency Button

When the emergency button is pressed, the ESP8266 sends an email using the configured SMTP account.

The email can include:

* Custom subject
* Custom message body
* Device location
* Device number
* Device IP address
* Configuration page link

---

## Web Configuration Portal

The web portal allows the user to configure:

* Wi-Fi SSID
* Wi-Fi password
* SMTP server
* SMTP port
* Sender email
* Email password or app password
* Recipient email
* Device location
* Email subject
* Email body
* Repeated email notification option
* Minimum and maximum repeat interval

---

## Required Libraries

Install the following libraries in Arduino IDE:

* ESP8266WiFi
* ESP8266WebServer
* DNSServer
* EEPROM
* ESP_Mail_Client

The `ESP_Mail_Client` library is used to send emails through SMTP.

---

## Gmail SMTP Example

For Gmail, the default SMTP configuration is:

```text
SMTP Host: smtp.gmail.com
SMTP Port: 465
```

A Gmail App Password is recommended instead of the normal Gmail account password.

---

## Recycling and Hardware Modification

This project was created by modifying an old SONOFF device that was no longer working correctly because its internal relay had failed.

Instead of throwing it away, the device was recycled and reused as an ESP8266-based emergency email pager.

A hardware modification was also made to allow the device to work with a 5V DC power supply.

This makes it possible to power the device using:

* Batteries
* USB power supply
* Power bank

By bypassing the original button contact terminals, the device can also be used as a simple alarm or access monitoring system.

For example, it can be connected to:

* External switch
* Magnetic door contact
* Dry contact sensor
* Access control contact

When the contact is triggered, the device sends an email notification to the configured recipient.

This makes the project useful not only as an emergency button, but also as a recycled Wi-Fi alarm system for monitoring doors, accesses, or other simple security events.

---

## Possible Uses

* Emergency call button
* Wi-Fi email pager
* Access monitoring system
* Door opening notification system
* Simple recycled alarm device
* Battery-powered notification device

---

## Notes

The device must be connected to a working Wi-Fi network with internet access in order to send email notifications.

If Wi-Fi is not available or the saved Wi-Fi credentials are wrong, the device starts its fallback Access Point and can be configured from:


## Programming Method

To upload the new firmware to the modified SONOFF device, an FTDI USB-to-TTL serial adapter was used.

The FTDI interface was purchased from AliExpress:

https://a.aliexpress.com/_Ew7QzRU

The required FTDI drivers were installed from the official FTDI website:

https://ftdichip.com/drivers/

After installing the drivers, the correct serial connections were made between TX and RX.

The SONOFF board was powered at 5V DC directly from the FTDI adapter.

To put the board into bootloader mode, the device was powered while keeping the onboard button pressed.

In this mode, the board enters bootloader mode and the new firmware can be uploaded.

The wiring diagram photos have been added to this repository.


## License

This project is open source and can be modified or reused for personal, educational, or experimental purposes.



