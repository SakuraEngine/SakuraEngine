from framework.attr_parser import *
from framework.config import *
from framework.database import *

# TODO. 需要提供依赖图谱


class FileCache:
    def __init__(self) -> None:
        self.__file_content: Dict[str, str] = {}

    def append_content(self, file: str, content: str):
        self.__file_content[file] += content

    def get_content(self, file: str) -> str:
        return self.__file_content[file]

    def output(self):
        for file, content in self.__file_content.items():
            with open(file, "w") as f:
                f.write(content)


class GeneratorBase:
    def require_include_dbs(self) -> bool:
        return False

    def load_functional(self, parser_manager: FunctionalManager):
        pass

    def pre_generate(self, config: GeneratorConfig, module_db: ModuleDatabase, include_dbs: List[ModuleDatabase], error_tracker: ErrorTracker, file_cache: FileCache):
        pass

    def generate(self, config: GeneratorConfig, module_db: ModuleDatabase, include_dbs: List[ModuleDatabase], error_tracker: ErrorTracker, file_cache: FileCache):
        pass

    def post_generate(self, config: GeneratorConfig, module_db: ModuleDatabase, include_dbs: List[ModuleDatabase], error_tracker: ErrorTracker, file_cache: FileCache):
        pass
