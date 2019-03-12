const Serialed = require('./serialed.js')
ctrl = new Serialed.SerialedController()

RUN = true
NPIX = 150

ctrl.addNode('/dev/ttyUSB0', NPIX)


/* CHASER FPS
  go to next pixel once previous has been drawn
  in this exemple speed reflects FPS !
*/
var currentLedFPS = 0
function animChaser_FPS() {
  ctrl.on('next', ()=>{
    ctrl.clear()
    currentLedFPS = (currentLedFPS+1)%NPIX
    ctrl.led( currentLedFPS,100,100,100 )
  })
}


/* CHASER FIXED
  go to next col every 20ms
  in this exemple speed is independant to FPS !
*/
var currentLedFIXED = 0
function animChaser_FIXED() {
  setTimeout(()=>{
    // ctrl.clear()
    currentLedFIXED = (currentLedFIXED+1)%NPIX
    ctrl.led( currentLedFIXED,100,0,0 )
    if (RUN) animChaser_FIXED()
  }, 20)
}


/* FPS Monitoring
*/
function printFPS() {
  setTimeout(()=>{
    console.log('FPS', ctrl.fps())
    if (RUN) printFPS()
  }, 2000)
}


// START
printFPS()
animChaser_FPS()
animChaser_FIXED()

// PROPERLY CLOSE
process.on('SIGINT', function() {
    ctrl.close()
    RUN = false
});
