import os
import framework.generator as gen
import framework.scheme as sc


class FunctionBindGenerator(gen.GeneratorBase):
    def generate(self):
        pass


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("script_function_bind", FunctionBindGenerator())
