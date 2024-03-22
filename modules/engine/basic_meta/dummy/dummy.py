from framework.generator import *
from framework.database import *
from framework.attr_parser import *
from framework.generator import *

class DummyGenerator(GeneratorBase):
    def load_functional(self, parser_manager: ParserManager):
        # guid
        parser_manager.add_record_parser("guid", StrParser())
        parser_manager.add_enum_parser("guid", StrParser())

        # serialize
        serialize_parser = FunctionalParser(
            options={
                    "bin": BoolParser(default_value=False),
                    "json": BoolParser(default_value=False),
            },
            shorthands=[
                StrShorthand(
                    options_mapping={
                        "bin": { "bin": True },
                        "json": { "json": True },
                    }
                ),
                ListShorthand(
                    options_mapping={
                        "bin": { "bin": True },
                        "json": { "json": True },
                    }
                )
            ]
        )
        parser_manager.add_record_parser("serialize", serialize_parser)
        parser_manager.add_enum_parser("serialize", serialize_parser)
        parser_manager.add_record_parser("serialize_config", StrParser())   # 向 recorder 的序列化函数添加自定义配置
        parser_manager.add_field_parser("serialize_config", StrParser())    # 向字段的序列化尾部添加自定义的配置
        parser_manager.add_record_parser("blob", BoolParser())      # 指定是否使用 blob 序列化
        parser_manager.add_field_parser("arena", StrParser())       # 字段使用哪个 arena
        parser_manager.add_record_parser("debug", BoolParser())     # 只生成 write 不生成 read（目前仅在 json 序列化中使用）
        parser_manager.add_field_parser("transient", BoolParser())  # 指定字段是否不参与序列化
        parser_manager.add_field_parser("no-text", BoolParser())    # 指定字段是否不参与 json 序列化
        parser_manager.add_field_parser("no-default", BoolParser()) # 指定字段在 json 反序列化时，如果不存在，强制报错而不是不处理

        # ecs
        parser_manager.add_record_parser("component", FunctionalParser(
            options={
                "pin": BoolParser(),    # SUGOI_TYPE_FLAG_PIN
                "chunk": BoolParser(),  # SUGOI_TYPE_FLAG_CHUNK
                "buffer": IntParser(),  # desc.elementSize
                "custom": StrParser(),  # 在注册期间插入自定义函数，目前用于插入（sugoi::managed_component）
                "unsafe": BoolParser(), # 不检查是否 managed（sugoi::check_managed）
            }
        )) 
        parser_manager.add_record_parser("query", StrParser()) # 通过 query 表达式生产 query

        # lua
        parser_manager.add_record_parser("scriptable", BoolParser())
        parser_manager.add_enum_parser("scriptable", BoolParser())
        parser_manager.add_field_parser("native", BoolParser())         # 字段是否生成 lua 绑定
        parser_manager.add_method_parser("native", BoolParser())        # 方法是否生成 lua 绑定
        parser_manager.add_parameter_parser("in", BoolParser())         # 参数作用标记
        parser_manager.add_parameter_parser("out", BoolParser())        # 参数作用标记
        parser_manager.add_parameter_parser("inout", BoolParser())      # 参数作用标记
        parser_manager.add_parameter_parser("userdata", BoolParser())   # 接入自定义的 userdata 导入逻辑（通过模板）
        
        # trait
        parser_manager.add_record_parser("trait", BoolParser()) # 是否作为 trait 使用
        parser_manager.add_method_parser("getter", StrParser())   # 作为某个 field 的 getter，将会自动绑定到该 field 上
        parser_manager.add_method_parser("setter", StrParser())   # 作为某个 field 的 setter，将会自动绑定到该 field 上
        
        # rttr
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
        
        # TODO. 静态调用标记
        parser_manager.add_method_parser("static_invoke", FunctionalParser())
        parser_manager.add_function_parser("static_invoke", FunctionalParser())

def load_generators():
    return [
        DummyGenerator()
    ]