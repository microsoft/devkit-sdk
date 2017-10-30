namespace DevKitTestTool
{
    using System;
    using System.Collections.Generic;
    using System.Configuration;
    using System.Diagnostics;
    using System.IO;
    using System.IO.Compression;
    using System.Linq;
    using System.Text;
    using System.Threading;

    class Program
    {
        private static readonly string port = ConfigurationManager.AppSettings["PortName"];
        private static readonly string workspace = ConfigurationManager.AppSettings["Workspace"];
        private static readonly string packageName = ConfigurationManager.AppSettings["PackageName"];

        private static SerialPortListener _listener;
        private static string versionFile;
        private static string resultFolderPath;

        private static Dictionary<string, string> examplesTestResult = new Dictionary<string, string>();
        private static Dictionary<string, string> unitTestResult = new Dictionary<string, string>();
        private static List<string> excludeTests = new List<string>();

        public static int Main(string[] args)
        {
            Stopwatch watch = new Stopwatch();

            try
            {
                if (File.Exists(Constants.ExcludedTestsFileName))
                {
                    excludeTests = File.ReadAllLines(Constants.ExcludedTestsFileName).ToList();
                }

                resultFolderPath = Path.Combine(workspace, ConfigurationManager.AppSettings["ResultFolder"]);
                if (!Directory.Exists(resultFolderPath))
                {
                    Directory.CreateDirectory(resultFolderPath);
                }

                versionFile = Path.Combine(workspace, Constants.SystemVersionFileName);

                switch (args[0])
                {
                    case "GenerateVersion":
                        Console.WriteLine("Generate DevKit version.");
                        GenerateSystemVersion();
                        break;

                    case "RunUnitTests":
                        // Run test cases and generate report
                        Console.WriteLine("Run Check-in Test for IoT DevKit.");
                        watch.Start();

                        string logFilePath = Path.Combine(resultFolderPath, "DevKit_Unit_Test_Result.txt");
                        _listener = new SerialPortListener(port, logFilePath);
                        _listener.start();

                        string testCasePath = Path.Combine(workspace, ConfigurationManager.AppSettings["TestCaseRelativePath"]);
                        RunUnitTests(testCasePath);
                        Thread.Sleep(30 * 1000);
                        _listener.Stop();
                        watch.Stop();
                        Console.WriteLine($"DevKit unit test execution time: {watch.Elapsed.Minutes} minutes");

                        Console.WriteLine("Generate test report");
                        GenerateReport("UnitTestReport", watch.Elapsed.Minutes, logFilePath);

                        if (unitTestResult.Where(kv => !string.Equals(kv.Value, "succeed", StringComparison.OrdinalIgnoreCase)).Count() > 0)
                        {
                            return 1;
                        }

                        break;

                    case "VerifyExamples":
                        Console.WriteLine("Verify the Examples for IoT DevKit.");
                        watch.Start();

                        string exampleFolderPath = Path.Combine(workspace, ConfigurationManager.AppSettings["ExampleFolderPath"]);
                        VerifyLibraryExamples(exampleFolderPath);

                        watch.Stop();
                        Console.WriteLine($"DevKit examples verification time: {watch.Elapsed.Minutes} minutes.");

                        Console.WriteLine("Generate test report");
                        GenerateReport("ExampleReport", watch.Elapsed.Minutes);

                        if (examplesTestResult.Where(kv => !string.Equals(kv.Value, "succeed", StringComparison.OrdinalIgnoreCase)).Count() > 0)
                        {
                            return 1;
                        }

                        break;

                    case "GenerateArduinoPackage":
                        GenerateArduinoPackage();
                        break;

                    case "TestInstallationPackage":
                        InstallationPackageTest installationPackageTest = new InstallationPackageTest(workspace);

                        string installPackageBlobUrl = string.Format(ConfigurationManager.AppSettings.Get("InstallPackageBlobUrl"), GetVersion());
                        int retCode = installationPackageTest.RunTest(installPackageBlobUrl);

                        if (retCode == 1)
                            return 1;

                        break;

                    case "GenerateSensorStatusBinFile":
                        string sensorStatusPath = Path.Combine(workspace, ConfigurationManager.AppSettings["SensorStatusPath"]);
                        VerifyLibraryExamples(sensorStatusPath);

                        break;

                    case "UpdateFirmwareVersion":
                        UpdateFirmwareVersion();
                        break;

                    case "UpdateBoardManagerUrl":
                        UpdateBoardManagerUrl();
                        break;

                    default:
                        Console.WriteLine("Done");
                        break;
                }

                return 0;
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.ToString());
                return 1;
            }
        }

        private static void GenerateSystemVersion()
        {
            string majorVersion = string.Empty;
            string minorVersion = string.Empty;
            string patchVersion = string.Empty;
            string[] list = null;

            string versionFilePath = Path.Combine(workspace, ConfigurationManager.AppSettings["SystemVersionFilePath"]);
            if (!File.Exists(versionFilePath))
            {
                throw new FileNotFoundException($"System version file is not found, version file path: {versionFilePath}");
            }

            using (StreamReader sr = new StreamReader(versionFilePath))
            {
                string str;
                while ((str = sr.ReadLine()) != null)
                {
                    if (str.Contains("#define DEVKIT_MAJOR_VERSION"))
                    {
                        majorVersion = str;
                        list = majorVersion.Split(' ');
                        majorVersion = list[list.Length - 1];
                    }
                    else if (str.Contains("#define DEVKIT_MINOR_VERSION"))
                    {
                        minorVersion = str;
                        list = minorVersion.Split(' ');
                        minorVersion = list[list.Length - 1];
                    }
                    else if (str.Contains("#define DEVKIT_PATCH_VERSION"))
                    {
                        patchVersion = str;
                        list = patchVersion.Split(' ');
                        patchVersion = list[list.Length - 1];
                    }

                    if (!string.IsNullOrEmpty(majorVersion) &&
                        !string.IsNullOrEmpty(minorVersion) &&
                        !string.IsNullOrEmpty(patchVersion))
                    {
                        break;
                    }
                }
            }

            string versionInfo = string.Format("{0}.{1}.{2}", majorVersion, minorVersion, patchVersion);
            Console.WriteLine($"Current version: {versionInfo}");

            // Write current version to file systemVersion.txt
            File.WriteAllText(versionFile, versionInfo);
        }

        private static void RunUnitTests(string testCasePath)
        {
            DirectoryInfo dir = new DirectoryInfo(testCasePath);
            FileInfo[] files = dir.GetFiles("*.ino", SearchOption.AllDirectories);

            string error;
            string argument;
            foreach (FileInfo file in files)
            {
                if (excludeTests.Contains(file.Name))
                {
                    continue;
                }

                Console.WriteLine(Constants.ReportLineSeperator);
                Console.WriteLine(string.Format("Start checking {0}...", file.Name));

                error = string.Empty;
                argument = string.Format(Constants.ArduinoArgTemplate, "upload", file.FullName, Path.Combine(workspace, "Build"));
                RunProcess(Constants.ArduinoExeFilePath, argument, out error);

                if (!string.IsNullOrEmpty(error))
                {
                    unitTestResult.Add(file.FullName, error);
                    Console.WriteLine(error);
                }
                else
                {
                    unitTestResult.Add(file.Name, "succeed");
                }

                Console.WriteLine("END\r\n");
            }
        }

        private static void VerifyLibraryExamples(string exampleFolderPath)
        {
            Console.WriteLine("Start to verify library examples.\r\n");

            if (!Directory.Exists(exampleFolderPath))
            {
                throw new DirectoryNotFoundException($"Example folder is not found: {exampleFolderPath}");
            }

            DirectoryInfo dir = new DirectoryInfo(exampleFolderPath);
            FileInfo[] files = dir.GetFiles("*.ino", SearchOption.AllDirectories);

            string error;
            string argument;
            foreach (FileInfo file in files)
            {
                if (excludeTests.Contains(file.Name))
                {
                    continue;
                }

                Console.WriteLine(Constants.ReportLineSeperator);
                Console.WriteLine(string.Format("Start verifying {0}...", file.Name));

                error = string.Empty;
                argument = string.Format(Constants.ArduinoArgTemplate, "verify", file.FullName, Path.Combine(workspace, "Build"));
                RunProcess(Constants.ArduinoExeFilePath, argument, out error);

                if (string.IsNullOrEmpty(error))
                {
                    examplesTestResult.Add(file.Name, "succeed");
                    Console.WriteLine($"Result: succeeded.\r\n");
                }
                else
                {
                    examplesTestResult.Add(file.Name, error);
                    Console.WriteLine(error);
                    Console.WriteLine("Result: failed.\r\n");
                }
            }
        }

        private static void GenerateReport(string type, int executionTimeInMinutes, string logFilePath = "")
        {
            string content = string.Empty;
            string reportFilePath = Path.Combine(resultFolderPath, "DevKit_Unit_Test_Result.htm");

            switch (type)
            {
                case "UnitTestReport":
                    content = GenerateReportContentForUnitTest(logFilePath, executionTimeInMinutes);
                    break;
                case "ExampleReport":
                    content = GenerateReportContentForExampleTest(executionTimeInMinutes);
                    break;
            }

            if (File.Exists(reportFilePath)) // add the related result to report
            {
                List<string> list = new List<string>();
                using (StreamReader sr = new StreamReader(reportFilePath))
                {
                    string line = sr.ReadLine();
                    while (!line.Equals("</body>"))
                    {
                        list.Add(line);

                        line = sr.ReadLine();
                    }
                }

                StreamWriter sw = new StreamWriter(reportFilePath);

                foreach (string str in list)
                {
                    sw.WriteLine(str);
                }
                sw.WriteLine(content);
                sw.WriteLine("</body>");
                sw.Flush();
                sw.Close();
            }
            else
            {
                StreamWriter sw = new StreamWriter(reportFilePath);

                // if no report, generate it
                sw.WriteLine("<html><head><title>DevKit Test Case Report</title>");
                sw.WriteLine("<style>");
                sw.WriteLine("h4, p { font-family: verdana; }");
                sw.WriteLine("p { line-height: 25px; font-size: small; padding-left: 30px;}");
                sw.WriteLine("</style>");
                sw.WriteLine("</head><body>");
                sw.WriteLine(content);
                sw.WriteLine("</body>");
                sw.Flush();
                sw.Close();
            }
        }

        private static string GenerateReportContentForUnitTest(string logFilePath, int executionTimeInMinutes)
        {
            if (string.IsNullOrEmpty(logFilePath) || !File.Exists(logFilePath))
            {
                throw new ArgumentException($"Invalid log file path: {logFilePath}");
            }

            StreamReader sr = new StreamReader(logFilePath, Encoding.Default);
            string line;
            string content = string.Empty;
            string retStr = string.Empty;

            int totalUnitTestCount = 0;
            int passUnitTestCount = 0;

            string fileName = string.Empty;
            string result = string.Empty;
            string info = string.Empty;
            string summary = string.Empty;


            while ((line = sr.ReadLine()) != null)
            {
                fileName = string.Empty;
                result = string.Empty;
                info = string.Empty;
                summary = string.Empty;

                if (line.EndsWith(".ino"))
                {
                    content += "<p>" + Constants.ReportLineSeperator;
                    fileName = Path.GetFileName(line);
                    content += "<br>start testing: " + "<strong>" + fileName + "</strong>";

                    while ((line = sr.ReadLine()) != null)
                    {
                        if (line.StartsWith("Test"))
                        {
                            content += "<br>" + line;

                            totalUnitTestCount++;
                            if (line.Contains("passed"))
                            {
                                passUnitTestCount++;
                            }
                            else if (line.Contains("failed"))
                            {
                                if (unitTestResult.ContainsKey(fileName))
                                {
                                    unitTestResult[fileName] = "failed";
                                }
                                else
                                {
                                    unitTestResult.Add(fileName, "failed");
                                }
                            }
                        }

                        if (line.StartsWith("Test summary: "))
                        {
                            content += "</p>";
                            totalUnitTestCount--;
                            passUnitTestCount--;
                            break;
                        }
                    }
                }
            }

            foreach (KeyValuePair<string, string> kvp in unitTestResult)
            {
                if (!kvp.Value.Equals("succeed"))
                {
                    totalUnitTestCount++;

                    content += "<p>" + Constants.ReportLineSeperator;
                    content += "<br>start testing: " + "<strong>" + kvp.Key + "</strong>";
                    content += "<br>" + kvp.Value + "</p>";
                }
            }

            retStr += Constants.ReportLineSeperator;
            retStr += "<h4>Unit Test Result</h4>";
            retStr += "<p><br>Total cases: " + totalUnitTestCount;
            retStr += "<br>Pass cases: " + passUnitTestCount;
            retStr += "<br>Pass Rate: " + ((double)passUnitTestCount / totalUnitTestCount).ToString("p");
            retStr += "<br>Total execution time: " + String.Format("{0:0.##}", executionTimeInMinutes) + " minutes.</p>";
            retStr += "<p/>";
            retStr += content;

            return retStr;
        }

        private static string GenerateReportContentForExampleTest(int executionTimeInMinutes)
        {
            string content = string.Empty;
            string retStr = string.Empty;

            int totalUnitTestCount = examplesTestResult.Count;
            int passUnitTestCount = 0;

            if (examplesTestResult.Count > 0)
            {
                foreach (KeyValuePair<string, string> kvp in examplesTestResult)
                {
                    content += "<p>" + Constants.ReportLineSeperator;
                    content += "<br>start testing: " + "<strong>" + kvp.Key + "</strong>";
                    content += "<br>" + kvp.Value + "</p>";

                    if (kvp.Value.Equals("succeed"))
                    {
                        passUnitTestCount++;
                    }
                }
            }

            retStr += Constants.ReportLineSeperator;
            retStr += "<h4>Examples Test Result</h4>";
            retStr += "<p><br>Total cases: " + totalUnitTestCount;
            retStr += "<br>Pass cases: " + passUnitTestCount;
            retStr += "<br>Pass Rate: " + ((double)passUnitTestCount / totalUnitTestCount).ToString("p");
            retStr += "<br>Total execution time: " + String.Format("{0:0.##}", executionTimeInMinutes) + " minutes.</p>";
            retStr += "<p/>";
            retStr += content;

            return retStr;
        }

        private static void GenerateArduinoPackage()
        {
            Console.WriteLine("Start to generate the Arduino package for DevKit.");

            string versionInfo = GetVersion();
            string sourceFolder = Path.Combine(workspace, ConfigurationManager.AppSettings.Get("DevKitSourceRelativePath"));
            string targetFolder = Path.Combine(resultFolderPath, "AZ3166");

            DirectoryInfo sourceDir = new DirectoryInfo(sourceFolder);
            DirectoryInfo targetDir = new DirectoryInfo(targetFolder);
            CopyAll(sourceDir, targetDir);

            string packageFile = Path.Combine(resultFolderPath, string.Format(packageName, versionInfo, Environment.GetEnvironmentVariable("BUILD_NUMBER")));
            if (File.Exists(packageFile))
            {
                File.Delete(packageFile);
            }

            ZipFile.CreateFromDirectory(targetFolder, packageFile, CompressionLevel.Fastest, true);

            Console.WriteLine($"Package is generated at: {packageFile}");
            Directory.Delete(targetFolder, true);
        }

        private static void CopyAll(DirectoryInfo source, DirectoryInfo target)
        {
            Directory.CreateDirectory(target.FullName);

            // Copy each file into the new directory.
            foreach (FileInfo fi in source.GetFiles())
            {
                fi.CopyTo(Path.Combine(target.FullName, fi.Name), true);
            }

            // Copy each subdirectory using recursion.
            foreach (DirectoryInfo diSourceSubDir in source.GetDirectories())
            {
                DirectoryInfo nextTargetSubDir = target.CreateSubdirectory(diSourceSubDir.Name);
                CopyAll(diSourceSubDir, nextTargetSubDir);
            }
        }

        private static string GetVersion()
        {
            return File.ReadAllText(versionFile).Trim();
        }

        private static bool RunProcess(string exeFileName, string exeArgument, out string errorString)
        {
            bool bPass = true;

            errorString = "";
            Process proc = new Process();

            try
            {
                ProcessStartInfo psi = new ProcessStartInfo();
                //psi.Verb = "runas";
                psi.FileName = exeFileName;
                psi.Arguments = exeArgument;
                psi.UseShellExecute = false;
                psi.RedirectStandardError = true;
                psi.RedirectStandardOutput = true;

                proc = Process.Start(psi);
                int timeout = Constants.Timeout;

                while (timeout >= 0 && !proc.HasExited)
                {
                    proc.WaitForExit(1000); // wait for 1 seconds

                    timeout--;
                }

                // Sleep for 1 second to ensure the action is completed
                Thread.Sleep(1000);
                if (proc.ExitCode != 0)
                {
                    bPass = false;

                    errorString += "Process exited with code [ " + proc.ExitCode + " ].\n\nError Message:\n";   // TODO: include the standard error/output? 
                    errorString += proc.StandardError.ReadToEnd();
                }
            }
            catch (Exception ex)
            {
                bPass = false;
                errorString += string.Format("Failed to execute the command as admin: [ {0} {1} ]{2}", exeFileName, exeArgument, Environment.NewLine);
                errorString += ex.StackTrace;
            }
            finally
            {
                if (!proc.HasExited)
                {
                    proc.Kill();
                }
            }

            return bPass;
        }

        private static void UpdateFirmwareVersion()
        {
            string filePath = Path.Combine(workspace, ConfigurationManager.AppSettings["TelemetryFilePath"]);
            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException($"Failed to find the telemetry.js, file path: {filePath}");
            }

            string versionInfo = GetVersion();
            string content = File.ReadAllText(filePath);

            Console.WriteLine($"Change firmware version to {versionInfo}");
            content = content.Replace(Constants.FirmwareVersionString, versionInfo);

            File.WriteAllText(filePath, content);
        }

        private static void UpdateBoardManagerUrl()
        {
            string filePath = Path.Combine(workspace, ConfigurationManager.AppSettings["TaskInstallationScriptFilePath"]);
            if (!File.Exists(filePath))
            {
                throw new FileNotFoundException($"Failed to find the task-installation.js, file path: {filePath}");
            }

            string newUrl = ConfigurationManager.AppSettings["BoardManagerURL"].ToString().Trim();
            string content = File.ReadAllText(filePath);

            content = content.Replace("BOARD_URL_PLACEHOLDER", newUrl);
            Console.WriteLine($"Set board manager URL to {newUrl}");

            File.WriteAllText(filePath, content);
        }
    }
}