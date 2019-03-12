const Serialed = require('./serialed.js')
ctrl = new Serialed.SerialedBassin()

RUN = true

/* VERTICAL
 go to next row once previous has been drawn
 in this exemple speed reflects FPS !
*/
var Y = 0
function animVertic() {
  ctrl.on('next', ()=>{
    Y = (Y+1)%(5*16)
    X = (X+1)%(32)

    ctrl.clear()
    for (var x=0; x<32; x++) ctrl.pixelMatrix(x,Y,10,10,10)
    for (var x=0; x<32; x++) ctrl.pixelMatrix(x,(Y*2)%80,10,0,0)
    for (var x=0; x<32; x++) ctrl.pixelMatrix(x,(79-Math.round(Y/2)),0,0,10)
    for (var x=0; x<32; x++) ctrl.pixelMatrix(x,(79-(Y*2)%80),10,0,0)
    for (var y=0; y<(5*16); y++) ctrl.pixelMatrix(X,y,10,10,0)
    for (var y=0; y<(5*16); y++) ctrl.pixelMatrix(31-(X*2)%32,y,0,0,10)
  })
}


/* HORIZONTAL
 go to next col every 50ms
 in this exemple speed is independant to FPS !
*/
var X = 0
function animHorizon() {
  setTimeout(()=>{
      X = (X+1)%(32)
      ctrl.clear()
      for (var y=0; y<(5*16); y++) ctrl.pixelMatrix(X,y,10,10,10)
      if (RUN) animHorizon()
    }, 50)
}

// FPS
function printFPS() {
  setTimeout(()=>{
    console.log('FPS', ctrl.fps())
    if (RUN) printFPS()
  }, 2000)
}


// START DEMO
printFPS()
// animHorizon()
animVertic()

// PROPERLY CLOSE
process.on('SIGINT', function() {
    RUN = false
    ctrl.close()
});
