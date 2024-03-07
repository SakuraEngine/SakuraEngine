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
    from framework.error_tracker import ErrorTracker
    from framework.attr_parser import *

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
    generators = []
    for i, x in enumerate(args.generators):
        generators.append(load_generator(i, x))

    # collect meta files
    meta_files = glob.glob(os.path.join(args.root, "**", "*.h.meta"), recursive=True)

    print(args.root)
    for file in meta_files:
        database = HeaderDatabase()
        database.load_header(file)
        tracker = ErrorTracker()
        tracker.set_phase("CheckStructure")
        database.attr_check_structure(tracker, FunctionalManager())
