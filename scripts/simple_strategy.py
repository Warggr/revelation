class Strategy:
    def choose(self, choices, message):
        for i, choice in enumerate(choices):
            print(f'[{i}]: {choice}')
        while True:
            choice = int(input(message + ':'))
            if 0 <= choice and choice < len(choices):
                return choice
            else:
                print(f'Please enter a number between 0 and {len(choices)}!')
