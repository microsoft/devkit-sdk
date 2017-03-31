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
export function execStdout(command) {
    return exec(command).then(result => result.stdout);
}

export function execStderr(command) {
    return exec(command).then(result => result.stderr);
}