import {
    buildNewCode,
    removeDevKitInstall,
    copyScripts,
	getVersionInfo,
	zipAZ3166Package,
    copyAZ3166Package,
    copyToolChain,
	removeInvalidFiles,
	updateSerialPort,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeDevKitInstall();
    copyScripts();
	getVersionInfo();
	await zipAZ3166Package();
    copyAZ3166Package();
    copyToolChain();
	removeInvalidFiles();
	updateSerialPort();
    await zipFinal();
}

main();