namespace DevKitTestTool
{
    using System;
    using System.IO;
    using System.IO.Ports;

    class SerialPortListener
    {
        SerialPort m_serialPort = null;
        StreamWriter m_sw = null;

        public SerialPortListener(string portName, string logFile)
        {
            m_serialPort = new SerialPort(portName, 115200);
            m_serialPort.DataReceived += new SerialDataReceivedEventHandler(SerialPort_DataReceived);
            m_sw = File.AppendText(logFile);
        }

        public bool IsOpen
        {
            get
            {
                return m_serialPort == null ? false : m_serialPort.IsOpen;
            }
        }

        public bool start()
        {
            try
            {
                m_serialPort.Open();

                return true;
            }
            catch (Exception ex)
            {
                throw new ApplicationException("Error: Failed to start the serial port.", ex);
            }
        }

        public void Stop()
        {
            m_serialPort.Close();

            m_sw.Flush();
            m_sw.Close();
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            PrintResult();
        }

        void PrintResult()
        {
            string info = m_serialPort.ReadLine();
            
            m_sw.WriteLine(info);
        }

        public void writeToPort(string text)
        {
            m_serialPort.WriteLine(text);
        }
    }
}
