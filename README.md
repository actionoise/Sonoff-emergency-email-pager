# Sonoff-emergency-email-pager
Wi-Fi emergency pager made by recycling an old SONOFF device with a faulty internal relay. Instead of throwing it away, the hardware was reused as an emergency button system with a web configuration portal and automatic email alerts. When the button is pressed, the device sends an email notification. 

Features
ESP8266-based emergency button system
Sends an email when the emergency button is pressed
Configurable Wi-Fi settings
Configurable SMTP email settings
Web configuration portal
Automatic fallback Access Point mode
Configuration page available at 192.168.4.1 if Wi-Fi connection fails
Device IP shown in the Serial Monitor when connected to Wi-Fi
Device IP can also be sent by email
Optional repeated emergency email notifications
EEPROM storage for saved settings
LED status indication


Emergency Button

When the emergency button is pressed, the ESP8266 sends an email using the configured SMTP account.

The email can include:

Custom subject
Custom message body
Device location
Device number
Device IP address
Configuration page link



Web Configuration Portal

The web portal allows the user to configure:

Wi-Fi SSID
Wi-Fi password
SMTP server
SMTP port
Sender email
Email password or app password
Recipient email
Device location
Email subject
Email body
Repeated email notification option
Minimum and maximum repeat interval


Required Libraries

Install the following libraries in Arduino IDE:

ESP8266WiFi
ESP8266WebServer
DNSServer
EEPROM
ESP_Mail_Client

The ESP_Mail_Client library is used to send emails through SMTP.


Gmail SMTP Example

For Gmail, the default SMTP configuration is:

SMTP Host: smtp.gmail.com
SMTP Port: 465

A Gmail App Password is recommended instead of the normal Gmail account password.



Recycling and Hardware Modification

This project was created by modifying an old Sonoff device that was no longer working correctly because its internal relay had failed.

Instead of throwing it away, the device was recycled and reused as an ESP8266-based emergency email pager.

A hardware modification was also made to allow the device to work with a 5V DC power supply.
This makes it possible to power the device using batteries, a USB power supply, or a power bank.

By bypassing the original button contact terminals, the device can also be used as a simple alarm or access monitoring system.
For example, it can be connected to an external switch, magnetic door contact, or other dry contact sensor.

When the contact is triggered, the device sends an email notification to the configured recipient.

This makes the project useful not only as an emergency button, but also as a recycled Wi-Fi alarm system for monitoring doors, accesses, or other simple security events.

