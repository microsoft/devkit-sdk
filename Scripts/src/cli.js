import _ from 'lodash'
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

let _context = {

};
let _executeTaskAsync = async(_task, cb) => {
    let success = false;
    let result;
    try {
        let fmt = color('pending', '  - %s');
        console.log(fmt, 'checking ' + _task.name);
        result = await Promise.resolve(_task.run(_context));
        success = true;
    } catch (error) {
        console.log('got error', result);
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
            } catch(error) {
                console.error(error);
            }

        });
    } else {
        let obj = name;
        _.each(obj, (value, key) => {
            _.each(value, name => {
                _tasks.push(key + ":" + name);
                gulp.task(key + ":" + name, (cb) => {
                    let _task = require('./task-' + key)[name];
                    _executeTaskAsync(_task, cb).catch(error => {
                        cb(error);
                    });
                });
            });

        });
    }
};

let registerTasks = (...tasks) => {
    for (let name of tasks) {
        registerTask(name);
    }
};

registerTasks('nodejs', 'azurecli'/*,  {'arduino': ["checkArduinoIde", "checkBoard", "checkPort", "build"]}*/);
setImmediate(() => {
    runSequence(..._tasks);
});