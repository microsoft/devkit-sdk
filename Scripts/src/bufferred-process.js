import ChildProcess from 'child_process';
export default class BufferedProcess {
    constructor(arg) {
        this.command = arg.command;
        this.args = arg.args;
        let emptyFunc = () => {};
        this.stdout = arg.stdout || emptyFunc;
        this.stderr = arg.stderr|| emptyFunc;
        this.exit = arg.exit || emptyFunc;
        this.partialLine = {err: '', out: ''};
    }
    spawn() {
        return new Promise((resolve, reject) => {
            console.log([this.command, ...this.args].join(' '));
            this.process = ChildProcess.spawn(this.command, this.args);
            this.process.stderr.on('data', (data) => {
                return this._stdio(data, 'err');
            });
            this.process.stdout.on('data', (data) => {
                return this._stdio(data, 'out');
            });


            let error, ok = () => {
                this.process.removeListener('error', error);
                return resolve(this);
            };

            error = (err) => {
                this.process.removeListener('data', ok);
                return reject(err);
            };
            this.process.stderr.once('data', ok);
            this.process.stdout.once('data', ok);
            this.process.once('error', error);
            this.exitPromise = new Promise(exitResolve => {
                this.process.on('exit', (code) => {
                    exitResolve();
                    this.exit(code);
                });
            });

        });
    }

    stdin(line) {
        return this.process.stdin.write(line + '\n');
    }

    kill(signal) {
        this.process.kill(signal);
        return this.exitPromise;
    }

    _stdio(data, channel) {
        let i, len, line, lines;
        data = this.partialLine[channel] + data.toString();
        lines = data.replace(/(\r|\n)+/g, '\n').split('\n');
        this.partialLine[channel] = lines.slice(-1);
        lines = lines.slice(0, -1);
        for (i = 0, len = lines.length; i < len; i++) {
            line = lines[i];
            channel === 'err' ? this.stderr(line) : this.stdout(line);
        }
    };
}

// let bp = new BufferedProcess({command: 'node', args: [],
//     stdout: line => console.log('x', line),
//     exit: ()=> console.log('exit')
// });
// bp.spawn().then(() => {
//     console.log('exit');
// });
// bp.stdin('quit');