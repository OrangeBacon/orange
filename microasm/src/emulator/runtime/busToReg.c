#define _str(x) #x
#define str(x) _str(x)
#ifdef DEBUG_OUTPUT
fprintf(logFile, str(REGISTER)"(%u) = "str(BUS)"(%u)\n", REGISTER, BUS);
#endif
REGISTER = BUS;
#undef _str
#undef str