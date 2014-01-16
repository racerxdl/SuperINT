
import serial
import time

ser = serial.Serial('/dev/ttyUSB1', 38400)
ser.close()
ser.open()
ser.isOpen()

# Test on, Note on 65535ms 255us
print "Turning on"
ser.write("\x00\x01\x02\xff\xff")
time.sleep(0.5)

print "Turning off"
ser.write("\x00\x02\xff\xff\xff")
