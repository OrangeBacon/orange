#define _str(x) #x
#define str(x) _str(x)
#ifdef DEBUG_OUTPUT
fprintf(logFile, str(data)" = mem["str(address)"(%u)](%u)\n", address, memory[address]);
#endif
data = memory[address];
#undef _str
#undef str
