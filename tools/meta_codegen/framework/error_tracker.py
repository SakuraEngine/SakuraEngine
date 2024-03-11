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
        self.phase: str = ""
        self.source_file: str = ""
        self.source_line: int = 0
        self.path: List[str] = []
        self.error_data: List[ErrorData] = []

    def set_phase(self, phase: str) -> None:
        self.phase = phase

    def set_source(self, file: str, line: int) -> None:
        self.source_file = file
        self.source_line = line

    def push_path(self, name: str) -> None:
        self.path.append(name)

    def pop_path(self) -> None:
        self.path.pop()

    def path_guard(self, name: str) -> None:
        class __PathGuard:
            def __init__(self, tracker) -> None:
                self.tracker = tracker

            def __enter__(self):
                self.tracker.push_path(name)
                return self

            def __exit__(self, type, value, exc_tb):
                self.tracker.pop_path()
        return __PathGuard(self)

    def make_copy(self, with_message: bool = False) -> 'ErrorTracker':
        new_tracker = ErrorTracker()
        new_tracker.phase = self.phase
        new_tracker.source_file = self.source_file
        new_tracker.source_line = self.source_line
        new_tracker.path = self.path.copy()
        if with_message:
            new_tracker.error_data = self.error_data.copy()
        return new_tracker

    def merge(self, tracker: 'ErrorTracker') -> None:
        self.error_data.extend(tracker.error_data)

    def error(self, message: str) -> None:
        self.error_data.append(ErrorData(
            phase=self.phase,
            source_file=self.source_file,
            source_line=self.source_line,
            path=self.path.copy(),
            message=message,
            level=ErrorLevel.ERROR
        ))

    def warning(self, message: str) -> None:
        self.error_data.append(ErrorData(
            phase=self.phase,
            source_file=self.source_file,
            source_line=self.source_line,
            path=self.path.copy(),
            message=message,
            level=ErrorLevel.WARNING
        ))

    def any_error(self) -> bool:
        for error in self.error_data:
            if error.level == ErrorLevel.ERROR:
                return True
        return False

    def any_warning(self) -> bool:
        for error in self.error_data:
            if error.level == ErrorLevel.WARNING:
                return True
        return False

    def clear_message(self) -> None:
        self.error_data = []

    def reset(self) -> None:
        self.phase = ""
        self.source_file = ""
        self.source_line = 0
        self.path = []
        self.error_data = []

    def dump(self) -> None:
        print()
        for error in self.error_data:
            print(f"[{error.phase}] {error.source_file}:{error.source_line}")
            print(f"\033[35m{'>'.join(error.path)}\033[0m")
            if error.level == ErrorLevel.ERROR:
                print(f"\033[31merror: {error.message}\033[0m")
            elif error.level == ErrorLevel.WARNING:
                print(f"\033[33mwarning: {error.message}\033[0m")
            else:
                raise ValueError(f"Unknown error level: {error.level}")
            print()
