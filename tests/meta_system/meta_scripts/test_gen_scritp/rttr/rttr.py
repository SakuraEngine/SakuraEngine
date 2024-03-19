from framework.generator import *
from framework.database import *
from framework.attr_parser import *
import os
from typing import List
import mako
import mako.template


class RTTRGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        # record
        parser_manager.add_record_parser(
            "rttr",
            FunctionalParser(
                options={
                    "reflect_bases": BoolParser(default_value=True),
                    "exclude_bases": ListParser(default_value=[]),
                    "reflect_fields": BoolParser(default_value=False),
                    "reflect_methods": BoolParser(default_value=False),
                },
                shorthands=[
                    StrShorthand(
                        {
                            "all": {
                                "reflect_bases": True,
                                "reflect_fields": True,
                                "reflect_methods": True,
                            }
                        }
                    )
                ]
            )
        )

        # method
        parser_manager.add_method_parser(
            "rttr",
            FunctionalParser()
        )

        # field
        parser_manager.add_field_parser(
            "rttr",
            FunctionalParser()
        )

        # enum
        parser_manager.add_enum_parser(
            "rttr",
            FunctionalParser()
        )

        # enum value
        parser_manager.add_enum_value_parser(
            "rttr",
            FunctionalParser()
        )

    def generate(self, env: GenerateCodeEnv):
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_template = env.file_cache.load_template(os.path.join(script_dir, "rttr.hpp.mako"))
        source_template = env.file_cache.load_template(os.path.join(script_dir, "rttr.cpp.mako"))

        # gen header
        for header_db in env.module_db.header_dbs:
            env.file_cache.append_content(
                header_db.relative_target_header_path,
                header_template.render(header_db=header_db, api=env.module_db.api)
            )

        # gen source
        env.file_cache.append_content(
            "generated.cpp",
            source_template.render(module_db=env.module_db)
        )


def load_generators():
    return [
        RTTRGenerator()
    ]
