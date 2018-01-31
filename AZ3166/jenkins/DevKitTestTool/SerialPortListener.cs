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
               if(m_serialPort.IsOpen)
                {
                    Stop();
                }
                m_serialPort.Open();
                m_serialPort.DiscardInBuffer();
                m_serialPort.DiscardOutBuffer();

                return true;
            }
            catch (Exception ex)
            {
                throw new ApplicationException("Error: Failed to start the serial port.", ex);
            }
        }

        public void Stop()
        {
            try
            {                
                m_serialPort.Dispose();
                m_serialPort.Close();

                m_sw.Flush();
                m_sw.Close();
            }
            catch (Exception ex)
            {
                throw new Exception("Error: Failed to stop serial port", ex);
            }
        }

        private void SerialPort_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            try
            {
                SerialPort sp = (SerialPort)sender;
                string data = sp.ReadExisting();

                m_sw.Write(data);
            }
            catch (Exception ex)
            {
                throw new Exception("Error: Failed to get the data from serial port", ex);
            }
        }
    }
}
