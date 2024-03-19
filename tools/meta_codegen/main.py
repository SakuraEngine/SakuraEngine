import argparse
import importlib
import importlib.util
import os
import sys
from typing import List, Dict


if __name__ == '__main__':
    sys.path.insert(0, os.path.dirname(sys.argv[0]))
    from framework.database import HeaderDatabase
    from framework.error_tracker import *
    from framework.attr_parser import *
    from framework.generator import *
    from framework.database import *
    import framework.config

    # parse args
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('-c', '--config', help="config file.", required=True, type=str)
    args = parser.parse_args()

    # load config
    config = framework.config.load_config(args.config)

    # load generators
    generators: List[GeneratorBase] = []
    for i, generator_config in enumerate(config.generators):
        # add import dir
        for path in generator_config.import_dirs:
            sys.path.insert(0, path)

        # load module
        spec = importlib.util.spec_from_file_location("Generator%d" % i, generator_config.entry_file)
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)

        # get generators
        for generator in module.load_generators():
            generators.append(generator)

    # init error tracker
    error_tracker = ErrorTracker()

    # step1. load parser
    parser_manager = ParserManager()
    for generator in generators:
        generator.load_functional(parser_manager)

    # step2. load main module
    main_module_db = ModuleDatabase()
    main_module_db.load(config.main_module, config)
    main_module_db.solve_attr(config, error_tracker, parser_manager)

    # step3. load include module
    include_module_dbs = []
    require_include_dbs = False
    for generator in generators:
        require_include_dbs = require_include_dbs or generator.require_include_dbs()
    if require_include_dbs:
        for include_module in config.include_modules:
            module_db = ModuleDatabase()
            module_db.load(include_module, config)
            module_db.solve_attr(config, error_tracker, parser_manager)
            include_module_dbs.append(module_db)

    # step4. generate code
    env = GenerateCodeEnv(
        file_cache=FileCache(config.output_dir),
        module_db=main_module_db,
        include_dbs=include_module_dbs,
        codegen_config=config,
        error_tracker=error_tracker,
    )
    for generator in generators:
        generator.pre_generate(env)
    for generator in generators:
        generator.generate(env)
    for generator in generators:
        generator.post_generate(env)
    env.file_cache.output()
    if error_tracker.any_error():
        error_tracker.dump()
        raise Exception("generate code failed")

    # dump error
    error_tracker.dump()
