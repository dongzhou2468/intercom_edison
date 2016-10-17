shared.pin.dir(1);
var value = shared.pin.read();
console.log("press:",value, CONFIG.pin);
if(value) {
    sendOUT({calling : !!value});
}

