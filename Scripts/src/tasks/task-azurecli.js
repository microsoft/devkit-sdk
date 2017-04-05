import fs from 'fs-plus'
import _ from 'lodash'
import uuidV4 from 'uuid/v4'
import open from 'open'
import * as util from '../util'
import inquirer from 'inquirer'
import username from 'username'
let azureCliLocation;
let subscriptions;
let currentSubs;
let versionCheck = async() => {
    try {
        let location = await util.findExecutive("az");
        if (location && fs.isFileSync(location)) {
            let ver = /\(([\w\.]+)\)/g.exec((await util.execStdout(util.cstr(location) + " --version", 60000)).split('\n')[0]);
            azureCliLocation = location;
            return ver[1];
        } else {
            throw new Error('Cannot find azure cli, have you installed Azure Cli 2.0 by `pip install azure-cli`');
        }
    }
    catch (error) {
        throw new Error('Cannot find azure cli, have you installed Azure Cli 2.0 by `pip install azure-cli`, if installed, please add `%APPDATA%\\Python\\Scripts` to path');
    }
};

exports.version = {
    name: "azure cli version",
    run: versionCheck
};
let _login = async() => {
    await util.executeWithProgress(azureCliLocation, ['login'], (line, channel) => {
        if (channel === 'stdout') {
            console.log('[azurecli]', line);
        } else {
            if (line.includes('To sign in, use a web browser to open the page')) {
                let codeRegex = /enter\sthe\scode\s(\w+)\sto\sauthenticate/g.exec(line);
                if (codeRegex && codeRegex[1]) {

                    open('https://aka.ms/devicelogin', (err) => {
                        if (err) {
                            console.log('Please open https://aka.ms/devicelogin and input the following code:', codeRegex[1]);
                        } else {
                            console.log('Please input the following code:', codeRegex[1]);
                        }
                    });
                }
            }
        }
    });
};
exports.login = {
    name: "azure cli login",
    run: async() => {
        try {
            let output = await util.execStdout(util.cstr(azureCliLocation) + " account list", 60000);
            if (output) {
                subscriptions = JSON.parse(output);
            }
        } catch (err) {
            await _login();
        }
        if (!subscriptions) {
            let output = await util.execStdout(util.cstr(azureCliLocation) + " account list", 60000);
            if (output) {
                subscriptions = JSON.parse(output);
            }
        }
        if (!subscriptions) {
            throw new Error('Cannot get subscriptions.');
        }
        let subsList = _.map(subscriptions, sub => {
            return {
                name: sub.name,
                value: sub
            };
        });
        let result = await inquirer.prompt([{
            name: 'choose',
            type: 'list',
            message: `What subscription would you like to choose?`,
            choices: _.flatten([
                new inquirer.Separator('Select Subscription'),
                subsList,
                new inquirer.Separator()
            ])
        }]);
        await util.execStdout(util.cstr(azureCliLocation) + ` account set --subscription ${result.choose.id}`, 60000);
        currentSubs = result.choose;
        return result.choose.name;
    }
};

exports.provision = {
    name: "azure cli provision",
    run: async() => {
        if (!currentSubs) {
            throw new Error('Please select subscription first.');
        }
        let guid = uuidV4().slice(0, 4);
        let resourceGroupName = (await username()) + '-resource-' + guid;
        console.log(`Creating resource group ${resourceGroupName}`);
        console.log(`az group create --name ${resourceGroupName} --location westus`);
        let output = await util.execStdout(util.cstr(azureCliLocation) + ` group create --name ${resourceGroupName} --location westus`, 5 * 60000);
        if (output && JSON.parse(output)) {
            let createResourceResponse = JSON.parse(output);
            console.log(`Create resource group ${resourceGroupName} ${createResourceResponse.properties.provisioningState}`);
        }
        let iotHubName = (await username()) + '-iothub-' + guid;
        console.log(`Creating iothub ${iotHubName}`);
        console.log(`az iot hub create -g ${resourceGroupName} -n ${iotHubName} --location westus`);
        let _result = await util.execShort(util.cstr(azureCliLocation) + ` iot hub create -g ${resourceGroupName} -n ${iotHubName} --location westus`, 5 * 60000);

        if (_result.stdout && JSON.parse(_result.stdout)) {
            let createIotHubResponse = JSON.parse(_result.stdout);
            console.log(`Create iot-hub ${createIotHubResponse.name} Success.`);
        } else {
            throw new Error(`Cannot create iot hub due to error: ${_result.stderr}.`);
        }
         //show connection string
         output = await util.execStdout(util.cstr(azureCliLocation) + ` iot hub show-connection-string -n ${iotHubName}`, 5 * 60000);
         if (!output && JSON.parse(output)) {
             let connectionString = JSON.parse(output).connectionString;
             if (!connectionString) {
                return connectionString;
             } else {
                 throw new Error(`Cannot get iot hub connection string.`);
             }
         } else {
             throw new Error(`Cannot get iot hub connection string.`);
         }
    }
};
