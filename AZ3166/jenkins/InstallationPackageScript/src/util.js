import childProcess from "child_process";

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
            console.log(`Unexpected output when executing ${command} : ${result.stdout}`);
        }
        return result.stderr;
    });
}
