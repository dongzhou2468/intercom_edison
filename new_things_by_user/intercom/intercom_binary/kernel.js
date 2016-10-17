console.log("speak_call: ", IN.speak_call);

if(IN.speak_call) {
    var spawn = require("child_process").spawn; 
    var child = spawn("./socket_server", [IN.app_ip], {cwd: __dirname});
}
//shared.pid = child.pid;

