import threading
import time
import sys
import serial
# from score import ScoreboardServer, ScoreboardFake


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
        self.need_cmd = False
    
    def forward(self):
        self.cmd_stream.append(32)
    
    def left(self):
        self.cmd_stream.append(64)
    
    def right(self):
        self.cmd_stream.append(96)
    
    def back(self):
        self.cmd_stream.append(128)
    
    def stop(self):
        self.cmd_stream.append(0)

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
                uid_string = bytes(uid).hex().upper()
                print(uid_string)              # 12345678
                # scoreboard.add_UID(uid_string)
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
        
        bt.cmd_stream = list(map(int, msgWrite.split()))
        bt.write()


if __name__ == "__main__":
    # TODO: Please modify the port name.
    bt = BluetoothRemoteController("COM4")
    while not bt.is_open():
        pass
    print("BT Connected!")
    # scoreboard = ScoreboardFake("TeamName", "data/fakeUID.csv")

    readThread = threading.Thread(target = read)
    readThread.daemon = True
    readThread.start()

    write()

        


# 0: transmission ended
# 128 + s: forward for s step (0 <= s <= 15)
# 64 + t: turn right for t * 90 degrees (0 <= t <= 15)
# 32 + t: turn left for t * 90 degrees (0 <= t <= 15)
# 16 ~ 31: wait until two wheels are stopped
# 8: turn back


# forward 32
# turn left 64
# turn right 96
# turn back 128
# end 0