import * as util from '../util'
let run = async () => {
    try {
        let ver = /\(([\w\.]+)\)/g.exec((await util.execStdout("az --version")).split('\n')[0]);
        return ver[1];
    } catch (error) {

        throw new Error("Missing python executable, please install Python, if already installed make sure it can be reached from current environment.");
    }
};


exports.default = {
    name: "azure cli version",
    run
};