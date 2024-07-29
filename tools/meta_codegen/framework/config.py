
from dataclasses import dataclass, field
from typing import Dict, List
import json

# TODO. Generator 跟着 Module 走，访问性在 Python 中进行解析


def _get_list_or_empty(dict: Dict, key: str) -> list:
    return dict[key] if key in dict and type(dict[key]) is list else []


@dataclass
class ModuleConfig:
    module_name: str = ""
    meta_dir: str = ""
    api: str = ""
    generators: List['GeneratorConfig'] = field(default_factory=lambda: [])

    def load(self, json_data: Dict):
        self.module_name = json_data["module_name"]
        self.meta_dir = json_data["meta_dir"]
        self.api = json_data["api"]


@dataclass
class GeneratorConfig:
    entry_file: str = ""
    import_dirs: List[str] = field(default_factory=lambda: [])

    def load(self, json_data: Dict):
        self.entry_file = json_data["entry_file"]

        # load dirs
        self.import_dirs = _get_list_or_empty(json_data, "import_dirs")


@dataclass
class CodegenConfig:
    output_dir: str = ""
    main_module: ModuleConfig = field(default_factory=lambda: ModuleConfig())
    include_modules: List[ModuleConfig] = field(default_factory=lambda: [])
    generators: List[GeneratorConfig] = field(default_factory=lambda: [])

    def load(self, json_data: Dict):
        self.output_dir = json_data["output_dir"]

        # load main module
        self.main_module.load(json_data["main_module"])

        # load include modules
        for module in _get_list_or_empty(json_data, "include_modules"):
            module_config = ModuleConfig()
            module_config.load(module)
            self.include_modules.append(module_config)

        # load generators
        for generator in _get_list_or_empty(json_data, "generators"):
            generator_config = GeneratorConfig()
            generator_config.load(generator)
            self.generators.append(generator_config)
