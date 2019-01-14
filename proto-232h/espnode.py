# Enable pyserial extensions
import pyftdi.serialext
import time
import threading



class ESPNode:
    def __init__(self, path, baudrate=12000000):
        self.path = path
        self.baudrate = baudrate

        self.nbrPanel = 4
        self.flushPause = .001

        self.lastTime = time.time()
        self.recvThread = threading.Thread(target=self.readESP)

        self.dogFeed = True
        self.watchdogThread = threading.Thread(target=self.watchESP)


    def start(self):
        self.port = pyftdi.serialext.serial_for_url(self.path, self.baudrate)
        self.resetESP()
        self.recvThread.start()
        self.watchdogThread.start()

    def resetESP(self):
        print('ESP Reset')
        self.port.rts = True
        time.sleep(0.1)
        # port.reset_input_buffer()
        self.port.reset_output_buffer()
        self.port.rts = False

    def watchESP(self):
        while True:
            time.sleep(1)
            if self.dogFeed: self.dogFeed = False #eat
            else: self.resetESP() #bite !

    def readESP(self):
        line = ""
        while True:
            b = self.port.read(1)
            try:
                b = b.decode()
            except:
                continue

            self.dogFeed = True
            if b == "\n":
                # print(line)
                self.parseLine(line)
                line = ""
            else:
                line += str(b)


    def parseLine(self, line):
        if line == 'WAIT' or line.startswith('DRAW') :
            print(line)

            # Adjust flush time
            if line == 'WAIT' or line != 'DRAW '+str(self.nbrPanel):
                if self.flushPause <= .002:
                    self.flushPause += .0001

            # Benchmark
            now = time.time()
            print( 1/(now - self.lastTime), self.flushPause )
            self.lastTime = now

            # Prepare buffers
            start = bytearray([250, 2, 1, 0, 0, 255])
            panel = bytearray(256*3)
            panel[256*3-2]=253
            panel[256*3-1]=255

            # Send buffers
            for k in range(self.nbrPanel):
                self.port.write(start)
                self.port.write(panel)
                time.sleep(self.flushPause)

            # Draw
            draw = bytearray([254, 255])
            self.port.write(draw)





node = ESPNode('ftdi://ftdi:232h/1', 12000000)
node.start()


# Open a serial port on the second FTDI device interface (IF/2) @ 3Mbaud
# port = pyftdi.serialext.serial_for_url('ftdi://ftdi:232h/1', baudrate=12000000)


# port.stopbits = serial.STOPBITS_TWO

# Send bytes
# port.write(b'Hello World')

# reset
# port.rts = True
# time.sleep(0.1)
# # port.reset_input_buffer()
# port.reset_output_buffer()
# port.rts = False

# lastTime = time.time()
# counter = 0
#
# def parseLine(line):
#     if line == 'WAIT' or line.startswith('DRAW') :
#         print(line)
#
#         global lastTime
#         now = time.time()
#         print( 1/(now - lastTime) )
#         lastTime = now
#
#         start = bytearray([250, 2, 1, 0, 0, 255])
#         panel = bytearray(256*3)
#         panel[256*3-2]=253
#         panel[256*3-1]=255
#
#         for k in range(4):
#             port.write(start)
#             port.write(panel)
#             time.sleep(.0018)
#
#         bufferD = bytearray(2)
#         bufferD[0] = 254
#         bufferD[1] = 255
#         port.write(bufferD)
#
#
#
# # Receive bytes
# line = ""
# while True:
#     b = port.read(1)
#     try:
#         b = b.decode()
#     except:
#         continue
#     if b == "\n":
#         # print(line)
#         parseLine(line)
#         line = ""
#     else:
#         line += str(b)
