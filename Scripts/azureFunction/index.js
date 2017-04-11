'use strict';
// This function is triggered each time a message is revieved in the IoTHub.
// The message payload is persisted in an Azure Storage Table
const Message = require('azure-iot-common').Message;
const iotHubConnectionString = process.env['iotHubConnectionString'];
const cloudClient = require('azure-iothub').Client.fromConnectionString(iotHubConnectionString);
const request = require('request');


function retriveTweetByTopic(context, topic) {
    let tweet = '';
    let options = {
        url: process.env['twitterAPI']+'?count=1&q=%23'+topic,
        headers: {
            'Authorization': "Bearer "+process.env['twitterBearerKey']
        }
    };
    const callback = (error, response, body) => {
        if (!error && response.statusCode == 200) {
            let info = JSON.parse(body);
            context.log(info);
            tweet = info.statuses[0].text;
            context.log(tweet);
        }
    };
    request(options, callback);
    return tweet;
}

module.exports = function (context, myEventHubMessage) {
    context.log(myEventHubMessage);
    if (myEventHubMessage.DeviceID) {
        cloudClient.open(function (err) {
            if (err) {
                context.log('Could not connect: ' + err.message);
            } else {
                context.log('Client connected');
                const message = new Message(retriveTweetByTopic(context, myEventHubMessage.topic));
                cloudClient.send(myEventHubMessage.DeviceID, message, function (err, res) {
                    if (err) {
                        context.log(`Error in send C2D message: ${err}`);
                    } else {
                        context.log(`send status: ${res.constructor.name}`);
                    }
                });
            }
            cloudClient.close();
        });
    }
    context.done();
};
