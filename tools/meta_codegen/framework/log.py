from typing import List, Dict
from dataclasses import dataclass, field
from enum import Enum


class LogHelper:
    def red(raw_msg: str) -> str:
        return f"\033[31m{raw_msg}\033[0m"

    def green(raw_msg: str) -> str:
        return f"\033[32m{raw_msg}\033[0m"

    def yellow(raw_msg: str) -> str:
        return f"\033[33m{raw_msg}\033[0m"

    def blue(raw_msg: str) -> str:
        return f"\033[34m{raw_msg}\033[0m"

    def cyan(raw_msg: str) -> str:
        return f"\033[36m{raw_msg}\033[0m"

    def magenta(raw_msg: str) -> str:
        return f"\033[35m{raw_msg}\033[0m"


class LogLevel:
    ERROR = 0
    WARNING = 1
    VERBOSE = 2


@dataclass
class LogData:
    level: LogLevel
    message: str
    stack: List['LogStack']


class Logger:
    def __init__(self) -> None:
        # env
        self.__stack: List['LogStack'] = []

        # data
        self.__log_data: List[LogData] = []

        # raise for debug
        self.__raise_error: bool = False
        self.__raise_warning: bool = False

    def push_stack(self, stack: 'LogStack') -> None:
        self.__stack.append(stack)

    def pop_stack(self) -> None:
        self.__stack.pop()

    def stack_guard(self, stack: 'LogStack') -> None:
        class __StackGuard:
            def __init__(self, logger: Logger, stack: 'LogStack') -> None:
                self.__logger = logger
                self.__stack = stack

            def __enter__(self) -> None:
                self.__logger.push_stack(self.__stack)

            def __exit__(self, exc_type, exc_value, traceback) -> None:
                self.__logger.pop_stack()

        return __StackGuard(self, stack)

    def error(self, message: str, stack: 'LogStack' = None) -> None:
        if self.__raise_error:
            raise Exception(f"error: {message}")
        if stack:
            self.push_stack(stack)

        self.__log_data.append(LogData(
            level=LogLevel.ERROR,
            message=message,
            stack=self.__stack.copy()
        ))

        if stack:
            self.pop_stack

    def warning(self, message: str, stack: 'LogStack' = None) -> None:
        if self.__raise_warning:
            raise Warning(f"warning: {message}")
        if stack:
            self.push_stack(stack)

        self.__log_data.append(LogData(
            level=LogLevel.WARNING,
            message=message,
            stack=self.__stack.copy()
        ))

        if stack:
            self.pop_stack

    def verbose(self, message: str, stack: 'LogStack' = None) -> None:
        if stack:
            self.push_stack(stack)

        self.__log_data.append(LogData(
            level=LogLevel.VERBOSE,
            message=message,
            stack=self.__stack.copy()
        ))

        if stack:
            self.pop_stack

    def clear(self) -> None:
        self.__log_data.clear()

    def dump(self):
        # empty line
        print()

        for log in self.__log_data:
            # print message
            if log.level == LogLevel.ERROR:
                print(LogHelper.red(log.message))
            elif log.level == LogLevel.WARNING:
                print(LogHelper.yellow(log.message))
            elif log.level == LogLevel.VERBOSE:
                print(LogHelper.blue(log.message))

            # print stack
            for stack in reversed(log.stack):
                stack.print()


class LogStack:
    def print(self) -> None:
        pass


@dataclass
class CppSourceStack(LogStack):
    file: str
    line: int

    def print(self) -> None:
        print(f"in cpp source {self.file}: {self.line}")


@dataclass
class AttrStack(LogStack):
    path: List[str]
    val: object

    def print(self) -> None:
        if self.val:
            print(f"in attribute {' > '.join(self.path)}: {self.val}")
        else:
            print(f"in attribute {' > '.join(self.path)}")


@dataclass
class PythonSourceStack(LogStack):
    file: str
    line: int

    def print(self) -> None:
        print(f"in python source {self.file}: {self.line}")
