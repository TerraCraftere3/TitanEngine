namespace Titan
{
    public class Log
    {
        public static void Trace(string message) => InternalCalls.InternalClientLogTrace(message);
        public static void Info(string message) => InternalCalls.InternalClientLogInfo(message);
        public static void Warn(string message) => InternalCalls.InternalClientLogWarn(message);
        public static void Error(string message) => InternalCalls.InternalClientLogError(message);
        public static void Critical(string message) => InternalCalls.InternalClientLogCritical(message);
    }
}