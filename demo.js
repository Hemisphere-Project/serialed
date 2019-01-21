const SerialedController = require('./serialed.js');
ctrl = new SerialedController()

RUN = true

/* VERTICAL
 go to next row once previous has been drawn
 in this exemple speed reflects FPS !
*/
var Y = 0
function animVertic() {
  ctrl.on('next', ()=>{
    Y = (Y+1)%(5*16)
    ctrl.clear()
    for (var x=0; x<32; x++) ctrl.pixel(x,Y,10,10,10)
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
      for (var y=0; y<(5*16); y++) ctrl.pixel(X,y,10,10,10)
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
