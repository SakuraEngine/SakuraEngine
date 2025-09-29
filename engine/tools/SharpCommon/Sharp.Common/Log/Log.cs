using Serilog;
using Serilog.Events;
using Serilog.Sinks.SystemConsole.Themes;

namespace Sakura
{
    public class Logging
    {
        public static void InitializeLogger(LogEventLevel LogLevel = LogEventLevel.Information)
        {
            Console.OutputEncoding = System.Text.Encoding.UTF8;
            SystemConsoleTheme ConsoleLogTheme = new SystemConsoleTheme(
               new Dictionary<ConsoleThemeStyle, SystemConsoleThemeStyle>
               {
                   [ConsoleThemeStyle.Text] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.SecondaryText] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.TertiaryText] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.DarkGray },
                   [ConsoleThemeStyle.Invalid] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Yellow },
                   [ConsoleThemeStyle.Null] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Blue },
                   [ConsoleThemeStyle.Name] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.String] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.Number] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Green },
                   [ConsoleThemeStyle.Boolean] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Blue },
                   [ConsoleThemeStyle.Scalar] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Green },

                   [ConsoleThemeStyle.LevelVerbose] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.LevelDebug] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Gray },
                   [ConsoleThemeStyle.LevelInformation] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White },
                   [ConsoleThemeStyle.LevelWarning] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.Yellow },
                   [ConsoleThemeStyle.LevelError] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White, Background = ConsoleColor.Red },
                   [ConsoleThemeStyle.LevelFatal] = new SystemConsoleThemeStyle { Foreground = ConsoleColor.White, Background = ConsoleColor.Red },
               });

            Log.Logger = new LoggerConfiguration()
                .MinimumLevel.Is(LogLevel)
                // .Enrich.WithThreadId()
                // .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Timestamp:yyyy-MM-dd HH:mm:ss.ffff zzz} {Message:lj}{NewLine}{Exception}")
                .WriteTo.Async(a => a.Logger(l => l
                    .Filter.ByIncludingOnly(e => e.Level == LogLevel)
                    .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Message:lj}{NewLine}{Exception}", theme: ConsoleLogTheme)
                ))
                .WriteTo.Async(a => a.Logger(l => l
                    .Filter.ByExcluding(e => e.Level == LogLevel)
                    .WriteTo.Console(restrictedToMinimumLevel: LogLevel, outputTemplate: "{Level:u}: {Message:lj}{NewLine}{Exception}", theme: ConsoleLogTheme)
                ))
                .CreateLogger();
        }
    }
}