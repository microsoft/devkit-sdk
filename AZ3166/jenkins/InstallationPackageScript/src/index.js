import {
    buildNewCode,
    removeDevKitInstall,
    copyScripts,
    getVersionInfo,
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
    copyToolChain();
    removeInvalidFiles();
    copyTaskFolder();
    updateSerialPort();
    await zipFinal();
}

main();