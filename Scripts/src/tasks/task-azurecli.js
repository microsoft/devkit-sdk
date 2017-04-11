import fs from 'fs-plus'
import _ from 'lodash'
import uuidV4 from 'uuid/v4'
import open from 'open'
import * as util from '../util'
import inquirer from 'inquirer'
import * as pref from '../preference'
import AzureCli from '../azurecli'
import username from 'username'
let azureCliLocation;
let subscriptions;
let currentSubs;
let azurecli;
let versionCheck = async() => {
    try {
        let location = await util.findExecutive("az");
        if (location && fs.isFileSync(location)) {
            azurecli = new AzureCli(location);
            azureCliLocation = location;
            return await azurecli.version();
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
    run: async(context) => {
        try {
             subscriptions = await azurecli.execResultJson("account list");
        } catch (err) {
            await _login();
        }
        if (!subscriptions) {
            subscriptions = await azurecli.execResultJson("account list");
        }
        if (!subscriptions) {
            throw new Error('Cannot get subscriptions.');
        }
        let subsList = _.map(subscriptions, sub => {
            return {
                name: `${sub.name} (${sub.id})`,
                value: sub
            };
        });
        let subsPreference = pref.getValue('subscription');
        let result = await inquirer.prompt([{
            name: 'choose',
            type: 'list',
            message: `What subscription would you like to choose?`,
            choices: _.flatten([
                new inquirer.Separator('Select Subscription'),
                subsList,
                new inquirer.Separator()
            ]),
            default: subsPreference ? _.findIndex(subsList, {name: subsPreference}) : 0
        }]);
        await azurecli.execNoOutput(`account set --subscription ${result.choose.id}`);
        currentSubs = result.choose;
        context.subscription = currentSubs;
        pref.setValue('subscription', `${result.choose.name} (${result.choose.id})`);
        pref.savePreference();
        return result.choose.name;
    }
};

exports.iothub = {
    name: "azure cli iothub",
    run: async(context) => {
        if (!currentSubs) {
            throw new Error('Please select subscription first.');
        }
        // register provider for subscription 
        await azurecli.execNoOutput(`provider register -n "Microsoft.Devices"`);
        
        let guid = uuidV4().slice(0, 4);
        let defaultPreffix = await username();
        let iotHubName = '';
        let resourceGroupName ='';
        let hubPreference = pref.getValue('iot_event_hub');
        let hubList = JSON.parse(await util.execStdout(util.cstr(azureCliLocation) + ` iot hub list`, 60000));
        if (hubList && hubList.length) {
            let result = await inquirer.prompt([{
                name: 'choose',
                type: 'list',
                message: `What iot hub would you like to choose?`,
                choices: _.flatten([
                    new inquirer.Separator('Select Iot Hub'),
                    hubList,
                    new inquirer.Separator(),
                    {name:"Create new...", value: "$$"},
                    new inquirer.Separator()
                ]),

                pageSize: 10,
                default: hubPreference
            }]);
            if (result.choose != '$$') {
                iotHubName = result.choose;
            }
        }
        if (!iotHubName) {
            // create iothub
            let groupList = await azurecli.execResultJson('group list');
            resourceGroupName = '';
            if (groupList && groupList.length) {
                let result = await inquirer.prompt([{
                    name: 'choose',
                    type: 'list',
                    message: `Which Resource Group would you like to choose?`,
                    choices: _.flatten([
                        new inquirer.Separator('Select Resouce Group'),
                        groupList,
                        new inquirer.Separator(),
                        {name:"Create new...", value: "$$"},
                        new inquirer.Separator()
                    ]),
                    pageSize: 10
                }]);
                if (result.choose != '$$') {
                    resourceGroupName = result.choose;
                }
            }
            if(!resourceGroupName) {
                //create resource group
                let result = await inquirer.prompt([{
                    name: 'resourceGroupName',
                    type: 'input',
                    message: `Resource Group Name: `,
                    default: defaultPreffix
                }]);
                resourceGroupName =`${result.resourceGroupName}-group-${guid}`;

                console.log(`Creating resource group [${resourceGroupName}] ...`);
                console.log(`az group create --name ${resourceGroupName} --location westus`);
                let output = await util.execStdout(util.cstr(azureCliLocation) + ` group create --name ${resourceGroupName} --location westus`, 5 * 60000);
                if (output && JSON.parse(output)) {
                    let createResourceResponse = JSON.parse(output);
                    console.log(`Create resource group [${resourceGroupName}] ${createResourceResponse.properties.provisioningState}.`);
                }
            }

            let result = await inquirer.prompt([{
                name: 'iotHubName',
                type: 'input',
                message: `IoT Hub Name: `,
                default: defaultPreffix
            }]);
            iotHubName = `${result.iotHubName}-iothub-${guid}`;

            console.log(`Creating iothub [${iotHubName}] ...`);
            console.log(`az iot hub create -g ${resourceGroupName} -n ${iotHubName} --location westus --sku F1`);
            let _result = await util.execShort(util.cstr(azureCliLocation) + ` iot hub create -g ${resourceGroupName} -n ${iotHubName} --location westus --sku F1`, 15 * 60000);
            
            if (_result.stdout && JSON.parse(_result.stdout)) {
                let createIotHubResponse = JSON.parse(_result.stdout);
                console.log(`Create iot-hub [${createIotHubResponse.name}] ${createResourceResponse.properties.provisioningState}.`);
            } else {
                throw new Error(`Cannot create iot hub due to error: ${_result.stderr}.`);
            }
        }
        
        context.iothub= await azurecli.execResultJson(`iot hub show -n ${iotHubName}`);
        pref.setValue('iot_event_hub', iotHubName);
        pref.savePreference();
        //show connection string
        console.log(`az iot hub show-connection-string -n ${iotHubName}`);
        let output = await azurecli.execResultJson(`iot hub show-connection-string -n ${iotHubName}`);
        let connectionString = output.connectionString;
        if (connectionString) {
            console.log(`[${iotHubName}]: ${connectionString}`);
            context.connectionString = connectionString; 
        } else {
            throw new Error(`Cannot get iot hub connection string.`);
        }
        let deviceList = await azurecli.execResultJson(`iot device list --hub-name ${iotHubName}`);
        let device1 = _.find(deviceList, {"deviceId" : 'myDevice1'});
        if (device1 && device1.status !== 'enabled') {
            console.log(`enable ${device1.deviceId}`);
            await azurecli.execNoOutput(`iot device update --hub-name ${iotHubName} --device-id myDevice1 --set status=enabled`);
        } else if (!device1) {
            let createDeviceRes = await azurecli.execResultJson(`iot device create --hub-name ${iotHubName} --device-id myDevice1`);
            console.log(createDeviceRes);
        }
        console.log("enable/create devices success");
        return JSON.stringify(await azurecli.execResultJson(`iot device show-connection-string --hub-name ${iotHubName} --device-id myDevice1 --key secondary`));
    }
};
