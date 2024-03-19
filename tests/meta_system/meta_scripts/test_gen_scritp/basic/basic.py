from framework.generator import *
from framework.database import *
from framework.attr_parser import *
from framework.generator import *


class BasicCPPGenerator(GeneratorBase):
    def pre_generate(self, env: GenerateCodeEnv):
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_top_template = env.file_cache.load_template(os.path.join(script_dir, "header_top.mako"))
        source_top_template = env.file_cache.load_template(os.path.join(script_dir, "source_top.mako"))

        # gen header
        for header_db in env.module_db.header_dbs:
            env.file_cache.append_content(
                header_db.relative_target_header_path,
                header_top_template.render(header_db=header_db)
            )

        # gen source
        env.file_cache.append_content(
            "generated.cpp",
            source_top_template.render(module_db=env.module_db)
        )

    def post_generate(self, env: GenerateCodeEnv):
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_bottom_template = env.file_cache.load_template(os.path.join(script_dir, "header_bottom.mako"))
        source_bottom_template = env.file_cache.load_template(os.path.join(script_dir, "source_bottom.mako"))

        # gen header
        for header_db in env.module_db.header_dbs:
            env.file_cache.append_content(
                header_db.relative_target_header_path,
                header_bottom_template.render(header_db=header_db)
            )

        # gen source
        env.file_cache.append_content(
            "generated.cpp",
            source_bottom_template.render(module_db=env.module_db)
        )


def load_generators():
    return [
        BasicCPPGenerator()
    ]
