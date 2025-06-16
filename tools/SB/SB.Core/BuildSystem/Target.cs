using SB.Core;
using System.Runtime.CompilerServices;

namespace SB
{
    public partial class Target : TargetSetters
    {
        internal Target(string Name, bool IsFromPackage, [CallerFilePath] string? Location = null, [CallerLineNumber] int LineNumber = 0)
            : base(Location!)
        {
            this.Name = Name;
            this.Location = Location!;
            this.LineNumber = LineNumber;
            this.IsFromPackage = IsFromPackage;

            BuildSystem.TargetDefaultSettings(this);
            if (BuildSystem.Configurations.TryGetValue(BuildSystem.GlobalConfiguration, out var GlobalConfig))
            {
                GlobalConfig(this);
            }
        }

        public T FileList<T>()
            where T : FileList, new()
        {
            foreach (var FileList in FileLists)
            {
                if (FileList is T)
                    return (FileList as T)!;
            }
            var FL = new T { Target = this };
            FileLists.Add(FL);
            return FL;
        }

        public bool HasFilesOf<T>() => FileLists.Exists(FL => FL is T && FL.Files.Count > 0);

        public Target Depend(Visibility Visibility, params string[] DependNames)
        {
            foreach (var DependName in DependNames)
            {
                switch (Visibility)
                {
                    case Visibility.Public:
                        {
                            if (DependName.Contains("@"))
                                PublicPackageTargetDependencies.Add(DependName);
                            else
                                PublicTargetDependencies.Add(DependName);
                        }
                        break;
                    case Visibility.Private:
                        {
                            if (DependName.Contains("@"))
                                PrivatePackageTargetDependencies.Add(DependName);
                            else
                                PrivateTargetDependencies.Add(DependName);
                        }
                        break;
                    case Visibility.Interface:
                        {
                            if (DependName.Contains("@"))
                                InterfacePackageTargetDependencies.Add(DependName);
                            else
                                InterfaceTargetDependencies.Add(DependName);
                        }
                        break;
                }
            }
            return this;
        }

        public Target Require(string Package, PackageConfig Config)
        {
            if (PackageDependencies.TryGetValue(Package, out var _))
                throw new ArgumentException($"Target {Name}: Required package {Package} is already required!");
            PackageDependencies.Add(Package, Config);
            return this;
        }

        public Target BeforeBuild(Action<Target> Action)
        {
            BeforeBuildActions.Add(Action);
            return this;
        }

        public Target AfterLoad(Action<Target> Action)
        {
            AfterLoadActions.Add(Action);
            return this;
        }

        public Target SetAttribute<T>(T Attribute, bool OverrideIfExisted = false)
        {
            if (OverrideIfExisted && Attributes.TryGetValue(typeof(T), out var _))
            {
                Attributes.Remove(typeof(T));
            }
            Attributes.Add(typeof(T), Attribute);
            return this;
        }

        public T? GetAttribute<T>()
        {
            if (Attributes.TryGetValue(typeof(T), out var Attribute))
                return (T?)Attribute;
            return default;
        }
        public object? GetAttribute(string TypeName)
        {
            foreach (var Attribute in Attributes)
            {
                if (Attribute.Key.Name == TypeName)
                    return Attribute.Value;
            }
            return default;
        }
        public bool HasAttribute<T>() => GetAttribute<T>() != null;

        private bool PackagesResolved = false;
        internal void ResolvePackages(ref Dictionary<string, Target> OutPackageTargets)
        {
            if (!PackagesResolved)
            {
                foreach (var KVP in PackageDependencies)
                {
                    var PackageName = KVP.Key;
                    var PackageConfig = KVP.Value;
                    var Package = BuildSystem.GetPackage(PackageName);
                    if (Package == null)
                        throw new ArgumentException($"Target {Name}: Required package {PackageName} does not exist!");

                    var ResolvePackageTargetDependencies =
                        (SortedSet<string> PackageTargetDependencies, ref SortedSet<string> ResolvedTargetDependencies, ref Dictionary<string, Target> OutPackageTargets) =>
                        {
                            foreach (var NickName in PackageTargetDependencies)
                            {
                                var Splitted = NickName.Split("@");
                                if (Splitted[0] == PackageName)
                                {
                                    var PackageTarget = Package.AcquireTarget(Splitted[1], PackageConfig);
                                    {
                                        PackageTarget.ResolvePackages(ref OutPackageTargets);
                                        OutPackageTargets.TryAdd(PackageTarget.Name, PackageTarget);
                                    }
                                    ResolvedTargetDependencies.Add(PackageTarget.Name);
                                }
                            }
                        };

                    ResolvePackageTargetDependencies(PrivatePackageTargetDependencies, ref PrivateTargetDependencies, ref OutPackageTargets);
                    ResolvePackageTargetDependencies(PublicPackageTargetDependencies, ref PublicTargetDependencies, ref OutPackageTargets);
                    ResolvePackageTargetDependencies(InterfacePackageTargetDependencies, ref InterfaceTargetDependencies, ref OutPackageTargets);
                }
                PackagesResolved = true;
            }
        }

        internal void ResolveDependencies()
        {
            RecursiveMergeDependencies(FinalTargetDependencies, PublicTargetDependencies);
            RecursiveMergeDependencies(FinalTargetDependencies, PrivateTargetDependencies);
        }

        internal void RecursiveMergeDependencies(ISet<string> To, IReadOnlySet<string> DepNames)
        {
            To.AddRange(DepNames);
            foreach (var DepName in DepNames)
            {
                Target? DepTarget = BuildSystem.GetTarget(DepName)!;
                if (DepTarget == null)
                    throw new TaskFatalError($"Target {Name}: Dependency {DepName} does not exist!");
                RecursiveMergeDependencies(To, DepTarget.PublicTargetDependencies);
                RecursiveMergeDependencies(To, DepTarget.InterfaceTargetDependencies);
            }
        }

        internal void ResolveArguments()
        {
            // Files
            foreach (var FileList in FileLists)
            {
                using (Profiler.BeginZone($"GlobFiles | {Name}", color: (uint)Profiler.ColorType.Pink))
                {
                    FileList.GlobFiles();
                }
            }
            // Arguments
            FinalArguments.Merge(PublicArguments);
            FinalArguments.Merge(PrivateArguments);
            foreach (var DepName in Dependencies)
            {
                Target DepTarget = BuildSystem.GetTarget(DepName)!;
                FinalArguments.Merge(DepTarget.PublicArguments);
                FinalArguments.Merge(DepTarget.InterfaceArguments);
            }
        }

        private Dictionary<Type, object?> Attributes = new();
        public IReadOnlySet<string> Dependencies => FinalTargetDependencies;
        public IReadOnlySet<string> PublicDependencies => PublicTargetDependencies;
        public IReadOnlySet<string> PrivateDependencies => PrivateTargetDependencies;
        public IReadOnlySet<string> InterfaceDependencies => InterfaceTargetDependencies;

        public string Name { get; }
        public string Location { get; }
        public int LineNumber { get; }

        #region Files
        internal List<FileList> FileLists = new();
        #endregion

        #region Dependencies
        private SortedDictionary<string, PackageConfig> PackageDependencies = new();

        private SortedSet<string> FinalTargetDependencies = new();
        private SortedSet<string> PublicTargetDependencies = new();
        private SortedSet<string> PrivateTargetDependencies = new();
        private SortedSet<string> InterfaceTargetDependencies = new();

        private SortedSet<string> PublicPackageTargetDependencies = new();
        private SortedSet<string> PrivatePackageTargetDependencies = new();
        private SortedSet<string> InterfacePackageTargetDependencies = new();
        #endregion

        #region Package
        public bool IsFromPackage { get; } = false;
        #endregion

        internal List<Action<Target>> BeforeBuildActions = new();
        internal List<Action<Target>> AfterLoadActions = new();
    }

    public static partial class TargetExtensions
    {
        public static ArgumentDictionary Copy(this ArgumentDictionary @this)
        {
            var Copy = new ArgumentDictionary();
            Copy.Merge(@this);
            return Copy;
        }

        public static void Merge(this ArgumentDictionary To, ArgumentDictionary? From)
        {
            if (From is null)
                return;
                
            foreach (var KVP in From)
            {
                var K = KVP.Key;
                var V = KVP.Value;
                if (V is IArgumentList)
                {
                    var ArgList = V as IArgumentList;
                    if (To.TryGetValue(K, out var Existed))
                    {
                        var COW = (Existed as IArgumentList)!.Copy();
                        COW.Merge(ArgList!);
                        To[K] = COW;
                    }
                    else
                        To.Add(K, ArgList!.Copy());
                }
                else
                {
                    if (!To.TryGetValue(K, out var Existed))
                    {
                        To.Add(K, V);
                    }
                    else
                        throw new TaskFatalError("Argument Confict!");
                }
            }
        }

        public static TargetType? GetTargetType(this Target Target)
        {
            if (Target.Arguments.TryGetValue("TargetType", out var V))
                return (TargetType?)V;
            return null;
        }
    }
}
