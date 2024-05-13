# Wi-Fi Provisioning Over NFC
This **experimental** sample demonstrates how you can provision the [nRF7002 DK](https://www.nordicsemi.com/Products/Development-hardware/nRF7002-DK) or the ([nRF5340 DK](https://www.nordicsemi.com/Products/Development-hardware/nrf5340-dk) + [nRF7002 EK](https://www.nordicsemi.com/Products/Development-hardware/nRF7002-EK) ) to a Wi-Fi Network using NFC.
The sample uses the PSA Protected Storage service available in the TF-M implementation to store the credentials. 

### How to use:

1.	Connect the NFC antenna to your development kit. 
2.	Build/flash the demo application to your development kit.
Supported build targets are :

•	nrf7002dk_nrf5340_cpuapp_ns 

•	nrf5340dk_nrf5340_cpuapp_ns -DSHIELD=nrf7002ek 

3.	Download NFC Tools, available on [Android](https://play.google.com/store/apps/details?id=com.wakdev.wdnfc&hl=en&gl=US) and [iOS](https://apps.apple.com/us/app/nfc-tools/id1252962749), on your smartphone or tablet.
4.	Make sure that NFC is turned ON on your smart device.
5.	Press on the Write tab -> Add a record -> Select Wi-Fi network.
6.	Type your SSID and password for the network to which you want to connect your development kit. Note that for Authentication, only WPA2-Personal is currently supported in the sample.
7.	Press on Write and tap the NFC Antenna on your development kit(DK).
8.	LED1 will turn on, indicating your DK is provisioned to the network.
9.  A sample Wi-Fi application (UDP client) will start automatically after the DK provisioning. 
10. Press Button1 on the DK. Pressing Button1 sends a message to a custom UDP echo server. The UDP echo server replies with the current time at the server and appends the original message received. This can be viewed on the serial terminal, as shown below: 
```
[00:02:28.396,942] <inf> udp_connection: Successfully connected to server
[00:02:28.396,942] <inf> udp_connection: Press Button1 on your DK to send your message
[00:02:31.407,470] <inf> udp_connection: Successfully sent message: Hello from nRF70 Series
[00:02:31.590,454] <inf> udp_connection: Data received from the server: (Time: 2024-05-13 07:58:12 Message: Hello from nRF70 Series)
```
**Note:** 
Pressing and holding Button 1 for 8 seconds, then releasing it, will erase all stored credentials in the DK and disconnect it from the current Wi-Fi network. The DK can then be re-provisioned to other Wi-Fi networks using NFC.

### Limitations:

*Only WPA2-Personal support is implemented.

