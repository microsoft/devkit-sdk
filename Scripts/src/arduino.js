import os from "os";
import path from "path";
import glob from 'glob'
import fs from 'fs-plus'
import mkdirp from 'mkdirp'
import * as util from './util'
import BufferedProcess from './bufferred-process'

let readConfigFileToJson = (file) => {
    if (!fs.isFileSync(file)) return {};
    const regex = /([\w\.]+)=(.+)/;
    const rawText = fs.readFileSync(file, "utf8");
    let result = {};
    let lines = rawText.split(/[\r|\r\n|\n]/);
    lines.forEach((line) => {
        // Ignore comments and menu description lines.
        if (!line || line.startsWith("#") || line.startsWith("menu.")) {
            return;
        }
        let match = regex.exec(line);
        if (match && match[1] && match.length > 2) {
            result[match[1]] = match[2];
        }
    });
    return result;
};


export default class Arduino {
    constructor(opt) {
        this.opt = Object.assign({}, opt);
        const platform = os.platform();
        if (platform === "win32") {
            this._packagePath = path.join(process.env.LOCALAPPDATA, "Arduino15");
            this._libPath = path.join(process.env.USERPROFILE, "Documents/Arduino/libraries");
        } else if (platform === "linux") {
            this._packagePath = path.join(process.env.HOME, ".arduino15");
            this._libPath = path.join(process.env.HOME, "Arduino/libraries");
        } else if (platform === "darwin") {
            this._packagePath = path.join(process.env.HOME, "Library/Arduino15");
            this._libPath = path.join(process.env.HOME, "Documents/Arduino/libraries");
        }

        if (!fs.isDirectorySync(this._packagePath)) {
            throw new Error(`Cannot find package path at ${this._packagePath}`);
        }

        if (!fs.isDirectorySync(this._libPath)) {
            throw new Error(`Cannot find lib path at ${this._libPath}`);
        }
    }
    async _internalResolveArduinoPath() {
        const plat = os.platform();
        try {
            if (plat === "win32") {
                let arduinoLocation = path.resolve(await util.execStdout("where arduino_debug")).trim();
                if (fs.existsSync(arduinoLocation)) {
                    return path.dirname(path.resolve(arduinoLocation));
                }
            } else if (plat === "linux") {
                let arduinoLocation = path.resolve(await util.execStdout("readlink -f $(which arduino)")).trim();
                if (fs.existsSync(arduinoLocation)) {
                    return path.dirname(path.resolve(arduinoLocation));
                }
            } else if (plat === 'darwin') {
                let arduinoLocation = path.resolve(await util.execStdout("readlink -n $(which arduino)")).trim();
                if (fs.existsSync(arduinoLocation)) {
                    return path.dirname(path.resolve(arduinoLocation));
                }
            }
        } catch (ex) {
            // Ignore the errors.
        }
        if (plat === "win32") {
            const defaultCommonPaths = [process.env.ProgramFiles, process.env["ProgramFiles(x86)"]];
            for (let scanPath of defaultCommonPaths) {
                if (scanPath && fs.isDirectorySync(path.join(scanPath, "Arduino"))) {
                    return path.join(scanPath, "Arduino");
                }
            }
        } else if (plat === "darwin") {
            const defaultCommonPaths = [path.join(process.env.HOME, "Applications"), "/Applications"];
            for (let scanPath of defaultCommonPaths) {
                if (fs.isDirectorySync(path.join(scanPath, "Arduino.app"))) {
                    return scanPath;
                }
            }
        }
    }
    _resolveArduinoExecutive(arduinoPath) {
        const platform = os.platform();
        let arduinoExe = "";
        if (platform === "darwin") {
            arduinoExe = path.join(arduinoPath, "Arduino.app/Contents/MacOS/Arduino");
        } else if (platform === "linux") {
            arduinoExe = path.join(arduinoPath, "arduino");
        } else if (platform === "win32") {
            arduinoExe = path.join(arduinoPath, "arduino_debug.exe");
        }
        if (!fs.isFileSync(arduinoExe)) {
            throw new Error(`Cannot find arduino executive at ${arduinoExe}`);
        }
        return arduinoExe;
    }

    async resolveArduinoPath() {
        this.arduinoPath = this.opt["arduinoPath"] || await this._internalResolveArduinoPath();
        if (!fs.isDirectorySync(this.arduinoPath)) {
            throw new Error(`Cannot find arduino path at ${this.arduinoPath}`);
        }
        this.command = this._resolveArduinoExecutive(this.arduinoPath);
    }

    _parseBoardDescriptorFile(boardDescriptor, platform, architecture, platformDisplayName) {
        const boardLineRegex = /([^\.]+)\.(\S+)=(.+)/;

        let result = {};
        let lines = boardDescriptor.split(/[\r|\r\n|\n]/);
        lines.forEach((line) => {
            // Ignore comments and menu description lines.
            if (line.startsWith("#") || line.startsWith("menu.")) {
                return;
            }
            let match = boardLineRegex.exec(line);
            if (match && match.length > 3) {
                let boardObject = result[match[1]];
                if (!boardObject) {
                    boardObject = {
                        board: match[1],
                        platform: platform,
                        platformName: platformDisplayName,
                        architecture: architecture,
                    };
                    result[boardObject.board] = boardObject;
                }
                if (match[2] === "name") {
                    boardObject.name = match[3].trim();
                }
            }
        });
        return result;
    }


    loadInstalledBoards() {
        this._boards = {};
        let files = glob.sync([this._packagePath.replace(/\\/g, '/'), "packages", "*", "hardware", "*", "*", "boards.txt"].join('/'));
        files.forEach(file => {
            let boardContent = fs.readFileSync(file, "utf8");
            let _paths = path.dirname(file).split('/');
            let packagesIndex = _paths.indexOf('packages');
            if (packagesIndex < 0) {
                return;
            }
            let platformObj = readConfigFileToJson(path.join(path.dirname(file), 'platform.txt'));

            let platformName = _paths[packagesIndex + 1];
            let architecture = _paths[packagesIndex + 3];

            this._boards = {...this._boards, ...this._parseBoardDescriptorFile(boardContent, platformName, architecture, platformObj.name)};
        });
        // for (let k of Object.keys(this._boards)) {
        //     console.log(k, ' ==> ', this._boards[k].name, this._boards[k].platformName);
        // }
    }

    compile(boardPlatformArch, sketch, outputDir, outputFunc) {
        if (!this._boards) {
            throw new Error('Please initialize first before compile.');
        }
        if (!fs.isDirectorySync(outputDir)) {
            mkdirp.sync(outputDir);
        }
        let args = ["--verify", "--board", boardPlatformArch, "--preferences-file",
            `${outputDir}/pref.txt`, "--pref",
            "build.path=" +
            `${outputDir}`, sketch];
        let files = glob.sync([outputDir.replace(/\\/g, '/'), "*.elf"].join('/'));
        if (files && files.length) {
            files.forEach(function(file) {
                console.log('Deleting' ,file);
                fs.unlinkSync(file);
            });
        }
        return new Promise(async(resolve, reject) => {
            try {
                
                await new BufferedProcess({
                    command: this.command,
                    args: args,
                    stderr: (data) => {
                        console.error(data);
                    },
                    stdout: (data) => {
                        console.log(data);
                    },
                    exit: (exitCode) => {
                        if (exitCode === 0) {
                            console.log(`Arduino Build Success!`, 'stdout');
                            resolve();
                        } else {
                            console.log(`Arduino compile exit with code ${exitCode}.`, 'stderr');
                            reject(`Arduino compile exit with code ${exitCode}.`);
                        }
                    }
                }).spawn();

            } catch(err) {
                reject(err);
            }
        });
    }

    getArduinoToolPath(tool, executive) {
        if (fs.isDirectorySync(path.join(this._packagePath, 'packages', 'arduino', 'tools', tool))) {
            const platform = os.platform();
            let parent = path.join(this._packagePath, 'packages', 'arduino', 'tools', tool).replace(/\\/g,  '/');
            let files = glob.sync(parent + '/**/bin/' +
                (platform === 'win32' ? path.basename(executive, '.exe') + '.exe': executive));
            if (files.length) return files[0];
            else return undefined;
        }
        return undefined;
    }
}