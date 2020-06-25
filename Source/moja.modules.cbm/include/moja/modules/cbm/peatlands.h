#ifndef MOJA_MODULES_CBM_PEATLANDS_H_
#define MOJA_MODULES_CBM_PEATLANDS_H_

namespace moja {
namespace modules {
namespace cbm {
	enum class  Peatlands : int {  
		OPEN_PEATLAND_BOG = 1,			// open bog
		OPEN_PEATLAND_POORFEN = 4,		// open poor fen
		OPEN_PEATLAND_RICHFEN = 7,		// open rich fen	
		TREED_PEATLAND_BOG = 2,			// treed bog
		TREED_PEATLAND_POORFEN = 5,		// treed poor fen
		TREED_PEATLAND_RICHFEN = 8,		// treed rich fen
		TREED_PEATLAND_SWAMP = 10,		// treed swamp
		FOREST_PEATLAND_BOG = 3,		// forested bog
		FOREST_PEATLAND_POORFEN = 6,	// forested poor fen
		FOREST_PEATLAND_RICHFEN = 9,	// forested rich fen
		FOREST_PEATLAND_SWAMP = 11		// forested swamp
	};
}}} // namespace moja::Modules::cbm
#endif // MOJA_MODULES_CBM_PEATLANDS_H_
