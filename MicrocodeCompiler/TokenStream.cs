using System;
using System.Collections.Generic;
using System.Linq;

namespace MicrocodeCompiler {
    class TokenStream {
        private readonly FileStream file;

        private readonly Queue<Token> tokens = new Queue<Token>();

        public TokenStream(FileStream file) {
            this.file = file;
        }

        public Token Next() {
            if(tokens.Count > 0) {
                return tokens.Dequeue();
            } else {
                return Parse();
            }
        }

        public Token Peek(int n = 0) {
            while(tokens.Count <= n) {
                if(file.Eof()) {
                    return null;
                }
                tokens.Enqueue(Parse());
            }
            return tokens.ElementAt(n);
        }

        public bool Eof() {
            return file.Eof();
        }

        private Token Parse() {
            switch(file.Peek()) {
                case ' ':
                case '\n':
                case '\r':
                case '\t': Whitespace(); return Parse();
                case '#': Comment(); return Parse();
                case '{': return PopToken(TokenType.LEFT_BRACKET);
                case '}': return PopToken(TokenType.RIGHT_BRACKET);
                case '(': return PopToken(TokenType.LEFT_PAREN);
                case ')': return PopToken(TokenType.RIGHT_PAREN);
                case '*': return PopToken(TokenType.STAR);
                case ':': return PopToken(TokenType.COLON);
                case ',': return PopToken(TokenType.COMMA);
                case ';': return PopToken(TokenType.SEMI_COLON);
                case '=': return PopToken(TokenType.EQUALS);
                default: return Identifier();
            }
        }

        private Token PopToken(TokenType type) {
            file.Next();
            return new Token(file.Line, file.Column, type);
        }

        private void Comment() {
            while(!file.Eof()) {
                var c = file.Peek();
                if(c == '\n') {
                    return;
                }
                file.Next();
            }
        }

        private void Whitespace() {
            while(!file.Eof()) {
                var c = file.Peek();
                if(!(c == '\n' || c == ' ' || c == '\t' || c == '\r')) {
                    return;
                }
                file.Next();
            }
        }

        private Token Identifier() {
            var data = "";
            var type = TokenType.IDENTIFIER;
            while(!file.Eof()) {
                var c = file.Peek();
                if(";:,{}()*#=".IndexOf(c) != -1) {
                    break;
                }
                if(data == "header"
                    || data == "opcode" 
                    || data == "macro" 
                    || data == "bits") {
                    type = TokenType.BLOCK;
                    break;
                }
                data += file.Next();
            }
            if(int.TryParse(data.Trim(), out int result)) {
                return new Token(file.Line, file.Column, TokenType.NUMBER, result);
            }
            return new Token(file.Line, file.Column, type, data.Trim());
        }
    }
}
