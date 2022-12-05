from . import common
from . import element
from . import blanklines
from . import classstructunion
from . import codeblock
from . import comment
from . import define
from . import enumelement
from . import enum
from . import error
from . import externc
from . import fielddeclaration
from . import functionargument
from . import functiondeclaration
from . import functionpointertype
from . import headerfile
from . import headerfileset
from . import include
from . import namespace
from . import pragma
from . import preprocessorif
from . import template
from . import type
from . import typedef
from . import undef
from . import unparsablething

__all__ = ["blanklines", "classstructunion", "codeblock", "comment", "define", "element",
           "enumelement", "error", "externc", "fielddeclaration", "functionargument", "functiondeclaration",
           "functionpointertype", "headerfile", "headerfileset", "include", "namespace", "pragma",
           "preprocessorif", "template", "type", "typedef", "undef", "unparsablething"]

# Set up aliases to avoid having to refer to things inside the module by verbose names
# There's probably a better way to do this but most of the things I've tried end up causing
# circular reference problems inside the code_dom module

DOMBlankLines = blanklines.DOMBlankLines
DOMClassStructUnion = classstructunion.DOMClassStructUnion
DOMCodeBlock = codeblock.DOMCodeBlock
DOMComment = comment.DOMComment
DOMDefine = define.DOMDefine
DOMElement = element.DOMElement
DOMEnum = enum.DOMEnum
DOMEnumElement = enumelement.DOMEnumElement
DOMError = error.DOMError
DOMExternC = externc.DOMExternC
DOMFieldDeclaration = fielddeclaration.DOMFieldDeclaration
DOMFunctionArgument = functionargument.DOMFunctionArgument
DOMFunctionDeclaration = functiondeclaration.DOMFunctionDeclaration
DOMFunctionPointerType = functionpointertype.DOMFunctionPointerType
DOMHeaderFile = headerfile.DOMHeaderFile
DOMHeaderFileSet = headerfileset.DOMHeaderFileSet
DOMInclude = include.DOMInclude
DOMNamespace = namespace.DOMNamespace
DOMPragma = pragma.DOMPragma
DOMPreprocessorIf = preprocessorif.DOMPreprocessorIf
DOMTemplate = template.DOMTemplate
DOMType = type.DOMType
DOMTypedef = typedef.DOMTypedef
DOMUndef = undef.DOMUndef
DOMUnparsableThing = unparsablething.DOMUnparsableThing

ParseContext = common.ParseContext
WriteContext = common.WriteContext
