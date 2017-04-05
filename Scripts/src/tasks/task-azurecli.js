import fs from 'fs-plus'
import * as util from '../util'
let run = async () => {
    try {
        let location = await util.findExecutive("az");
        if (location && fs.isFileSync(location)) {
            let ver = /\(([\w\.]+)\)/g.exec((await util.execStdout(util.cstr(location) + " --version", 10000)).split('\n')[0]);
            return ver[1];
        } else {
            throw new Error('Cannot find azure cli, have you installed Azure Cli 2.0 by `pip install azure-cli`');
        }
    }
     catch (error) {
        throw new Error('Cannot find azure cli, have you installed Azure Cli 2.0 by `pip install azure-cli`, if installed, please add `%APPDATA%\\Python\\Scripts` to path');
    }
};


exports.default = {
    name: "azure cli version",
    run
};