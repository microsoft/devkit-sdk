import fs from 'fs-plus'
import path from 'path'
import * as util from '../util'
import Arduino from '../arduino'
let arduino  = new Arduino();
let checkArduinoVersion = async () => {
    try {
        await arduino.resolveArduinoPath();
        if (fs.isFileSync(arduino.command) || fs.isFileSync(path.join(path.dirname(arduino.command), "lib/version.txt"))) {
            return fs.readFileSync(path.join(path.dirname(arduino.command), "lib/version.txt"), 'utf-8') + ' @ ' + path.dirname(arduino.command);
        }
    } catch (error) {
        console.log(error);
    }
    throw new Error("Missing ArduinoIde, please install Arduino Ide.");
};


exports.checkArduinoIde = {
    name: "Arduino IDE version and location",
    run : checkArduinoVersion
};


exports.checkBoard = {
    name: "Arduino board",
    run : () => {
        arduino.loadInstalledBoards();
        return 'TODO';
    }
};
