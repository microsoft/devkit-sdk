import fs from 'fs-plus'
import path from 'path'
import _ from 'lodash'
let run = async(context) => {
    try {
        if (context.arduino_project && context.arduino_project.settings) {
            let settings = context.arduino_project.settings;
            if (settings.config && settings.config_data) {
                let outputFile = path.join(context.arduino_project.rootFolder, settings.config);
                let lines = [];
                _.each(settings.config_data, (value, key) => {
                    if (_.isString(value)) {
                        lines.push(`#define ${key} "${value}"`);
                    } else if (_.isNumber(value)) {
                        lines.push(`#define ${key} ${value}`);
                    }
                });
                let dataContent = `#ifndef CONFIG_H
#define CONFIG_H
${lines.join('\n')}
#endif
`;
                fs.writeFileSync(outputFile, dataContent);
                return 'ok';
            }
        } else {
            return 'ignore';
        }


    } catch (error) {
        console.error(error);
        throw new Error("Error on generating config.", error);
    }
};

exports.default = {
    name: "configuration",
    run
};