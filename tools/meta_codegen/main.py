import argparse
import importlib
import importlib.util
import glob
import os
import sys


def load_generator(i, path):
    spec = importlib.util.spec_from_file_location("Generator%d" % i, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return getattr(module, "Generator")()


if __name__ == '__main__':
    sys.path.insert(0, os.path.dirname(sys.argv[0]))
    from framework.database import HeaderDatabase
    from framework.error_tracker import *
    from framework.attr_parser import *
    from framework.generator import *
    from framework.database import *

    # parse args
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('-root', help="root directory of meta files.", required=True, type=str)
    parser.add_argument('-outdir', help="output directory.", required=True, type=str)
    parser.add_argument("-api", help="api name.", required=True, type=str)
    parser.add_argument("-module", help="module name.", required=True, type=str)
    parser.add_argument("-includes", help="include directory list.", nargs="+")
    parser.add_argument('-generators', help="generator file list.", nargs="*")
    args = parser.parse_args()

    # load generators
    generators: List[GeneratorBase] = []
    for i, x in enumerate(args.generators):
        generators.append(load_generator(i, x))

    # load parser
    parser_manager = FunctionalManager()
    for generator in generators:
        generator.load_functional(parser_manager)

    # collect meta files
    meta_files = glob.glob(os.path.join(args.root, "**", "*.h.meta"), recursive=True)

    # load meta files
    module_db = ModuleDatabase()
    module_db.module_name = args.module

    for file in meta_files:
        tracker = ErrorTracker()
        tracker.set_phase("CheckStructure")
        # tracker.set_raise_error(True)

        module_db.load_header(file)
        module_db.expand_shorthand_and_path(tracker, parser_manager)
        module_db.check_structure(tracker, parser_manager)
        module_db.to_object(tracker, parser_manager)

        tracker.dump()
