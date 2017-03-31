import fs from 'fs-plus'
import path from 'path'
import * as util from '../util'
import Arduino from '../arduino'
import os from "os"
import plist from 'plist'
let arduino = new Arduino();

let parsePackageInfo = (file) => {
    return plist.parse(fs.readFileSync(file, 'utf8'));
};
let checkArduinoVersion = async() => {
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
                        return parsePackageInfo(path.join(current, 'Info.plist')).CFBundleShortVersionString + ' @ ' + current;
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
    run: checkArduinoVersion
};


exports.checkBoard = {
    name: "Arduino board",
    run: (context) => {
        arduino.loadInstalledBoards();
        if (!context.arduino_project) {
            throw new Error('Missing project in context.');
        }
        let boards = arduino._boards;
        let definition = context.arduino_project.settings.board;
        if (!definition) {
            throw new Error(`Cannot find board in arduino.json`);
        }
        let cs = definition.split(':');
        if (!boards[cs[2].trim()]) {
            throw new Error(`Cannot find board '${cs[2]}' in installed board list.`);
        }
        let board = boards[cs[2].trim()];
        if (board.platform !== cs[0].trim()) {
            throw new Error(`Invalid platform '${cs[0]}' in board settings: ${definition}.`);
        }
        if (board.architecture !== cs[1].trim()) {
            throw new Error(`Invalid arch '${cs[1]}' in board settings: ${definition}.`);
        }
        return cs[2].trim() + ' as ' + boards[cs[2].trim()].name;
    }
};


exports.checkPort = {
    name: "Arduino Port",
    run: () => {
        return 'TODO';
    }
};

exports.build = {
    name: "Sketch Build",
    run: async(context) => {

        try {
            let settings = context.arduino_project.settings;
            let rootFolder = context.arduino_project.rootFolder;
            let sketchFile = path.resolve(rootFolder, settings.sketch || 'app/app.info');
            if (!fs.isFileSync(sketchFile)) {
                throw new Error(`${sketchFile} cannot be found.`);
            }
            await arduino.compile(settings.board, sketchFile, path.join(rootFolder, '.build'));
        } catch (error) {
            throw new Error(error.message);
        }
        return 'success';
    }
};