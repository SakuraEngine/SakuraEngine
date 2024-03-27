from framework.attr_parser import *
from framework.attr_parser import ErrorTracker, List
from framework.config import *
from framework.config import CodegenConfig, List
from framework.database import *
import framework.log as log
import mako
import mako.template

from framework.database import CodegenConfig, ErrorTracker, List, ModuleDatabase

# codegen workflow
#   --- generator load，加载生成器，做前期处理
#   1. load generators
#   2. init generators (在此处阶段可以检查 generator 依赖，向别的 generator 注入信息等)
#   3. load schema
#   --- schema parse，本阶段只进行语法解析，不进复杂的验证操作
#   4. expand shorthand and path
#   5. check structure
#   6. to object (with attr stack)，本阶段可以顺便将如 guid 等复杂属性解析出来
#   --- check attribute，检查属性和属性组合是否合法，如果有值域限制，也在此处检查
#   7. check attribute
#   --- generate code
#   8. pre generate，用于生成文件的头部内容，也可以在此阶段向其它 generator（如 generate body）注入内容
#   9. generate，用于生成文件的中段内容，通常在此阶段生成代码
#   10. post generate，用于生成文件的尾部内容


class FileCache:
    def __init__(self, root_dir: str) -> None:
        self.__file_content: Dict[str, str] = {}
        self.root_dir: str = root_dir

    def load_template(self, template_path: str) -> mako.template.Template:
        with open(template_path, "rb") as f:
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
            directory = os.path.dirname(file)
            if not os.path.exists(directory):
                os.makedirs(directory, exist_ok=True)
            with open(file, "wb") as f:
                f.write(content.encode("utf-8"))


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
