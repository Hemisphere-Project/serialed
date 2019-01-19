# Enable pyserial extensions
import pyftdi.serialext
import time, signal, sys, threading
from functools import reduce
from random import randint

class ESPNode:
    def __init__(self, path, baudrate=12000000, verbose=False):
        self.path = path
        self.baudrate = baudrate
        self.verbose = verbose

        self.nbrPanel = 1
        self.flushPause = .001

        self.fpsMetrics = [0] * 10
        self.lastTime = time.time()
        self.recvThread = threading.Thread(target=self.readESP)

        self.dogFeed = True
        self.watchdogThread = threading.Thread(target=self.watchESP)

        self._stop_event = threading.Event()

    def start(self):
        self.port = pyftdi.serialext.serial_for_url(self.path, self.baudrate)
        self.resetESP()
        self.recvThread.start()
        self.watchdogThread.start()

    def stop(self):
        self._stop_event.set()
        self.watchdogThread.join()
        self.recvThread.join()

    def resetESP(self):
        print('ESP Reset')
        self.port.rts = True
        time.sleep(0.1)
        # port.reset_input_buffer()
        self.port.reset_output_buffer()
        self.port.rts = False

    def watchESP(self):
        while not self._stop_event.is_set():
            time.sleep(1)
            if self.dogFeed: self.dogFeed = False #eat
            else: self.resetESP() #bite !

    def readESP(self):
        line = ""
        while not self._stop_event.is_set():
            b = self.port.read(1)
            try:
                b = b.decode()
            except:
                continue

            self.dogFeed = True
            if b == "\n":
                if self.verbose:
                    print(line)
                self.parseLine(line)
                line = ""
            else:
                line += str(b)


    def parseLine(self, line):
        if line == 'WAIT' or line.startswith('DRAW') :
            if self.verbose:
                print(self.path, line)

            # Adjust flush time
            if line == 'WAIT' or line != 'DRAW '+str(self.nbrPanel):
                if self.flushPause <= .01:
                    self.flushPause += .0001

            # Benchmark
            now = time.time()
            self.fpsMetrics.append(1/(now - self.lastTime))
            self.fpsMetrics.pop(0)
            self.lastTime = now

            # Prepare buffers
            pSize = 256
            buffer = bytearray(256*3)
            for k in range(256*3):
                buffer[k] = k%255

            self.port.write(bytearray([253]))
            time.sleep(0.1)
            self.port.write(buffer)
            time.sleep(0.2)
            self.port.write(bytearray([254,255]))
            time.sleep(0.1)


    def getFPS(self):
        return round(reduce(lambda x, y: x + y, self.fpsMetrics) / len(self.fpsMetrics), 1)



NODESsize = 1
#
# START
#
nodes = [None]*NODESsize
for i in range(NODESsize):
    nodes[i] = ESPNode('ftdi://ftdi:4232h/'+str(i+1), baudrate=12000000, verbose=True)
    nodes[i].start()

#
# EXIT
#
def signal_handler(sig, frame):
        print('You pressed Ctrl+C!')
        for n in nodes:
            n.stop()
        global RUN
        RUN = False
signal.signal(signal.SIGINT, signal_handler)

#
# RUN
#
RUN = True
while RUN:
    time.sleep(1)
    for i in range(NODESsize):
        print('Node '+str(i+1)+' FPS =', nodes[i].getFPS(), ' (', round(nodes[i].flushPause, 4), ')')
    print('-----------------------------')

# sys.exit(0)

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
