#include "moja/modules/cbm/lmeval.h"

namespace moja {
namespace modules {
namespace cbm {
	
	/**
	 * Evaluation function
	 * 
	 * Description of the paraeters : \n
	 * par is an input array. At the end of the minimization, it contains the approximate solution vector. \n
	 * m_dat is a positive integer input variable set to the number of functions \n
	 * fvec is an output array of length m_dat which contains the function values the square sum of which ought to be minimized. \n
	 * data is a read-only pointer to lm_data_type, as specified by lm_eval.h. \n
	 * info is an integer output variable. If set to a negative value, the minimization procedure will stop.
	 * 
	 * Create a pointer variable mydata, of type lm_data_type. Type cast parameter data to lm_data_type * and assign it to mydata \n
	 * For index in the range 0 to parameter m_dat, set fvec[index] to the difference of  
	 * user_y[index] and user_func(user_t[index], par) on mydata \n
	 * Set parameter *info to *info to prevent a 'unused variable' warning
	 * 
	 * @param par double*
	 * @param m_dat int
	 * @param fvec double*
	 * @param data void*
	 * @param info int*
	 * @return void
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


	/**
	 * Description of the parameters : \n
	 * data  : for soft control of printout behaviour, add control variables to the data struct \n
	 * iflag : 0 (init) 1 (outer loop) 2(inner loop) -1(terminated) \n
	 * iter  : outer loop counter \n
	 * nfev  : number of calls to *evaluate \n
	 * 
	 * Create 3 variables f, y, t of type double, a pointer variable mydata, of type lm_data_type. Type cast parameter data to lm_data_type * and assign it to mydata \n
	 * For index in the range 0 to parameter n_par, if parameter iflag is -1, set variable t to mydata->user_t[index],
	 * variable y to mydata->user_y[index], and variable f to mydata->user_func(t, par) on mydata \n
	 * 
	 * @param n_par int
	 * @param par double*
	 * @param m_dat int
	 * @param fvec double*
	 * @param data void*
	 * @param iflag int
	 * @param iter int
	 * @param nfev int
	 * @return void
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