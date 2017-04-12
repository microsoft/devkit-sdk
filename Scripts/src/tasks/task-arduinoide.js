import _ from 'lodash'
import childProcess from "child_process";
import fs from 'fs-plus'
import path from 'path'
import * as util from '../util'
import Arduino from '../arduino'
import os from "os"
import plist from 'plist'
import Openocd from '../openocd'
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
            let sketchFile = path.resolve(rootFolder, settings.sketch || 'app/app.ino');
            if (!fs.isFileSync(sketchFile)) {
                throw new Error(`${sketchFile} cannot be found.`);
            }
            await arduino.compile(settings.board, sketchFile, path.join(rootFolder, '.build'), output=> {
                console.log(output);
            });
        } catch (error) {
            throw new Error(error.message);
        }
        return 'success';
    }
};


exports.upload = {
    name: "Sketch Upload",
    run: async(context) => {

        try {
            let settings = context.arduino_project.settings;
            let rootFolder = context.arduino_project.rootFolder;
            let sketchFile = path.resolve(rootFolder, settings.sketch || 'app/app.ino');
            let binFile = path.join(rootFolder, '.build', path.basename(sketchFile) + '.bin');
            if (!fs.isFileSync(binFile)) {
                throw new Error(`could not find bin file : ${binFile}`);
            }

            if (settings.upload_program) {
                //console.log(settings.upload);
                let upload_program = settings.upload_program;
                console.log(path.resolve(upload_program) + ' ' + settings.upload_param.replace(':PROGRAM', util.cstr(binFile)));
                let result =  await util.execShort(util.cstr(path.resolve(upload_program)) + ' ' + settings.upload_param.replace(':PROGRAM', util.cstr(binFile)), 1000*100);
                console.log(result.stderr);
                console.log(result.stdout);
                return 'ok';
            }
            let openocdPath = path.dirname(arduino.getArduinoToolPath('openocd', 'openocd'));
            let openocd = new Openocd({
                openocdPath, outFunc: (data) => {
                    console.log('[openocd]', data);
                }});
            await openocd.execute(settings.debug_interface, settings.transport, settings.target, settings.openocd_scripts.replace(':PROGRAM', binFile));

            return 'ok';
        } catch (error) {
            throw new Error(error.message);
        }
        return 'success';
    }
}
