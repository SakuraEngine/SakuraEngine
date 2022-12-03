from src import code_dom
from src import utils
from src.code_dom.common import write_c_line


# This provides functionality to efficiently generate sets of preprocessor conditionals that match the state
# at a given element in the DOM
class ConditionalGenerator:
    def __init__(self):
        self.current_conditionals = []  # The current stack of preprocessor conditionals we have emitted

    # Write the conditionals necessary to bring us to the state needed by element
    def write_conditionals(self, element, file, indent=0):
        wanted_conditionals = utils.get_preprocessor_conditionals(element)

        # Remove the include guard from our list of conditionals
        for i in range(0, len(wanted_conditionals)):
            if wanted_conditionals[i].is_include_guard:
                del wanted_conditionals[i]
                break

        # Convert any cases where the element is in an else block into an inverted conditional
        for i in range(0, len(wanted_conditionals)):
            if utils.is_in_else_clause(element, wanted_conditionals[i]):
                wanted_conditionals[i] = wanted_conditionals[i].clone()
                wanted_conditionals[i].is_negated = not wanted_conditionals[i].is_negated

        # Close any unwanted conditionals
        first_endif = True
        while (len(self.current_conditionals) > len(wanted_conditionals)) or \
                ((len(self.current_conditionals) > 0) and
                 (not self.current_conditionals[len(self.current_conditionals) - 1].condition_matches(
                  wanted_conditionals[len(self.current_conditionals) - 1]))):
            if first_endif:
                file.write("\n")
                first_endif = False
            conditional = self.current_conditionals.pop(len(self.current_conditionals) - 1)
            write_c_line(file, indent, "#endif // " + conditional.get_opening_clause())

        # Add any new conditionals
        first_if = True
        while len(self.current_conditionals) < len(wanted_conditionals):
            conditional = wanted_conditionals[len(self.current_conditionals)]
            if first_if:
                file.write("\n")
                first_if = False
            write_c_line(file, indent, conditional.get_opening_clause())
            self.current_conditionals.append(conditional)

    # Close off any existing conditionals
    def finish_writing(self, file, indent=0):
        first_endif = True
        while len(self.current_conditionals) > 0:
            if first_endif:
                file.write("\n")
                first_endif = False
            conditional = self.current_conditionals.pop(len(self.current_conditionals) - 1)
            write_c_line(file, indent, "#endif // " + conditional.get_opening_clause())
        self.current_conditionals = []

