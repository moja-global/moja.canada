#include "results.h"


Sawtooth_CBMBiomassPools operator+(const Sawtooth_CBMBiomassPools& lh,
	const Sawtooth_CBMBiomassPools& rh) {
	Sawtooth_CBMBiomassPools result;
	result.SWM = lh.SWM + rh.SWM;
	result.SWF = lh.SWF + rh.SWF;
	result.SWO = lh.SWO + rh.SWO;
	result.SWCR = lh.SWCR + rh.SWCR;
	result.SWFR = lh.SWFR + rh.SWFR;
	result.HWM = lh.HWM + rh.HWM;
	result.HWF = lh.HWF + rh.HWF;
	result.HWO = lh.HWO + rh.HWO;
	result.HWCR = lh.HWCR + rh.HWCR;
	result.HWFR = lh.HWFR + rh.HWFR;
	return result;
}

Sawtooth_CBMBiomassPools operator-(const Sawtooth_CBMBiomassPools& lh,
	const Sawtooth_CBMBiomassPools& rh) {
	Sawtooth_CBMBiomassPools result;
	result.SWM = lh.SWM - rh.SWM;
	result.SWF = lh.SWF - rh.SWF;
	result.SWO = lh.SWO - rh.SWO;
	result.SWCR = lh.SWCR - rh.SWCR;
	result.SWFR = lh.SWFR - rh.SWFR;
	result.HWM = lh.HWM - rh.HWM;
	result.HWF = lh.HWF - rh.HWF;
	result.HWO = lh.HWO - rh.HWO;
	result.HWCR = lh.HWCR - rh.HWCR;
	result.HWFR = lh.HWFR - rh.HWFR;
	return result;
}


