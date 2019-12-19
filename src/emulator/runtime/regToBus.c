#define _str(x) #x
#define str(x) _str(x)
#ifdef DEBUG_OUTPUT
fprintf(logFile, str(BUS)"(%u) = "str(REGISTER)"(%u)\n", BUS, REGISTER);
#endif
BUS = REGISTER;
#undef _str
#undef str
