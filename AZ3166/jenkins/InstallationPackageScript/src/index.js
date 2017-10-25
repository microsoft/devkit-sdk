import {
    buildNewCode,
    removeDevKitInstall,
    copyScripts,
	getVersionInfo,
    copyAZ3166Package,
    copyToolChain,
	removeInvalidFiles,
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
    await zipFinal();
}

main();