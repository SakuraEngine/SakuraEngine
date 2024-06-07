from test_framework import *
import framework.generator as gen


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("test_path_expand", TestPathExpandGenerator())
    generate_manager.add_generator("test_functional_shorthand", TestFunctionalShorthand())
