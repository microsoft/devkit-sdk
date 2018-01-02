import {
    buildNewCode,
    removeDevKitInstall,
    copyScripts,
	getVersionInfo,
    copyAZ3166Package,
    copyToolChain,
	removeInvalidFiles,
	copyTaskFolder,
	updateSerialPort,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeDevKitInstall();
    copyScripts();
	getVersionInfo();
    copyAZ3166Package();
    copyToolChain();
	removeInvalidFiles();
	copyTaskFolder();
	updateSerialPort();
    await zipFinal();
}

main();