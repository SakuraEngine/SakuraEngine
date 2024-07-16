import os
import framework.generator as gen
import framework.scheme as sc


class BasicCPPGenerator(gen.GeneratorBase):
    def load_scheme(self):
        # dummy scheme to prevent warning
        # serialize
        serialize_parser = sc.Functional(
            {
                "bin": sc.Bool(),
                "json": sc.Bool(),
            },
            shorthands=[
                sc.OptionShorthand(
                    mappings={
                        "bin": {"bin": True},
                        "json": {"json": True},
                    }
                )
            ])
        self.owner.add_record_scheme(
            sc.Namespace({
                "serialize": serialize_parser,
                "serialize_config": sc.Str(),  # 向 recorder 的序列化函数添加自定义配置
                "blob": sc.Bool(),  # 指定是否使用 blob 序列化
                "debug": sc.Bool(),  # 只生成 write 不生成 read（目前仅在 json 序列化中使用）
            })
        )
        self.owner.add_field_scheme(
            sc.Namespace({
                "serialize_config": sc.Str(),  # 向字段的序列化尾部添加自定义的配置
                "arena": sc.Str(),  # 字段使用哪个 arena
                "transient": sc.Bool(),  # 指定字段是否不参与序列化
                "no-text": sc.Bool(),  # 指定字段是否不参与 json 序列化
                "no-default": sc.Bool(),  # 指定字段在 json 反序列化时，如果不存在，强制报错而不是不处理
            })
        )
        self.owner.add_enum_scheme(
            sc.Namespace({
                "serialize": serialize_parser
            })
        )

        # trait
        self.owner.add_record_scheme(
            sc.Namespace({
                "trait": sc.Bool(),  # 是否作为 trait 使用
            })
        )
        self.owner.add_method_scheme(
            sc.Namespace({
                "getter": sc.Str(),  # 作为某个 field 的 getter，将会自动绑定到该 field 上
                "setter": sc.Str(),  # 作为某个 field 的 setter，将会自动绑定到该 field 上
            })
        )

        # 静态调用标记
        self.owner.add_method_scheme(
            sc.Namespace({
                "static_invoke": sc.Bool(),
            })
        )
        self.owner.add_function_scheme(
            sc.Namespace({
                "static_invoke": sc.Bool(),
            })
        )

    def pre_generate(self):
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_top_template = self.owner.load_template(os.path.join(script_dir, "header_top.mako"))
        source_top_template = self.owner.load_template(os.path.join(script_dir, "source_top.mako"))

        # test generate body
        for record in self.owner.database.get_records():
            if record.generated_body_content and not record.has_generate_body_flag:
                self.owner.logger.error(f"Record {record.name} has generated body content but lost SKR_GENERATE_BODY()", [record.make_log_stack()])

        # gen header
        for header_db in self.owner.database.main_module.header_dbs:
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_top_template.render(header_db=header_db)
            )

        # gen source
        self.owner.append_content(
            "generated.cpp",
            source_top_template.render(module_db=self.owner.database.main_module)
        )

    def post_generate(self):
        script_dir = os.path.dirname(os.path.realpath(__file__).replace("\\", "/"))
        header_bottom_template = self.owner.load_template(os.path.join(script_dir, "header_bottom.mako"))
        source_bottom_template = self.owner.load_template(os.path.join(script_dir, "source_bottom.mako"))

        # gen header
        for header_db in self.owner.database.main_module.header_dbs:
            self.owner.append_content(
                header_db.relative_target_header_path,
                header_bottom_template.render(header_db=header_db)
            )

        # gen source
        self.owner.append_content(
            "generated.cpp",
            source_bottom_template.render(module_db=self.owner.database.main_module)
        )


def load_generators(generate_manager: gen.GenerateManager):
    generate_manager.add_generator("basic", BasicCPPGenerator())
