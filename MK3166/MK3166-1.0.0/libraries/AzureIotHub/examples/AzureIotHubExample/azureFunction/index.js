'use strict';
// This function is triggered each time a message is revieved in the IoTHub.
// The message payload is persisted in an Azure Storage Table
const Message = require('azure-iot-common').Message;
const iotHubConnectionString = process.env['iotHubConnectionString'];
const cloudClient = require('azure-iothub').Client.fromConnectionString(iotHubConnectionString);
const request = require('request');


module.exports = function (context, myEventHubMessage) {
    context.log(myEventHubMessage);
    if (myEventHubMessage.DeviceID) {
        cloudClient.open(function (err) {
            if (err) {
                context.log('Could not connect: ' + err.message);
            } else {
                context.log('Client connected');
                let tweet = '';
                let options = {
                    url: process.env['twitterAPI']+'?count=1&q=%23'+myEventHubMessage.topic||'',
                    headers: {
                        'Authorization': "Bearer "+process.env['twitterBearerKey']
                    }
                };
                request(options, (error, response, body) => {
                    if (!error && response.statusCode == 200) {
                        let info = JSON.parse(body);
                        context.log(info);
                        tweet = (info.statuses && info.statuses.length)? info.statuses[0].text : "No new tweet for the moment";
                        context.log(tweet);
                        const message = new Message(tweet);
                        cloudClient.send(myEventHubMessage.DeviceID, message, function (err, res) {
                            if (err) {
                                context.log(`Error in send C2D message: ${err}`);
                            } else {
                                context.log(`send status: ${res.constructor.name}`);
                            }
                        });
                    }
                });
            }
        });
    }
    context.done();
};
