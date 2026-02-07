using System;
using System.Diagnostics;

namespace PolyHack
{
    public static class DllInjector
    {
        public static bool Inject()
        {
            try
            {
                const string injectorPath = "Injector.exe";

                if (!System.IO.File.Exists(injectorPath))
                {
                    Console.WriteLine("Injector.exe not found!");
                    return false;
                }

                using (Process process = new Process())
                {
                    process.StartInfo.FileName = injectorPath;
                    process.StartInfo.UseShellExecute = false;
                    process.StartInfo.CreateNoWindow = true;
                    process.StartInfo.RedirectStandardOutput = true;
                    process.StartInfo.RedirectStandardError = true;

                    process.Start();
                    process.WaitForExit();
                    return process.ExitCode == 0;
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error executing Injector: {ex.Message}");
                return false;
            }
        }
    }
}