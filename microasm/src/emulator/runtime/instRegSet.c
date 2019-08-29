value = inst;
opcode = (inst >> 9) & 0x7F; // first 7 bits
arg1 =   (inst >> 6) & 0x7;  // next 3 bits
arg2 =   (inst >> 3) & 0x7;  // next 3 bits
arg3 =   (inst >> 0) & 0x7;  // next 3 bits
arg12 = (arg1 << 0x7) + arg2;  // bits 8-13
arg123 = (arg1 << 0x7) + 
    (arg2 << 0x7) + arg3; // bits 8-16