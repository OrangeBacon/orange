using System;

namespace vm
{
    public class Log
    {
        public static void Info(string msg)
        {
            Console.WriteLine($"[{DateTime.Now.ToString()}] INFO: {msg}");
        }
        public static void Warn(string msg)
        {
            Console.Write($"[{DateTime.Now.ToString()}] ");
            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine($"WARN: {msg}");
            Console.ForegroundColor = ConsoleColor.White;
        }
        public static void Error(string msg)
        {
            Console.Error.Write($"[{DateTime.Now.ToString()}] ");
            Console.ForegroundColor = ConsoleColor.Red;
            Console.Error.WriteLine($"ERROR: {msg}");
            Console.ForegroundColor = ConsoleColor.White;
        }
    }
}
