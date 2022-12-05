# This encapsulates a stream of lexed tokens in a manner that allows tokens to be returned to the stream if unparsed
class TokenStream:
    def __init__(self, lexer):
        self.lexer = lexer
        self.token_history = []
        self.buffered_tokens = []
        self.current_token_index = 0
        self.max_history_length = 1024  # Maximum number of tokens that can be rewound

    # Fetch the next token in the stream, returns None if the stream is finished
    # Optionally skips newline/whitespace tokens
    def get_token(self, skip_newlines=True, skip_whitespace=True):
        if len(self.buffered_tokens) > 0:
            token = self.buffered_tokens.pop()
        else:
            token = self.lexer.token()

        if token is not None:
            self.token_history.append(token)
            while len(self.token_history) > self.max_history_length:
                self.token_history.pop(0)
            self.current_token_index += 1

            if (skip_newlines and (token.type == 'NEWLINE')) or (skip_whitespace and (token.type == 'WHITESPACE')):
                return self.get_token(skip_newlines, skip_whitespace)

        return token

    # Fetch the next token without removing it from the stream
    # Optionally skips newline/whitespace tokens (in which case it will scan forward to the next suitable token
    # and peek that)
    def peek_token(self, skip_newlines=True, skip_whitespace=True):
        checkpoint = self.get_checkpoint()
        token = self.get_token(skip_newlines, skip_whitespace)
        self.rewind(checkpoint)
        return token

    # Fetch the next token in the stream, failing (returning None) if it is not of one of the types specified
    # Optionally skips newline/whitespace tokens
    def get_token_of_type(self, acceptable_types, skip_newlines=True, skip_whitespace=True):
        checkpoint = self.get_checkpoint()
        token = self.get_token(skip_newlines, skip_whitespace)
        if token is None:
            return None
        if token.type not in acceptable_types:
            self.rewind(checkpoint)
            return None
        return token

    # Fetch the next token in the stream without removing it from the stream, failing (returning None)
    # if it is not of one of the types specified
    # Optionally skips newline/whitespace tokens
    def peek_token_of_type(self, acceptable_types, skip_newlines=True, skip_whitespace=True):
        token = self.peek_token(skip_newlines, skip_whitespace)
        if token is None:
            return None
        if token.type not in acceptable_types:
            return None
        return token

    # Rewind the stream by one token
    # If skip_newlines is true, will rewind by one /non-newline/ token (and the same for skip_whitespace)
    def rewind_one_token(self, skip_newlines=True, skip_whitespace=True):
        if len(self.token_history) < 1:
            raise Exception("Cannot rewind as no tokens in history! (rewound more than max_history_length?)")

        token = self.token_history.pop()
        self.buffered_tokens.append(token)
        self.current_token_index -= 1
        if (skip_newlines and (token.type == 'NEWLINE')) or (skip_whitespace and (token.type == 'WHITESPACE')):
            self.rewind_one_token(skip_newlines, skip_whitespace)

    # Get a checkpoint in the stream that can later be returned to
    def get_checkpoint(self):
        return self.current_token_index

    # Rewind the stream to a checkpoint
    def rewind(self, checkpoint):
        if self.current_token_index < checkpoint:
            raise Exception("Cannot rewind to a point further in the stream")
        while self.current_token_index > checkpoint:
            self.rewind_one_token(skip_newlines=False, skip_whitespace=False)
        if self.current_token_index != checkpoint:
            raise Exception("Rewind to checkpoint failed")
