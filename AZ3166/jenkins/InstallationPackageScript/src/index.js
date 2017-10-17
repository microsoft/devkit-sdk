import {
    buildNewCode,
    removeUsbInstall,
    copyScripts,
	getVersionInfo,
    removePackage,
    copyPackage,
    copyAZ3166,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeUsbInstall();
    copyScripts();
	getVersionInfo();
    removePackage();
    copyPackage();
    copyAZ3166();
    await zipFinal();
}

main();