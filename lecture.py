class Lecture:
    def __init__(self, name, ec, area, theo=False, grade=None):
        self.name = name
        self.ec = ec
        self.theo = theo
        self.area = area
        self.grade = grade

    def __cmp__(self, other):
        return self.name < other.name

    def __lt__(self, other):
        return self.name < other.name

    def _tostr(self):
        out = f'{self.name} - EC: {self.ec} Area: {self.area}'
        if self.theo:
            out += ' - THEO'
        return out

    def __str__(self):
        return self._tostr()

    def __repr__(self):
        return self._tostr()
