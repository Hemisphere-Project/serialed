# Enable pyserial extensions
import pyftdi.serialext
import time, signal, sys, threading, math
from functools import reduce
from random import randint
import serial

INC = 0

class ESPNode:
    def __init__(self, path, baudrate=12000000, verbose=False):
        self.path = path
        self.baudrate = baudrate
        self.verbose = verbose

        self.fpsMetrics = [0] * 10
        self.lastTime = time.time()
        self.recvThread = threading.Thread(target=self.readESP)

        self.dogFeed = True
        self.watchdogThread = threading.Thread(target=self.watchESP)

        self._stop_event = threading.Event()

    def start(self):
        if self.path.startswith('ftdi'):
            self.port = pyftdi.serialext.serial_for_url(self.path, self.baudrate)
        else:
            self.port = serial.Serial(self.path, self.baudrate)
        self.resetESP()
        self.recvThread.start()
        self.watchdogThread.start()

    def stop(self):
        print('Shuting down ', self.path)
        self._stop_event.set()
        self.watchdogThread.join(1.)
        self.recvThread.join(1.)

    def resetESP(self):
        print('ESP Reset')
        try:
            self.port.rts = True
            time.sleep(0.1)
            # port.reset_input_buffer()
            self.port.reset_output_buffer()
            self.port.rts = False
        except:
            quit()

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

            # Benchmark
            now = time.time()
            self.fpsMetrics.append(1/(now - self.lastTime))
            self.fpsMetrics.pop(0)
            self.lastTime = now

            global INC
            INC += 10

            # Prepare buffers
            DATASIZE = 256*3*2
            buffer = [253]                       # FRAME init
            for k in range(DATASIZE):
                val = (k*4+INC)%100
                if val > 20:
                    val = 0
                buffer.append( val )             # RGB data
            buffer.append(254)                   # FRAME end
            buffer.append(255)                   # MSG end
            self.port.write(bytearray(buffer))
            # time.sleep(0.10)
            self.port.flush()
            if self.verbose:
                print('Data sent ('+str(self.getFPS())+')')

        elif line.startswith('ERROR'):
            # quit()
            pass

    def getFPS(self):
        return round(reduce(lambda x, y: x + y, self.fpsMetrics) / len(self.fpsMetrics), 1)



VERBOSE = False
#
# START
#
nodes = [None]*5
nodes[0] = ESPNode('ftdi://ftdi:4232h/1', baudrate=921600, verbose=True)
# nodes[1] = ESPNode('ftdi://ftdi:4232h/2', baudrate=921600, verbose=VERBOSE)
# nodes[2] = ESPNode('ftdi://ftdi:4232h/3', baudrate=921600, verbose=VERBOSE)
# nodes[3] = ESPNode('ftdi://ftdi:4232h/4', baudrate=921600, verbose=VERBOSE)
# nodes[4] = ESPNode('ftdi://ftdi:232h/1',  baudrate=921600, verbose=VERBOSE)

for n in nodes:
    n.start()

#
# EXIT
#
def quit(*args):
    global RUN
    RUN = False

signal.signal(signal.SIGINT, quit)

#
# RUN
#
RUN = True
while RUN:
    time.sleep(1)
    for i, n in enumerate(nodes):
        print('Node '+str(i+1)+' FPS =', n.getFPS())
    print('-----------------------------')

for n in nodes:
    n.stop()
