#include "annotation_module.hpp"

// create type factory, so that type can be bound
MAKE_TYPE_FACTORY(Color, Color);

// registering module, so that its available via 'NEED_MODULE' macro
REGISTER_MODULE(Module_Tutorial03);

#define TUTORIAL_NAME   "/scripts/daSTestAnnotation/annotation.das"
#include "tutorial.inc"

int main( int argc, char **argv ) {
    // request all da-script built in modules
    NEED_ALL_DEFAULT_MODULES;
    NEED_MODULE(Module_UriParser)
    NEED_MODULE(Module_JobQue)
    // request our custom module
    NEED_MODULE(Module_Tutorial03);
    das::setCommandLineArguments(argc, argv);
    // Initialize modules
    Module::Initialize();
    // run the tutorial
    tutorial();
    // shut-down daScript, free all memory
    Module::Shutdown();
    return 0;
}
