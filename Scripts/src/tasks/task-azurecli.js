import fs from 'fs-plus'
import * as util from '../util'
let azureCliLocation;
let versionCheck = async () => {
    try {
        let location = await util.findExecutive("az");
        if (location && fs.isFileSync(location)) {
            let ver = /\(([\w\.]+)\)/g.exec((await util.execStdout(util.cstr(location) + " --version", 10000)).split('\n')[0]);
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

exports.login = {
    name: "azure cli login",
    run: () => {

    }
};