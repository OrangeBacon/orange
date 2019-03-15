using System;
using System.Collections.Generic;
using System.IO;

namespace MicrocodeCompiler {
    class FileStream {
        private readonly System.IO.FileStream file;

        public int Column { get; private set; }
        public int Line { get; private set; }

        public FileStream(string fileName) {
            try {
                file = File.Open(fileName, FileMode.Open, FileAccess.Read);
            } catch {
                Console.WriteLine("Could not find file");
            }
        }

        ~FileStream() {
            file.Close();
        }

        public char Next() {
            var c = (char)file.ReadByte();
            if(c == '\n') {
                Line++;
                Column = 0;
            } else {
                Column++;
            }
            return c;
        }

        public char Peek() {
            var c = (char)file.ReadByte();
            file.Seek(-1, SeekOrigin.Current);
            return c;
        }

        public bool Eof() {
            return file.Position >= file.Length;
        }
    }
}
