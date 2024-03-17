import argparse
import importlib
import importlib.util
import glob
import os
import sys
from types import SimpleNamespace
from dataclasses import *
from typing import List, Dict


@dataclass
class ModuleConfig:
    module_name: str
    meta_dir: str
    api: str


@dataclass
class GeneratorConfig:
    entry_file: str
    import_dirs: List[str]


@dataclass
class CodegenConfig:
    output_dir: str
    main_module: ModuleConfig
    include_modules: List[ModuleConfig] = field(default_factory=lambda: [])
    generators: List[GeneratorConfig] = field(default_factory=lambda: [])


if __name__ == '__main__':
    sys.path.insert(0, os.path.dirname(sys.argv[0]))
    from framework.database import HeaderDatabase
    from framework.error_tracker import *
    from framework.attr_parser import *
    from framework.generator import *
    from framework.database import *

    # parse args
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('-c', '--config', help="config file.", required=True, type=str)
    args = parser.parse_args()

    # load config
    with open(args.config, "r") as f:
        config = json.load(f, object_hook=lambda d: SimpleNamespace(**d))

    # load generators
    generators: List[GeneratorBase] = []
    for i, generator_config in enumerate(config.generators):
        # add import dir
        # for path in generator_config.import_dirs:
        #     sys.path.insert(0, generator_config.import_dirs)

        # load module
        spec = importlib.util.spec_from_file_location("Generator%d" % i, generator_config.entry_file)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        # get generators
        for generator in module.load_generators():
            generators.append(generator)

    # load parser
    parser_manager = FunctionalManager()
    for generator in generators:
        generator.load_functional(parser_manager)

    # collect meta files
    meta_files = glob.glob(os.path.join(config.main_module.meta_dir, "**", "*.h.meta"), recursive=True)

    # load meta files
    module_db = ModuleDatabase()
    module_db.module_name = config.main_module.module_name

    for file in meta_files:
        tracker = ErrorTracker()
        tracker.set_phase("CheckStructure")
        # tracker.set_raise_error(True)

        module_db.load_header(file)
        module_db.expand_shorthand_and_path(tracker, parser_manager)
        module_db.check_structure(tracker, parser_manager)
        module_db.to_object(tracker, parser_manager)

        tracker.dump()
