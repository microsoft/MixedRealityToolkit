using System.Diagnostics;

namespace AudioIO_DLL.Utils
{
    static class Debug
    {
        [Conditional("DEBUG")]
        public static void Assert(bool condition)
        {
            if (!condition)
            {
                System.Diagnostics.Debugger.Break();
            }
        }

        public static void WriteLine(string msg)
        {
            System.Diagnostics.Debug.WriteLine(msg);
        }
    }
}