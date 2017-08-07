namespace DevKitTestTool
{
    using System;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Compression;
    using System.Net;
    using System.Threading;
    using System.Windows.Automation;

    public class InstallationPackageTest
    {
        private string workspace;

        public InstallationPackageTest(string workspace)
        {
            if (string.IsNullOrEmpty(workspace))
            {
                throw new ArgumentNullException(nameof(workspace));
            }
            if (!Directory.Exists(workspace))
            {
                throw new ArgumentException($"Workspace directory is not found, path: {workspace}");
            }

            this.workspace = workspace;
        }

        public int RunTest(string installPackageBlobUrl)
        {         
            string packageZipFile = DownloadZipPackage(installPackageBlobUrl);

            string extractFolder = ExtractZipPackage(packageZipFile);
            return InstallZipPackage(extractFolder);
        }

        public string DownloadZipPackage(string packageUrl)
        {
            Console.WriteLine("Download the installation package.");

            WebClient client = new WebClient();

            string installationZipFile = Path.Combine(workspace, Path.GetFileName(packageUrl));
            if (File.Exists(installationZipFile))
            {
                File.Delete(installationZipFile);
            }

            client.DownloadFile(packageUrl, installationZipFile);
            return installationZipFile;
        }

        public string ExtractZipPackage(string zipFilePath)
        {
            if (!File.Exists(zipFilePath))
            {
                throw new FileNotFoundException($"Installation package is not found, path: {zipFilePath}");
            }

            string extractFolder = Path.Combine(workspace, Path.GetFileNameWithoutExtension(zipFilePath));
            if (Directory.Exists(extractFolder))
            {
                Directory.Delete(extractFolder,true);
            }

            ZipFile.ExtractToDirectory(zipFilePath, extractFolder);
            return extractFolder;
        }

        public int InstallZipPackage(string extractFolder)
        {
            Console.WriteLine("Start to run installation package.");
            int retCode = 0;

            Process proc = new Process();
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = "cmd.exe";
            psi.Arguments = "/C " + Path.Combine(extractFolder, "install.cmd");

            psi.Verb = "runas";
            psi.UseShellExecute = false;
            psi.RedirectStandardError = true;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardInput = true;
            psi.CreateNoWindow = true;

            proc = Process.Start(psi);

            int timeout = 0;

            using (StreamReader sr = proc.StandardOutput)
            {
                string line = "";

                while ((line = sr.ReadLine()) != null)
                {
                    line = RemoveInValidChars(line);
                    Console.WriteLine(line);

                    // if the log contains error, return 1
                    if (line.Contains("error") || line.Contains("exception") || line.Contains("fail"))
                        retCode = 1;

                    Thread.Sleep(1000);

                    MonitorDialog();

                    proc.StandardInput.WriteLine();

                    timeout++;

                    if (timeout == 60 * 5)
                    {
                        retCode = 1;
                        proc.Kill();
                        throw new Exception("Error: The process is still running after wait for 5 minutes");
                    }
                }
            }

            return retCode;
        }

        public void MonitorDialog()
        {
            AutomationElement root;
            AutomationElement ArduinoDialog = null;
            AutomationElement installBtn = null;

            AutomationElement STLinkDialog = null;
            AutomationElement nextBtn = null;
            AutomationElement finishBtn = null;

            root = AutomationElement.RootElement;
            if (root == null)
                throw new Exception("Failed to find the desktop");

            // Arduino install popup
            ArduinoDialog = FindElementByClassName(root, "#32770", "Windows Security");
            if (ArduinoDialog != null)
            {
                installBtn = FindElementByID(ArduinoDialog, "CommandButton_1");
                if (installBtn != null && installBtn.Current.IsEnabled)
                    btnClick(installBtn);
            }

            STLinkDialog = FindElementByClassName(root, "#32770", "Device Driver Installation Wizard");
            if (STLinkDialog != null)
            {
                nextBtn = FindElementByID(STLinkDialog, "12324");
                if (nextBtn != null && nextBtn.Current.IsEnabled)
                    btnClick(nextBtn);

                finishBtn = FindElementByID(STLinkDialog, "12325");
                if (finishBtn != null && finishBtn.Current.IsEnabled)
                    btnClick(finishBtn);
            }
        }

        public AutomationElement FindElementByClassName(AutomationElement ae, string classname, string name)
        {
            AutomationElement element = null;
            Condition conditions = new AndCondition(new PropertyCondition(AutomationElement.NameProperty, name),
                                                    new PropertyCondition(AutomationElement.ClassNameProperty, classname));

            element = ae.FindFirst(TreeScope.Children, conditions);

            return element;
        }

        public AutomationElement FindElementByID(AutomationElement ae, string id)
        {
            AutomationElement element = null;
            element = ae.FindFirst(TreeScope.Children, new PropertyCondition(AutomationElement.AutomationIdProperty, id));

            return element;
        }

        public void btnClick(AutomationElement button)
        {
            InvokePattern pattern = (InvokePattern)button.GetCurrentPattern(InvokePattern.Pattern);
            pattern.Invoke();
        }

        public string RemoveInValidChars(string line)
        {
            string str = line;

            str = line.Replace("\u001b[36m", "")
                .Replace("\u001b[0m", "")
                .Replace("\u001b[2K", "")
                .Replace("\u001b[1G|", "")
                .Replace("\u001b[1G/", "")
                .Replace("\u001b[1G-", "")
                .Replace("\u001b[1G\\", "")
                .Replace("\u001b[32m", "")
                .Replace("\u001b[92m", "");

            return str.Trim();
        }
    }
}
