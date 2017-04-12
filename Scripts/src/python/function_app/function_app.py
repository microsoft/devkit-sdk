import subprocess
import json
import os
import requests

_TEMPLATE_LINK = 'https://raw.githubusercontent.com/Azure/azure-quickstart-templates/master/101-function-app-create-dynamic/azuredeploy.json'
_CREATE_FUNCTION_APP_PARAMETER_ = '{{"appName":{{"value":"{}"}},"storageAccountType":{{"value":"Standard_LRS"}}}}'
_CREATE_FUNCTION_APP_COMMAND = 'az group deployment create -g {} --template-uri "{}" --parameters "{}"'

_APP_SETTING_COMMAND = 'az appservice web config appsettings update --resource-group {} --name {} --settings {}={}'

_PUBLISH_PROFILE_REST_URL = 'https://management.azure.com/subscriptions/{}/resourceGroups/{}/providers/Microsoft.Web/sites/{}/publishxml?api-version=2016-08-01'
_SYNC_TRIGGER_REST_URL = 'https://management.azure.com/subscriptions/{}/resourceGroups/{}/providers/Microsoft.Web/sites/{}/functions/synctriggers?api-version=2015-08-01'
_BASE_URL = 'https://{}.scm.azurewebsites.net'
_SITE_ROOT_DIR = 'site/wwwroot'


class FunctionApp:

    def __init__(self, subscription_id, app_name, resource_group):
        self._subscription_id = subscription_id
        self.app_name = app_name
        self.resource_group = resource_group

    def create_function_using_template(self):
        from re import sub
        parameter_string = _CREATE_FUNCTION_APP_PARAMETER_.format(self.app_name)
        escaped_parameter = sub(r'"', '\\"', parameter_string)
        command_string = _CREATE_FUNCTION_APP_COMMAND.format(self.resource_group, _TEMPLATE_LINK, escaped_parameter)
        print command_string
        command = subprocess.Popen(command_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        result = ""
        for line in command.stdout.readlines():
            result += line
        command.wait()
        if 'ERROR' not in result:
            ret_body_string = result[result.index('{'):result.rindex('}')+1]
            ret_body = json.loads(ret_body_string)
            if ret_body['properties']['provisioningState'] == 'Succeeded':
                return True
            else:
                raise Exception('create function app fail:{}'.format(ret_body.text))
        else:
            raise Exception('create function app fail:{}'.format(result))

    def setup_app_setting(self, key, value):
        command_string = _APP_SETTING_COMMAND.format(self.resource_group, self.app_name, key, value)
        command = subprocess.Popen(command_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        result = ""
        for line in command.stdout.readlines():
            result += line
        fail = command.wait()
        if not fail:
            ret_body_string = result[result.index('{'):result.rindex('}')+1]
            ret_body = json.loads(ret_body_string)
            if ret_body[key] == value:
                return True
            else:
                # TODO: exception handler
                raise Exception('app setting fail:{}'.format(result))

    @staticmethod
    def _get_config_dir():
        if os.getenv('AZURE_CONFIG_DIR'):
            return os.getenv('AZURE_CONFIG_DIR')
        else:
            return os.path.expanduser(os.path.join('~', '.azure'))

    def _get_access_token(self):
        from datetime import datetime
        azure_folder = self._get_config_dir()
        access_token = ""
        with open(os.path.join(azure_folder, 'accessTokens.json'), 'r') as f:
            tokens = json.load(f)
            for token in tokens:
                expire_time = datetime.strptime(token['expiresOn'], "%Y-%m-%d %H:%M:%S.%f")
                now = datetime.now()
                if expire_time > now and token['resource'] == 'https://management.core.windows.net/' and token['tokenType'] == 'Bearer':
                    access_token = token['accessToken']
                    break
            f.close()
        if access_token != "":
            self.access_token = access_token
        else:
            raise Exception('fetch access token fail')

    def _get_publishing_profile(self):
        self._get_access_token()
        url = _PUBLISH_PROFILE_REST_URL.format(self._subscription_id, self.resource_group, self.app_name)
        headers = {'Authorization': 'Bearer {}'.format(self.access_token)}
        response = requests.post(url=url, headers=headers)
        if response.status_code not in [200, 201]:
            raise Exception('get publishing profile fail:{}'.format(response.text))
        else:
            from xml.dom import minidom
            xmldoc = minidom.parseString(response.text)
            publish_profile = xmldoc.getElementsByTagName('publishProfile')
            for element in publish_profile:
                if element.attributes['publishMethod'].value == 'MSDeploy':
                    if element.attributes['userName'].value:
                        self.usr = element.attributes['userName'].value
                    else:
                        raise Exception('no user name in publishing profile')
                    if element.attributes['userPWD'].value:
                        self.pwd = element.attributes['userPWD'].value
                    else:
                        raise Exception('no pwd in publishing profile')
                elif element.attributes['publishMethod'].value == 'FTP':
                    if element.attributes['userPWD'].value:
                        self.ftp = element.attributes['publishUrl'].value
                    else:
                        raise Exception('no ftp url in publishing profile')

    def create_folder(self, path):
        if not hasattr(self, 'usr'):
            self._get_publishing_profile()
        response = requests.put("{}{}".format(_BASE_URL.format(self.app_name), path), auth=(self.usr, self.pwd))
        if response.status_code not in [200, 201]:
            raise Exception('create folder fail:{}'.format(response.text))
        else:
            return True

    def upload_file(self, func_name, path):
        if not hasattr(self, 'usr'):
            self._get_publishing_profile()
        import ftplib
        url = self.ftp[len('ftp://'):self.ftp.index('.net') + len('.net')]
        ftp = ftplib.FTP(url, self.usr, self.pwd)
        ftp.cwd('/{}/{}/'.format(_SITE_ROOT_DIR, func_name))
        files = [f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))]
        for f in files:
            ftp.storlines('STOR ' + f, open(os.path.join(path, f), 'r'))
        ftp.quit()

    def sync_trigger(self):
        self._get_access_token()
        url = _SYNC_TRIGGER_REST_URL.format(self._subscription_id, self.resource_group, self.app_name)
        headers = {'Authorization': 'Bearer {}'.format(self.access_token)}
        response = requests.post(url=url, headers=headers)
        if response.status_code not in [200, 201]:
            raise Exception('sync trigger fail:{}'.format(response.text))
        else:
            return True

    def run_command(self, path, command, func_name):
        if not hasattr(self, 'usr'):
            self._get_publishing_profile()
        payload = {"command": command, "dir": "{}/{}".format(_SITE_ROOT_DIR, func_name)}
        response = requests.post("{}{}".format(_BASE_URL.format(self.app_name), path), auth=(self.usr, self.pwd), json=payload)
        if response.status_code not in [200, 201]:
            raise Exception('run command fail:{}'.format(response.text))
        else:
            import json
            json_body = json.loads(response.content)
            print json_body['Output']
            print json_body['Error']