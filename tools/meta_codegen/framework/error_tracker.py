from typing import List, Dict
from dataclasses import dataclass, field
from enum import Enum


@dataclass
class ErrorData:
    phase: str = ""
    source_file: str = ""
    source_line: int = 0
    path: List[str] = field(default_factory=lambda: [])
    message: str = ""


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

    def error(self, message: str) -> None:
        self.error_data.append(ErrorData(
            phase=self.phase,
            source_file=self.source_file,
            source_line=self.source_line,
            path=self.path.copy(),
            message=message
        ))

    def any_error(self) -> bool:
        return len(self.error_data) > 0

    def reset(self) -> None:
        self.phase = ""
        self.source_file = ""
        self.source_line = 0
        self.path = []
        self.error_data = []

    def print_errors(self) -> None:
        for error in self.error_data:
            print(f"[{error.phase}] {error.source_file}:{error.source_line}")
            print(f"\033[35m{'>'.join(error.path)}\033[0m")
            print(f"\033[31m{error.message}\033[0m")
            print()

    def print_as_warning(self) -> None:
        for error in self.error_data:
            print(f"[{error.phase}] {error.source_file}:{error.source_line}")
            print(f"\033[35m{'>'.join(error.path)}\033[0m")
            print(f"\033[33m{error.message}\033[0m")
            print()
