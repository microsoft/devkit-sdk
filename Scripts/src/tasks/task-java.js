import * as util from './util'
import _ from 'lodash'
let run = async (context) => {
    try {
        let ver = await util.execStderr("java -version");
        return _.trim(ver.split('\n')[0].replace('java version', '').trim(), "\"");
    } catch (error) {
        throw new Error("Missing java executable, please install jdk, if already installed make sure it can be reached from current environment.");
    }
};

exports.default = {
    name: "java version",
    run
};