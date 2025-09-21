// Declares clang::SyntaxOnlyAction.
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "ASTConsumer.h"
#include "IgnoreGeneratedFileSystem.h"
#include "OptionsParser.h"
#include "meta.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TimeProfiler.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "clang/Frontend/PCHContainerOperations.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>

namespace tooling = clang::tooling;
// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory ToolCategoryOption("meta options");
static llvm::cl::cat ToolCategory(ToolCategoryOption);

// command args
static llvm::cl::opt<std::string> Output(
    "output", llvm::cl::Required,
    llvm::cl::desc("Specify database output directory, depending on extension"),
    ToolCategory, llvm::cl::value_desc("directory"));
static llvm::cl::opt<std::string>
    Root("root", llvm::cl::Required,
         llvm::cl::desc("Specify parse root directory"), ToolCategory,
         llvm::cl::value_desc("directory"));

// new command args
// static llvm::cl::opt<std::string> Config(
//     "config", llvm::cl::Required,
//     llvm::cl::desc("Specify config file"),
//     ToolCategory, llvm::cl::value_desc("file path"));
// static llvm::cl::opt<std::string> Outdir(
//     "outdir", llvm::cl::Required,
//     llvm::cl::desc("Specify database output directory, depending on extension"),
//     ToolCategory, llvm::cl::value_desc("directory"));

// custom action
static meta::FileDataMap data_map;
class ReflectFrontendAction : public clang::ASTFrontendAction {
public:
  ReflectFrontendAction() {}


  std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &compiler, llvm::StringRef file) override {
    // fronted opts
    auto &FO = compiler.getFrontendOpts();
    FO.SkipFunctionBodies = true;
    FO.ProgramAction = clang::frontend::ParseSyntaxOnly;

    // lang opts
    auto &LO = compiler.getLangOpts();
    LO.CommentOpts.ParseAllComments = true;

    return std::make_unique<meta::ASTConsumer>(data_map, Root);
  }
};

int main(int argc, const char **argv) {
  // copy args
  std::vector<const char *> args{};
  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  // meta def
  args.insert(args.begin() + 1, "--extra-arg=-D__meta__");
  argc = args.size();

  // parse args
  llvm::outs() << "===========start parse arg===========\n";
  auto ExpectedParser = meta::OptionsParser::create(
      argc,
      args.data(),
      llvm::cl::ZeroOrMore,
      ToolCategoryOption);
  if (!ExpectedParser) {
    // Fail gracefully for unsupported options.
    llvm::errs() << ExpectedParser.takeError();
    return 1;
  }
  meta::OptionsParser &OptionsParser = ExpectedParser.get();
  llvm::outs() << "===========end parse arg===========\n";

  // init time trace
  timeTraceProfilerInitialize(32, llvm::StringRef{args[0]});

  // run tool
  // auto start = std::chrono::high_resolution_clock::now();
  
  // Setup custom virtual file system to ignore .generated.h files
  auto BaseFS = llvm::vfs::getRealFileSystem();
  auto CustomFS = llvm::makeIntrusiveRefCnt<meta::IgnoreGeneratedFileSystem>(BaseFS);
  
  tooling::ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList(),
                          std::make_shared<clang::PCHContainerOperations>(), CustomFS);
  
  llvm::outs() << "===========start compile===========\n";
  int result = Tool.run(tooling::newFrontendActionFactory<ReflectFrontendAction>().get());
  llvm::outs() << "===========end compile===========\n";
  // auto end = std::chrono::high_resolution_clock::now();
  // std::cout << "[" << Root << "]\n"
  //           << "Elapsed time: "
  //           << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
  //           << "ms\n";

  // serialize
  std::string OutPath;
  OutPath = Output;
  llvm::outs() << "===========start write===========\n";
  for (auto &pair : data_map) {
    if (pair.second.is_empty())
      continue;

    // replace extension to .h.meta
    llvm::SmallString<1024> MetaPath(OutPath + pair.first);
    llvm::sys::path::replace_extension(MetaPath, ".h.meta");

    // create meta dir
    llvm::SmallString<1024> MetaDir = MetaPath;
    llvm::sys::path::remove_filename(MetaDir);
    auto error_code = llvm::sys::fs::create_directories(MetaDir);
    if (error_code) {
      llvm::errs() << "failed to create directory: " << MetaDir << "\n";
      llvm::errs() << "error: " << error_code.message() << "\n";
      return 1;
    }

    // write meta file
    std::ofstream of(MetaPath.str().str());
    of << pair.second.serialize();
  }
  llvm::outs() << "===========end write===========\n";

  // output time trace
  llvm::outs() << "===========start dump trace===========\n";
  {
    // get trace path
    llvm::SmallString<1024> time_trace_path(OutPath);
    llvm::sys::path::append(time_trace_path, "meta_time.json");

    // remove old file and create dir
    llvm::SmallString<1024> time_trace_dir = time_trace_path;
    llvm::sys::path::remove_filename(time_trace_dir);
    auto error_code = llvm::sys::fs::create_directories(time_trace_dir);
    if (error_code) {
      llvm::errs() << "failed to create directory: " << time_trace_dir << "\n";
      llvm::errs() << "error: " << error_code.message() << "\n";
      return 1;
    }

    // output time trace
    std::error_code ec;
    llvm::raw_fd_stream fs(time_trace_path, ec);
    if (ec) {
      llvm::errs() << "failed to write time trace: " << time_trace_path << "\n";
      llvm::errs() << "error: " << ec.message() << "\n";
      return 1;
    }
    llvm::timeTraceProfilerWrite(fs);
    fs.close();
  }
  llvm::outs() << "===========end dump trace===========\n";

  return result;
}