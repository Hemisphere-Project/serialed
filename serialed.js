const { spawn } = require('child_process');
const EventEmitter = require('events');
const pathUtils = require('path')

class SerialNode extends EventEmitter {
  constructor(path, nPixels) {
    super()
    var that = this

    this.open = true
    this.ready = false
    this.next = false
    this.fps = 0
    this.path = path
    this.nPixels = nPixels

    this.frame = new Array(nPixels*3)
    this.frame.fill(0)

    this.process = spawn('python3', [pathUtils.join(__dirname, 'serialed.py'), this.path, this.nPixels]);
    this.process.stdin.setEncoding('utf-8');

    this.process.stdout.on('data', (data) => {
      // console.log(`stdout: ${data}`);
      data = String(data).trim().split("\n")
      for(var d of data) that.parseData(d)
    });

    this.process.stderr.on('data', (data) => {
      console.log(`stderr: ${data}`);
    });

    this.process.on('close', (code) => {
      console.log(`child process exited with code ${code}`);
      this.open = false
      this.emit('close')
    });
  }

  parseData(data) {
    data = data.trim().split(" ")
    if (data.length > 1) {
      if (data[1] == 'reset') {
        this.ready = false
        this.emit('reset')
      }
      else if (data[1] == 'ready') {
        this.ready = true
        this.emit('ready')
      }
      else if (data[1] == 'next') {
        if (!this.ready) {
          this.ready = true
          this.emit('ready')
        }
        this.next = true
        this.emit('next')
      }
      else if (data[1] == 'FPS') {
        this.fps = parseFloat(data[2])
        this.emit('fps', this.fps)
        // console.log(this.path, this.fps)
      }
      else console.log(data)
    }
  }

  send(msg) {
    // console.log(msg)
    this.process.stdin.write(msg+"\n");
  }

  close() {
    this.send("quit");
  }

  pixelMatrix(x, y, red, green, blue) {
    var led = 0

    x = x%32
    while (x > 15) {
      led += 256
      x -= 16
    }

    y = 15-y%16

    if ((x % 2) === 1) led += x*16+15-y
    else led += y+x*16

    this.frame[led*3] = red;
    this.frame[led*3+1] = green;
    this.frame[led*3+2] = blue;
  }

  led(led, red, green, blue) {
    this.frame[led*3] = red;
    this.frame[led*3+1] = green;
    this.frame[led*3+2] = blue;
  }

  clear() {
    this.frame.fill(0)
  }

  draw() {
    this.next = false
    if (this.ready)
      this.send("frame " + this.frame.map(String).join(' '))
  }

}


/*
 *  Controller - Generic controller
 */
class SerialedController extends EventEmitter {
  constructor() {
    super()
    var that = this

    this.esp = []

    // trigger next draw when everyone is ready
    this.on('next', ()=>{
      for(var esp of that.esp) esp.draw()
      that.emit('draw')
    })
  }

  addNode(path, nPixels) {
    var that = this

    var esp = new SerialNode(path, nPixels)

    // Detect when all esp are ready to draw
    esp.on('next', ()=>{
        var ok = true
        for(var e of that.esp) ok = ok && e.next
        if (ok) that.emit('next')
      })

    this.esp.push(esp)
  }

  node(i) {
    return this.esp[i];
  }

  totalPixels() {
    var n = 0
    for(var e of this.esp) n += e.nPixels
    return n
  }

  led(n, r, g, b) {
    n = n % this.totalPixels()

    for(var e of this.esp) {
      if (n < e.nPixels) {
        e.led(n, r, g, b)
        break;
      }
      else n -= e.nPixels
    }

  }

  // clear all pixels
  clear() {
    for(var esp of this.esp) esp.clear()
  }

  // close interfaces
  close() {
    for(var esp of this.esp) esp.close()
  }

  // get lower FPS
  fps() {
    var f = 1000
    for(var esp of this.esp) if (esp.fps < f) f = esp.fps
    return f
  }

}


/*
 *  Controller - Bassin v1 specific
 */
class SerialedBassin extends SerialedController {
  constructor() {
    super()

    this.addNode('ftdi://ftdi:4232h/1', 512)
    this.addNode('ftdi://ftdi:4232h/2', 512)
    this.addNode('ftdi://ftdi:4232h/3', 512)
    this.addNode('ftdi://ftdi:4232h/4', 512)
    this.addNode('ftdi://ftdi:232h/1', 512)
  }

  // set a pixel
  pixelMatrix(x, y, r, g, b) {
    y = 16*5-1-y  // Y = 0 on top
    var e = Math.floor(y/16) % (this.esp.length)
      this.esp[e].pixelMatrix(x, (y%16), r, g, b)
  }
}



module.exports.SerialedController = SerialedController
module.exports.SerialedBassin = SerialedBassin
