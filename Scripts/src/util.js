import BufferedProcess from './bufferred-process'
import fs from 'fs-plus'
import path from 'path'
import _ from 'lodash'
import os from 'os'
import childProcess from "child_process";
export const cstr = (s) => {
    let c, esc, i, len;
    esc = '';
    for (i = 0, len = s.length; i < len; i++) {
        c = s[i];
        switch (c) {
            case '"':
                c = '\\"';
                break;
            case '\\':
                c = '\\\\';
        }
        esc += c;
    }
    return "\"" + esc + "\"";
};

export const timeout = (ms) => {
    return new Promise(resolve => setTimeout(resolve, ms));
};

export function exec(command) {
    try {
        let _process = childProcess.exec(command, {encoding: "utf8"});
        let _stdout = [];
        let _stderr = [];
        _process.stdout.on('data', function (data) {
            _stdout.push(data);
        });

        _process.stderr.on('data', function (data) {
            _stderr.push(data);
        });


        return new Promise((resolve, reject) => {
            _process.on('exit', function (code) {
                if (code === 0) {
                    resolve({sucess: true, stdout: _stdout.join(''), stderr: _stderr.join('')});
                } else {
                    reject({stdout: _stdout.join(''), stderr: _stderr.join('')});
                }
            });
        });
    } catch (error) {
        throw new Error(`failed to execute ${command}`);
    }
}

export function execShort(command, timeout) {
    return new Promise((resolve, reject) => {
        childProcess.exec(command, {
            encoding: 'utf8',
            timeout: timeout || 1000 * 10
        }, (error, stdout, stderr) => {
            if (error) reject(error);
            else resolve({stderr, stdout});
        })
    });
}


export function execStdout(command, timeout) {
    return execShort(command, timeout).then(result => {
        if (result.stderr) {
            console.error(`Unexpected error when executing ${command} : ${result.stderr}`);
        }
        return result.stdout;
    });
}

export function execStderr(command, timeout) {
    return execShort(command, timeout).then(result => {
        if (result.stdout) {
            console.stdout(`Unexpected output when executing ${command} : ${result.stdout}`);
        }
        return result.stderr;
    });
}

export async function executeWithProgress(command, args, outFunc) {
    let bp = new BufferedProcess({
        command: command,
        args: args || [],
        stdout: (data) => {
            outFunc(data, 'stdout');
        },
        stderr: (data) => {
            outFunc(data, 'stderr');
        },
        exit: (code) => {
            if (code === 0) {
                outFunc(`${path.basename(command)} exited.`, 'stdout');
            }
            else outFunc(`${path.basename(command)} exited with error code ${code}.`, 'stderr');
        }
    });
    await bp.spawn();
    return bp.exitPromise;
}
async function isShebang(file) {
    return new Promise((resolve, reject) => {
        fs.open(file, 'r',  (err, fd) => {
            if (err) return reject(err);
            fs.read(fd, new Buffer(2), 0, 2, 0, (err, bytesRead, buf) => {
                if (err) return reject(err);
                resolve(bytesRead >= 2 && buf.readUInt8(0) === 0x23 /*'#'*/
                    && buf.readUInt8(1) === 0x21 /*'!'*/);
            });
        });
    });

}
export async function findExecutive(command) {
    const platform = os.platform();
    let executiveList = [];
    let isWin32 = false;
    if (platform === "win32") {
        let list = (await execStdout("where " + command)).trim().split('\n');
        executiveList = _.map(list, l => l.trim());
        isWin32 = true;
    }  else {
        let location = path.resolve(await execStdout(`which ${command}`)).trim();
        if (fs.isSymbolicLinkSync(location)) {
            executiveList.push(fs.realpathSync(location));
        } else if (fs.isFileSync(location)) {
            executiveList.push(location);
        }
    }
    for (let executiveFile of executiveList ) {
        let shebang = await isShebang(executiveFile);
        if (shebang && !isWin32) return executiveFile;
        if (!shebang && isWin32) return executiveFile;
    }
    return executiveList[0];
}
// findExecutive('apt').catch(console.log).then(data => console.log(data));
