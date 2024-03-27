import typing as t
from dataclasses import dataclass
from enum import Enum
import framework.log as log

# -------------------------- schema --------------------------


@dataclass
class Scheme:
    description: str = None

    def dispatch_expand_shorthand_and_path(self, value: 'JsonObject', logger: log.Logger) -> None:
        if value.is_dict and type(value.val) is list:
            # expand shorthand

            # expand path
            if self.should_expand_path():
                def __recursive_make_path(cut, index, parent, source, scheme) -> 'JsonObject':
                    pass

            # dispatch expand shorthand
            for child in value.val:
                child_scheme = self.find_children(child.key)
                if child_scheme:
                    child_scheme.dispatch_expand_shorthand_and_path(child, logger)

    def dispatch_check_structure(self, value: 'JsonObject', logger: log.Logger) -> None:
        pass

    def should_expand_path(self) -> bool:
        return False

    def expand_shorthand(self, shorthand_value: 'JsonObject', logger: log.Logger) -> 'JsonObject':
        raise NotImplementedError("expand_shorthand() is not implemented")

    def check_structure(self, value: 'JsonObject', logger: log.Logger) -> None:
        raise NotImplementedError("check_structure() is not implemented")

    def visit_children(self, visitor) -> None:
        raise NotImplementedError("visit_children() is not implemented")

    def find_children(self, option_name: str) -> 'Scheme':
        raise NotImplementedError("find_children() is not implemented")


@dataclass
class Namespace(Scheme):
    # 命名空间，单纯的用来构建 json 的层级结构
    pass


@dataclass
class Functional(Scheme):
    # functional 规则，提供 enable 和简写服务
    pass


@dataclass
class LeafValue(Scheme):
    def visit_children(self, visitor) -> None:
        pass

    def find_children(self, option_name: str) -> Scheme:
        return None


@dataclass
class Bool(LeafValue):
    pass


@dataclass
class Str(LeafValue):
    pass


@dataclass
class Int(LeafValue):
    pass


@dataclass
class Float(LeafValue):
    pass


@dataclass
class List(LeafValue):
    pass


@dataclass
class Dict(LeafValue):
    pass


# -------------------------- shorthand --------------------------

@dataclass
class Shorthand:
    pass


@dataclass
class OptionShorthand(Shorthand):
    pass

# TODO. schema manager


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


class JsonSourceKind(Enum):
    SHORTHAND = 0
    PATH = 1


@dataclass
class JsonObject:
    # key & parent
    key: str = None
    override_mark: JsonOverrideMark = JsonOverrideMark.NONE  # 由 key 上的 override 标记提供
    parent: 'JsonObject' = None  # json 结构上的父节点

    # value
    val: str | int | float | bool | t.List['JsonObject'] = None
    is_dict: bool = False

    # expand shorthand and path phase
    source: 'JsonObject' = None  # 用于标记该节点的来源（由 shorthand/source 展开的节点）
    source_kind: 'JsonSourceKind' = None  # 来源类型

    # structure check phase
    is_recognized: bool = False  # 用于未知 attr 的检查

    # solve override phase
    passed_override_mark: JsonOverrideMark = JsonOverrideMark.NONE  # 在 solve override 时记录从父亲向下传播的 override mark
    ignored_by: 'JsonObject' = None  # 在 solve override 时记录同级的 ignore 标记

    def make_path(path_nodes: t.List[t.Tuple[str, JsonOverrideMark]], source: 'JsonObject') -> 'JsonObject':
        def __recursive_make_path(path_nodes, index, parent, source) -> 'JsonObject':
            (key, mark) = path_nodes[index]
            result = JsonObject(
                key=key,
                override_mark=mark,
                parent=parent,
                is_dict=True,
                source=source,
                source_kind=JsonSourceKind.PATH,
            )
            if index + 1 < len(path_nodes):
                result.val = [__recursive_make_path(path_nodes, index + 1, result, source)]
            else:
                result.val = source.val
            return result
        return __recursive_make_path(path_nodes, 0, source.parent, source)

    def expand_path(self) -> None:
        if not self.is_dict or type(self.val) is not list:
            raise Exception("expand_path() only works for dict")

        for index in range(len(self.val)):
            json_object = self.val[index]
            path_nodes = [parse_override(node) for node in json_object.key.split("::")]

            if len(path_nodes) > 1:
                self.val[index] = JsonObject.make_path(
                    path_nodes=path_nodes,
                    source=json_object)

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
            child = JsonObject(
                parent=result,
                source=source,
                source_kind=source_kind,
            )

            if type(v) is dict:
                child.val = JsonObject.load_from_dict(v, source=source, source_kind=source_kind)
            elif type(v) is list:
                child.val = JsonObject.load_from_list(v, source=source, source_kind=source_kind)
            else:
                child.val = v

            result.val.append(child)

        return result

    def load_from_dict(data: t.Dict, source: 'JsonObject' = None, source_kind: 'JsonSourceKind' = None) -> 'JsonObject':
        result = JsonObject(
            val=[],
            is_dict=True,
            source=source,
            source_kind=source_kind,
        )

        for (k, v) in data.items():
            key, mark = parse_override(k)
            child = JsonObject(
                key=key,
                override_mark=mark,
                parent=result,
                source=source,
                source_kind=source_kind,
            )

            if type(v) is dict:
                child.val = JsonObject.load_from_dict(v, source=source, source_kind=source_kind)
            elif type(v) is list:
                child.val = JsonObject.load_from_list(v, source=source, source_kind=source_kind)
            else:
                child.val = v

            result.val.append(child)

        return result

    def unique_dict(self) -> t.Dict:
        if type(self.val) is list and self.is_dict:
            result = {}
            for child in self.val:
                if child.key is None:
                    raise ValueError("empty key in dict")
                result[child.key] = child.val
            return result
        else:
            raise ValueError("unique_dict() only works for dict")


def json_hook(data: t.List[t.Tuple[str, object]]) -> JsonObject:
    result = JsonObject(val=[])

    for (k, v) in data:
        key, mark = parse_override(k)
        child = JsonObject(
            key=key,
            override_mark=mark,
            parent=result,
        )

        if type(v) is dict:
            child.val = JsonObject.load_from_dict(v)
        elif type(v) is list:
            child.val = JsonObject.load_from_list(v)
        else:
            child.val = v

        result.val.append(child)

    return result


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
