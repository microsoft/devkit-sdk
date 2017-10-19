import {
    buildNewCode,
    removeUsbInstall,
    copyScripts,
	getVersionInfo,
    zipAZ3166Package,
    copyToolChain,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeUsbInstall();
    copyScripts();
	getVersionInfo();
    await zipAZ3166Package();
    copyToolChain();
    await zipFinal();
}

main();