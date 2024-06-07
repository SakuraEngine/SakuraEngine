import typing as t
from dataclasses import dataclass, field
from enum import Enum
import framework
import framework.log as log
import inspect

# -------------------------- scheme --------------------------


class JsonType(Enum):
    BOOL = 0
    STR = 1
    INT = 2
    FLOAT = 3
    LIST = 4
    DICT = 5


@dataclass
class ParseResult:
    Value = str | int | float | bool | t.List['ParseResult.Value'] | t.Dict[str, 'ParseResult.Value']

    parsed_value: Value = None  # value, none means never appear or override by none
    override_stack: t.List['JsonObject'] = None  # override stack

    def is_visited(self) -> bool:
        return self.override_stack and len(self.override_stack) > 0

    def is_functional(self) -> bool:
        return isinstance(self.parsed_value, dict) and "enable" in self.parsed_value

    def get_child(self, key: str) -> 'ParseResult':
        if isinstance(self.parsed_value, dict):
            return self.parsed_value.get(key)
        return None

    def __getitem__(self, key: str) -> 'ParseResult':
        return self.get_child(key)

    def is_function_enable(self, enable_if_never_visited=False, enable_if_visited_child=False) -> bool:
        if not isinstance(self.parsed_value, dict) or "enable" not in self.parsed_value:
            raise ValueError("parsed value must be a functional")
        if not self.is_visited():
            return enable_if_never_visited
        elif self.parsed_value["enable"].is_visited():
            return self.parsed_value["enable"].parsed_value
        else:
            return enable_if_visited_child


class Scheme:
    def __init__(self) -> None:
        # owner & parent
        self.owner: 'framework.generator.GeneratorBase' = None
        self.parent: 'Scheme' = None

        # scheme data
        self.description: str = None
        self.enable_path_shorthand: bool = True
        self.supported_json_types: t.List[JsonType] = None
        self.is_leaf: bool = False

        # call location
        self.construct_file: str
        self.construct_line: int
        self.construct_function: str

        # get init call location
        frame = inspect.currentframe()
        found_frame = None
        while frame:
            if frame.f_code.co_name == "__init__":
                found_frame = frame
            frame = frame.f_back
        found_frame = found_frame.f_back

        # record call location
        if found_frame:
            self.construct_file = found_frame.f_code.co_filename
            self.construct_line = found_frame.f_lineno
            self.construct_function = found_frame.f_code.co_name
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
                            # copy shorthand object data
                            new_child.key = child.key
                            new_child.override_mark = child.override_mark
                            new_child.parent = child.parent

                            # indicate source
                            new_child.source = child
                            new_child.source_kind = JsonSourceKind.SHORTHAND

                            # replace child
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
        # mark self is recognized
        value.is_recognized = True

        # get json type
        if type(value.val) is bool:
            json_type = JsonType.BOOL
            json_type_name = "bool"
        elif type(value.val) is str:
            json_type = JsonType.STR
            json_type_name = "str"
        elif type(value.val) is int:
            json_type = JsonType.INT
            json_type_name = "int"
        elif type(value.val) is float:
            json_type = JsonType.FLOAT
            json_type_name = "float"
        elif type(value.val) is list and value.is_dict:
            json_type = JsonType.DICT
            json_type_name = "dict"
        else:
            json_type = JsonType.LIST
            json_type_name = "list"

        # check type
        if self.supported_json_types is None:
            raise Exception("must support at least one json type")
        if json_type not in self.supported_json_types:
            logger.error(f"value must be [{', '.join([str(item) for item in self.supported_json_types])}]", stack=self.make_log_stack(value))

        # dispatch check structure
        if value.is_dict and type(value.val) is list:
            for child in value.val:
                child_scheme = self.find_child(child.key)
                if child_scheme:
                    child_scheme.dispatch_check_structure(child, logger)

    def dispatch_solve_override(self, override_solve: 'JsonOverrideSolver', logger: log.Logger) -> ParseResult:
        # create result
        result = ParseResult()
        result.override_stack = override_solve.get_current_override_stack().copy()

        # parse value
        if self.is_leaf:
            result.parsed_value = override_solve.solve(logger)
        else:
            result.parsed_value = {}

            def __visitor(key: str, scheme: 'Scheme'):
                with override_solve.key_scope(key, logger):
                    result.parsed_value[key] = scheme.dispatch_solve_override(override_solve, logger)
            self.visit_children(__visitor)

        return result

    def merge_scheme(self, scheme: 'Scheme') -> 'Scheme':
        pass

    def visit_children(self, visitor) -> None:
        # @visitor: (key, Scheme) -> bool, return True to stop visit
        pass

    def find_child(self, key: str) -> 'Scheme':
        return None

    def expand_shorthand(self, shorthand_object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        return None


class Namespace(Scheme):
    # 命名空间，单纯的用来构建 json 的层级结构
    def __init__(self, options: t.Dict[str, 'Scheme'] = None) -> None:
        super().__init__()

        # namespace data
        self.options = options if options else {}

        # scheme config
        self.supported_json_types = [JsonType.DICT]

        # check options
        for scheme in self.options.values():
            if not isinstance(scheme, Scheme):
                raise ValueError("options must be Scheme object" + str(options))
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


class Functional(Scheme):
    # functional 规则，提供 enable 和简写服务
    def __init__(self, options: t.Dict[str, 'Scheme'] = None, shorthands: t.List['Shorthand'] = None):
        super().__init__()

        # functional data
        self.options = options if options else {}
        self.shorthands = shorthands if shorthands else []

        # scheme config
        self.supported_json_types = [JsonType.DICT]

        # check options
        for scheme in self.options.values():
            if not isinstance(scheme, Scheme):
                raise ValueError("options must be Scheme object")
            scheme.parent = self

        # check shorthand
        has_enable_shorthand = False
        for shorthand in self.shorthands:
            if not isinstance(shorthand, Shorthand):
                raise ValueError("shorthands must be Shorthand object")
            has_enable_shorthand = has_enable_shorthand or isinstance(shorthand, EnableShorthand)
            shorthand.owner = self

        # append enable option if not exist
        if "enable" not in self.options:
            enable_option = Bool()
            enable_option.parent = self
            self.options["enable"] = enable_option

        # append enable shorthand if not exist
        if not has_enable_shorthand:
            enable_shorthand = EnableShorthand()
            enable_shorthand.owner = self
            self.shorthands.append(enable_shorthand)

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


class LeafValue(Scheme):
    def __init__(self) -> None:
        super().__init__()

        # scheme config
        self.enable_path_shorthand = False
        self.is_leaf = True


class Bool(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.BOOL]


class Str(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.STR]


class Int(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.INT]


class Float(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.FLOAT]


class List(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.LIST]


class Dict(LeafValue):
    def __init__(self) -> None:
        super().__init__()
        self.supported_json_types = [JsonType.DICT]


# -------------------------- shorthand --------------------------
class Shorthand:
    def __init__(self) -> None:
        self.owner: Functional = None

    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        pass


class EnableShorthand(Shorthand):
    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        if type(object.val) is bool:
            return JsonObject.load_from_dict({
                "enable": object.val
            })


class OptionShorthand(Shorthand):
    def __init__(self, mappings: t.Dict[str, Dict]):
        super().__init__()
        self.mappings = mappings

    def expand(self, object: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        if type(object.val) is str:
            if object.val in self.mappings:
                return JsonObject.load_from_dict(self.mappings[object.val])
        elif not object.is_dict and type(object.val) is list:
            final_map = {}
            for shorthand_object in object.val:
                if type(shorthand_object.val) is str and shorthand_object.val in self.mappings:
                    final_map.update(self.mappings[shorthand_object.val])

            return JsonObject.load_from_dict(final_map) if final_map else None


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
        while True:
            # end key
            if cur_object.parent is None and not cur_object.source:
                if cur_object.key != "attrs":
                    raise ValueError(f"root object must have key 'attrs', but got '{cur_object.key}'")
                break

            # append path
            if cur_object.key:
                attr_stack.path.append(cur_object.key)

            # switch stack
            if cur_object.source:
                # end current stack
                result.append(attr_stack)

                # update state
                cur_source = cur_object.source
                source_kind = cur_object.source_kind
                cur_object = cur_object.source

                # start new stack
                if source_kind == JsonSourceKind.PATH:
                    attr_stack = log.AttrPathStack(val=attr_val)
                elif source_kind == JsonSourceKind.SHORTHAND:
                    attr_stack = log.AttrStack(val=cur_source.dump_json(with_key=False))
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
                if child.key in result:
                    raise ValueError(f"duplicate key '{child.key}' in dict")

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

    def solve_value_as_default_json(self) -> Value:
        if type(self.val) is list and self.is_dict:  # dict case
            result = {}
            for child in self.val:
                if child.key is None:
                    raise ValueError("empty key in dict")
                result[child.key] = child.solve_value_as_default_json()
            return result
        elif type(self.val) is list:  # list case
            result = []
            for child in self.val:
                result.append(child.solve_value_as_default_json())
            return result
        else:  # value case
            return self.val


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
                logger.error(f"override value without '!' mark", object.make_log_stack())
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
            else:  # normal case, update current value, type conflict will be checked in above phase
                # 走到 solve 的必定是叶子节点，当前的路径简写和 override 功能只提供给拥有 scheme 的部分
                cur_value = object.solve_value_as_default_json()
                cur_rewrite_by = object.rewrite_by

        return None if cur_rewrite_by else cur_value

    def get_current_override_stack(self) -> t.List[JsonObject]:
        return self.__node_stack[-1].objects if len(self.__node_stack) > 0 else [self.root_object]

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
