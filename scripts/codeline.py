class CodeLine:
    def __init__(self, line):
        self.line = line
        self.unread_line = line

    def read_symbol(self):
        if not self.unread_line:
            return None

        symbol = ''
        for ch in self.unread_line:
            if ch.isalpha():
                symbol += str(ch)
            else:
                if not symbol:
                    symbol += str(ch)
                    break
                elif ch.isalnum():
                    symbol += str(ch)
                else:
                    break
        self.unread_line = self.unread_line[len(symbol):]
        return symbol