import * as util from '../util'
let run = async () => {
    try {
        let ver = /\(([\w\.]+)\)/g.exec((await util.execStdout("az --version")).split('\n')[0]);
        return ver[1];
    } catch (error) {

        throw new Error('Cannot find azure cli, have you installed Azure Cli 2.0 by `pip install azure-cli`, if installed, please add `%APPDATA%\\Python\\Scripts` to path');
    }
};


exports.default = {
    name: "azure cli version",
    run
};