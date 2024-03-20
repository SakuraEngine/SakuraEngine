from dataclasses import *
from typing import List, Dict
import json


def get_list(dict: Dict, key: str) -> list:
    return dict[key] if key in dict and type(dict[key]) is list else []


@dataclass
class ModuleConfig:
    module_name: str = ""
    meta_dir: str = ""
    api: str = ""

    def load(self, json_data: Dict):
        self.module_name = json_data["module_name"]
        self.meta_dir = json_data["meta_dir"]
        self.api = json_data["api"]


@dataclass
class GeneratorConfig:
    entry_file: str = ""
    import_dirs: List[str] = field(default_factory=lambda: [])
    use_new_framework: bool = False  # TODO. 过渡用，后续移除

    def load(self, json_data: Dict):
        self.entry_file = json_data["entry_file"]

        # load dirs
        self.import_dirs = get_list(json_data, "import_dirs")


@dataclass
class CodegenConfig:
    output_dir: str = ""
    main_module: ModuleConfig = ModuleConfig()
    include_modules: List[ModuleConfig] = field(default_factory=lambda: [])
    generators: List[GeneratorConfig] = field(default_factory=lambda: [])

    def load(self, json_data: Dict):
        self.output_dir = json_data["output_dir"]

        # load main module
        self.main_module.load(json_data["main_module"])

        # load include modules
        for module in get_list(json_data, "include_modules"):
            module_config = ModuleConfig()
            module_config.load(module)
            self.include_modules.append(module_config)

        # load generators
        for generator in get_list(json_data, "generators"):
            generator_config = GeneratorConfig()
            generator_config.load(generator)
            self.generators.append(generator_config)


def load_config(path: str) -> CodegenConfig:
    with open(path, "r", encoding="utf-8") as f:
        json_data = json.load(f)
        config = CodegenConfig()
        config.load(json_data)
        return config
