class Attribute(object):

    def __init__(self, layer_name, db_name=None, filter=None, substitutions=None):
        self._name = layer_name
        self._db_name = db_name or layer_name
        self._filter = filter
        self._substitutions = substitutions or {}

    @property
    def name(self):
        return self._name

    @property
    def db_name(self):
        return self._db_name

    def filter(self, value):
        return self._filter(value) if self._filter else True

    def sub(self, value):
        return self._substitutions.get(value) or value
