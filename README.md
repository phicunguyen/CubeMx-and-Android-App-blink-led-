# Cubmex-and-Android-App-blink-led-
This android app send commands over usb to toggle the led on stm32f407-disco every 200ms

How to run it:

    1.	Use stm32 st-link utility to program the stm32f407-disco board with stm32f407disco_vcp_led.hex.
    2.	Install the SerialLedBlinking-debug.apk on your android phone or android tablet.
    
Note: If you're already program your stm32f407-disco board with stm32f407disco_vcp_led.hex then you don't need to program it again the android app send same commands to stm32.

Press a button on android app to turn on or off the red led. The green led is still blinking (the green led is updated by the idle task in stm32 (freertos default task).

The packet sending from android app to stm32 as below.
        [ascii]

     1. '[' indicates start of packet.
     2. ']' indicates end of packet.
     3. First 2 bytes is opcode.
     4. follow bytes are data. 
     5. All the data bytes and including the opcode will be convert to ascii except the '[' and ']'
     6. Every hex byte now become two ascii bytes.

This code does not have any flow control. it could cause the data overwritten if the sending is faster than the receving.
Every packet send should have a response packet. That will be my goal.
