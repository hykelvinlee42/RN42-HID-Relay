#include <hidboot.h>
#include <usbhid.h>
#include <usbhub.h>
#include <avr/power.h>
#include <SoftwareSerial.h>  

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

int bluetoothTx = 2;  // TX-O pin of bluetooth mate, Arduino D2
int bluetoothRx = 3;  // RX-I pin of bluetooth mate, Arduino D3

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

USB usb;
USBHub Hub(&usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD> HidKeyboard(&usb);

class KeyboardParser: public HIDReportParser {
  void Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf); 
};

uint32_t next_time;
KeyboardParser parser;

void KeyboardParser::Parse(USBHID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf) {
  bluetooth.write((uint8_t)0xFD); // BT-HID: Start Byte
  bluetooth.write((uint8_t)len+1); // BT-HID: Data Length
  bluetooth.write((uint8_t)0x01); // BT-HID: Descriptor (1=Keyboard, 2=Mouse, 3=KybdMouse combo)
  for (uint8_t i = 0; i < len; i++) {
    bluetooth.write((uint8_t)buf[i]);
  }
  bluetooth.write("");
}

void rn42ModuleSetup() {
  // Reference: https://learn.sparkfun.com/tutorials/using-the-bluesmirf/example-code-using-command-mode
  Serial.begin(9600);  // Begin the serial monitor at 9600bps
  bluetooth.begin(115200);  // The Bluetooth Mate defaults to 115200bps
  bluetooth.print("$$$");  // Enter command mode
  delay(100);  // Short delay, wait for the Mate to send back CMD
  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity
  // 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  bluetooth.begin(9600);  // Start bluetooth serial at 9600
}

void rn42ModuleConfigLoop() {
  if(bluetooth.available()) { // If the bluetooth sent any characters
    // Send any characters the bluetooth prints to the serial monitor
    Serial.print((char)bluetooth.read());  
  }
  if(Serial.available()) { // If stuff was typed in the serial monitor
    // Send any characters the Serial monitor prints to the bluetooth
    bluetooth.print((char)Serial.read());
  }
}

void setup() {
  // put your setup code here, to run once:
  //rn42ModuleSetup();
  Serial.begin(115200);
  bluetooth.begin(115200); // Comment out if you are configuring bluetooth module

  #if !defined(__MIPSEL__)
    while (!Serial);
  #endif

  if (usb.Init() == -1) {
    Serial.println("OSC did not start");
  }

  delay(200);
  next_time = millis() + 5000;

  HidKeyboard.SetReportParser(0, (HIDReportParser*)&parser);
}

void loop() {
  // put your main code here, to run repeatedly:
  //rn42ModuleConfigLoop();
  usb.Task();
}
