#ifndef MOJA_MODULES_CBM_LMMIN_H_
#define MOJA_MODULES_CBM_LMMIN_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"

namespace moja {
	namespace modules {
		namespace cbm {

			/* Collection of control parameters. */
			typedef struct {
				double ftol;      /* relative error desired in the sum of squares. */
				double xtol;      /* relative error between last two approximations. */
				double gtol;      /* orthogonality desired between fvec and its derivs. */
				double epsilon;   /* step used to calculate the jacobian. */
				double stepbound; /* initial bound to steps in the outer loop. */
				double fnorm;     /* norm of the residue vector fvec. */
				int maxcall;      /* maximum number of iterations. */
				int nfev;	      /* actual number of iterations. */
				int info;	      /* status of minimization. */
			} lm_control_type;


			class CBM_API LmMin {
			public:
				/* Type of user-supplied subroutine that calculates fvec. */
				typedef void (lm_evaluate_ftype)(double *par, int m_dat, double *fvec, void *data, int *info);

				/* Default implementation therof, provided by lm_eval.c. */
				void lm_evaluate_default(double *par, int m_dat, double *fvec, void *data, int *info);

				/* Type of user-supplied subroutine that informs about fit progress. */
				typedef void (lm_print_ftype)(int n_par, double *par, int m_dat, double *fvec, void *data, int iflag, int iter, int nfev);

				/* Default implementation therof, provided by lm_eval.c. */
				void lm_print_default(int n_par, double *par, int m_dat, double *fvec, void *data, int iflag, int iter, int nfev);

				/* Initialize control parameters with default values. */
				void lm_initialize_control(lm_control_type * control);

				/* Refined calculation of Eucledian norm, typically used in printout routine. */
				double lm_enorm(int, double *);

				/* The actual minimization. */
				void lm_minimize(int m_dat, int n_par, double *par, lm_evaluate_ftype * evaluate, lm_print_ftype * printout, void *data, lm_control_type * control);

				void lm_lmdif(int m, int n, double *x, double *fvec, double ftol, double xtol, double gtol, int maxfev, double epsfcn,
					double *diag, int mode, double factor, int *info, int *nfev, double *fjac, int *ipvt, double *qtf, double *wa1,
					double *wa2, double *wa3, double *wa4, lm_evaluate_ftype * evaluate, lm_print_ftype * printout, void *data);

				void lm_qrfac(int m, int n, double *a, int pivot, int *ipvt, double *rdiag, double *acnorm, double *wa);

				void lm_qrsolv(int n, double *r, int ldr, int *ipvt, double *diag, double *qtb, double *x, double *sdiag, double *wa);

				void lm_lmpar(int n, double *r, int ldr, int *ipvt, double *diag, double *qtb, double delta, double *par, double *x,
					double *sdiag, double *wa1, double *wa2);
			private:
			};
		}
	}
}
#endif