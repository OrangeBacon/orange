using System;
using System.Collections.Generic;

namespace MicrocodeCompiler.AST {
    class Microcode {
        public readonly Header header;
        public readonly List<Opcode> opcodes;

        public Microcode(Header header, List<Opcode> opcodes) {
            this.header = header;
            this.opcodes = opcodes;
        }

        public void Print() {
            Console.Write($"Microcode {{ ");
            if(header != null)header.Print();
            Console.Write(", ");
            foreach(var line in opcodes) {
                line.Print();
            }
            Console.Write(" }");
        }
    }

    class Block { }

    class Header : Block {
        public readonly List<Line> lines;

        public Header(List<Line> lines) {
            this.lines = lines;
        }

        public void Print() {
            Console.Write("Header { ");
            foreach(var line in lines) {
                line.Print();
            }
            Console.Write(" }");
        }
    }
    
    class Opcode : Block {
        public readonly string name;
        public readonly List<string> arguments;
        public readonly int code;
        public readonly List<Line> lines;

        public Opcode(string name, List<string> arguments, int code, List<Line> lines) {
            this.name = name;
            this.arguments = arguments;
            this.code = code;
            this.lines = lines;
        }

        public void Print() {
            Console.Write($"Opcode({name}) = {code} {{ ");
            foreach(var line in lines) {
                line.Print();
            }
            Console.Write(" }");
        }
    }
    class Line {
        public readonly Condtion run;
        public readonly List<string> bits;

        public Line(Condtion run, List<string> bits) {
            this.run = run;
            this.bits = bits;
        }

        public void Print() {
            Console.Write("Line { ");
            run.Print();
            foreach(var item in bits) {
                Console.Write($", {item}");
            }
            Console.Write(" } ");
        }
    }
    class Condtion {
        public readonly bool AlwaysRun;
        public readonly string condition;
        public readonly int conditionValue;

        public Condtion() {
            AlwaysRun = true;
        }

        public Condtion(string condition, int conditionValue) {
            AlwaysRun = false;
            this.condition = condition;
            this.conditionValue = conditionValue;
        }

        public void Print() {
            if(AlwaysRun) {
                Console.Write("Condition { Always }");
            } else {
                Console.Write($"Condition {{ {condition} = {conditionValue} }}");
            }
        }
    }
}
