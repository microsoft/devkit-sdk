import * as util from '../util'
import fs from 'fs-plus'
import uuidV4 from 'uuid/v4'
let run = async () => {
    try {
        let ver = await util.execStderr("python --version");
        //TODO check ver
        return ver.replace('Python', '').trim();
    } catch (error) {
        throw new Error("Missing python executable, please install Python, if already installed make sure it can be reached from current environment.");
    }
};


exports.default = {
    name: "python version",
    run
};

exports.azureFunc = {
    name: "azure function deploy",
    run: async (context) => {
        console.log(context);
        if(context.skipDeployAzureFunction){
            return 'skipped';
        }
        let endpointPath = context.iothub.properties.eventHubEndpoints.events.path;

        let functionName = `tweetfunction-${uuidV4().slice(0, 4)}`;
        console.log(functionName);
        let functionAppPath = "./azureFunction/";

        let functionConf = {
            "bindings": [
                {
                "type": "eventHubTrigger",
                "name": "myEventHubTrigger",
                "direction": "in",
                "path": endpointPath,
                "connection": "eventHubConnectionString"
                }
            ],
            "disabled": false
        };

        // update function.json with eventhub.path
        fs.writeFileSync(`${functionAppPath}/function.json`, JSON.stringify(functionConf), 'utf8');
        let scriptPath = "./src/python/main.py"

        // TODO: hard code my personal twitter dev token, 450 quotas/15min
        let params = {
            twitterAPI: "https://api.twitter.com/1.1/search/tweets.json",
            twitterBearerKey: "AAAAAAAAAAAAAAAAAAAAAGVU0AAAAAAAucpxA9aXc2TO6rNMnTcVit1P3YM%3DrQpyFeQ6LOwyvy7cqW5djhLPnFfjEK8H3hA1qfGDh93JRbI1le",
            iotHubConnectionString: context.connectionString,
            eventHubConnectionString: `Endpoint=${context.iothub.properties.eventHubEndpoints.events.endpoint};${context.connectionString}`
        };

        let cmd = `python ${scriptPath} -S ${context.subscription.id} -N ${functionName} -G ${context.iothub.resourceGroup} -D ${functionAppPath} --settings `;
        Object.keys(params).forEach((k, v)=>{cmd+=`${k}=${params[k]} `});
        console.log(cmd);

        // npm install takes long time
        await util.execStdout(cmd, 10*60000);
        return 'success';
    }
}