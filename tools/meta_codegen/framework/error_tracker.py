from typing import List, Dict
from dataclasses import dataclass, field
from enum import Enum


class ErrorLevel(Enum):
    ERROR = 0
    WARNING = 1


@dataclass
class ErrorData:
    phase: str = ""
    source_file: str = ""
    source_line: int = 0
    path: List[str] = field(default_factory=lambda: [])
    message: str = ""
    level: ErrorLevel = ErrorLevel.ERROR


class ErrorTracker:
    def __init__(self) -> None:
        self.__phase: str = ""
        self.__source_file: str = ""
        self.__source_line: int = 0
        self.__path: List[str] = []
        self.__error_data: List[ErrorData] = []
        self.__error_check_scope_stack: List[int] = []
        self.__raise_error: bool = False
        self.__raise_warning: bool = False

    def set_raise_error(self, raise_error: bool) -> None:
        self.__raise_error = raise_error

    def set_raise_warning(self, raise_warning: bool) -> None:
        self.__raise_warning = raise_warning

    def set_phase(self, phase: str) -> None:
        self.__phase = phase

    def set_source(self, file: str, line: int) -> None:
        self.__source_file = file
        self.__source_line = line

    def set_source_file(self, file: str) -> None:
        self.__source_file = file

    def set_source_line(self, line: int) -> None:
        self.__source_line = line

    def path_push(self, name: str) -> None:
        if name is None:
            raise ValueError("name should not be None")
        self.__path.append(name)

    def path_pop(self) -> None:
        self.__path.pop()

    def path_guard(self, name: str) -> None:
        class __PathGuard:
            def __init__(self, tracker) -> None:
                self.tracker = tracker

            def __enter__(self):
                self.tracker.path_push(name)
                return self

            def __exit__(self, type, value, exc_tb):
                self.tracker.path_pop()
        return __PathGuard(self)

    def error_scope_push(self) -> None:
        self.__error_check_scope_stack.append(len(self.__error_data))

    def any_error_in_scope(self) -> bool:
        index = self.__error_check_scope_stack.pop()
        while len(self.__error_data) > index:
            if self.__error_data[-1].level == ErrorLevel.ERROR:
                return True
        return False

    def any_warning_in_scope(self) -> bool:
        index = self.__error_check_scope_stack.pop()
        while len(self.__error_data) > index:
            if self.__error_data[-1].level == ErrorLevel.WARNING:
                return True
        return False

    def error_scope_pop(self) -> None:
        self.__error_check_scope_stack.pop()

    def error_scope_guard(self) -> None:
        class __ErrorScopeGuard:
            def __init__(self, tracker) -> None:
                self.tracker = tracker

            def __enter__(self):
                self.tracker.error_scope_push()
                return self

            def __exit__(self, type, value, exc_tb):
                self.tracker.error_scope_pop()
        return __ErrorScopeGuard(self)

    def make_copy(self, with_message: bool = False) -> 'ErrorTracker':
        new_tracker = ErrorTracker()
        new_tracker.__phase = self.__phase
        new_tracker.__source_file = self.__source_file
        new_tracker.__source_line = self.__source_line
        new_tracker.__path = self.__path.copy()
        if with_message:
            new_tracker.__error_data = self.__error_data.copy()
        return new_tracker

    def merge(self, tracker: 'ErrorTracker') -> None:
        self.__error_data.extend(tracker.__error_data)

    def error(self, message: str) -> None:
        if self.__raise_error:
            raise Exception(f"error: {message}")
        self.__error_data.append(ErrorData(
            phase=self.__phase,
            source_file=self.__source_file,
            source_line=self.__source_line,
            path=self.__path.copy(),
            message=message,
            level=ErrorLevel.ERROR
        ))

    def warning(self, message: str) -> None:
        if self.__raise_warning:
            raise Warning(f"warning: {message}")
        self.__error_data.append(ErrorData(
            phase=self.__phase,
            source_file=self.__source_file,
            source_line=self.__source_line,
            path=self.__path.copy(),
            message=message,
            level=ErrorLevel.WARNING
        ))

    def any_error(self) -> bool:
        for error in self.__error_data:
            if error.level == ErrorLevel.ERROR:
                return True
        return False

    def any_warning(self) -> bool:
        for error in self.__error_data:
            if error.level == ErrorLevel.WARNING:
                return True
        return False

    def clear_message(self) -> None:
        self.__error_data = []

    def reset(self) -> None:
        self.__phase = ""
        self.__source_file = ""
        self.__source_line = 0
        self.__path = []
        self.__error_data = []

    def dump(self) -> None:
        print()
        for error in self.__error_data:
            print(f"[{error.phase}] {error.source_file}:{error.source_line}")
            print(f"\033[35m{' > '.join(error.path)}\033[0m")
            if error.level == ErrorLevel.ERROR:
                print(f"\033[31merror: {error.message}\033[0m")
            elif error.level == ErrorLevel.WARNING:
                print(f"\033[33mwarning: {error.message}\033[0m")
            else:
                raise ValueError(f"Unknown error level: {error.level}")
            print()


class ErrorStack:
    def print() -> str:
        pass


class CPPSourceStack:
    def print() -> str:
        pass


class AttrPathStack:
    def print() -> str:
        pass


class PythonSourceStack:
    def print() -> str:
        pass
