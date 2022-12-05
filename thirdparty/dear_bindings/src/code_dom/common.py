# Common stuff for the code DOM


class ParseContext:
    def __init__(self):
        self.current_content_parser = None
        self.last_element = None


class WriteContext:
    def __init__(self):
        self.for_implementation = False  # Are we outputting code for an implementation file (i.e. not a header)?
        self.use_original_names = False  # Do we want to use the original (unmodified C++) names for things?
        self.for_c = False  # Are we outputting C (as opposed to C++) code?
        self.known_structs = None  # List of known struct names, for applying struct tags when writing C
        self.include_leading_colons = False  # Do we want to include leading colons to fully-qualify all names?


# Collapse a list of tokens back into a C-style string, attempting to be reasonably intelligent and/or aesthetic
# about the use of whitespace
def collapse_tokens_to_string(tokens):
    result = ""
    need_space = False
    need_forced_space = False
    for token in tokens:
        token_is_punctuation = token.value in ['+', '-', '<', '>', '(', ')', '=', '/', '\\', '!', '~',
                                               '[', ']', '&', '"', "'", '%', '^', '*', ':', ';', '?',
                                               '!', ',', '.', '{', '}']
        if (need_space and not token_is_punctuation) or need_forced_space:
            result += " "
        result += token.value
        need_space = not token_is_punctuation
        # Special-case here - semicolon and comma do not get a space before them, but do get a space after them,
        # even if the next character is punctuation
        need_forced_space = token.value in [';', ',']
    return result


# Collapse a list of tokens back into a C-style string, assuming the tokens already have suitable whitespace
def collapse_tokens_to_string_with_whitespace(tokens):
    result = ""
    for token in tokens:
        result += token.value
    return result


# Write a C-style line with indentation, and any trailing whitespace removed
def write_c_line(file, indent, text):
    file.write("".ljust(indent * 4) + text.rstrip() + "\n")
