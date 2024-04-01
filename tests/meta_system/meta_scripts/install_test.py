from test_framework import *
import framework.generator as gen


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("test_framework", TestFrameworkGenerator())
