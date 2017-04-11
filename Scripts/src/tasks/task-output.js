import fs from 'fs-plus'
import path from 'path'
import _ from 'lodash'
import mkdirp from 'mkdirp'
let run = async(context) => {
    if (context.arduino_project && context.output && _.keys(context.output)) {
        mkdirp.sync(path.join(context.arduino_project.rootFolder, '.build'));
        let outputFile = path.join(context.arduino_project.rootFolder, '.build', 'runtime.json');
        fs.writeFileSync(outputFile, JSON.stringify(context.output, null, 4));
        return outputFile;
    }
};
exports.default = {
    name: "output",
    run
};