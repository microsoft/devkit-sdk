using System;
using System.IO;

namespace CRC16Calculator
{
    using System;

    static class Program
    {
        static void PrintUsage()
        {
            Console.WriteLine("Usage: CRC16Calculator filepath.");
        }

        static int Main(string[] args)
        {
            string filePath = null;
            if (args.Length == 0) {
                PrintUsage();
                return 1;
            }
            filePath = args[0];
            byte[] fileBytes;
            if (!File.Exists(filePath))
            {
                Console.WriteLine("File Not Exist: " + filePath);
                PrintUsage();
                return 1;
            }
            fileBytes = File.ReadAllBytes(filePath);
            string hex = calcCrc(fileBytes).ToString("x2").PadLeft(4, '0').ToUpper();
            Console.WriteLine("File size: {0}, CRC16 Checksum: {1}", fileBytes.Length, hex);
            return 0;
        }

        private static short calcCrc(byte[] data)
        {
            unchecked
            {
                short crc = 0;

                for (int a = 0; a < data.Length; a++)
                {
                    crc ^= (short)(data[a] << 8);
                    for (int i = 0; i < 8; i++)
                    {
                        if ((crc & 0x8000) != 0)
                            crc = (short)((crc << 1) ^ 0x1021);
                        else
                            crc = (short)(crc << 1);
                    }
                }
                return crc;
            }
        }
    }
}
