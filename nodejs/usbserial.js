const EventEmitter = require('events')
const SerialPort = require('serialport')
const Readline = require('@serialport/parser-readline')

class ESPNode extends EventEmitter {
  constructor(path) {
    super()
    var that = this;
    this.path = path;

    this.showInput = false
    this.isReady = false
    this.retryCounter = 0

    this.benchFps = []
    this.lastEllaps = process.hrtime();
    this.lastNews = 0
  }

  start() {
    var that = this;

    this.port = new SerialPort(this.path, { baudRate: 921600 }) //460800 //921600
    this.parser = this.port.pipe(new Readline({ delimiter: '\n' }))

    this.port.on('error', function(err) {
      console.log('Error: ', err.message)
    })

    this.port.on('open', function() {
      console.log(that.path, 'port open')
      that.port.flush()
      that.checkReady()
    })

    this.parser.on('data', (data)=> {
      if (that.showInput) console.log(this.path, '-' ,data)

      if (data.trim() == "INIT" && !that.isReady) {
        that.isReady = true
        that.retryCounter = 0
        console.log(this.path, 'Device Found!')
        that.emit('init')
      }
      if (data.trim() == "DRAW") that.emit('draw')

      that.lastNews = Date.now()
    })

  }

  stop() {
    this.retryCounter = 0
    this.port.close(function (err) {
      if (err) {
        return console.log(this.path, 'Error closing port: ', err.message)
      }
    })
  }

  reset() {
    console.log(this.path, '... reset')
    var that = this
    this.port.set({rts: true, dtr: false});

    this.isReady = false
    this.retryCounter = 0
    setTimeout(function(){that.stop()}, 100)
    setTimeout(function(){that.start()}, 200)
  }

  verbose() {
    this.showInput = true
  }

  checkReady() {
    if (this.isReady) return;
    var that = this

    this.retryCounter += 1
    if (this.retryCounter > 3) {
      console.log(this.path, 'no device...')
      this.reset()
      return
    }

    console.log(this.path, 'looking for device...')
    var buffer = new Buffer(1)
    buffer[0] = 254
    this.send(buffer)

    setTimeout(function(){that.checkReady()}, 300)
  }

  elapsed_time(note) {
      var precision = 3; // 3 decimal places
      var elapsed = process.hrtime(this.lastEllaps)[1] / 1000000; // divide by a million to get nano to milli
      if (note) console.log(process.hrtime(this.lastEllaps)[0] + " s, " + elapsed.toFixed(precision) + " ms - " + note); // print message + time
      var val = process.hrtime(this.lastEllaps)[0]*1000+elapsed.toFixed(precision)
      this.lastEllaps = process.hrtime(); // reset the timer
      return val
  }

  benchmark() {
    var that = this

    that.on('init', ()=>{
      this.elapsed_time();
      this.send(makeAllBuff())
    })

    that.on('draw', ()=>{
      var ms = that.elapsed_time()
      if (that.benchFps.push(Math.floor(1000/ms)) > 100) that.benchFps.shift()
      var avg = that.benchFps.reduce((a, b) => a + b, 0) / that.benchFps.length
      console.log(that.path, 'FPS:', avg.toFixed(2))

      this.elapsed_time();
      this.send(makeAllBuff())
    })
  }

  send(buffer) {

    this.port.write(buffer, function (err, result) {
        if (err) {
            console.log(this.path, 'Error while sending message : ' + err);
        }
        if (result) {
            console.log(this.path, 'Response received after sending message : ' + result);
        }
    });
    this.port.drain((err) => {
      if (err) console.log(this.path, 'Error while sending message : ' + err);
      // else this.elapsed_time("end send()");
    })

  }
}

var node0 = new ESPNode('/dev/ttyUSB0')
node0.benchmark()
node0.verbose()
node0.start()

var node1 = new ESPNode('/dev/ttyUSB1')
node1.benchmark()
node1.verbose()
node1.start()

var node2 = new ESPNode('/dev/ttyUSB2')
node2.benchmark()
node2.verbose()
node2.start()



//
// UTILS
//

process.on('SIGINT', function() {
    console.log("Caught interrupt signal");
    process.exit();
});

function makePanelBuff(panel) {
  var size = 256*3+3
  var buffer = new Buffer(size);
  for (var k= 2; k<size-1; k++) buffer[k] = randomInt (0,250)
  buffer[0] = 2;
  buffer[1] = panel;
  buffer[size-1] = 255;
  return buffer
}

function makeAllBuff() {
  var size = 256*4*3+2
  var buffer = new Buffer(size);
  for (var k= 1; k<size-1; k++) buffer[k] = randomInt (0,250)
  buffer[0] = 3;
  buffer[size-1] = 255;
  return buffer
}

function randomInt(low, high) {
  return Math.floor(Math.random() * (high - low) + low)
}
