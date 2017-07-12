class Block(object):

    def __init__(self, x, y, tile, pixel_size, extent):
        self._x = x
        self._y = y
        self._tile = tile
        self._pixel_size = pixel_size
        self._extent = extent

    @property
    def x_size(self):
        return int(self._extent / self._pixel_size)

    @property
    def y_size(self):
        return int(self._extent / self._pixel_size)

    @property
    def x_offset(self):
        return abs(self._tile.x_offset + self._x * self.x_size)

    @property
    def y_offset(self):
        return abs(self._tile.y_offset + self._y * self.y_size)
