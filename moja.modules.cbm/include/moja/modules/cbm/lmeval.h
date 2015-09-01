#ifndef MOJA_MODULES_CBM_LMEVAL_H_
#define MOJA_MODULES_CBM_LMEVAL_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace cbm {	

	typedef struct {
		/* may be modified to hold arbitrary data */
		double *user_t;
		double *user_y;
		double (*user_func) (double user_t_point, double *par);
	} lm_data_type;


	class CBM_API LmEval{
	public:
		LmEval() = delete;
		//Prototypes of procedures
		static void lm_evaluate_default(double *par, int m_dat, double *fvec,  void *data, int *info);

		static void lm_print_default(int n_par, double *par, int m_dat, double *fvec, void *data, int iflag, int iter, int nfev);
	
	};
}}}
#endif