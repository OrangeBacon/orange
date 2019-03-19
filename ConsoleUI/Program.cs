using System;
using VMLib;
using static VMLib.StarfishVM;
using System.Threading.Tasks;
using System.Threading;
using System.Linq;

namespace ConsoleUI {
    class Program {
        static void Main(string[] args) {
            var VM = CreateStarfishVM(new ConsoleLog());
            var run = true;
            Task.Run(delegate {
                Thread.Sleep(10000);
                run = false;
            });
            while(true) {
                VM.Clock.RunCycle();
                if(!run)
                    break;
            }
            Console.WriteLine(VM.Components.OfType<PhaseCounter>().ElementAt(0).Phase);
        }
    }

    class ConsoleLog : ILogger {
        public void Error(string message) {
            Console.Error.WriteLine(message);
        }

        public void Info(string message) {
            Console.Write(message);
        }

        public void Warn(string message) {
            Console.Write(message);
        }
    }
}
