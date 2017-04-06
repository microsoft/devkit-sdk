'use strict';

var clientFromConnectionString = require('azure-iot-device-mqtt').clientFromConnectionString;
var Message = require('azure-iot-device').Message;
const connectionString = 'HostName=vsciotdemo.azure-devices.net;DeviceId=myDevice1;SharedAccessKey=blz6Hgr0O8dQJay88eKl5sDKEkW6D6vO6lJiMIXWhAs=';
var client = clientFromConnectionString(connectionString);
function printResultFor(op) {
    return function printResult(err, res) {
        if (err) console.log(op + ' error: ' + err.toString());
        if (res) console.log(op + ' status: ' + res.constructor.name);
    };
}
var connectCallback = function (err) {
    if (err) {
        console.log('Could not connect: ' + err);
    } else {
        console.log('Client connected');
        client.on('message', function (msg) {
            client.complete(msg, () => {
                console.log('completed');
                console.log(msg.data.toString());
            });
        });
        // Create a message and send it to the IoT Hub every second
        setInterval(function(){
            var windSpeed = 10 + (Math.random() * 4);
            var data = JSON.stringify({ deviceId: 'myFirstNodeDevice', windSpeed: windSpeed });
            var message = new Message(data);
            console.log("Sending message: " + message.getData().toString());
            client.sendEvent(message, printResultFor('send'));
        }, 1000);
    }
};
client.open(connectCallback);