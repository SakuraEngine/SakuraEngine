import typing as t
from dataclasses import dataclass, field
from enum import Enum
import framework
import framework.log as log
import inspect

# -------------------------- scheme --------------------------
# TODO. 使用 json type 来进行 structure check
# TODO. Namespace 新类别 MutexNamespace 用于择取 Namespace 下某个特定功能子集，这些子集不能同时出现
#       或者可以在外部的 Check Attribute 阶段进行检查，好处是 Scheme 会变得简单
# TODO. functional 的 enable 字段为自动增加，同时持有自动的填充规则（visit 即为 enable，以及默认值）


class JsonType(Enum):
    BOOL = 0
    STR = 1
    INT = 2
    FLOAT = 3
    LIST = 4
    DICT = 5


@dataclass
class Scheme:
    owner: 'framework.generator.GeneratorBase' = None
    parent: 'Scheme' = None
    description: str = None
    enable_path_shorthand: bool = True
    # enable_override: bool = True # TODO. override 解析也应当根据 scheme 来，这样，对 json 特性的扩展就可以完全由 scheme 控制
    supported_json_types: t.List[JsonType] = None

    def __post_init__(self):
        # get init call location
        frame = inspect.currentframe().f_back
        while frame:
            if frame.f_code.co_name == "__init__":
                break
            frame = frame.f_back
        frame = frame.f_back

        # record call location
        if frame:
            self.construct_file = frame.f_code.co_filename
            self.construct_line = frame.f_lineno
            self.construct_function = frame.f_code.co_name
        else:
            self.construct_file = 'Unknown'
            self.construct_line = 0
            self.construct_function = 'Unknown'

    def make_log_stack(self, json_object: 'JsonObject') -> t.List[log.LogStack]:
        result = []

        # make call stack
        frame = inspect.currentframe().f_back
        result.append(log.PythonSourceStack(file=frame.f_code.co_filename, line=frame.f_lineno))

        # make scheme stack
        result.append(log.SchemeStack(self.construct_file, self.construct_line))

        # make attr stack
        result.extend(json_object.make_log_stack(end_key="attrs"))

        return result

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
                            child_scheme.dispatch_expand_path(new_child, logger)

    def dispatch_expand_path(self, value: 'JsonObject', logger: log.Logger) -> None:
        if self.enable_path_shorthand and value.is_dict and type(value.val) is list:
            def __recursive_make_path(cur_path: str, cur_scheme: 'Scheme', parent: 'JsonObject', source_val) -> 'JsonObject':
                found_path_token = cur_path.find("::")
                result = JsonObject(
                    parent=parent,
                    is_dict=True,
                )

                if found_path_token == -1:
                    (key, mark) = parse_override(cur_path)
                    result.key = key
                    result.override_mark = mark
                    result.val = source_val
                else:
                    (key, mark) = parse_override(cur_path[:found_path_token])
                    child_scheme = cur_scheme.find_child(key)

                    if child_scheme and child_scheme.enable_path_shorthand:
                        result.key = key
                        result.override_mark = mark
                        result.val = [__recursive_make_path(
                            cur_path=cur_path[found_path_token + 2:],
                            cur_scheme=child_scheme,
                            parent=result,
                            source_val=source_val,
                        )]
                    else:
                        result.key = cur_path
                        result.override_mark = JsonOverrideMark.NONE
                        result.val = source_val
                return result

            # expand path for self
            for index in range(len(value.val)):
                child = value.val[index]
                if "::" in child.key:
                    # do expand
                    new_child = __recursive_make_path(
                        cur_path=child.key,
                        cur_scheme=self,
                        parent=value,
                        source_val=child.val,
                    )
                    new_child.source = child
                    new_child.source_kind = JsonSourceKind.PATH
                    value.val[index] = new_child

                    # dispatch solve for new child part
                    new_child = value.val[index]
                    child_scheme = self.find_child(new_child.key)
                    if child_scheme:
                        child_scheme.dispatch_expand_shorthand(new_child, logger)
                        child_scheme.dispatch_expand_path(new_child, logger)

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

    # TODO. check structure 与 parse_to_object 不应该被继承，scheme 最好只作为单独的 Json -> Object 转译器
    def check_structure(self, object: 'JsonObject', logger: log.Logger) -> None:
        object.is_recognized = True

        # get json type
        if type(object.val) is bool:
            json_type = JsonType.BOOL
            json_type_name = "bool"
        elif type(object.val) is str:
            json_type = JsonType.STR
            json_type_name = "str"
        elif type(object.val) is int:
            json_type = JsonType.INT
            json_type_name = "int"
        elif type(object.val) is float:
            json_type = JsonType.FLOAT
            json_type_name = "float"
        elif type(object.val) is list and object.is_dict:
            json_type = JsonType.DICT
            json_type_name = "dict"
        else:
            json_type = JsonType.LIST
            json_type_name = "list"

        # check type
        if self.supported_json_types is None:
            raise Exception("must support at least one json type")
        if json_type not in self.supported_json_types:
            logger.error(f"value must be {json_type_name}", stack=self.make_log_stack(object))

    def parse_to_object(self, value: t.Any, logger: log.Logger) -> object:
        return value


@dataclass
class Namespace(Scheme):
    # 命名空间，单纯的用来构建 json 的层级结构
    options: t.Dict[str, 'Scheme'] = field(default_factory=lambda: {})
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.DICT])

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


# TODO. functional 的 default enable 细化:
#   1. 存在内部赋值时候的情况
#   2. 默认的情况
@dataclass
class Functional(Scheme):
    # functional 规则，提供 enable 和简写服务
    # TODO. enable shorthand 可以在 post init 中自动添加，并使用一个标记来控制这个行为
    options: t.Dict[str, 'Scheme'] = field(default_factory=lambda: {})
    shorthands: t.List['Shorthand'] = field(default_factory=lambda: [])
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.DICT])

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


@dataclass
class LeafValue(Scheme):
    enable_path_shorthand: bool = False
    default_value: t.Any = None


@dataclass
class Bool(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.BOOL])


@dataclass
class Str(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.STR])


@dataclass
class Int(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.INT])


@dataclass
class Float(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.FLOAT])


@dataclass
class List(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.LIST])


@dataclass
class Dict(LeafValue):
    supported_json_types: t.List[JsonType] = field(default_factory=lambda: [JsonType.DICT])


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
            result = JsonObject.load_from_dict(
                {
                    "enable": object.val
                },
                key=object.key,
            )
            result.source = object
            result.source_kind = JsonSourceKind.SHORTHAND
            return result


@dataclass
class OptionShorthand(Shorthand):
    mappings: t.Dict[str, Dict] = field(default_factory=lambda: {})

    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        if type(object.val) is str:
            if object.val in self.mappings:
                result = JsonObject.load_from_dict(
                    self.mappings[object.val],
                    key=object.key,
                )
                result.source = object
                result.source_kind = JsonSourceKind.SHORTHAND
                return result
        elif not object.is_dict and type(object.val) is list:
            final_map = {}
            for shorthand_object in object.val:
                if type(shorthand_object.val) is str and shorthand_object.val in self.mappings:
                    final_map.update(self.mappings[shorthand_object.val])
            if final_map:
                result = JsonObject.load_from_dict(
                    final_map,
                    key=object.key,
                )
                result.source = object
                result.source_kind = JsonSourceKind.SHORTHAND
                return result


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

    def escape_from_parent(self):
        if self.parent:
            self.parent.val.remove(self)
            self.parent = None

    def dump_json(self, with_key=True) -> str:
        result = f'"{self.key}": ' if self.key and with_key else ""

        if self.is_dict and type(self.val) is list:
            result += f"{{{', '.join([child.dump_json() for child in self.val])}}}"
        elif type(self.val) is list:
            result += f"[{', '.join([child.dump_json() for child in self.val])}]"
        elif type(self.val) is str:
            result += f'"{self.val}"'
        else:
            result += f"{self.val}"

        return result

    def make_log_stack(self, end_key: str = None) -> t.List[log.LogStack]:
        result = []

        # make attr stack
        attr_val = self.dump_json(with_key=False)
        attr_stack = log.AttrStack(val=attr_val)
        cur_object = self
        while cur_object.parent:
            # end key
            if end_key and cur_object.key == end_key:
                break

            # append path
            if cur_object.key:
                attr_stack.path.append(cur_object.key)

            # switch stack
            if cur_object.source:
                result.append(attr_stack)
                source_kind = cur_object.source_kind
                cur_object = cur_object.source
                if source_kind == JsonSourceKind.PATH:
                    attr_stack = log.AttrPathStack(val=attr_val)
                elif source_kind == JsonSourceKind.SHORTHAND:
                    attr_stack = log.AttrStack(val=cur_object.source.dump_json(with_key=False))
            else:
                cur_object = cur_object.parent
        result.append(attr_stack)

        result.reverse()
        return result

    def check_unrecognized_attrs(self, logger: log.Logger) -> None:
        if not self.is_recognized and self.key != "attrs":
            logger.warning(f"unrecognized value", stack=self.make_log_stack(end_key="attrs"))
        elif self.is_dict and type(self.val) is list:
            for child in self.val:
                child.check_unrecognized_attrs(logger)

    def load_from_list(data: t.List) -> 'JsonObject':
        result = JsonObject(
            val=[],
        )

        for v in data:
            if type(v) is JsonObject:
                child = v
                child.parent = result
            elif type(v) is list:
                child = JsonObject.load_from_list(v)
                child.parent = result
            else:
                child = JsonObject(
                    parent=result,
                    val=v,
                )

            result.val.append(child)

        return result

    def load_from_dict(data: t.Dict, key=None) -> 'JsonObject':
        result = JsonObject(
            key=key,
            val=[],
            is_dict=True,
        )

        for (k, v) in data.items():
            key, mark = parse_override(k)

            if type(v) is dict:
                child = JsonObject.load_from_dict(v)
                child.key = key
                child.override_mark = mark
                child.parent = result
            elif type(v) is list:
                child = JsonObject.load_from_list(v)
                child.key = key
                child.override_mark = mark
                child.parent = result
            else:
                child = JsonObject(
                    key=key,
                    override_mark=mark,
                    parent=result,
                    val=v,
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
