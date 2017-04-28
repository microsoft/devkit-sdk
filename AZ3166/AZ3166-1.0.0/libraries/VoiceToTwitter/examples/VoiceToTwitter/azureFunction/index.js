'use strict';

const request = require('request');
const uuidV4 = require('uuid/v4');
const async = require('async');
const moment = require('moment');
const config = require('./config.json');
const connectionString = process.env['speechRecognitionIothub'];
const Message = require('azure-iot-common').Message;
const client = require('azure-iothub').Client.fromConnectionString(connectionString);

module.exports = function (context, myBlob) {
    const fileName = context.bindingData.name;
    async.waterfall([
        callback => {
            context.log('getting speech service api token...');
            const option = {
                method: 'POST',
                url: config.tokenRequestUrl,
                headers: {
                    'Ocp-Apim-Subscription-Key': process.env['speechServiceKey'],
                }
            };
            request(option, function (err, response, body) {
                if (err) {
                    callback(err);
                } else {
                    if (response && response.statusCode == 200) {
                        callback(null, body);
                    } else {
                        callback(`Error in getApiToken, response code: ${response.statusCode}. body: ${body}`);
                    }
                }
            });
        },
        (token, callback) => {
            context.log('uploading speech...');
            const option = {
                method: 'POST',
                url: config.speechRecognitionUrl,
                qs: {
                    scenarios: config.speechSenario,
                    appid: config.speechAppId,
                    locale: config.speechLocale,
                    'device.os': config.speechDevice,
                    version: config.speechVersion,
                    format: config.speechFormat,
                    requestid: uuidV4(),
                    instanceid: config.speechInstanceId,
                },
                headers: {
                    'Content-Type': 'audio/wav',
                    'Authorization': `Bearer ${token}`,
                    'Host': config.speechHeaderHost,
                },
                body: myBlob,
            };
            request(option, function (err, response, body) {
                if (err) {
                    callback(err);
                } else {
                    if (response && response.statusCode == 200) {
                        const responseBody = JSON.parse(body);
                        if (responseBody.results && responseBody.results[0] && responseBody.results[0].name) {
                            callback(null, responseBody.results[0].name);
                        } else {
                            callback('error in parse speech recognize request: responseBody.results[0].name not defined');
                        }
                    } else {
                        callback(`Error in speechRecognition, response code: ${response.statusCode}. body: ${body}`);
                    }
                }
            });
        },
        (text, callback) => {
            context.log(`result: ${text}`);
            context.log('trigger the logic app to send tweet');
            const timestamp = moment().format();
            let options = {
                method: 'POST',
                url: process.env['logicAppTrigger'],
                body: `${text} [${timestamp}]`
            };
            request(options, (error, response, body) => {
                if (response && response.statusCode == 202) {
                    callback(null, text);
                } else {
                    callback(`Error in speechRecognition, response code: ${response.statusCode}. body: ${body}. error: ${error}`);
                }
            });
        },
    ], function (err, result) {
        context.log('sending C2D msg');
        const deviceId = fileName.substr(0, fileName.indexOf('/'));
        const blobName = fileName.substr(fileName.indexOf('/') + 1);
        client.open(function (error) {
            if (error) {
                context.log('Could not connect: ' + err.message);
                client.close();
            } else {
                var c2dMsg = {
                    blob: blobName,
                };
                err ? c2dMsg.error = err : c2dMsg.text = result;
                var message = new Message(JSON.stringify(c2dMsg));
                client.send(deviceId, message, function (err, res) {
                    if (err) {
                        context.log(err);
                    }
                    client.close();
                });
            }
        });
        context.done();
    });
};