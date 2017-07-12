from mojadata.layer.layer import Layer
from mojadata.layer.rasterlayer import RasterLayer
from mojadata.layer.attribute import Attribute
from gdalconst import *

class DisturbanceLayer(Layer):

    def __init__(self, transition_rule_manager, lyr, year, disturbance_type, transition=None):
        self._transition_rule_manager = transition_rule_manager
        self._layer = lyr
        self._year = year
        self._disturbance_type = disturbance_type
        self._transition = transition
        self._attributes = self._build_attributes(lyr, transition)

    @property
    def name(self):
        return self._layer.name

    @property
    def path(self):
        return self._layer.path

    @path.setter
    def path(self, value):
        self._layer.path = value

    @property
    def attributes(self):
        return self._attributes

    @property
    def attribute_table(self):
        attr_table = {}
        for pixel_value, attr_values in self._layer.attribute_table.iteritems():
            attr_values = dict(zip(self._layer.attributes, attr_values))
            transition_id = None
            if self._transition:
                regen_delay = attr_values[self._transition.regen_delay.db_name] \
                    if isinstance(self._transition.regen_delay, Attribute) \
                    else self._transition.regen_delay

                age_after = attr_values[self._transition.age_after.db_name] \
                    if isinstance(self._transition.age_after, Attribute) \
                    else self._transition.age_after

                transition_values = {c: attr_values[c] for c in self._transition.classifiers}
                transition_id = self._transition_rule_manager.get_or_add(
                    regen_delay, age_after, transition_values)

            values = [
                attr_values[self._year.db_name] \
                    if isinstance(self._year, Attribute) else self._year,
                attr_values[self._disturbance_type.db_name] \
                    if isinstance(self._disturbance_type, Attribute) else self._disturbance_type]

            if transition_id:
                values.append(transition_id)

            attr_table[pixel_value] = values

        return attr_table

    def as_raster_layer(self, srs, min_pixel_size, block_extent,
                        requested_pixel_size=None, data_type=None,
                        bounds=None):
        raster = self._layer.as_raster_layer(
            srs, min_pixel_size, block_extent, requested_pixel_size,
            data_type, bounds)

        return RasterLayer(raster.path, self.attributes, self.attribute_table)

    def _build_attributes(self, lyr, transition):
        attributes = ["year", "disturbance_type"]
        if transition:
            attributes.append("transition")

        return attributes