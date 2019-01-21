const { spawn } = require('child_process');
const EventEmitter = require('events');

class SerialNode extends EventEmitter {
  constructor(path) {
    super()
    this.open = true
    this.ready = false
    this.fps = 0
    this.path = path

    this.frame = new Array(512*3)
    this.frame.fill(0)

    this.process = spawn('python3', ['serialed.py', this.path]);
    this.process.stdin.setEncoding('utf-8');

    this.process.stdout.on('data', (data) => {
      // console.log(`stdout: ${data}`);
      data = String(data).trim().split(" ")
      if (data.length > 1) {
        if (data[1] == 'reset') {
          this.ready = false
          this.emit('reset')
        }
        else if (data[1] == 'ready') {
          this.ready = true
          this.emit('ready')
        }
        else if (data[1] == 'FPS') {
          this.fps = parseFloat(data[2])
          this.emit('fps', this.fps)
        }
      }
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

  send(msg) {
    // console.log(msg)
    this.process.stdin.write(msg+"\n");
  }

  close() {
    this.send("quit");
  }

  pixel(x, y, red, green, blue) {
    var led = 0

    x = x%32
    if (x > 15) {
      led = 256
      x -= 16
    }

    y = 15-y%16

    if ((x % 2) === 1) led += x*16+15-y
    else led += y+x*16

    this.frame[led*3] = red;
    this.frame[led*3+1] = green;
    this.frame[led*3+2] = blue;
  }

  clear() {
    this.frame.fill(0)
  }

  update() {
    if (this.ready)
      this.send("frame " + this.frame.map(String).join(' '));
      // console.log("frame " + this.frame.map(String).join(' '))
  }

  draw() {
    if (this.ready)
      this.send("draw");
  }

}


class SerialedController extends EventEmitter {
  constructor() {
    super()

    this.esp = []
    this.esp[0] = new SerialNode('ftdi://ftdi:4232h/1');
    this.esp[1] = new SerialNode('ftdi://ftdi:4232h/2');
    this.esp[2] = new SerialNode('ftdi://ftdi:4232h/3');
    this.esp[3] = new SerialNode('ftdi://ftdi:4232h/4');
    this.esp[4] = new SerialNode('ftdi://ftdi:232h/1');
  }

  pixel(x, y, r, g, b) {
    y = 16*5-1-y  // Y = 0 on top
    var e = Math.floor(y/16) % (this.esp.length)
      this.esp[e].pixel(x, (y%16), r, g, b)
  }

  draw() {
    for(var esp of this.esp) esp.draw()
  }

  update() {
    for(var esp of this.esp) esp.update()
  }

  clear() {
    for(var esp of this.esp) esp.clear()
  }

  close() {
    for(var esp of this.esp) esp.close()
  }

}





ctrl = new SerialedController()

// VERTICAL
// var Y = 0
// setInterval(()=>{
//     Y = (Y+1)%(5*16)
//     ctrl.clear()
//     for (var x=0; x<32; x++) ctrl.pixel(x,Y,10,10,10)
//     ctrl.draw()
//   }, 50)

// HORIZONTAL
var X = 0
setInterval(()=>{
    X = (X+1)%(32)
    ctrl.clear()
    for (var y=0; y<(5*16); y++) ctrl.pixel(X,y,10,10,10)
    ctrl.update()
    ctrl.draw()
  }, 30)


process.on('SIGINT', function() {
    ctrl.close()
});
