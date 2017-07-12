class TransitionRule(object):

    def __init__(self, regen_delay=0, age_after=-1, classifiers=None):
        self._regen_delay = regen_delay
        self._age_after = age_after
        self._classifiers = classifiers or []

    def __hash__(self):
        result = 37
        result += 3 * hash(self._regen_delay)
        result += 7 * hash(self._age_after)
        result += 13 * sum(hash(c) for c in self._classifiers)
        return result

    def __eq__(self, other):
        return self._regen_delay == other._regen_delay \
            and self._age_after == other._age_after \
            and self._classifiers == other._classifiers

    def __ne__(self, other):
        return not self == other

    @property
    def regen_delay(self):
        return self._regen_delay

    @property
    def age_after(self):
        return self._age_after

    @property
    def classifiers(self):
        return self._classifiers
