from framework.attr_parser import *
from framework.attr_parser import ErrorTracker, List
from framework.config import *
from framework.config import CodegenConfig, List
from framework.database import *
import mako
import mako.template

from framework.database import CodegenConfig, ErrorTracker, List, ModuleDatabase

# TODO. 需要提供依赖图谱


class FileCache:
    def __init__(self, root_dir: str) -> None:
        self.__file_content: Dict[str, str] = {}
        self.root_dir: str = root_dir

    def load_template(self, template_path: str) -> mako.template.Template:
        with open(template_path, "r", encoding="utf-8") as f:
            return mako.template.Template(
                f.read(),
                filename=template_path,
                input_encoding="utf-8",
                strict_undefined=True,
            )

    def append_content(self, relative_path: str, content: str):
        solve_path = os.path.normpath(os.path.join(self.root_dir, relative_path))
        if solve_path in self.__file_content:
            self.__file_content[solve_path] = self.__file_content[solve_path] + content
        else:
            self.__file_content[solve_path] = content

    def get_content(self, relative_path: str) -> str:
        solve_path = os.path.normpath(os.path.join(self.root_dir, relative_path))
        return self.__file_content[solve_path]

    def output(self):
        for file, content in self.__file_content.items():
            with open(file, "w") as f:
                f.write(content)


@dataclass
class GenerateCodeEnv:
    file_cache: FileCache
    module_db: ModuleDatabase
    include_dbs: List[ModuleDatabase]
    codegen_config: CodegenConfig
    error_tracker: ErrorTracker


class GeneratorBase:
    def require_include_dbs(self) -> bool:
        return False

    def load_functional(self, parser_manager: ParserManager):
        pass

    def check_attrs(self, module_db: ModuleDatabase, error_tracker: ErrorTracker):
        pass

    def pre_generate(self, env: GenerateCodeEnv):
        pass

    def generate(self, env: GenerateCodeEnv):
        pass

    def post_generate(self, env: GenerateCodeEnv):
        pass
