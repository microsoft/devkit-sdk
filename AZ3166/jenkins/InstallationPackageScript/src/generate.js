import glob from 'glob'
import _ from 'lodash'
import * as util from './util';
import path from 'path';
import fsExtra from 'fs-extra';
import fs from 'fs';

const constants = {
    projectDir: __dirname,
    scriptDir: path.join(__dirname, '../IoTDevKitInstallation.Win'),
	packageOrigin: path.join(__dirname, '../devkit-sdk/AZ3166/src'),
	versionFile: path.join(__dirname, `../system_version.txt`),
	
    usbDirForWin: path.join(__dirname, '../usb_install_windows'),	
	toolsOriginForWin: path.join(__dirname, '../Tools/windows'),		
	toolsDestForWin: path.join(__dirname, '../usb_install_windows/tools/staging'),    
	finalZipForWin: path.join(__dirname, `../TestResult/usb_install_windows_{version}.${process.env.BUILD_NUMBER}.zip`),
	
	usbDirForMac: path.join(__dirname, '../usb_install_mac'),
	toolsOriginForMac: path.join(__dirname, '../Tools/mac'),
	toolsDestForMac: path.join(__dirname, '../usb_install_mac/tools/staging'),
	finalZipForMac: path.join(__dirname, `../TestResult/usb_install_mac_{version}.${process.env.BUILD_NUMBER}.zip`)
};

const command = {
    npmInstall: 'npm install',
    gulpBabel: 'gulp babel',
	
    zipPackageForWin: '7z a -r ../usb_install_windows/tools/staging/AZ3166-{version}.zip ../devkit-sdk/AZ3166/src/*',		
    zipFinalForWin: `7z a -r ../TestResult/usb_install_windows_{version}.${process.env.BUILD_NUMBER}.zip ../usb_install_windows/*`,
	
	zipPackageForMac: '7z a -r ../usb_install_mac/tools/staging/AZ3166-{version}.zip ../devkit-sdk/AZ3166/src/*',
	zipFinalForMac: `7z a -r ../TestResult/usb_install_mac_{version}.${process.env.BUILD_NUMBER}.zip ../usb_install_mac/*`
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
    fsExtra.removeSync(constants.usbDirForWin);
    fsExtra.mkdirSync(constants.usbDirForWin);
	
	fsExtra.removeSync(constants.usbDirForMac);
    fsExtra.mkdirSync(constants.usbDirForMac);
}

export function copyScripts() {
    console.log('copy scripts source');
    let files = fsExtra.readdirSync(constants.scriptDir);
    for (let i = 0; i < files.length; ++i) {
        if (['node_modules', '.git', '.gitignore', 'LICENSE', 'README.md'].indexOf(files[i]) === -1) {
            fsExtra.copySync(path.join(constants.scriptDir, files[i]), path.join(constants.usbDirForWin, files[i]));
			fsExtra.copySync(path.join(constants.scriptDir, files[i]), path.join(constants.usbDirForMac, files[i]));
        }
    }
}
export function getVersionInfo(){
	let data = fs.readFileSync(constants.versionFile);
	versionInfo = data.toString();
}

export async function zipAZ3166Package() {
    console.log('zip the board package');
    await util.execStdout(command.zipPackageForWin.replace("{version}",versionInfo), timeout);
	await util.execStdout(command.zipPackageForMac.replace("{version}",versionInfo), timeout);
}

 export function copyToolChain() {
     console.log('copy tools');
     let winFiles = fsExtra.readdirSync(constants.toolsOriginForWin);
     for (let i = 0; i < winFiles.length; ++i) {
         fsExtra.copySync(path.join(constants.toolsOriginForWin, winFiles[i]), path.join(constants.toolsDestForWin, winFiles[i]));
     }
	 
	 let macFiles = fsExtra.readdirSync(constants.toolsOriginForMac);
     for (let i = 0; i < macFiles.length; ++i) {
         fsExtra.copySync(path.join(constants.toolsOriginForMac, macFiles[i]), path.join(constants.toolsDestForMac, macFiles[i]));
     }
 }

export async function zipFinal() {
    console.log('zip the final package');
     let winFiles = glob.sync(path.dirname(constants.finalZipForWin.replace("{version}",versionInfo)).replace(/\\/g, '/') + '/usb_install_windows*.zip');
     _.each(winFiles, file => {
         console.log('deleting' ,file);
         fs.unlinkSync(file);
     });
	 
	 let macFiles = glob.sync(path.dirname(constants.finalZipForMac.replace("{version}",versionInfo)).replace(/\\/g, '/') + '/usb_install_mac*.zip');
     _.each(macFiles, file => {
         console.log('deleting' ,file);
         fs.unlinkSync(file);
     });

	await util.execStdout(command.zipFinalForMac.replace("{version}",versionInfo), timeout);
    await util.execStdout(command.zipFinalForWin.replace("{version}",versionInfo), timeout);
}