using System;
using System.Collections.Generic;
using System.Linq;

namespace MicrocodeCompiler {
    class Parser {
        private readonly TokenStream tokens;

        public Parser(TokenStream tokenStream) {
            tokens = tokenStream;
        }

        public AST.Microcode Parse() {
            AST.Header header = null;
            List<AST.Opcode> opcodes = new List<AST.Opcode>();
            AST.Block x;
            while((x = Block()) != null) {
                if(x.GetType() == typeof(AST.Header)) {
                    header = (AST.Header)x;
                } else if(x.GetType() == typeof(AST.Opcode)) {
                    opcodes.Add((AST.Opcode)x);
                } else {
                    Console.WriteLine("Syntax Error");
                    Environment.Exit(-1);
                    return null;
                }
            }
            if(!tokens.Eof()) {
                Console.WriteLine("Unexpected end of file");
                Environment.Exit(-1);
                return null;
            }
            return new AST.Microcode(header, opcodes);
        }

        private AST.Block Block() {
            if(!tokens.Eof() && tokens.Peek().Type == TokenType.BLOCK) {
                switch(tokens.Next().Data) {
                    case "header":
                        return Header();
                    case "opcode":
                        return Opcode();
                    default:
                        return null;
                }
            } else {
                return null;
            }
        }

        private AST.Header Header() {
            Expect(TokenType.LEFT_BRACKET);
            var head = new AST.Header(Lines());
            Expect(TokenType.RIGHT_BRACKET);
            return head;
        }

        private AST.Opcode Opcode() {
            var name = Expect(TokenType.IDENTIFIER);
            Expect(TokenType.LEFT_PAREN);
            var args = Many(TokenType.IDENTIFIER, TokenType.COMMA);
            Expect(TokenType.RIGHT_PAREN);
            Expect(TokenType.EQUALS);
            var code = Expect(TokenType.NUMBER);
            Expect(TokenType.LEFT_BRACKET);
            var lines = Lines();
            Expect(TokenType.RIGHT_BRACKET);
            return new AST.Opcode(name.Data, args.Select(x=>x.Data).ToList(), code.Number ?? default(int), lines);
        }

        private List<AST.Line> Lines() {
            var lines = new List<AST.Line>();
            while(tokens.Peek().Type != TokenType.RIGHT_BRACKET || !tokens.Eof()) {
                lines.Add(Line());
            }
            return lines;
        }
        
        private AST.Line Line() {
            var c = Condition();
            var l = Many(TokenType.IDENTIFIER, TokenType.COMMA);
            var line = new AST.Line(c, l.Select(x=>x.Data).ToList());
            Expect(TokenType.SEMI_COLON);
            return line;
        }

        private AST.Condtion Condition() {
            switch(tokens.Peek().Type) {
                case TokenType.STAR: {
                    tokens.Next();
                    Expect(TokenType.COLON);
                    return new AST.Condtion();
                }
                case TokenType.IDENTIFIER: {
                    var token = tokens.Next();
                    Expect(TokenType.EQUALS);
                    var value = Expect(TokenType.NUMBER);
                    Expect(TokenType.COLON);
                    return new AST.Condtion(token.Data, value.Number ?? default(int));
                }
                default: Console.WriteLine("Syntax Error");
                    Environment.Exit(-1);
                    return null;
            }
        }

        private List<Token> Many(TokenType type, TokenType seperator) {
            var list = new List<Token>();
            var t = Maybe(type);
            if(t != null) {
                list.Add(t);
            } else {
                return list;
            }
            while(Maybe(seperator) != null) {
                list.Add(Expect(type));
            }
            return list;
        }

        private Token Expect(TokenType type) {
            if(tokens.Peek().Type == type) {
                return tokens.Next();
            }
            Console.WriteLine("Syntax Error");
            Environment.Exit(-1);
            return null;
        }

        private Token Maybe(TokenType type) {
            if(tokens.Peek().Type == type) {
                return tokens.Next();
            }
            return null;
        }
    }
}
