using System;

namespace MicrocodeCompiler {
    class Token {
        public readonly int Line;
        public readonly int Column;
        public readonly TokenType Type;
        public readonly string Data;
        public readonly int? Number;

        public Token(int line, int column, TokenType type) {
            Line = line;
            Column = column;
            Type = type;
        }

        public Token(int line, int column, TokenType type, string data) : this(line, column, type) {
            Data = data;
        }

        public Token(int line, int column, TokenType type, int number) : this(line, column, type) {
            Number = number;
        }

        public void Print() {
            Console.Write(Type);
            if(Data != null) {
                Console.Write($" = '{Data}'");
            }
            if(Number != null) {
                Console.Write($" = '{Number}'");
            }
            Console.Write('\n');
        }
    }

    enum TokenType {
        HEADER, OPCODE, MACROS, BITS, STAR, COLON,
        LEFT_BRACKET, RIGHT_BRACKET, IDENTIFIER,
        COMMA, SEMI_COLON, EQUALS, BLOCK,
        LEFT_PAREN, RIGHT_PAREN, NUMBER
    }
}
