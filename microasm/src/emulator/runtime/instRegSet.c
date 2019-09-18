#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
    (byte & 0x80 ? '1' : '0'), \
    (byte & 0x40 ? '1' : '0'), \
    (byte & 0x20 ? '1' : '0'), \
    (byte & 0x10 ? '1' : '0'), \
    (byte & 0x08 ? '1' : '0'), \
    (byte & 0x04 ? '1' : '0'), \
    (byte & 0x02 ? '1' : '0'), \
    (byte & 0x01 ? '1' : '0')
opcode = (inst >> 9) & 0x7F; // first 7 bits
arg1 = (inst >> 6) & 0x7;  // next 3 bits
arg2 = (inst >> 3) & 0x7;  // next 3 bits
arg3 = (inst >> 0) & 0x7;  // next 3 bits
arg12 = (arg1 << 0x7) + arg2;  // bits 8-13
arg123 = (arg1 << 0x7) + 
    (arg2 << 0x7) + arg3; // bits 8-16
#ifdef DEBUG_OUTPUT
fprintf(logFile, "ISet("BYTE_TO_BINARY_PATTERN" "BYTE_TO_BINARY_PATTERN") => %u\n", BYTE_TO_BINARY(inst>>8), BYTE_TO_BINARY(inst), opcode);
#endif
#undef BYTE_TO_BINARY_PATTERN
#undef BYTE_TO_BINARY
