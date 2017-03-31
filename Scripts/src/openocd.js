import os from 'os'
import { Emitter } from 'event-kit'
import fs from 'fs-plus'
import path from 'path'
import glob from 'glob'
import util from 'util'
import childProcess from 'child_process'
import BufferedProcess from './bufferred-process'
const OPENOCD_MAGIC_HEADER = 'Open On-Chip Debugger ';
const OPENOCD_TRANSPORT_LIST = ['aice_jtag',
    'stlink_swim',
    'hla_jtag',
    'hla_swd',
    'jtag',
    'swd'];
const OPENOCD_INTERFACE_LIST = [
    'stlink-v1',
    'stlink-v2',
    'stlink-v2.1',
    'jlink',
    'cmsis-dap',
];
export default class Openocd {
    constructor(opt) {
        this.opt = Object.assign({}, opt);
        this.logLevel = Number.isInteger(opt.logLevel) ? opt.logLevel : 1;
        this.port = Number.isInteger(opt.port) ? opt.port : 3333;
        this.openocdPath = this.opt["openocdPath"];
        if (!this.openocdPath || !fs.isDirectorySync(this.openocdPath)) {
            throw new Error(`Cannot find openocd path, please specify openocdPath in options.`);
        }
        if (this.openocdPath && path.basename(this.openocdPath) === 'bin') {
            this.openocdPath = path.dirname(this.openocdPath);
        }
        this.isWin32 = os.platform() === 'win32';
        this.command = path.join(this.openocdPath, 'bin', this.isWin32 ? 'openocd.exe' : 'openocd');
        if (!fs.isFileSync(this.command)) {
            throw new Error(`Cannot find openocd at ${this.openocdPath}`);
        }
        // finds the scripts folder
        this.scriptsFolder = this.opt["scripts"];
        if (!this.scriptsFolder) {
            this.scriptsFolder = path.join(this.openocdPath, 'share', 'openocd', 'scripts');
        }
        if (!this.scriptsFolder || !fs.isDirectorySync(this.scriptsFolder)) {
            throw new Error(`Cannot find script directory please specify scripts in options.` + this.scriptsFolder || '');
        }

        this.outFunc = this.opt.output || (() => { });
    }

    async version() {
        let output = await util.execStdout(util.cstr(this.command) + ' --version');
        let ver = output.slice(OPENOCD_MAGIC_HEADER.length).trim();
        let match = /^(\d+\.\d+\.\d+)/g.exec(ver);
        if (match && match[1]) {
            return match[1];
        } else {
            throw new Error(`invalid version ${ver}`);
        }
    }
    async execute(debug_interface, transport, target, script) {
        if (!debug_interface) {
            throw new Error('Missing debug interface');
        }
        if (!target) {
            throw new Error('Missing debug target');
        }
        debug_interface = path.basename(debug_interface.replace('.cfg', ''));
        target = path.basename(target.replace('.cfg', ''));

        if (OPENOCD_INTERFACE_LIST.indexOf(debug_interface) < 0) {
            throw new Error(`Unsupported interface ${debug_interface}`);
        }
        if (transport && OPENOCD_TRANSPORT_LIST.indexOf(transport) < 0) {
            throw new Error(`Unsupported transport ${transport}`);
        }
        if (!fs.isFileSync(path.join(this.scriptsFolder, 'target', target + '.cfg'))) {
            throw new Error(`Cannot find file ${path.join(this.scriptsFolder, 'target', target + '.cfg')}`);
        }
        let config_file = [`interface/${debug_interface.replace(/\./g, '-')}.cfg`];

         if (transport) {
            config_file.push(':transport select ' + transport);
        }

        config_file.push(`target/${target}.cfg`);
        //config_file.push(``);
        config_file.push(`:${script}`);

        let scriptParam = [];
        let fillScript = item => {
            if (item.startsWith(':')) {
                scriptParam.push('-c');
                scriptParam.push(`${item.substring(1)}`);
            } else {
                scriptParam.push('-f');
                scriptParam.push(item);
            }
        };
        config_file.forEach(config => fillScript(config));
        if (this.port !== 3333) {
            fillScript(`:gdb_port ${this.port}`);
        }
        if (this.opt.debug) {
            scriptParam.push('-d');
            scriptParam.push('3');
        }
        
        let bp = new BufferedProcess({
            command: this.command,
            args: ['-s', this.scriptsFolder, ...scriptParam],
            stdout: (data) => {
                this.outFunc(data, 'stdout');
            },
            stderr: (data) => {
                // change channel to stdout because openocd always use stderr
                this.outFunc(data, 'stdout');
            },
            exit: (code) => {
                if (code === 0) {
                    this.outFunc('openocd exited.', 'stdout');
                }
                else this.outFunc(`openocd exited with error code ${code}.`, 'stderr');
            }
        });
        await bp.spawn();
        let exitCode = await bp.exitPromise;
        console.log('xxxx', exitCode);

    }
}

