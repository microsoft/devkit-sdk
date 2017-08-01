import {
    buildNewCode,
    removeUsbInstall,
    copyScripts,
	getVersionInfo,
    removePackage,
    copyPackage,
    zipPackage,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeUsbInstall();
    copyScripts();
	getVersionInfo();
    removePackage();
    copyPackage();
    await zipPackage();
    await zipFinal();
}

main();