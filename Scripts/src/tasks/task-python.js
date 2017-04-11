import * as util from '../util'
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