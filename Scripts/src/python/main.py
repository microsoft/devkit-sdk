from function_app.function_app import FunctionApp

import os
import argparse


def check_dir_existance(path):
    if not os.path.isdir(path):
        raise Exception('path: {} is not a directory'.format(path))
    if os.path.exists(path):
        return True
    else:
        raise Exception('directory path: {} not exist'.format(path))


def get_key_value(setting_list):
    res = {}
    for setting in setting_list:
        try:
            index = setting.index('=')
            key = setting[:index]
            value = setting[index+1:]
            res[key] = value
        except Exception, e:
            raise Exception('in get_key_value: {}'.format(e))
    return res


if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    requiredNamed = parser.add_argument_group('required named arguments')
    requiredNamed.add_argument("-S", "--subscription-id", action="store",
                               help="choose the active subscription id", required=True)

    requiredNamed.add_argument("-N", "--app-name", action="store",
                               help="Name your azure function app", required=True)

    requiredNamed.add_argument("-G", "--resource-group", action="store",
                               help="Resource group where you want to place the service", required=True)

    requiredNamed.add_argument("-D", "--directory", action="store",
                               help="Directory that contains files you want to upload to function app", required=True)

    requiredNamed.add_argument("--settings", action="store",
                               help="Space separated app settings in a format of <name>=<value>",
                               required=True, nargs='+')

    args = parser.parse_args()

    try:
        check_dir_existance(args.directory)
        setting_dist = get_key_value(args.settings)

        function_app = FunctionApp(args.subscription_id, args.app_name, args.resource_group)
        res = function_app.create_function_using_template()
        if res:
            print 'function app create success'

        for key in setting_dist.keys():
            res = function_app.setup_app_setting(key, setting_dist[key])
            if res:
                print 'setting {} variable successful'.format(key)

        res = function_app.create_folder('/api/vfs/site/wwwroot/MyFunc/')
        if res:
            print 'folder created'

        function_app.upload_file( 'MyFunc', args.directory)
        print 'file uploaded'

        function_app.run_command('/api/command', 'npm install', 'MyFunc')

    except Exception, e:
        print e
