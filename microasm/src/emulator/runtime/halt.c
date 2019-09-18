(void)arg123;
(void)arg12;
#ifdef DEBUG_OUTPUT
fprintf(logFile, "A: %u, B: %u\n", A, B);
fprintf(logFile, "A1: %u, A2: %u, A3: %u\n", arg1, arg2, arg3);
fprintf(logFile, "OP: %u, A12: %u, A123: %u\n", opcode, arg12, arg123);
#endif
exit(0);