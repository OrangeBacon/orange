namespace VMLib {
    // Logger class that does nothing, used to prevent a null pointer when calling log functions
    class NullLogger : ILogger {
        public void Error(string message) {
        }

        public void Info(string message) {
        }

        public void Warn(string message) {
        }
    }
}
