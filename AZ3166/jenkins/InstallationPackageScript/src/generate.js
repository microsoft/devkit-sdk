import glob from 'glob'
import _ from 'lodash'
import * as util from './util';
import path from 'path';
import fsExtra from 'fs-extra';
import fs from 'fs';

const constants = {
    projectDir: __dirname,
    scriptDir: path.join(__dirname, '../IoTDevKitInstallation.Win'),
	packageOrigin: path.join(__dirname, `../TestResult/AZ3166-{version}.${process.env.BUILD_NUMBER}.zip`),
	versionFile: path.join(__dirname, '../system_version.txt'),
	
    devkitDirForWin: path.join(__dirname, '../devkit_install_win'),
	packageDestForWin : path.join(__dirname, '../devkit_install_win/tools/staging/AZ3166-{version}.zip'),
	toolsOriginForWin: path.join(__dirname, '../Tools/win'),		
	toolsDestForWin: path.join(__dirname, '../devkit_install_win/tools/staging'),    
	invalidFolderForWin: path.join(__dirname, '../devkit_install_win/tools/darwin'),
	jsonFileDirForWin: path.join(__dirname, '../devkit_install_win/package.json'),
	finalZipForWin: path.join(__dirname, `../TestResult/devkit_install_win_{version}.${process.env.BUILD_NUMBER}.zip`),
	
	devkitDirForMac: path.join(__dirname, '../devkit_install_mac'),
	packageDestForMac : path.join(__dirname, '../devkit_install_mac/tools/staging/AZ3166-{version}.zip'),
	toolsOriginForMac: path.join(__dirname, '../Tools/mac'),
	toolsDestForMac: path.join(__dirname, '../devkit_install_mac/tools/staging'),
	invalidFolderForMac: path.join(__dirname, '../devkit_install_mac/tools/win32'),
	jsonFileDirForMac: path.join(__dirname, '../devkit_install_mac/package.json'),
	finalZipForMac: path.join(__dirname, `../TestResult/devkit_install_mac_{version}.${process.env.BUILD_NUMBER}.zip`)
};

const command = {
    npmInstall: 'npm install',
    gulpBabel: 'gulp babel',
		
    zipFinalForWin: `7z a -r ../TestResult/devkit_install_win_{version}.${process.env.BUILD_NUMBER}.zip ../devkit_install_win/*`,

	zipFinalForMac: `7z a -r ../TestResult/devkit_install_mac_{version}.${process.env.BUILD_NUMBER}.zip ../devkit_install_mac/*`
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

export function removeDevKitInstall() {
    console.log('remove devkit_install');
    fsExtra.removeSync(constants.devkitDirForWin);
    fsExtra.mkdirSync(constants.devkitDirForWin);
	
	fsExtra.removeSync(constants.devkitDirForMac);
    fsExtra.mkdirSync(constants.devkitDirForMac);
}

export function copyScripts() {
    console.log('copy scripts source');
    let files = fsExtra.readdirSync(constants.scriptDir);
    for (let i = 0; i < files.length; ++i) {
        if (['node_modules', '.git', '.gitignore', 'LICENSE', 'README.md', 'src'].indexOf(files[i]) === -1) {
            fsExtra.copySync(path.join(constants.scriptDir, files[i]), path.join(constants.devkitDirForWin, files[i]));
			fsExtra.copySync(path.join(constants.scriptDir, files[i]), path.join(constants.devkitDirForMac, files[i]));
        }
    }
}

export function getVersionInfo(){
	let data = fs.readFileSync(constants.versionFile);
	versionInfo = data.toString();
}

export function copyAZ3166Package() {
    console.log('copy AZ3166 zip package');
		
	fsExtra.copySync(constants.packageOrigin.replace("{version}",versionInfo), constants.packageDestForWin.replace("{version}",versionInfo));
	fsExtra.copySync(constants.packageOrigin.replace("{version}",versionInfo), constants.packageDestForMac.replace("{version}",versionInfo));
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

export function removeInvalidFiles()
{
	console.log('remove invalid files');
	fsExtra.removeSync(path.join(constants.devkitDirForWin, 'install.sh'));
	fsExtra.removeSync(constants.invalidFolderForWin);
	 
	fsExtra.removeSync(path.join(constants.devkitDirForMac, 'install.cmd'));
	fsExtra.removeSync(constants.invalidFolderForMac);
}

export function updateSerialPort()
{
	console.log('update serialPort');	 
	 	 
	var winFile = require(constants.jsonFileDirForWin);	  
	winFile.dependencies["serialport"] = "^6.0.4";
	 
	var winDest = JSON.stringify(winFile, null, 2);
	fs.writeFile(constants.jsonFileDirForWin, winDest);
	  
	var macFile = require(constants.jsonFileDirForMac);	  
	macFile.dependencies["serialport"] = "^6.0.3";
	 
	var macDest = JSON.stringify(macFile, null, 2);
	fs.writeFile(constants.jsonFileDirForMac, macDest);
	  
}
 
export async function zipFinal() {
    console.log('zip the final package');
    let winFiles = glob.sync(path.dirname(constants.finalZipForWin.replace("{version}",versionInfo)).replace(/\\/g, '/') + '/usb_install_win*.zip');
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