
using System.Collections.Concurrent;
using System.Reflection;
using System.Text;
using SB.Core;
using SB.XCode;
using Serilog;

namespace SB
{
    namespace XCode
    {
        // The base node for all XCode nodes.
        public abstract class XCNode
        {
            public string IsA = "";

            public XCNode(string IsA)
            {
                this.IsA = IsA;
            }
        }

        // 24 digits of Hex, for example: “3A35328B1E99974500C194AD”
        public struct UUID
        {
            public string ID { get; set; }

            public UUID(string ID)
            {
                this.ID = ID;
            }
        }

        [AttributeUsage(AttributeTargets.All)]
        public class XCNameAttribute : Attribute
        {
            public string Name { get; set; }

            public XCNameAttribute(string Name)
            {
                this.Name = Name;
            }
        }

        public enum FileType
        {
            [XCName("text")]
            Text,
            [XCName("com.apple.product-type.library.static")]
            StaticLibrary,
            [XCName("com.apple.product-type.library.dynamic")]
            SharedLibaray,
            [XCName("com.apple.product-type.application")]
            Application,
            [XCName("com.apple.product-type.tool")]
            CommandLineTool,
            [XCName("wrapper.framework")]
            Framework,
            [XCName("sourcecode.c.h")]
            SourceH,
            [XCName("sourcecode.c.c")]
            SourceC,
            [XCName("sourcecode.c.objc")]
            SourceObjC,
            [XCName("sourcecode.cpp.h")]
            SourceHpp,
            [XCName("sourcecode.cpp.cpp")]
            SourceCpp,
            [XCName("sourcecode.cpp.objcpp")]
            SourceObjCpp,
            [XCName("sourcecode.metal")]
            SourceMetal,
            [XCName("text.plist")]
            PropertyList,
            [XCName("text.plist.xml")]
            PropertyListXML,
            [XCName("file.storyboard")]
            Storyboard,
            [XCName("wrapper.application")]
            ApplicationWrapper,
        }

        // Refers one file on disk.
        public class PBXFileReference : XCNode
        {
            public FileType? ExplicitFileType;
            public FileType? LastKnownFileType;
            public string Name = "";
            public string? Path;
            public string? SourceTree;
            public int? IncludeInIndex;

            public PBXFileReference() : 
                base("PBXFileReference") { }
        }

        public class PBXGroup : XCNode
        {
            public string? Name;
            public string? SourceTree;
            public List<UUID> Children = new();

            public PBXGroup() : 
                base("PBXGroup") { }
        }

        public class XCBuildConfiguration : XCNode
        {
            public string Name = "";
            public Dictionary<string, object> BuildSettings = new();

            public XCBuildConfiguration() : 
                base("XCBuildConfiguration") { }
        }

        public class XCConfigurationList : XCNode
        {
            public List<UUID> BuildConfigurations = new();

            public XCConfigurationList() : 
                base("XCConfigurationList") { }
        }

        public class PBXShellScriptBuildPhase : XCNode
        {
            public string Name = "Run custom shell script";
            public string ShellScript = "";

            public PBXShellScriptBuildPhase() : 
                base("PBXShellScriptBuildPhase") { }
        }

        public class PBXNativeTarget : XCNode
        {
            public string Name = "";
            public string ProductName = "";
            public UUID? ProductReference;
            public FileType? ProductType;
            public UUID HeaderFileGroup;
            public UUID SourceFileGroup;
            public UUID MainGroup;
            public UUID? BuildConfigurationList;
            public List<UUID> BuildPhases = new();
            
            public PBXNativeTarget() :
                base("PBXNativeTarget") { }
        }

        public class PBXProject : XCNode
        {
            public UUID BuildConfigurationList;
            public UUID MainGroup;
            public UUID ProductRefGroup;
            public List<UUID> Targets = new();
            
            public PBXProject() :
                base("PBXProject") { }
        }
    }
    
    public class XCodeSections
    {
        public Dictionary<UUID, XCode.PBXFileReference> PBXFileReference = new();
        public Dictionary<UUID, XCode.PBXGroup> PBXGroup = new();
        public Dictionary<UUID, XCode.PBXNativeTarget> PBXNativeTarget = new();
        public Dictionary<UUID, XCode.PBXProject> PBXProject = new();
        public Dictionary<UUID, XCode.PBXShellScriptBuildPhase> PBXShellScriptBuildPhase = new();
        public Dictionary<UUID, XCode.XCBuildConfiguration> XCBuildConfiguration = new();
        public Dictionary<UUID, XCode.XCConfigurationList> XCConfigurationList = new();
    }

    public class XCodeTargetInfo
    {
        // The name displayed on XCode project view.
        public string TargetName = "";
        // The product file name (including extension).
        public string ProductFileName = "";
        // The product type.
        public XCode.FileType ProductType = XCode.FileType.CommandLineTool;
        // Header file paths.
        public List<string> HeaderFiles = new();
        // Source file paths.
        public List<string> SourceFiles = new();
        // Whether this target is an application bundle.
        public bool IsApplicationBundle = false;
    }

    // Collects information used to generate xcode project.
    public class XCodeProjectInfo
    {
        // The absolute directory to output project files.
        public string ProjectDir = "";

        // The file name of the project bundle (.xcodeproj) file.
        public string ProjectBundle = "";

        public int ProjectDirUUID = 0;

        public int ProjectBundleUUID = 0;

        public int UUIDCounter = 0;

        public XCodeSections Sections = new();

        // The root project object.
        public UUID RootObject;
        
        public ConcurrentDictionary<Target, XCodeTargetInfo> Targets = new();

        public XCodeProjectInfo(string ProjectDir, string ProjectBundle)
        {
            this.ProjectDir = ProjectDir;
            this.ProjectBundle = ProjectBundle;
            ProjectDirUUID = this.ProjectDir.GetHashCode();
            ProjectBundleUUID = this.ProjectBundle.GetHashCode();
        }

        // Generates UUID for all nodes.
        // The UUID generated is consistent during multiple runs, so long as the 
        // project path and structure is not changed.
        public UUID GenUUID()
        {
            int counter = UUIDCounter;
            ++UUIDCounter;
            string id = ProjectDirUUID.ToString("X8") + ProjectBundleUUID.ToString("X8") + UUIDCounter.ToString("X8");
            return new UUID(id);
        }

        public UUID CreatePBXProject(out PBXProject pbxProject)
        {
            PBXProject obj = new();
            UUID ret = GenUUID();
            Sections.PBXProject[ret] = obj;
            pbxProject = obj;
            return ret;
        }

        public UUID CreateXCBuildConfiguration(XCodeTargetInfo? target, string mode, out XCBuildConfiguration buildConfiguration)
        {
            XCBuildConfiguration obj = new();
            UUID ret = GenUUID();
            Sections.XCBuildConfiguration[ret] = obj;
            obj.Name = mode;
            obj.BuildSettings["ENABLE_USER_SCRIPT_SANDBOXING"] = false;
            if (target != null)
            {
                // TODO: for application bundle, we need to set GENERATE_INFOPLIST_FILE and PRODUCT_NAME.
            }
            else
            {
                // This is for root project.
                obj.BuildSettings["SUPPORTED_PLATFORMS"] = "\"macosx iphoneos\"";
                obj.BuildSettings["SDKROOT"] = "\"macosx\"";
                obj.BuildSettings["\"SDKROOT[sdk=macosx*]\""] = "macosx";
                obj.BuildSettings["\"SDKROOT[sdk=iphoneos]\""] = "iphoneos";
            }
            buildConfiguration = obj;
            return ret;
        }

        public UUID CreateXCConfigurationList(XCodeTargetInfo? target, out XCConfigurationList xcConfigurationList)
        {
            XCConfigurationList obj = new();
            UUID ret = GenUUID();
            Sections.XCConfigurationList[ret] = obj;
            foreach (var config in BuildSystem.Configurations)
            {
                obj.BuildConfigurations.Add(CreateXCBuildConfiguration(target, config.Key, out XCBuildConfiguration xcConfiguration));
            }
            xcConfigurationList = obj;
            return ret;
        }

        public UUID CreatePBXGroup(string? name, string? sourceTree, out PBXGroup group)
        {
            PBXGroup obj = new();
            UUID ret = GenUUID();
            Sections.PBXGroup[ret] = obj;
            obj.Name = name;
            obj.SourceTree = sourceTree;
            group = obj;
            return ret;
        }

        public UUID CreatePBXFileReference(string name, out PBXFileReference fileReference)
        {
            PBXFileReference obj = new();
            UUID ret = GenUUID();
            Sections.PBXFileReference[ret] = obj;
            obj.Name = name;
            fileReference = obj;
            return ret;
        }

        public UUID CreatePBXShellScriptBuildPhase(XCodeTargetInfo target, out PBXShellScriptBuildPhase phase)
        {
            PBXShellScriptBuildPhase obj = new();
            UUID ret = GenUUID();
            Sections.PBXShellScriptBuildPhase[ret] = obj;
            StringBuilder script = new();
            script.AppendLine($"cd {BuildDirs.ProjectRoot}");
            script.AppendLine($"export DOTNET_PROGRAM_FILE=/usr/local/share/dotnet/dotnet");
            script.AppendLine("# Running SB build command.");
            script.AppendLine("${DOTNET_PROGRAM_FILE} run SB -- build --target " + target.TargetName + " -m ${CONFIGURATION} -p ${PLATFORM_NAME} -a ${NATIVE_ARCH} -o ${BUILD_DIR} || exit -1");
            script.AppendLine("# Copy files to build path.");
            script.AppendLine("SB_BUILD_DIR=${BUILD_DIR}/${PLATFORM_NAME}/${NATIVE_ARCH}/${CONFIGURATION}");
            script.AppendLine("if test -e ${CONFIGURATION_BUILD_DIR}; then");
            script.AppendLine("    rm -r ${CONFIGURATION_BUILD_DIR}/*");
            script.AppendLine("fi");
            script.AppendLine("if test -e ${SB_BUILD_DIR}; then");
            script.AppendLine("    cp -r ${XMAKE_BUILD_DIR}/* ${CONFIGURATION_BUILD_DIR}");
            script.AppendLine("fi");
            obj.ShellScript = script.ToString();
            phase = obj;
            return ret;
        }

        public void AddTargetFiles(PBXGroup group, List<string> files)
        {
            foreach (string file in files)
            {
                string filename = Path.GetFileName(file);
                group.Children.Add(CreatePBXFileReference(filename, out PBXFileReference fileReference));
                fileReference.Path = file;
                fileReference.SourceTree = "\"<absolute>\"";
                string ext = Path.GetExtension(file);
                switch (ext)
                {
                    case ".h":
                        fileReference.LastKnownFileType = FileType.SourceH;
                        break;
                    case ".c":
                        fileReference.LastKnownFileType = FileType.SourceC;
                        break;
                    case ".m":
                        fileReference.LastKnownFileType = FileType.SourceObjC;
                        break;
                    case ".hpp":
                        fileReference.LastKnownFileType = FileType.SourceHpp;
                        break;
                    case ".cpp":
                        fileReference.LastKnownFileType = FileType.SourceCpp;
                        break;
                    case ".mm":
                        fileReference.LastKnownFileType = FileType.SourceObjCpp;
                        break;
                    case ".plist":
                        fileReference.LastKnownFileType = FileType.PropertyList;
                        break;
                    default:
                        fileReference.LastKnownFileType = FileType.Text;
                        break;
                }
            }
        }

        public UUID CreatePBXNativeTarget(XCodeTargetInfo target, out PBXNativeTarget nativeTarget)
        {
            PBXNativeTarget obj = new();
            UUID ret = GenUUID();
            Sections.PBXNativeTarget[ret] = obj;
            obj.Name = target.TargetName;
            // Add product file info.
            obj.ProductName = target.TargetName;
            obj.ProductReference = CreatePBXFileReference(target.ProductFileName, out PBXFileReference productFileObj);
            productFileObj.ExplicitFileType = target.ProductType;
            productFileObj.Path = productFileObj.Name;
            productFileObj.SourceTree = "BUILT_PRODUCTS_DIR";
            productFileObj.IncludeInIndex = 0;
            if (target.IsApplicationBundle)
            {
                // In this case productFileObj.ExplicitFileType is ApplicationWrapper.
                obj.ProductType = XCode.FileType.Application;
            }
            else
            {
                obj.ProductType = productFileObj.ExplicitFileType;
            }
            // Add files.
            obj.HeaderFileGroup = CreatePBXGroup("Header Files", "<group>", out PBXGroup headerFileGroup);
            obj.SourceFileGroup = CreatePBXGroup("Source Files", "<group>", out PBXGroup sourceFileGroup);
            AddTargetFiles(headerFileGroup, target.HeaderFiles);
            AddTargetFiles(sourceFileGroup, target.SourceFiles);
            obj.MainGroup = CreatePBXGroup(obj.ProductName, "<group>", out PBXGroup mainGroup);
            mainGroup.Children.Add(obj.HeaderFileGroup);
            mainGroup.Children.Add(obj.SourceFileGroup);
            // Add config list.
            obj.BuildConfigurationList = CreateXCConfigurationList(target, out XCConfigurationList xcConfigurationList);
            // Add build phases.
            obj.BuildPhases.Add(CreatePBXShellScriptBuildPhase(target, out PBXShellScriptBuildPhase phase));
            nativeTarget = obj;
            return ret;
        }

        public void BuildXCNodes()
        {
            UUID projectUUID = CreatePBXProject(out PBXProject projectObj);
            RootObject = projectUUID;
            projectObj.BuildConfigurationList = CreateXCConfigurationList(null, out XCConfigurationList rootConfigList);
            projectObj.MainGroup = CreatePBXGroup(null, "<group>", out PBXGroup mainGroup);
            projectObj.ProductRefGroup = CreatePBXGroup("Products", "<group>", out PBXGroup productRefGroup);
            foreach (var target in Targets)
            {
                UUID targetUUID = CreatePBXNativeTarget(target.Value, out PBXNativeTarget nativeTarget);
                projectObj.Targets.Add(targetUUID);
                mainGroup.Children.Add(nativeTarget.MainGroup);
            }
            mainGroup.Children.Add(projectObj.ProductRefGroup);
        }
    }

    public class XCodeEmitter : TaskEmitter
    {
        public XCodeEmitter(IToolchain Toolchain, string ProjectDir, string ProjectBundle)
        {
            this.Toolchain = Toolchain;
            ProjectInfo = new(ProjectDir, ProjectBundle);
        }
        
        private IToolchain Toolchain { get; }

        public XCodeProjectInfo ProjectInfo;
        
        public override bool EnableEmitter(Target Target) =>
            Target.HasFilesOf<CppFileList>() ||
            Target.HasFilesOf<CFileList>() ||
            Target.HasFilesOf<ObjCppFileList>() ||
            Target.HasFilesOf<ObjCFileList>();

        public override bool EmitTargetTask(Target Target) => true;

        public override IArtifact? PerTargetTask(Target Target)
        {
            XCodeTargetInfo info = new();
            info.TargetName = Target.Name;
            info.ProductFileName = Target.Name;
            TargetType targetType = (TargetType)Target.GetTargetType()!;
            switch (targetType)
            {
                case TargetType.Dynamic:
                    info.ProductType = FileType.SharedLibaray;
                    break;
                case TargetType.Executable:
                    info.ProductType = FileType.CommandLineTool;
                    break;
                case TargetType.Static:
                    info.ProductType = FileType.StaticLibrary;
                    break;
                default:
                    break;
            }
            ProjectInfo.Targets.TryAdd(Target, info);
            return null;
        }

        public override bool EmitFileTask(Target Target, FileList FileList) => FileList.Is<CppFileList>() || FileList.Is<CFileList>() || FileList.Is<ObjCppFileList>() || FileList.Is<ObjCFileList>();
        public override IArtifact? PerFileTask(Target Target, FileList FileList, FileOptions? Options, string SourceFile)
        {
            XCodeTargetInfo? info;
            ProjectInfo.Targets.TryGetValue(Target, out info);
            lock (info!)
            {
                info!.SourceFiles.Add(SourceFile);
            }
            return null;
        }

        public static string GetXCodeFileTypeXCName(XCode.FileType FileType)
        {
            var field = FileType.GetType().GetField(FileType.ToString());
            var attr = field?.GetCustomAttribute<XCNameAttribute>();
            return attr?.Name ?? FileType.ToString();
        }

        private static void WriteSectionsPBXFileReference(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if (ProjectInfo.Sections.PBXFileReference.Count == 0) return;
            Lines.AppendLine("/* Begin PBXFileReference section */");
            foreach (var kvp in ProjectInfo.Sections.PBXFileReference)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                if (item.ExplicitFileType.HasValue)
                {
                    Lines.AppendLine($"\t\t\texplicitFileType = {GetXCodeFileTypeXCName(item.ExplicitFileType.Value)};");
                }
                if (item.LastKnownFileType.HasValue)
                {
                    Lines.AppendLine($"\t\t\tlastKnownFileType = {GetXCodeFileTypeXCName(item.LastKnownFileType.Value)};");
                }
                Lines.AppendLine($"\t\t\tname = \"{item.Name}\";");
                if (item.Path != null)
                {
                    Lines.AppendLine($"\t\t\tpath = \"{item.Path}\";");
                }
                if (item.SourceTree != null)
                {
                    Lines.AppendLine($"\t\t\tsourceTree = {item.SourceTree};");
                }
                if (item.IncludeInIndex.HasValue)
                {
                    Lines.AppendLine($"\t\t\tincludeInIndex = {item.IncludeInIndex.Value.ToString()};");
                }
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End PBXFileReference section */");
        }

        private static void WriteSectionsPBXGroup(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.PBXGroup.Count == 0) return;
            Lines.AppendLine("/* Begin PBXGroup section */");
            foreach (var kvp in ProjectInfo.Sections.PBXGroup)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                if (item.Name is not null)
                {
                    Lines.AppendLine($"\t\t\tname = \"{item.Name}\";");
                }
                Lines.AppendLine("\t\t\tchildren = (");
                foreach (var child in item.Children)
                {
                    XCNode? childObj = null;
                    if(ProjectInfo.Sections.PBXFileReference.ContainsKey(child))
                    {
                        childObj = ProjectInfo.Sections.PBXFileReference[child];
                    }
                    else if (ProjectInfo.Sections.PBXGroup.ContainsKey(child))
                    {
                        childObj = ProjectInfo.Sections.PBXGroup[child];
                    }
    
                    // Just used in comments in the generated project file.
                    string childName = "";
                    if (childObj != null)
                    {
                        PBXFileReference? fileRef = childObj as PBXFileReference;
                        if (fileRef != null)
                        {
                            childName = fileRef.Name;
                        }
                        PBXGroup? group = childObj as PBXGroup;
                        if (group != null && group.Name != null)
                        {
                            childName = group.Name;
                        }
                    }
                    Lines.AppendLine($"\t\t\t\t{child.ID} /* {childName} */,");
                }
                Lines.AppendLine("\t\t\t);");
                if (item.SourceTree != null)
                {
                    Lines.AppendLine($"\t\t\tsourceTree = \"{item.SourceTree}\";");
                }
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End PBXGroup section */");
        }

        private static void WriteSectionsPBXNativeTarget(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.PBXNativeTarget.Count == 0) return;
            Lines.AppendLine("/* Begin PBXNativeTarget section */");
            foreach (var kvp in ProjectInfo.Sections.PBXNativeTarget)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                if (item.BuildConfigurationList.HasValue)
                {
                    Lines.AppendLine($"\t\t\tbuildConfigurationList = {item.BuildConfigurationList.Value.ID} /* Build configuration list for PBXNativeTarget {item.Name} */;");
                }
                Lines.AppendLine("\t\t\tbuildPhases = (");
                foreach (var phase in item.BuildPhases)
                {
                    Lines.AppendLine($"\t\t\t\t{phase.ID},");
                }
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\tbuildRules = (");
                Lines.AppendLine("\t\t\t);");
                // We track dependency in SB, not in XCode, so leave blank here.
                Lines.AppendLine("\t\t\tdependencies = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine($"\t\t\tname = \"{item.Name}\";");
                Lines.AppendLine("\t\t\tpackageProductDependencies = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine($"\t\t\tproductName = \"{item.ProductName}\";");
                if (item.ProductReference != null)
                {
                    string productReferenceName = ""; // Used in comments only.
                    PBXFileReference? productReferenceObj = null;
                    if (ProjectInfo.Sections.PBXFileReference.ContainsKey(item.ProductReference.Value))
                    {
                        productReferenceObj =  ProjectInfo.Sections.PBXFileReference[item.ProductReference.Value];
                    }
                    if (productReferenceObj != null)
                    {
                        productReferenceName = productReferenceObj.Name;
                    }
                    Lines.AppendLine($"\t\t\tproductReference = {item.ProductReference.Value.ID} /* {productReferenceName} */;");
                }
                if (item.ProductType.HasValue)
                {
                    Lines.AppendLine($"\t\t\tproductType = \"{GetXCodeFileTypeXCName(item.ProductType.Value)}\";");
                }
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End PBXNativeTarget section */");
        }

        private static void WriteSectionsPBXProject(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.PBXProject.Count == 0) return;
            Lines.AppendLine("/* Begin PBXProject section */");
            foreach (var kvp in ProjectInfo.Sections.PBXProject)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                Lines.AppendLine("\t\t\tattributes = {");
                Lines.AppendLine("\t\t\t\tBuildIndependentTargetsInParallel = 1;");
                Lines.AppendLine("\t\t\t};");
                Lines.AppendLine($"\t\t\tbuildConfigurationList = {item.BuildConfigurationList.ID} /* Build configuration list for PBXProject */;");
                Lines.AppendLine("\t\t\tdevelopmentRegion = en;");
                Lines.AppendLine("\t\t\thasScannedForEncodings = 0;");
                Lines.AppendLine("\t\t\tknownRegions = (");
                Lines.AppendLine("\t\t\t\ten,");
                Lines.AppendLine("\t\t\t\tBase,");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine($"\t\t\tmainGroup = {item.MainGroup.ID};");
                Lines.AppendLine("\t\t\tminimizedProjectReferenceProxies = 1;");
                Lines.AppendLine("\t\t\tpreferredProjectObjectVersion = 77;");
                Lines.AppendLine($"\t\t\tproductRefGroup = {item.ProductRefGroup.ID};");
                Lines.AppendLine("\t\t\tprojectDirPath = \"\";");
                Lines.AppendLine("\t\t\tprojectRoot = \"\";");
                Lines.AppendLine("\t\t\ttargets = (");
                foreach (var target in item.Targets)
                {
                    string targetName = "";
                    PBXNativeTarget? targetObj = null;
                    if (ProjectInfo.Sections.PBXNativeTarget.ContainsKey(target))
                    {
                        targetObj = ProjectInfo.Sections.PBXNativeTarget[target];
                    }
                    if (targetObj != null)
                    {
                        targetName = targetObj.Name;
                    }
                    Lines.AppendLine($"\t\t\t\t{target.ID} /* {targetName} */,");
                }
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End PBXProject section */");
        }

        private static void WriteSectionsPBXShellScriptBuildPhase(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.PBXShellScriptBuildPhase.Count == 0) return;
            Lines.AppendLine("/* Begin PBXShellScriptBuildPhase section */");
            foreach (var kvp in ProjectInfo.Sections.PBXShellScriptBuildPhase)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                Lines.AppendLine("\t\t\talwaysOutOfDate = 1;"); // Force this script to run in every build action.
                Lines.AppendLine("\t\t\tbuildActionMask = 2147483647;"); // Run this script in all action types.
                Lines.AppendLine("\t\t\tfiles = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\tinputFileListPaths = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\tinputPaths = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine($"\t\t\tname = \"{item.Name}\";");
                Lines.AppendLine("\t\t\toutputFileListPaths = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\toutputPaths = (");
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\trunOnlyForDeploymentPostprocessing = 0;");
                Lines.AppendLine("\t\t\tshellPath = /bin/sh;");
                Lines.AppendLine($"\t\t\tshellScript = \"{item.ShellScript}\";");
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End PBXShellScriptBuildPhase section */");
        }

        private static void WriteSectionsXCBuildConfiguration(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.XCBuildConfiguration.Count == 0) return;
            Lines.AppendLine("/* Begin XCBuildConfiguration section */");
            foreach (var kvp in ProjectInfo.Sections.XCBuildConfiguration)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                Lines.AppendLine("\t\t\tbuildSettings = {");
                foreach (var buildSetting in item.BuildSettings)
                {
                    string? buildSettingStr = null;
                    Type valueType = buildSetting.Value.GetType();
                    if (valueType == typeof(string))
                    {
                        buildSettingStr = buildSetting.Value as string;
                    }
                    else if (valueType == typeof(bool))
                    {
                        buildSettingStr = buildSetting.Value as bool? == true ? "YES" : "NO";
                    }
                    else
                    {
                        buildSettingStr =  buildSetting.Value.ToString();
                    }
                    Lines.AppendLine($"\t\t\t\t{buildSetting.Key} =  {buildSettingStr};");
                }
                Lines.AppendLine("\t\t\t};");
                Lines.AppendLine($"\t\t\tname = \"{item.Name}\";");
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End XCBuildConfiguration section */");
        }

        private static void WriteSectionsXCConfigurationList(XCodeProjectInfo ProjectInfo, StringBuilder Lines)
        {
            if(ProjectInfo.Sections.XCConfigurationList.Count == 0) return;
            Lines.AppendLine("/* Begin XCBuildConfiguration section */");
            foreach (var kvp in ProjectInfo.Sections.XCConfigurationList)
            {
                Lines.AppendLine("\t\t" + kvp.Key.ID + " = {");
                var item = kvp.Value;
                Lines.AppendLine($"\t\t\tisa = {item.IsA};");
                Lines.AppendLine("\t\t\tbuildConfigurations = (");
                foreach (var buildConfiguration in item.BuildConfigurations)
                {
                    string buildConfigurationStr = "";
                    if (ProjectInfo.Sections.XCBuildConfiguration.ContainsKey(buildConfiguration))
                    {
                        XCBuildConfiguration buildConfigurationObj = ProjectInfo.Sections.XCBuildConfiguration[buildConfiguration];
                        buildConfigurationStr = buildConfigurationObj.Name;
                    }
                    Lines.AppendLine($"\t\t\t\t{buildConfiguration.ID} /* {buildConfigurationStr} */,");
                }
                Lines.AppendLine("\t\t\t);");
                Lines.AppendLine("\t\t\tdefaultConfigurationIsVisible = 0;");
                Lines.AppendLine("\t\t};");
            }
            Lines.AppendLine("/* End XCBuildConfiguration section */");
        }

        public static void GenerateProjectFile(XCodeProjectInfo ProjectInfo)
        {
            ProjectInfo.BuildXCNodes();
            StringBuilder sb = new();
            sb.AppendLine("// !$*UTF8*$!");
            sb.AppendLine("{");
            sb.AppendLine("\tarchiveVersion = 1;");
            sb.AppendLine("\tclasses = {");
            sb.AppendLine("\t};");
            sb.AppendLine("\tobjectVersion = 77;");
            sb.AppendLine("\tobjects = {");
            // Write sections.
            WriteSectionsPBXFileReference(ProjectInfo, sb);
            WriteSectionsPBXGroup(ProjectInfo, sb);
            WriteSectionsPBXNativeTarget(ProjectInfo, sb);
            WriteSectionsPBXProject(ProjectInfo, sb);
            WriteSectionsPBXShellScriptBuildPhase(ProjectInfo, sb);
            WriteSectionsXCBuildConfiguration(ProjectInfo, sb);
            WriteSectionsXCConfigurationList(ProjectInfo, sb);
            sb.AppendLine("\t};");
            sb.AppendLine($"\trootObject = {ProjectInfo.RootObject.ID} /* Project object */;");
            sb.AppendLine("}");
            string content = sb.ToString();
            string bundlePath = Path.Combine(ProjectInfo.ProjectDir, ProjectInfo.ProjectBundle + ".xcodeproj");
            if (!Directory.Exists(bundlePath))
            {
                Directory.CreateDirectory(bundlePath);
            }
            string pbxprojPath = Path.Combine(bundlePath, "project.pbxproj");
            string oldContent = "";
            if (File.Exists(pbxprojPath))
            {
                oldContent = File.ReadAllText(pbxprojPath);
            }
            if (content == oldContent)
            {
                Log.Information($"Skipped writing file {pbxprojPath} since the file has the same content");
            }
            else
            {
                File.WriteAllText(pbxprojPath, content);
            }
        }
    }
}