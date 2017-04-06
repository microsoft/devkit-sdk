'use strict';

var EventHubClient = require('azure-event-hubs').Client;

var connectionString = 'HostName=andyiot3.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=GTbtSBydidMY1iIOIU5gAnSI32qfbSgY0rdjo1xP1W4=';
connectionString = "HostName=vsciotdemo.azure-devices.net;SharedAccessKeyName=iothubowner;SharedAccessKey=8HTurLeQjOazE3zNxA4KHVHqHHOstxpjxw4ZKYTS9PY=";
var printError = function (err) {
    console.log(err.message);
};

var printMessage = function (message) {
    console.log('Message received: ');
    console.log(JSON.stringify(message.body));
    console.log('');
    if (Buffer.isBuffer(message.body))
    console.log('Buffer', message.body.toString());
};

var client = EventHubClient.fromConnectionString(connectionString);
client.open()
    .then(client.getPartitionIds.bind(client))
    .then(function (partitionIds) {
        return partitionIds.map(function (partitionId) {
            return client.createReceiver('$Default', partitionId, { 'startAfterTime' : Date.now()}).then(function(receiver) {
                console.log('Created partition receiver: ' + partitionId)
                receiver.on('errorReceived', printError);
                receiver.on('message', printMessage);
            });
        });
    })
    .catch(printError);