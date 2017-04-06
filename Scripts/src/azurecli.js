import fs from 'fs-plus'
import path from 'path'
import * as util from './util'
const TIMEOUT = 60000;
export default class AzureCli {
    constructor(location) {
        if (!fs.isFileSync(location)) {
            throw new Error(`"${location}" doesn't exist.`);
        }
        this.location = util.cstr(location);
    }

    exec(command) {
        return util.execShort(this.location + ' ' + command, TIMEOUT);
    }

    async version() {
        let result = await this.exec("--version");
        if (result.stderr) {
            throw new Error(result.stderr);
        }
        if (!result.stdout) {
            throw new Error('no output for az --version');
        }
        return /\(([\w\.]+)\)/g.exec(result.stdout.split('\n')[0])[1];
    }

    async execResultJson(command, timeout) {
        let output = await util.execStdout(this.location + " " + command, timeout || TIMEOUT);
        if (output) {
            try {
                return JSON.parse(output);
            } catch (error) {
                throw new Error(`Bad result when executing "az ${command}":${output}`);
            }
        } else {
            throw new Error(`No output when executing "az ${command}"`);
        }
    }
    execNoOutput(command, timeout) {
         return util.execStdout(this.location + " " + command,  timeout || TIMEOUT);
    }
}