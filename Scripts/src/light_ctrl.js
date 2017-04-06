var Client = require('azure-iothub').Client;
var Message = require('azure-iot-common').Message;
var targetDevice = 'myDevice1';
var connectionString = 'HostName=vsciotdemo.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=8HTurLeQjOazE3zNxA4KHVHqHHOstxpjxw4ZKYTS9PY=';

var serviceClient = Client.fromConnectionString(connectionString);
function printResultFor(op) {
    return function printResult(err, res) {
        if (err) console.log(op + ' error: ' + err.toString());
        if (res) console.log(op + ' status: ' + res.constructor.name);
    };
}

function receiveFeedback(err, receiver){
    receiver.on('message', function (msg) {
        console.log('Feedback message:')
        console.log(msg.getData().toString('utf-8'));
    });
}
let i = 0;
serviceClient.open(function (err) {
    if (err) {
        console.error('Could not connect: ' + err.message);
    } else {
        console.log('Service client connected');
        setInterval(function(){
            var message = new Message(i++ % 2 ? 'light on' : 'light off');
            message.ack = 'full';
            message.messageId = "andy_message";
            console.log('Sending message: ' + message.getData());
            serviceClient.send(targetDevice, message, printResultFor('send'));
        }, 5000);
        serviceClient.getFeedbackReceiver(receiveFeedback);

    }
});