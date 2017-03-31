import fs from 'fs-plus'
import path from 'path'
import * as util from '../util'
import Arduino from '../arduino'
import os from "os"
import plist from 'plist'
let arduino  = new Arduino();

let parsePackageInfo = (file) => {
    return plist.parse(fs.readFileSync(file, 'utf8'));
};
let checkArduinoVersion = async () => {
    try {
        await arduino.resolveArduinoPath();

        if (fs.isFileSync(arduino.command)) {
              const plat = os.platform();
            if (plat === 'win32') {
                if (fs.isFileSync(path.join(path.dirname(arduino.command), "lib/version.txt"))) {
                    return fs.readFileSync(path.join(path.dirname(arduino.command), "lib/version.txt"), 'utf-8') + ' @ ' + path.dirname(arduino.command);
                } else {
                    throw new Error('cannnot find lib/version.txt in arduino installation path.');
                }
            } else if (plat === 'darwin') {
                let current = path.dirname(arduino.command);
                for (let i = 0; i < 3; i++) {
                    if (fs.isFileSync(path.join(current, 'Info.plist'))) {
                        return parsePackageInfo(path.join(current, 'Info.plist'), 'utf-8') + ' @ ' + current;
                    }
                    current = path.dirname(current);
                }                
                throw new Error('cannnot find Info.plist in arduino installation path.');
            }       
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
