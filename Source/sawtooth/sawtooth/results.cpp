#include "results.h"

Sawtooth_CBMBiomassPools::Sawtooth_CBMBiomassPools() {
	SWM = 0.0;
	SWF = 0.0;
	SWO = 0.0;
	SWCR = 0.0;
	SWFR = 0.0;
	HWM = 0.0;
	HWF = 0.0;
	HWO = 0.0;
	HWCR = 0.0;
	HWFR = 0.0;
}

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

//total number of elements in storage
size_t Sawtooth_Matrix::size() { return rows*cols; }

double Sawtooth_Matrix::GetValue(size_t row, size_t col) {
	return values[row * cols + col];
}

void Sawtooth_Matrix::SetValue(size_t row, size_t col, double value) {
	values[row * cols + col] = value;
}
