import {
    buildNewCode,
    removeDevKitInstall,
    copyScripts,
	getVersionInfo,
    copyAZ3166Package,
    copyToolChain,
    zipFinal,
} from './generate';

async function main() {
    await buildNewCode();
    removeDevKitInstall();
    copyScripts();
	getVersionInfo();
    copyAZ3166Package();
    copyToolChain();
    await zipFinal();
}

main();