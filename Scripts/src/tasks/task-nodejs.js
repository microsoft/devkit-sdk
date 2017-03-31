import * as util from '../util'
let run = async (context) => {
    try {
        let ver = (await util.execStdout("node --version")).trim();
        //TODO check ver

        return ver;
    } catch (error) {
        throw new Error("Missing node.js executable, please install node.js, if already installed make sure it can be reached from current environment.");
    }
};

exports.default = {
    name: "node.js version",
    run
};