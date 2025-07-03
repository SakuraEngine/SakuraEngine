using System.Collections.Concurrent;
using System.Reflection;
using System.Diagnostics;
using Serilog;

namespace SB
{
    public interface IDoctor
    {
        public abstract bool Check();
        public abstract bool Fix();
    }

    public abstract class DoctorAttribute : Attribute {}

    [AttributeUsage(AttributeTargets.Class, Inherited = true, AllowMultiple = true)]
    public class DoctorAttribute<T> : DoctorAttribute
        where T : IDoctor, new()
    {
        public DoctorAttribute()
        {
            BuildSystem.AddDoctor<T>();
        }
    }

    public partial class BuildSystem
    {
        public static void RunDoctors()
        {
            using (Profiler.BeginZone("RunDoctors", color: (uint)Profiler.ColorType.WebMaroon))
            {
                var DoctorAttributes = new ConcurrentBag<DoctorAttribute>();
                foreach (var Assembly in AppDomain.CurrentDomain.GetAssemblies())
                {
                    if (!Assembly.GetReferencedAssemblies().Any(A => A.Name == "SB.Core") && Assembly.GetName().Name != "SB.Core")
                        continue;

                    foreach (var Type in Assembly.GetTypes())
                    {
                        var DoctorAttrs = Type.GetCustomAttributes<DoctorAttribute>();
                        foreach (var DoctorAttr in DoctorAttrs)
                        {
                            DoctorAttributes.Add(DoctorAttr);
                        }
                    }
                }

                Log.Verbose("Starting doctors with {ProcessorCount} threads ...", Environment.ProcessorCount);
                Parallel.ForEach(_AllDoctors,
                new ParallelOptions { MaxDegreeOfParallelism = Environment.ProcessorCount },
                (Doctor) =>
                {
                    using (Profiler.BeginZone($"{Doctor.GetType().Name}", color: (uint)Profiler.ColorType.WebMaroon))
                    {
                        Stopwatch sw = new();
                        sw.Start();
                        Log.Verbose("Doctor {Name} starts ...", Doctor.GetType().Name);
                        if (!Doctor.Check())
                        {
                            if (!Doctor.Fix())
                            {
                                throw new Exception("Doctor failed to fix the issue");
                            }
                        }
                        sw.Stop();
                        float Seconds = sw.ElapsedMilliseconds / 1000.0f;
                        Log.Verbose("Doctor {Name} finished... cost {Seconds}s", Doctor.GetType().Name, Seconds);
                    }
                });
            }
        }

        public static void AddDoctor<T>()
            where T : IDoctor, new()
        {
            var Doctor = new T();
            _AllDoctors.Add(Doctor);
        }

        private static ConcurrentBag<IDoctor> _AllDoctors = new ConcurrentBag<IDoctor>();
    }
}