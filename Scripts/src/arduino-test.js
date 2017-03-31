import fs from 'fs-plus'
import path from 'path'
let example = (rootFolder) => {
    rootFolder = path.resolve(__dirname, rootFolder);
    if (!fs.isDirectorySync(rootFolder)) {
        throw new Error(`Folder ${rootFolder} doesn't exist.`);
    }
    let settings = JSON.parse(fs.readFileSync(path.join(rootFolder, 'arduino.json')));
    return {
        rootFolder,
        settings: settings
    };
};

export function blink() {
    return example('../test/Blink');
}