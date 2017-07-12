from block import Block

class Tile(object):

    def __init__(self, x, y, origin, pixel_size, tile_extent=1.0, block_extent=0.1):
        self._x = x
        self._y = y
        self._origin = origin
        self._pixel_size = pixel_size
        self._tile_extent = tile_extent
        self._block_extent = block_extent

    @property
    def x_min(self):
        return self._x

    @property
    def x_max(self):
        return self._x + 1

    @property
    def y_min(self):
        return self._y

    @property
    def y_max(self):
        return self._y + 1

    @property
    def x_offset(self):
        return abs(int((self.x_min - self._origin[0]) / self._pixel_size))

    @property
    def y_offset(self):
        return abs(int((self.y_max - self._origin[1]) / self._pixel_size))

    @property
    def x_size(self):
        return int(self._tile_extent / self._pixel_size)

    @property
    def y_size(self):
        return int(self._tile_extent / self._pixel_size)

    @property
    def blocks(self):
        num_blocks = int(self._tile_extent / self._block_extent)
        for y in xrange(num_blocks):
            for x in xrange(num_blocks):
                yield Block(x, y, self, self._pixel_size, self._block_extent)

    @property
    def name(self):
        return "{0}{1:03d}_{2}{3:03d}".format(
            "-" if self.x_min < 0 else "",
            abs(self.x_min),
            "-" if self.y_max < 0 else "",
            abs(self.y_max))
