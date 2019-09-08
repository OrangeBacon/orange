#define _str(x) #x
#define str(x) _str(x)
#ifdef DEBUG_OUTPUT
fprintf(logFile, "mem["str(address)"(%u)] = "str(data)"(%u)", address, data)
#endif
memory[address] = data;
#undef _str
#undef str