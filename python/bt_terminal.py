import threading
import time
import sys
import serial



class BluetoothRemoteController:
    def __init__(self, port: str, baudrate: int = 9600):
        """Initialize an BT object, and auto-connect it."""
        # The port name is the name shown in control panel
        # And the baudrate is the communication setting, default value of HC-05 is 9600.
        self.ser = serial.Serial(port, baudrate=baudrate)
        self.need_cmd = True
        self.cmd_stream = []

    def is_open(self) -> bool:
        return self.ser.is_open

    def waiting(self) -> bool:
        return self.ser.in_waiting

    def do_connect(self, port: str, baudrate: int = 9600) -> bool:
        """Connect to the specify port with particular baudrate"""
        # Connection function. Disconnect the previous communication, specify a new one.
        self.disconnect()

        try:
            self.ser = serial.Serial(port, baudrate=baudrate)
            return True
        except:
            return False

    def disconnect(self):
        """Close the connection."""
        self.ser.close()

    def write(self):
        # Write the byte to the output buffer, encoded by utf-8.
        self.cmd_stream.append(0)
        self.ser.write(bytes(self.cmd_stream))
        self.cmd_stream = []
    
    def forward(self):
        self.cmd_stream.append(130)
    
    def left(self):
        self.cmd_stream.append(33)
        self.cmd_stream.append(129)
    
    def right(self):
        self.cmd_stream.append(65)
        self.cmd_stream.append(129)
    
    def back(self):
        self.cmd_stream.append(8)
        self.cmd_stream.append(129)
    
    def stop(self):
        self.cmd_stream.append(16)

    def readStat(self) -> str:
        if bt.waiting():
            # Scan the input buffer until meet a '\n'. return none if doesn't exist.
            stat = self.ser.read()
            print(stat)
            if stat == b'U':
                byte_count = 0
                uid = []
                while byte_count < 4:
                    if (self.waiting):
                        uid.append(int.from_bytes(self.ser.read(), byteorder="big", signed=False))
                        byte_count += 1
                print(uid) # [TODO] send to server
            elif stat == b'I':
                self.need_cmd = True


def read():
    while True:
        bt.readStat()


def write():
    while True:
        msgWrite = input()

        if msgWrite == "exit":
            sys.exit()
        
        bt.write()


if __name__ == "__main__":
    # TODO: Please modify the port name.
    bt = BluetoothRemoteController("COM5")
    while not bt.is_open():
        pass
    print("BT Connected!")

    readThread = threading.Thread(target = read)
    readThread.daemon = True
    readThread.start()

    bt.forward(1)
    bt.write()

    while True:
        if bt.need_cmd:
            bt.left()
            bt.forward(2)
            bt.back()
            bt.write()
        
        


# 0: transmission ended
# 128 + s: forward for s step (0 <= s <= 15)
# 64 + t: turn right for t * 90 degrees (0 <= t <= 15)
# 32 + t: turn left for t * 90 degrees (0 <= t <= 15)
# 16 ~ 31: wait until two wheels are stopped
# 8: turn back



# 129 16 66 16 129 16 0
#
#