import typing as t
from dataclasses import dataclass, field
from enum import Enum
import framework
import framework.log as log

# -------------------------- scheme --------------------------


@dataclass
class Scheme:
    owner: 'framework.generator.GeneratorBase' = None
    parent: 'Scheme' = None
    description: str = None
    enable_path_shorthand: bool = True
    # enable_override: bool = True # TODO. override 解析也应当根据 scheme 来，这样，对 json 特性的扩展就可以完全由 scheme 控制

    def __post_init__(self):
        # TODO. 存储堆栈，用于解析时的错误提示
        pass

    def dispatch_expand_shorthand(self, value: 'JsonObject', logger: log.Logger) -> None:
        if value.is_dict and type(value.val) is list:
            # expand shorthand
            for index in range(len(value.val)):
                child = value.val[index]
                child_scheme = self.find_child(child.key)
                if child_scheme:
                    if child.is_dict and type(child.val) is list:  # dispatch case
                        child_scheme.dispatch_expand_shorthand(child, logger)
                    else:  # expand case
                        new_child = child_scheme.expand_shorthand(child, logger)
                        if new_child:
                            value.val[index] = new_child
                            # dispatch solve for new child part
                            child_scheme.dispatch_expand_shorthand(new_child, logger)
                            child_scheme.dispatch_check_structure(new_child, logger)

    def dispatch_expand_path(self, value: 'JsonObject', logger: log.Logger) -> None:
        if self.enable_path_shorthand and value.is_dict and type(value.val) is list:
            def __recursive_make_path(cur_path: str, cur_scheme: 'Scheme', parent: 'JsonObject', source: 'JsonObject') -> 'JsonObject':
                found_path_token = cur_path.find("::")
                result = JsonObject(
                    parent=parent,
                    is_dict=True,
                    source=source,
                    source_kind=JsonSourceKind.PATH
                )

                if found_path_token == -1:
                    (key, mark) = parse_override(cur_path)
                    result.key = key
                    result.override_mark = mark
                    result.val = source.val
                else:
                    (key, mark) = parse_override(cur_path[:found_path_token])
                    child_scheme = cur_scheme.find_child(key)

                    if child_scheme and child_scheme.enable_path_shorthand:
                        result.key = key
                        result.override_mark = mark
                        result.val = [__recursive_make_path(
                            cur_path=cur_path[found_path_token + 2:],
                            cur_scheme=cur_scheme,
                            parent=result,
                            source=source,
                        )]
                    else:
                        result.key = cur_path
                        result.override_mark = JsonOverrideMark.NONE
                        result.val = source.val
                return result

            # expand path for self
            for index in range(len(value.val)):
                child = value.val[index]
                if "::" in child.key:
                    # do expand
                    value.val[index] = __recursive_make_path(
                        cur_path=child.key,
                        cur_scheme=self,
                        parent=value,
                        source=value,
                    )

                    # dispatch solve for new child part
                    new_child = value.val[index]
                    child_scheme = self.find_child(new_child.key)
                    if child_scheme:
                        child_scheme.dispatch_expand_path(new_child, logger)
                        child_scheme.dispatch_check_structure(new_child, logger)

            # dispatch expand path
            for child in value.val:
                child_scheme = self.find_child(child.key)
                if child_scheme:
                    child_scheme.dispatch_expand_path(child, logger)

    def dispatch_check_structure(self, value: 'JsonObject', logger: log.Logger) -> None:
        # check structure for self
        self.check_structure(value, logger)

        # dispatch check structure
        if value.is_dict and type(value.val) is list:
            for child in value.val:
                child_scheme = self.find_child(child.key)
                if child_scheme:
                    child_scheme.dispatch_check_structure(child, logger)

    def dispatch_parse_to_object(self, override_solve: 'JsonOverrideSolver', logger: log.Logger) -> t.Any:
        # parse child dict
        child_dict = {}

        def __visitor(key: str, scheme: 'Scheme'):
            with override_solve.key_scope(key, logger):
                child_dict[key] = scheme.dispatch_parse_to_object(override_solve, logger)
        self.visit_children(__visitor)

        value = child_dict if child_dict else override_solve.solve(logger)
        return self.parse_to_object(value, logger)

    def merge_scheme(self, scheme: 'Scheme') -> 'Scheme':
        pass

    def visit_children(self, visitor) -> None:
        # @visitor: (key, Scheme) -> bool, return True to stop visit
        pass

    def find_child(self, key: str) -> 'Scheme':
        return None

    def expand_shorthand(self, shorthand_object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        return None

    def check_structure(self, object: 'JsonObject', logger: log.Logger) -> None:
        pass

    # TODO. 想想带 stack 追踪的 parse 怎么做，直接传入 override solve 是否是比较好的选择
    def parse_to_object(self, value: t.Any, logger: log.Logger) -> object:
        return value


@dataclass
class Namespace(Scheme):
    # 命名空间，单纯的用来构建 json 的层级结构
    options: t.Dict[str, 'Scheme'] = field(default_factory=lambda: {})

    def __post_init__(self):
        super().__post_init__()
        for scheme in self.options.values():
            if not isinstance(scheme, Scheme):
                raise ValueError("options must be Scheme object")
            scheme.parent = self

    def merge_scheme(self, scheme: 'Scheme') -> 'Scheme':
        if not isinstance(scheme, Namespace):
            raise ValueError("scheme must be Namespace object")
        for (key, child_scheme) in scheme.options.items():
            if key in self.options:  # if exist then merge
                self.options[key] = self.options[key].merge_scheme(child_scheme)
            else:  # else add
                self.options[key] = child_scheme
                child_scheme.parent = self
        return self

    def visit_children(self, visitor) -> None:
        for (key, scheme) in self.options.items():
            if visitor(key, scheme):
                break

    def find_child(self, key: str) -> Scheme:
        return self.options.get(key)

    def check_structure(self, object: 'JsonObject', logger: log.Logger) -> None:
        if not object.is_dict or type(object.val) is not list:
            with logger.stack_scope(object.make_attr_stack()):
                logger.error("value must be dict")

    def parse_to_object(self, value: 'JsonObject.Value', logger: log.Logger) -> object:
        if value and type(value) is not dict:
            raise ValueError("value must be dict")
        return super().parse_to_object(value, logger)


@dataclass
class Functional(Scheme):
    # functional 规则，提供 enable 和简写服务
    # TODO. enable shorthand 可以在 post init 中自动添加，并使用一个标记来控制这个行为
    options: t.Dict[str, 'Scheme'] = field(default_factory=lambda: {})
    shorthands: t.List['Shorthand'] = field(default_factory=lambda: [])

    def __post_init__(self):
        super().__post_init__()
        for scheme in self.options.values():
            if not isinstance(scheme, Scheme):
                raise ValueError("options must be Scheme object")
            scheme.parent = self

        has_enable_shorthand = False
        for shorthand in self.shorthands:
            if not isinstance(shorthand, Shorthand):
                raise ValueError("shorthands must be Shorthand object")
            has_enable_shorthand = has_enable_shorthand or isinstance(shorthand, EnableShorthand)
            shorthand.owner = self

        # append enable option if not exist
        if "enable" not in self.options:
            self.options["enable"] = Bool()
        if not has_enable_shorthand:
            self.shorthands.append(EnableShorthand(owner=self))

    def merge_scheme(self, scheme: Scheme) -> Scheme:
        if not isinstance(scheme, Functional):
            raise ValueError("scheme must be Functional object")

        # merge options
        for (key, child_scheme) in scheme.options.items():
            if key in self.options:  # if exist then merge
                self.options[key] = self.options[key].merge_scheme(child_scheme)
            else:  # else add
                self.options[key] = child_scheme
                child_scheme.parent = self

        # merge shorthands
        self.shorthands.extend(scheme.shorthands)

        return self

    def visit_children(self, visitor) -> None:
        for (key, scheme) in self.options.items():
            if visitor(key, scheme):
                break

    def expand_shorthand(self, shorthand_object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        for shorthand in self.shorthands:
            new_object = shorthand.expand(shorthand_object, logger)
            if new_object:
                return new_object

    def find_child(self, key: str) -> Scheme:
        return self.options.get(key)

    def check_structure(self, object: 'JsonObject', logger: log.Logger) -> None:
        if not object.is_dict or type(object.val) is not list:
            with logger.stack_scope(object.make_attr_stack()):
                logger.error("value must be dict")

    def parse_to_object(self, value: t.Any, logger: log.Logger) -> object:
        if value and type(value) is not dict:
            raise ValueError("value must be dict")
        return super().parse_to_object(value, logger)


@dataclass
class LeafValue(Scheme):
    enable_path_shorthand: bool = False
    default_value: t.Any = None

    def check_structure(self, object: 'JsonObject', logger: log.Logger) -> None:
        if type(object.val) is not self.get_value_type():
            with logger.stack_scope(object.make_attr_stack()):
                logger.error(f"value must be {self.get_value_name(self)}")

    def parse_to_object(self, value: t.Any, logger: log.Logger) -> object:
        if value and type(value) is not self.get_value_type():  # must be checked in check_structure phase
            raise ValueError(f"value must be {self.get_value_name(self)}")
        return super().parse_to_object(value, logger) if value else self.default_value

    def get_value_type(self) -> t.Type:
        raise NotImplementedError("get_value_type(self) must be implemented in subclass")

    def get_value_name(self) -> str:
        raise NotImplementedError("get_value_name(self) must be implemented in subclass")


@dataclass
class Bool(LeafValue):
    def get_value_type(self) -> t.Type:
        return bool

    def get_value_name(self) -> str:
        return "bool"


@dataclass
class Str(LeafValue):
    def get_value_type(self) -> t.Type:
        return str

    def get_value_name(self) -> str:
        return "str"


@dataclass
class Int(LeafValue):
    def get_value_type(self) -> t.Type:
        return int

    def get_value_name(self) -> str:
        return "int"


@dataclass
class Float(LeafValue):
    def get_value_type(self) -> t.Type:
        return float

    def get_value_name(self) -> str:
        return "float"


@dataclass
class List(LeafValue):
    def get_value_type(self) -> t.Type:
        return list

    def get_value_name(self) -> str:
        return "list"


@dataclass
class Dict(LeafValue):
    def get_value_type(self) -> t.Type:
        return dict

    def get_value_name(self) -> str:
        return "dict"


# -------------------------- shorthand --------------------------

@dataclass
class Shorthand:
    owner: Functional = None

    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        pass


@dataclass
class EnableShorthand(Shorthand):
    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        if type(object.val) is bool:
            return JsonObject.load_from_dict(
                {
                    "enable": object.val
                },
                key=object.key,
                source=object,
                source_kind=JsonSourceKind.SHORTHAND,
            )


@dataclass
class OptionShorthand(Shorthand):
    mappings: t.Dict[str, Dict] = field(default_factory=lambda: {})

    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        if type(object.val) is str:
            if object.val in self.mappings:
                return JsonObject.load_from_dict(
                    self.mappings[object.val],
                    key=object.key,
                    source=object,
                    source_kind=JsonSourceKind.SHORTHAND,
                )
        elif not object.is_dict and type(object.val) is list:
            final_map = {}
            for shorthand_object in object.val:
                if type(shorthand_object.val) is str and shorthand_object.val in self.mappings:
                    final_map.update(self.mappings[shorthand_object.val])
            return JsonObject.load_from_dict(
                final_map,
                key=object.key,
                source=object,
                source_kind=JsonSourceKind.SHORTHAND,
            ) if final_map else None


# -------------------------- json tool --------------------------
class JsonOverrideMark(Enum):
    NONE = 0  # no mark
    OVERRIDE = 1  # end with mark '!'
    REWRITE = 2  # end with mark '!!'
    APPEND = 3  # end with mark '+'


def parse_override(key: str) -> t.Tuple[str, JsonOverrideMark]:
    # path case
    if "::" in key:
        return (key, JsonOverrideMark.NONE)

    if key.endswith("!!"):
        return (key[:-2], JsonOverrideMark.REWRITE)
    elif key.endswith("!"):
        return (key[:-1], JsonOverrideMark.OVERRIDE)
    elif key.endswith("+"):
        return (key[:-1], JsonOverrideMark.APPEND)
    else:
        return (key, JsonOverrideMark.NONE)


def encode_override(key: str, mark: JsonOverrideMark) -> str:
    if mark == JsonOverrideMark.REWRITE:
        return f"{key}!!"
    elif mark == JsonOverrideMark.OVERRIDE:
        return f"{key}!"
    elif mark == JsonOverrideMark.APPEND:
        return f"{key}+"
    else:
        return key


def pass_override_mark(parent: JsonOverrideMark, child: JsonOverrideMark, logger: log.Logger) -> JsonOverrideMark:
    if parent == JsonOverrideMark.NONE:  # keep override mark
        return child
    if parent == JsonOverrideMark.OVERRIDE:
        if child == JsonOverrideMark.REWRITE:  # '!' => '!!, use '!!'
            return child
        elif child == JsonOverrideMark.APPEND:  # '!' => '+', ignore '+'
            logger.warning("append mark '+' under override mark '!' will be ignored!")
            return parent
        else:  # keep override mark
            return parent
    if parent == JsonOverrideMark.REWRITE:
        if child == JsonOverrideMark.OVERRIDE:  # '!!' => '!', ignore '!'
            logger.warning("override mark '!' under rewrite mark '!!' will be ignored!")
        elif child == JsonOverrideMark.APPEND:  # '!!' => '+', ignore '+'
            logger.warning("append mark '+' under rewrite mark '!!' will be ignored!")
        return parent
    if parent == JsonOverrideMark.APPEND:
        logger.error("append mark '+' only support for list")
        return parent


class JsonSourceKind(Enum):
    SHORTHAND = 0
    PATH = 1


@dataclass
class JsonObject:
    Value = str | int | float | bool | t.List['JsonObject']

    # key & parent
    key: str = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE  # 由 key 上的 override 标记提供
    parent: 'JsonObject' = None  # json 结构上的父节点

    # value
    val: Value = None
    is_dict: bool = False

    # expand shorthand and path phase
    source: 'JsonObject' = None  # 用于标记该节点的来源（由 shorthand/source 展开的节点，只有展开的根节点会有值）
    source_kind: 'JsonSourceKind' = None  # 来源类型（shorthand/path）

    # structure check phase
    is_recognized: bool = False  # 用于未知 attr 的检查，在 check_structure 阶段会被标记为 True，只有叶子节点是有意义的

    # solve override phase
    passed_override_mark: JsonOverrideMark = None  # 在 solve override 时记录同级的 override 标记
    rewrite_by: 'JsonObject' = None  # 在 solve override 时最近的 rewrite 来源（'!!' 标记）同时也会从父节点向子节点传播

    # def make_path(path_nodes: t.List[t.Tuple[str, JsonOverrideMark]], source: 'JsonObject') -> 'JsonObject':
    #     def __recursive_make_path(path_nodes, index, parent, source) -> 'JsonObject':
    #         (key, mark) = path_nodes[index]
    #         result = JsonObject(
    #             key=key,
    #             override_mark=mark,
    #             parent=parent,
    #             is_dict=True,
    #             source=source,
    #             source_kind=JsonSourceKind.PATH,
    #         )
    #         if index + 1 < len(path_nodes):
    #             result.val = [__recursive_make_path(path_nodes, index + 1, result, source)]
    #         else:
    #             result.val = source.val
    #         return result
    #     return __recursive_make_path(path_nodes, 0, source.parent, source)

    # def expand_path(self) -> None:
    #     if not self.is_dict or type(self.val) is not list:
    #         raise Exception("expand_path() only works for dict")

    #     for index in range(len(self.val)):
    #         json_object = self.val[index]
    #         path_nodes = [parse_override(node) for node in json_object.key.split("::")]

    #         if len(path_nodes) > 1:
    #             self.val[index] = JsonObject.make_path(
    #                 path_nodes=path_nodes,
    #                 source=json_object)

    # TODO. 展开为多个 stack 会更为合适
    # TODO. source 只存在于展开的 root 上做跳跃用途比较好
    def make_attr_stack(self) -> log.AttrStack:
        path: t.List[str] = []
        cur_object = self
        cur_source = self.source

        while cur_object.parent:
            if cur_object.parent.source == cur_source:  # same scope
                path.append(cur_object.key)
                cur_object = cur_object.parent
            elif cur_source:  # jump to source
                path.append(cur_object.key)
                # append source
                path.append(f"[{cur_object.source_kind}: {cur_source.key}]")
                cur_object = cur_source.parent
                cur_source = cur_source.parent.source
            else:
                raise Exception("source won't be None in the middle of path")

        path.reverse()

        stack = log.AttrStack(
            path=path,
            val=self.val
        )
        return stack

    def recursive_expand_path(self) -> None:
        if not self.is_dict or type(self.val) is not list:
            raise Exception("recursive_expand_path() only works for dict")

        self.expand_path()
        for child in self.val:
            if child.is_dict:
                child.recursive_expand_path()

    def load_from_list(data: t.List, source: 'JsonObject' = None, source_kind: 'JsonSourceKind' = None) -> 'JsonObject':
        result = JsonObject(
            val=[],
            source=source,
            source_kind=source_kind,
        )

        for v in data:
            if type(v) is dict:
                child = JsonObject.load_from_dict(v, source=source, source_kind=source_kind)
                child.parent = result
            elif type(v) is list:
                child = JsonObject.load_from_list(v, source=source, source_kind=source_kind)
                child.parent = result
            else:
                child = JsonObject(
                    parent=result,
                    val=v,
                    source=source,
                    source_kind=source_kind,
                )

            result.val.append(child)

        return result

    def load_from_dict(data: t.Dict, key=None, source: 'JsonObject' = None, source_kind: 'JsonSourceKind' = None) -> 'JsonObject':
        result = JsonObject(
            key=key,
            val=[],
            is_dict=True,
            source=source,
            source_kind=source_kind,
        )

        for (k, v) in data.items():
            key, mark = parse_override(k)

            if type(v) is dict:
                child = JsonObject.load_from_dict(v, source=source, source_kind=source_kind)
                child.key = key
                child.override_mark = mark
                child.parent = result
            elif type(v) is list:
                child.val = JsonObject.load_from_list(v, source=source, source_kind=source_kind)
                child.key = key
                child.override_mark = mark
                child.parent = result
            else:
                child = JsonObject(
                    key=key,
                    override_mark=mark,
                    parent=result,
                    val=v,
                    source=source,
                    source_kind=source_kind,
                )

            result.val.append(child)

        return result

    def unique_dict(self) -> t.Dict[str, 'JsonObject']:
        if type(self.val) is list and self.is_dict:
            result = {}
            for child in self.val:
                if child.key is None:
                    raise ValueError("empty key in dict")

                def __expand_child(child: 'JsonObject'):
                    if type(child.val) is list and child.is_dict:  # dict case
                        return child
                    elif type(child.val) is list:  # list case
                        return [__expand_child(c) for c in child.val]
                    else:
                        return child.val

                result[child.key] = __expand_child(child)
            return result
        else:
            raise ValueError("unique_dict() only works for dict")


def json_object_pairs_hook(data: t.List[t.Tuple[str, object]]) -> JsonObject:
    result = JsonObject(
        val=[],
        is_dict=True,
    )

    for (k, v) in data:
        key, mark = parse_override(k)
        if type(v) is JsonObject:  # child is dict
            v.key = key
            v.override_mark = mark
            v.parent = result
            child = v
        elif type(v) is list:  # child is list
            child = JsonObject.load_from_list(v)
            child.key = key
            child.override_mark = mark
            child.parent = result
        else:  # child is int/float/str/bool
            child = JsonObject(
                key=key,
                override_mark=mark,
                parent=result,
                val=v
            )

        result.val.append(child)

    return result


class JsonOverrideSolver:
    @dataclass
    class Node:
        key: str = None
        objects: t.List[JsonObject] = field(default_factory=lambda: [])

    def __init__(self, root_object: JsonObject) -> None:
        self.__node_stack: t.List[JsonOverrideSolver.Node] = []
        self.root_object: JsonObject = root_object

    def push_key(self, key: str, logger: log.Logger) -> None:
        # push node
        last_node = self.__node_stack[-1] if self.__node_stack else None
        new_node = JsonOverrideSolver.Node(key=key)
        self.__node_stack.append(new_node)

        # collect overrides
        parent_object_list = last_node.objects if last_node else [self.root_object]
        for parent_object in parent_object_list:
            if parent_object.is_dict and type(parent_object.val) is list:
                # each child to pickup override
                for child in parent_object.val:
                    if child.key == key:
                        child.passed_override_mark = pass_override_mark(
                            parent=parent_object.override_mark,
                            child=child.override_mark,
                            logger=logger
                        )
                        child.rewrite_by = parent_object.rewrite_by
                        new_node.objects.append(child)
            else:
                raise Exception("push_key() only works for dict")

        # solve rewrite
        cur_rewrite_source = None
        for object in reversed(new_node.objects):
            if object.passed_override_mark == JsonOverrideMark.REWRITE:
                cur_rewrite_source = object
            if not object.rewrite_by:
                object.rewrite_by = cur_rewrite_source

    def pop_key(self, key: str, logger: log.Logger) -> None:
        self.__node_stack.pop()

    def key_scope(self, key: str, logger: log.Logger):
        class _KeyGuard:
            def __init__(self, solver: JsonOverrideSolver, key: str, logger: log.Logger) -> None:
                self.solver = solver
                self.key = key
                self.logger = logger

            def __enter__(self):
                self.solver.push_key(self.key, self.logger)

            def __exit__(self, exc_type, exc_val, exc_tb):
                self.solver.pop_key(self.key, self.logger)
        return _KeyGuard(self, key, logger)

    def solve(self, logger: log.Logger) -> None:
        cur_value = None
        cur_rewrite_by = None
        for object in self.__node_stack[-1].objects:
            if object.passed_override_mark == JsonOverrideMark.NONE and cur_value:  # illegal override
                with logger.stack_scope(object.make_attr_stack()):
                    logger.error(f"override value without '!' mark")
            elif object.passed_override_mark == JsonOverrideMark.APPEND:  # append
                merge_source = None if cur_rewrite_by else cur_value

                # value type must be checked in above phase
                if type(cur_value) is not list or type(object.val) is not list:
                    raise Exception("append mark '+' only support for list")

                # merge list
                merge_source.extend(object.val)

                # update mark
                cur_value = merge_source
                cur_rewrite_by = object.rewrite_by
            else:  # normal case, update current value
                if object.is_dict and type(object.val) is list:  # dict case
                    # TODO. 递归解析 dict
                    cur_value = {}
                    for child in object.val:
                        cur_value[child.key] = child.val
                elif not object.is_dict and type(object.val) is list:  # list case
                    # TODO. 递归解析 list
                    cur_value = []
                    for child in object.val:
                        cur_value.append(child.val)
                else:
                    cur_value = object.val
                cur_rewrite_by = object.rewrite_by

        return None if cur_rewrite_by else cur_value


# -------------------------- dict <=> obj tool --------------------------


class ObjDictTools:
    @staticmethod
    def as_obj(dict: Dict) -> object:
        result = ObjDictTools()
        result.__dict__ = dict
        return result

    @staticmethod
    def as_dict(obj: object) -> Dict:
        return obj.__dict__

    @staticmethod
    def to_obj(dict: Dict) -> object:
        result = ObjDictTools()
        result.__dict__.update(dict)
        return result

    @staticmethod
    def to_dict(obj: object) -> Dict:
        return obj.__dict__.copy()
