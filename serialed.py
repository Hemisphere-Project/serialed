# Enable pyserial extensions
import pyftdi.serialext
import time, signal, sys, threading, math
from select import select
from functools import reduce
import serial

FRAMESIZE = (256*3*2)
BENCHSIZE = 40

def colorClamp(color):
    color = int(color)
    if not color or color < 0: color=0
    if color > 250: color=250
    return color

class ESPNode:
    def __init__(self, path, baudrate, verbose=False):
        self.path = path
        self.baudrate = baudrate
        self.verbose = verbose
        self.frameMutex = threading.Lock()
        self.run = False
        self._stop_event = threading.Event()

        # TEST
        self.inc = 0
        self.testMode = False

        # STATUS (FPS, ...)
        self.statusMode = True
        self.fpsMetrics = [0] * BENCHSIZE
        self.lastTime = time.time()
        self.statusThread = threading.Thread(target=self.status)

        # READ uart
        self.recvThread = threading.Thread(target=self.readESP)

        # WATCHDOG (reset ESP)
        self.dogFeed = True
        self.watchdogThread = threading.Thread(target=self.watchESP)

        # FRAME BUFFERS
        self.frameInput = bytearray( FRAMESIZE )
        self.frameOutput = bytearray( FRAMESIZE+3 )
        self.frameOutput[0] = 253
        self.frameOutput[FRAMESIZE+1] = 254
        self.frameOutput[FRAMESIZE+2] = 255


    def start(self):
        if self.path.startswith('ftdi'):
            self.port = pyftdi.serialext.serial_for_url(self.path, self.baudrate)
        else:
            self.port = serial.Serial(self.path, self.baudrate)
        self.resetESP()
        self.recvThread.start()
        self.watchdogThread.start()
        self.statusThread.start()
        self.run = True

    def stop(self):
        self.say('shutdown')
        self._stop_event.set()
        try: self.watchdogThread.join(1.)
        except: pass
        try: self.recvThread.join(1.)
        except: pass
        try: self.statusThread.join(1.)
        except: pass
        try: self.port.close()
        except: pass
        self.run = False

    def resetESP(self):
        self.say( 'reset')
        self.fpsMetrics = [0] * BENCHSIZE
        try:
            self.port.rts = True
            time.sleep(0.1)
            # port.reset_input_buffer()
            self.port.reset_output_buffer()
            self.port.rts = False
        except:
            self.stop()

    def say(self, *args):
        print(self.path, *args)
        sys.stdout.flush()

    def status(self):
        while not self._stop_event.is_set():
            if self.statusMode: self.say( 'FPS', self.getFPS())
            time.sleep(1)

    def watchESP(self):
        while not self._stop_event.is_set():
            time.sleep(1)
            if self.dogFeed: self.dogFeed = False #eat
            else: self.resetESP() #bite !

    def readESP(self):
        line = ""
        while not self._stop_event.is_set():
            try:
                b = self.port.read(1)
            except:
                self.stop()
                continue
            try:
                b = b.decode()
            except:
                continue

            self.dogFeed = True
            if b == "\n":
                if self.verbose:
                    self.say( 'recv', line)
                if line == 'WAIT' or line.startswith('DRAW') :
                    self.updateESP()
                elif line.startswith('ERROR'):
                    # self.stop()
                    pass
                elif line.startswith('READY'):
                    self.fpsMetrics = [0] * BENCHSIZE
                    self.say('ready')
                line = ""
            else:
                line += str(b)


    def updateESP(self):
        # Benchmark
        if self.statusMode:
            now = time.time()
            self.fpsMetrics.append(1/(now - self.lastTime))
            self.fpsMetrics.pop(0)
            self.lastTime = now

        # Test
        if self.testMode:
            self.test(self.testMode)

        # Send
        try:
            self.port.write( self.frameOutput )
            self.port.flush()
        except:
            self.stop()
        if self.verbose:
            self.say( 'sent', self.getFPS())

    # Copy Frame Buffer
    def draw(self, frame=None):
        with self.frameMutex:
            self.frameOutput[1:FRAMESIZE+1] = self.frameInput

    def led(self, led, red, green, blue):
        if led >= 512: return
        red     = colorClamp(red)
        green   = colorClamp(green)
        blue    = colorClamp(blue)
        with self.frameMutex:
            self.frameInput[led*3] = red
            self.frameInput[led*3+1] = green
            self.frameInput[led*3+2] = blue

    def set(self, frame):
        if len(frame) != len(self.frameInput):
            self.say( 'wrong input')
            return
        # else: print(all)
        with self.frameMutex:
            self.frameInput[:] = frame

    def blackout(self):
        with self.frameMutex:
            self.frameInput = bytearray( FRAMESIZE )
        self.draw()

    def test(self, enable):
        self.testMode = enable
        if not self.testMode: return
        with self.frameMutex:
            self.frameInput[self.inc] = 0
            self.frameInput[FRAMESIZE-(self.inc+1)] = 0
            self.inc = (self.inc+1)%(768)
            self.frameInput[self.inc] = 10
            self.frameInput[FRAMESIZE-(self.inc+1)] = 10

    def getFPS(self):
        return round(reduce(lambda x, y: x + y, self.fpsMetrics) / len(self.fpsMetrics), 1)


VERBOSE = False
#
# START
#
NODE = None
if len(sys.argv) > 1:
    NODE = ESPNode(sys.argv[1], baudrate=921600, verbose=VERBOSE)
    if len(sys.argv) > 2 and sys.argv[2] == '0':
        NODE.statusMode = False
    try:
        NODE.start()
    except:
        NODE.say('ERROR: device not found', sys.argv[1])
        sys.exit(1)
else:
    print('ERROR: no device path provided.')
    sys.exit(1)

#
# EXIT
#
signal.signal(signal.SIGINT, lambda *args:NODE.stop())

#
# RUN
#
while NODE.run:
    rlist, _, _ = select([sys.stdin], [], [], 1)
    if rlist:
        s = sys.stdin.readline().strip().split(' ')
        if len(s) == 0: continue

        # SET led
        if s[0] == 'led' and len(s) == 5:
            NODE.led(int(s[1]), int(s[2]), int(s[3]), int(s[4]))

        # DRAW
        elif s[0] == 'frame':
            NODE.set( [colorClamp(b) for b in s[1:]] )

        # DRAW
        elif s[0] == 'draw':
            NODE.draw()

        # BLACKOUT
        elif s[0] == 'blackout':
            NODE.blackout()

        # TEST
        elif s[0] == 'test':
            enable = not NODE.testMode
            NODE.test(enable)

        # STATUS
        elif s[0] == 'status':
            NODE.statusMode = not NODE.statusMode

        # QUIT
        elif s[0] == 'quit':
            NODE.stop()

        else:
            print('unknown cmd', s)


print(NODE.path, 'exit')
sys.exit(0)
