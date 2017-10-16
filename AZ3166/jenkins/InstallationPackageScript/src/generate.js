import glob from 'glob'
import _ from 'lodash'
import * as util from './util';
import path from 'path';
import fsExtra from 'fs-extra';
import fs from 'fs';

const constants = {
    projectDir: __dirname,
    scriptDir: path.join(__dirname, '../IoTDevKitInstallation.Win'),
    usbDir: path.join(__dirname, '../usb_install'),
    packageOrigin: path.join(__dirname, '../devkit-sdk/AZ3166/src'),
    packageDest: path.join(__dirname, '../AZ3166/AZ3166/hardware/stm32f4'),
    finalZip: path.join(__dirname, `../TestResult/usb_install_{version}.${process.env.BUILD_NUMBER}.zip`),
	versionFile: path.join(__dirname, `../system_version.txt`),
	az3166Origin: path.join(__dirname, '../AZ3166'),
	az3166Dest: path.join(__dirname, '../usb_install/tools/AZ3166')
};

const command = {
    npmInstall: 'npm install',
    gulpBabel: 'gulp babel',
    zipPackage: '7z a -r ../usb_install/tools/AZ3166.zip ../AZ3166/*',
    zipFinal: `7z a -r ../TestResult/usb_install_{version}.${process.env.BUILD_NUMBER}.zip ../usb_install/*`
};

const timeout = 600 * 1000;
var versionInfo = "0.0.0";

export async function buildNewCode() {
    process.chdir(constants.scriptDir);
    console.log('npm install');
    await util.execStdout(command.npmInstall, timeout);
    console.log('gulp babel');
    await util.execStdout(command.gulpBabel, timeout);
    process.chdir(constants.projectDir);
}

export function removeUsbInstall() {
    console.log('remove usb_install');
    fsExtra.removeSync(constants.usbDir);
    fsExtra.mkdirSync(constants.usbDir);
}

export function copyScripts() {
    console.log('copy scripts source');
    let files = fsExtra.readdirSync(constants.scriptDir);
    for (let i = 0; i < files.length; ++i) {
        if (['node_modules', '.git', '.gitignore', 'LICENSE', 'README.md'].indexOf(files[i]) === -1) {
            fsExtra.copySync(path.join(constants.scriptDir, files[i]), path.join(constants.usbDir, files[i]));
        }
    }
}
export function getVersionInfo(){
	let data = fs.readFileSync(constants.versionFile);
	versionInfo = data.toString();
}

export function removePackage() {
    console.log('remove package');
    fsExtra.removeSync(constants.packageDest);
	fsExtra.mkdirSync(constants.packageDest);
    fsExtra.mkdirSync(path.join(constants.packageDest,versionInfo));
}

export function copyPackage() {
    console.log('copy package source');
    let files = fsExtra.readdirSync(constants.packageOrigin);
    for (let i = 0; i < files.length; ++i) {
        fsExtra.copySync(path.join(constants.packageOrigin, files[i]), path.join(path.join(constants.packageDest,versionInfo), files[i]));
    }
}

export function copyAZ3166() {
    console.log('copy AZ3166 folder');	
    let files = fsExtra.readdirSync(constants.az3166Origin);
    for (let i = 0; i < files.length; ++i) {
		fsExtra.copySync(path.join(constants.az3166Origin, files[i]), path.join(constants.az3166Dest, files[i]));
    }
}

export async function zipFinal() {
    console.log('zip the final package');
    let files = glob.sync(path.dirname(constants.finalZip.replace("{version}",versionInfo)).replace(/\\/g, '/') + '/usb_install*.zip');
    _.each(files, file => {
        console.log('deleting' ,file);
        fs.unlinkSync(file);
    });

    await util.execStdout(command.zipFinal.replace("{version}",versionInfo), timeout);
}