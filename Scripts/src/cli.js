import fs from 'fs-plus'
import path from 'path'
import _ from 'lodash'
import os from "os"
import gulp from 'gulp'
import chalk from 'chalk'
import prettyTime from 'pretty-hrtime'
import gutil from 'gulp-util'
import {Emitter} from 'event-kit'
import {color, symbols} from './color-output'
import runSequenceLib from 'run-sequence'
const runSequence = runSequenceLib.use(gulp);

const formatError = (e) => {
    if (!e.err) {
        return e.message;
    }

    // PluginError
    if (typeof e.err.showStack === 'boolean') {
        return e.err.toString();
    }

    // Normal error
    if (e.err.stack) {
        return e.err.stack;
    }

    // Unknown (string, number, etc.)
    return new Error(String(e.err)).stack;
};
const logEvents = (gulpInst) => {
    gulpInst.on('task_start', (e) => {
    });

    gulpInst.on('task_stop', (e) => {
    });

    gulpInst.on('task_err', (e) => {
        const msg = formatError(e);
        const time = prettyTime(e.hrDuration);
        gutil.log(`'${chalk.cyan(e.task)}' ${chalk.red('errored after')} ${chalk.magenta(time)}`);
        gutil.log(msg);
    });

    gulpInst.on('task_not_found', (err) => {
        gutil.log(
            chalk.red(`Task '${err.task}' is not in your gulpfile`)
        );
        gutil.log('Please check the documentation for proper gulpfile formatting');
        process.exit(1);
    });
};

logEvents(gulp);

let _tasks = [];

let _context = {};
let _executeTaskAsync = async(_task, cb) => {
    let success = false;
    let result;
    try {
        let fmt = color('pending', '  - %s');
        console.log(fmt, 'checking ' + _task.name);
        result = await Promise.resolve(_task.run(_context));
        success = true;
    } catch (error) {
        //console.log('got error', result);
        result = error.message;
    }
    if (success) {
        let fmt = color('checkmark', '  ' + symbols.ok) +
            color('green', ' %s: ') +
            color('bright pass', '%s');
        console.log(fmt, _task.name, result);
        cb();
    } else {
        let fmt = color('bright fail', '  ' + symbols.err) +
            color('bright fail', ' %s: ') +
            color('bright yellow', '%s');
        console.log(fmt, _task.name, result);
        cb(result);
    }
};
let registerTask = (name) => {
    if (_.isString(name)) {
        _tasks.push(name);
        gulp.task(name, (cb) => {
            try {
                let _task = require('./tasks/task-' + name).default;
                _executeTaskAsync(_task, cb).catch(error => {
                    cb(error);
                });
            } catch (error) {
                console.error(error);
                cb(error);
            }

        });
    } else {
        let obj = name;

        _.each(obj, (value, key) => {
            if (!Array.isArray(value)) {
                value = [value];
            }
            _.each(value, name => {
                _tasks.push(key + ":" + name);
                gulp.task(key + ":" + name, (cb) => {
                    let _task = require('./tasks/task-' + key)[name];
                    _executeTaskAsync(_task, cb).catch(error => {
                        cb(error);
                    });
                });
            });

        });
    }
};

let registerTasks = (...tasks) => {
    for (let task of tasks) {
        registerTask(task);
    }
};

let action = process.argv.slice(2)[0];
let targetFolder = process.argv.slice(3)[0];
if (action === 'build') {
    let defaultUploadTool = path.resolve(path.join(__dirname, '../tools/:PLATFORM/st-flash:EXE_EXT')).replace(':PLATFORM', os.platform())
        .replace(":EXE_EXT", os.platform() === 'win32' ? ".exe" : "");

    let defaultJson = {
        "sketch": "app.ino",
        "board": "azureboard:stm32f4:MXCHIP_MK3166",
        "upload_program": defaultUploadTool,
        "upload_param": "--reset write :PROGRAM 0x8008000"
    };
    let settings = {... defaultJson, ... JSON.parse(fs.readFileSync(path.join(targetFolder, 'config.json')))};
    settings.sketch = path.resolve(path.join(targetFolder, settings.sketch));
    if (!fs.isFileSync(settings.upload_program)) {
        throw new Error(`Cannot find executive ${settings.upload_program}.`);
    }
    _context.arduino_project = {
        rootFolder: path.dirname(settings.sketch),
        settings: settings
    };
    if (settings.config && fs.isFileSync(path.join(_context.arduino_project.rootFolder, '.build', 'runtime.json'))) {
        _context.arduino_project.settings.config_data = JSON.parse(fs.readFileSync(path.join(_context.arduino_project.rootFolder, '.build', 'runtime.json'), 'utf-8'));
        registerTasks('nodejs', 'config', {'arduinoide': ["checkArduinoIde", "checkBoard", "build", "upload"]});
    } else {
        throw new Error("Please run provision command first.");
    }
} else if (action === 'provision') {
    let settings = JSON.parse(fs.readFileSync(path.join(targetFolder, 'config.json')));
    settings.sketch = path.resolve(path.join(targetFolder, settings.sketch));
    _context.arduino_project = {
        rootFolder: path.dirname(settings.sketch),
        settings: settings
    };
    registerTasks('nodejs',{ 'azurecli': ['version', 'login', 'iothub']}, 'output');
} else if (action === 'installation') {
    registerTask({'installation':
        [
            // 'checkPython',
            // 'checkPip',
            // 'installCli',
            // 'installPythonRequestsModule',
            // 'installVsCode',
            // 'installCppExtension',
            // 'installArduinoExtension',
            // 'installArduino',
            // 'setBoardUrl',
            // 'installBoardPackage',
            // 'installSTLink',
            'copyNpmPackage'
        ]
    });
}

setImmediate(() => {
    runSequence(..._tasks);
});
//
// if (testCase === 'provision') {
//     registerTasks('nodejs', { 'azurecli': ['version', 'login', 'iothub'] });
// } else {
//     let caseInstance = require('./arduino-test')[testCase];
//     if (!caseInstance) {
//         throw new Error(`invalid case ${testCase}.`);
//     }
//     _context.arduino_project = caseInstance();
//     registerTasks('nodejs', { 'arduinoide': ["checkArduinoIde", "checkBoard", "checkPort","build", "upload"] });
// }
// setImmediate(() => {
//     runSequence(..._tasks);
// });
//
