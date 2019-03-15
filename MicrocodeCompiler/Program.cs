using System;

namespace MicrocodeCompiler {
    class Program {
        static void Main(string[] args) {
            if(args.Length < 1) {
                Console.WriteLine("file name required");
                return;
            } else if(args.Length > 1) {
                Console.WriteLine("only one file name can be provided");
                return;
            }

            var s = new FileStream(args[0]);
            while(!s.Eof()) {
                Console.WriteLine($"{s.Line}:{s.Column} -> {s.Next()}");
            }
        }
    }
}
