# WIFI Provisioning Over NFC
This experimental sample demonstrates how you can provision your nRF7002 DK or your  (nRF5340 DK + nRF7002 EK ) to your Wi-Fi Network using NFC.
The sample uses the PSA Protected Storage service available in the TF-M implementation to store the credentials. 
How to use:
1.	Connect the NFC antenna to your development kit 
2.	Flash the demo application to your development kit.
Supported build targets are :

•	nrf7002dk_nrf5340_cpuapp_ns 
•	nrf5340dk_nrf5340_cpuapp_ns -DSHIELD=nrf7002ek 

4.	Download NFC Tools, available on Android and iOS, on your smartphone or tablet.
5.	Make sure that NFC is turned ON on your smart device.
6.	Press on the Write tab -> Add a record -> Select Wi-Fi network
7.	Type your SSID and password for the network you want to connect your development kit to. Note that for Authentication, only WPA2-Personal is currently supported in the sample.
8.	Press on Write and tab the NFC Antenna on your Kit
9.	LED1 will turn on, indicating that your board is provisioned to the network. 

Tested on Android 
