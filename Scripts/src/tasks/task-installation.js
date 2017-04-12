import * as util from '../util';
import semver from 'semver';
import path from 'path';
import fs from 'fs-extra';
import admzip from 'adm-zip';

const pythonMsiPath = path.join(__dirname, '../../tools/win32/python-2.7.13.msi');
const pythonInstallPath = 'C:\\Python27';
const nodeMsiPath = path.join(__dirname, '../../tools/win32/node-v6.10.2-x86.msi');
const nodeInstallPath = path.join(process.env['ProgramFiles(x86)'], 'nodejs');
const vsCodeInstallPath = path.join(__dirname, '../../tools/win32/VSCodeSetup-1.11.1.exe');
const arduinoInstallerPath = path.join(__dirname, '../../tools/win32/arduino-1.8.1-windows.exe');

const constants = {
    checkPython: 'python --version',
    pythonInstall: `msiexec /i "${pythonMsiPath}" TARGETDIR="${pythonInstallPath}" /passive /norestart ADDLOCAL=ALL`,
    nodeInstall: `msiexec /i "${nodeMsiPath}" TARGETDIR="${nodeInstallPath}" /passive /norestart ADDLOCAL=ALL`,
    vsCodeInstall: `${vsCodeInstallPath} /SILENT /mergetasks=!runcode`,
    vsCodeExePath: path.join(process.env['ProgramFiles(x86)'], 'Microsoft VS Code/bin/code'),
    vsCodeExtensionInstallPath: path.join(process.env['USERPROFILE'], '.vscode/extensions'),
    arduinoInstall: `${arduinoInstallerPath} /S`,
    arduinoPath: path.join(process.env['ProgramFiles(x86)'], 'Arduino/arduino_debug.exe'),
    arduinoExtensionNamePrefix: 'Microsoft.vscode-arduino',
    arduinoExtensionPath: path.join(__dirname, '../../tools/vscode-arduino-0.0.9-beta.vsix'),
    checkPip: 'pip --version',
    checkCli: 'az --version',
    checkNode: 'node --version',
    checkVsCode: 'code --version',
    boardManagerUrl: 'https://raw.githubusercontent.com/lirenhe/arduinoazureboard/master/package_azureboard_index.json',
    customBoardZip: path.join(__dirname, '../../tools/azureboard.zip'),
    arduinoPackagePath: path.join(process.env['USERPROFILE'], 'AppData/Local/Arduino15/packages'),

};

const timeout = 600 * 1000;

exports.checkPython = {
    name: 'python version',
    run: async () => {
        try {
            let ver = await util.execStderr(constants.checkPython);
            ver.replace('Python', '').trim();
            if (ver[0] === '2' && semver.lt(ver, '2.7.9') || ver[0] === '3' && semver.lt(ver, '3.4')) {
                await util.execStdout(constants.pythonInstall, timeout);
            } else {
                return ver;
            }
        } catch (error) {
            await util.execStdout(constants.pythonInstall, timeout);
        }
    }
};

exports.checkPip = {
    name: 'pip version',
    run: async (context) => {
        try {
            let ver = await util.execStdout(constants.checkPip, timeout);
            ver.replace('pip', '').trim();
            context.pip = 'pip';
            return ver;
        } catch (error) {
            const pipPath = path.join(pythonInstallPath, 'Scripts', 'pip.exe');
            try {
                let ver = await util.execStdout(`${pipPath} --version`, timeout);
                ver.replace('pip', '').trim();
                context.pip = pipPath;
                return ver;
            } catch (error) {
                throw error;
            }
        }
    }
};

exports.checkCli = {
    name: 'azure cli version',
    run: async (context) => {
        try {
            await util.execStdout(`${context.pip} install --user azure-cli`, timeout);
        } catch (error) {
            throw error;
        }
    }
};

exports.installPythonRequestsModule = {
    name: 'python requests module',
    run: async (context) => {
        try {
            await util.execStdout(`${context.pip} install requests`, timeout);
        }catch (error) {
            throw error;
        }
    }
};

exports.installNode = {
    name: 'node version',
    run: async () => {
        try {
            let ver = await util.execStdout(constants.checkNode);
            ver.replace('v', '').trim();
            if (semver.lt(ver, '4.9.0')) {
                await util.execStdout(constants.nodeInstall, timeout);
            } else {
                return ver;
            }
        } catch (error) {
            await util.execStdout(constants.nodeInstall, timeout);
        }
    }
};

exports.installVsCode = {
    name: 'vs code version',
    run: async(context) => {
        try {
            const ver = await util.execStdout(constants.checkVsCode);
            context.code = 'code';
            return ver;
        } catch (error) {
            await util.execStdout(constants.vsCodeInstall, timeout);
            context.code = constants.vsCodeExePath;
        }
    }
};

exports.installCppExtension = {
    name: 'install cpp extension',
    run: async(context) => {
        try {
            await util.execStdout(`"${context.code}" --install-extension ms-vscode.cpptools`, timeout);
        } catch (error) {
            throw error;
        }
    }
};

exports.installArduinoExtension = {
    name: 'install arduino extension',
    run: async(context) => {
        try {
            if (fs.existsSync(constants.vsCodeExtensionInstallPath)) {
                const dirs = fs.readdirSync(constants.vsCodeExtensionInstallPath);
                for (let i = 0; i < dirs.length; i++) {
                    if (dirs[i].startsWith(constants.arduinoExtensionNamePrefix)) {
                        fs.removeSync(path.join(constants.vsCodeExtensionInstallPath, dirs[i]));
                        break;
                    }
                }
            }
            await util.execStdout(`"${context.code}" --install-extension ${constants.arduinoExtensionPath}`, timeout);
        } catch (error) {
            throw error;
        }
    }
};

exports.installArduino = {
    name: 'arduino install',
    run: async() => {
        try {
            await util.execStdout(constants.arduinoInstall, timeout);
        } catch (error) {
            throw error;
        }
    }
};

exports.setBoardUrl = {
    name: 'set custom board manager url',
    run: async() => {
        try {
            await util.execStdout(`"${constants.arduinoPath}" --pref boardsmanager.additional.urls=${constants.boardManagerUrl} --save-prefs`);
        } catch (error) {
            throw error;
        }
    }
};

exports.copyBoardPackage = {
    name: 'copy board package',
    run: async() => {
        try {
            if (!fs.existsSync(constants.arduinoPackagePath)){
                fs.mkdirSync(constants.arduinoPackagePath);
            }
            const filePath = path.join(constants.arduinoPackagePath, 'azureboard.zip');
            fs.writeFileSync(filePath, fs.readFileSync(constants.customBoardZip));
            let zip = new admzip(filePath);
            zip.extractAllTo(constants.arduinoPackagePath, true);
        } catch (error) {
            throw error;
        }
    }
};


exports.copyNpmPackage = {
    name: 'copy npm package',
    run: async() => {
        // copy file
        // try {
        //     fs.copySync('D:\\work\\azure-python-cli\\files', 'D:\\aaa');
        // } catch (error) {
        //     throw error;
        // }
    }
};
