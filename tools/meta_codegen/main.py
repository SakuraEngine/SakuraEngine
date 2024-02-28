import argparse
import importlib
import importlib.util
import glob
import os


def load_generator(i, path):
    spec = importlib.util.spec_from_file_location("Generator%d" % i, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return getattr(module, "Generator")()


if __name__ == '__main__':
    # parse args
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('generators', help="generator file list.", nargs="*")
    parser.add_argument('-root', help="root directory of meta files.", required=True, type=str)
    parser.add_argument('-outdir', help="output directory.", required=True, type=str)
    parser.add_argument("-api", help="api name.", required=True, type=str)
    parser.add_argument("-module", help="module name.", required=True, type=str)
    parser.add_argument("-config", help="config file name.", required=True)
    parser.add_argument("-includes", help="include directory list.", nargs="+")
    args = parser.parse_args()

    # load generators
    generators = []
    for i, x in enumerate(args.generators):
        generators.append(load_generator(i, x))

    # collect meta files
    meta_files = glob.glob(os.path.join(args.root, "**", "*.h.meta"), recursive=True)

    # load meta files
    # parse meta files
    # codegen
