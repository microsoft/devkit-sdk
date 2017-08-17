namespace DevKitTestTool
{
    using System;

    public static class Constants
    {
        internal static readonly string SystemVersionFileName = "system_version.txt";

        internal static readonly string ExcludedTestsFileName = "excludedTests.txt";

        internal static readonly string ReportLineSeperator = "-------------------------------------------";

        internal static readonly string ArduinoExeFilePath = @"C:\Program Files (x86)\Arduino\arduino_debug.exe";

        internal static readonly string ArduinoArgTemplate = "--board AZ3166:stm32f4:MXCHIP_AZ3166:upload_method=OpenOCDMethod --{0} \"{1}\" --pref build.path=\"{2}\""; //--pref build.path=\"{2}\"

    }
}
