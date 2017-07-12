class LayerStack(object):

    def __init__(self, name, layers, requested_pixel_size, data_type, years, steps_per_year):
        self._name = name
        self._layers = layers
        self._requested_pixel_size = requested_pixel_size
        self._data_type = data_type
        self._years = years
        self._steps_per_year = steps_per_year

    @property
    def name(self):
        return self._name

    @property
    def layers(self):
        return self._layers;

    @property
    def requested_pixel_size(self):
        return self._requested_pixel_size

    @property
    def data_type(self):
        return self._data_type

    @property
    def years(self):
        return self._years

    @property
    def steps_per_year(self):
        return self._steps_per_year
