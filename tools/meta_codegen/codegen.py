import argparse
import importlib
import importlib.util
import os
import sys
from typing import List, Dict


if __name__ == '__main__':
    sys.path.insert(0, os.path.dirname(sys.argv[0]))
    import framework.config as config
    import framework.generator as generator

    # parse args
    parser = argparse.ArgumentParser(description="generate code from meta files")
    parser.add_argument('-c', '--config', help="config file.", required=True, type=str)
    args = parser.parse_args()

    generate_manager = generator.GenerateManager()

    # load config
    with open(args.config, 'r', encoding="utf-8") as f:
        generate_manager.load_config(config_file=f)

    # load
    generate_manager.load_generators()
    generate_manager.load_database()

    # parse and check attr
    generate_manager.parse_attrs()
    generate_manager.solve_attrs()

    # generate code
    generate_manager.generate_body()
    generate_manager.pre_generate()
    generate_manager.generate()
    # mix in old framework generate
    # TODO. remove it
    import framework.old_framework as old_framework
    old_framework.generate(generate_manager.config, generate_manager)
    generate_manager.post_generate()
    generate_manager.output_content()

    # dump log
    generate_manager.logger.dump(error_mode=False)
