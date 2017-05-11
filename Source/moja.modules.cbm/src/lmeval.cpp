#include "moja/modules/cbm/lmeval.h"

namespace moja {
namespace modules {
namespace cbm {
	
	/*
	*	par is an input array. At the end of the minimization, it contains
	*        the approximate solution vector.
	*
	*	m_dat is a positive integer input variable set to the number
	*	  of functions.
	*
	*	fvec is an output array of length m_dat which contains the function
	*        values the square sum of which ought to be minimized.
	*
	*	data is a read-only pointer to lm_data_type, as specified by lm_eval.h.
	*
	*      info is an integer output variable. If set to a negative value, the
	*        minimization procedure will stop.
	*/
	void LmEval::lm_evaluate_default(double *par, int m_dat, double *fvec,
		void *data, int *info)
	{
		int i;
		lm_data_type *mydata;
		mydata = (lm_data_type *)data;

		for (i = 0; i < m_dat; i++)
			fvec[i] = mydata->user_y[i]
			- mydata->user_func(mydata->user_t[i], par);

		*info = *info; /* to prevent a 'unused variable' warning */
	}


	/*
	*       data  : for soft control of printout behaviour, add control
	*                 variables to the data struct
	*       iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated)
	*       iter  : outer loop counter
	*       nfev  : number of calls to *evaluate
	*/
	void LmEval::lm_print_default(int n_par, double *par, int m_dat, double *fvec,
		void *data, int iflag, int iter, int nfev)
	{
		double f, y, t;
		int i;
		lm_data_type *mydata;
		mydata = (lm_data_type *)data;

		for (i = 0; i < n_par; ++i)
			if (iflag == -1) {
				for (i = 0; i < m_dat; ++i) {
					t = (mydata->user_t)[i];
					y = (mydata->user_y)[i];
					f = mydata->user_func(t, par);
				}
			}
	}
	 
}}}