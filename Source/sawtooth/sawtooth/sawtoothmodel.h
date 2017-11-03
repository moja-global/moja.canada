#ifndef sawtooth_model_h
#define sawtooth_model_h
#include <vector>
#include <ctype.h>
#include <algorithm>
#include "stand.h"
#include "spatialvariable.h"
#include "parameterset.h"
#include "constants.h"
#include "results.h"
#include "mortalityprobability.h"

namespace Sawtooth {

	class SawtoothModel {
	private:
		Sawtooth_ModelMeta Meta;
		Parameter::ParameterSet& Parameters;
		Rng::Random& random;
		Parameter::Constants constants;
		// tree age
		int _A(const Stand& s, int index) { return s.Age(index); }
		// Aboveground biomass of individual trees from t-1 (kg C tree-1)
		double _B(const Stand& s) { return s.Max_C_ag(); }
		// Tree height (m)
		double _H(const Stand& s, int index) { return s.Height(index); }
		// Stand age
		double _AS(const Stand& s) { return s.MeanAge(); }
		// Stand-level biomass from t-1 (Mg C ha-1)
		double _BS(const Stand& s) { return s.Total_C_ag() / s.Area() / 1000.0; }
		// Stand density from t-1 (stems ha-1)
		double _NS(const Stand& s) { return s.StandDensity(); }

		std::vector<double> _B_Larger(const Stand& s) { return s.B_Larger(1.0 / 1000.0); }
	public:
		SawtoothModel::SawtoothModel(Sawtooth_ModelMeta meta,
			Parameter::ParameterSet& params, Rng::Random& r);
		
		void InitializeStand(Stand& stand);
		

		// perform a step of the Sawtooth model, tracks stand level and tree 
		// level results
		void Step(Stand& stand, int t, int s,
			const Parameter::SpatialVariable &spatial, int disturbance,
			Sawtooth_StandLevelResult& standlevel,
			Sawtooth_CBMResult* cbmResult,
			Sawtooth_TreeLevelResult* treeLevel);

		// process the end of step results
		void ProcessResults(Sawtooth_StandLevelResult& standLevel,
			Sawtooth_TreeLevelResult* treeLevel, Stand& t1, int t,
			int s, int dist);

		std::vector<double> ComputeRecruitmentD1(const Stand& s) {

			// Stand - level biomass from t - 1 (Mg C ha - 1)
			double BS = _BS(s);

			std::vector<double> p_rec(s.MaxDensity(), 0.0);
			for (auto species : s.UniqueSpecies()) {
				const auto b = Parameters.GetParameterRecruitmentD1(species);

				// Standardize
				double BS_z = (BS - b->R_BS_mu) / b->R_BS_sig;
				
				double lgit = b->R_Int + b->R_BS * BS_z;

				double Pr = std::exp(lgit) / (1 + std::exp(lgit));
				for (auto iDead : s.iDead(species)) {
					p_rec[iDead] = Pr;
				}
			}
			return p_rec;
		}		


		std::vector<double> ComputeRecruitmentD2(const Stand& s) {

			// Stand - level biomass from t - 1 (Mg C ha - 1)
			double BS = _BS(s);

			// Stand age
			double AS = _AS(s);

			// Stand age squared
			double AS2 = std::pow(AS, 2);

			std::vector<double> p_rec(s.MaxDensity(), 0.0);
			for (auto species : s.UniqueSpecies()) {
				const auto b = Parameters.GetParameterRecruitmentD2(species);

				// Standardize
				double BS_z = (BS - b->R_BS_mu) / b->R_BS_sig;
				double AS_z = (AS - b->R_AS_mu) / b->R_AS_sig;
				double AS2_z = (AS2 - b->R_AS2_mu) / b->R_AS2_sig;

				double lgit = b->R_Int + b->R_BS * BS_z + b->R_AS * AS_z + b->R_AS2 * AS2_z;

				double Pr = std::exp(lgit) / (1 + std::exp(lgit));
				for (auto iDead : s.iDead(species)) {
					p_rec[iDead] = Pr;
				}
			}
			return p_rec;
		}

		std::vector<double> ComputeGrowthD1(const Stand& s)
		{
			std::vector<double> result(s.MaxDensity(), 0.0);
			std::vector<double> B_Larger = _B_Larger(s);

			double B = _B(s);
			double BS = _BS(s);
			
			for (auto species : s.UniqueSpecies()) {
				const auto p = Parameters.GetParameterGrowthD1(species);

				//Standardization
				double LnB_z = (log(B) - p->G_LnB_mu) / p->G_LnB_sig;
				double B_z = (B - p->G_B_mu) / p->G_B_sig;
				double BS_z = (BS - p->G_BS_mu) / p->G_BS_sig;
				

				for (auto li : s.iLive(species)) {
					double A = _A(s, li);

					double AS_z = (A - p->G_AS_mu) / p->G_AS_sig;

					double B_Larger_z = (B_Larger[li] - p->G_SBLT_mu) / p->G_SBLT_sig;

					//Add all effects to intercept
					double yhat = p->G_Int + p->G_LnB * LnB_z + p->G_B * B_z +
						p->G_SA * AS_z + p->G_SBLT * B_Larger_z + p->G_SB * BS_z;

					//Back - transform and apply log correction
					yhat = p->G_LogCorrection * std::exp(yhat);

					//Cap unrealistic growth
					yhat = std::min(yhat, constants.G_Max);

					result[li] = yhat;
				}
			}

			return result;
		}


		std::vector<double> ComputeGrowthD2(const Stand& s)
		{
			std::vector<double> result(s.MaxDensity(), 0.0);

			double B = _B(s);
			double BS = _BS(s);
			double NS = _NS(s);
			
			for (auto species : s.UniqueSpecies()) {
				const auto p = Parameters.GetParameterGrowthD2(species);

				//Standardization
				double LnB_z = (log(B) - p->G_LnB_mu) / p->G_LnB_sig;
				double B_z = (B - p->G_B_mu) / p->G_B_sig;
				double BS_z = (BS - p->G_BS_mu) / p->G_BS_sig;
				double NS_z = (NS - p->G_NS_mu) / p->G_NS_sig;

				for (auto li : s.iLive(species)) {
					double A = _A(s, li);

					double AS_z = (A - p->G_AS_mu) / p->G_AS_sig;

					//Add all effects to intercept
					double yhat = p->G_Int + p->G_LnB * LnB_z + p->G_B * B_z +
						p->G_AS * AS_z + p->G_BS * BS_z + p->G_NS * NS_z +
						p->G_LnBxBS * LnB_z * BS_z;

					//Back - transform and apply log correction
					yhat = p->G_LogCorrection * std::exp(yhat);

					//Cap unrealistic growth
					yhat = std::min(yhat, constants.G_Max);

					result[li] = yhat;
				}
			}

			return result;
		}


		std::vector<double> ComputeGrowthES1(
			const Parameter::SpatialVariable& c, const Stand& s)
		{
			std::vector<double> result(s.MaxDensity(), 0.0);

			//Aboveground biomass of individual trees from t - 1 (kg C tree - 1)
			double B = _B(s);

			//Stand-level biomass from t-1 (Mg C ha-1)
			double BS = _BS(s);

			//Stand density from t-1 (stems ha-1)
			double NS = _NS(s);
			for (auto species : s.UniqueSpecies()) 
			{
				const auto p = Parameters.GetParameterGrowthES1(species);

				double tmin_z = (c.tmin - p->G_Tmin_mu) / p->G_Tmin_sig;
				double tmean_z = (c.tmean - p->G_T_mu) / p->G_T_sig;
				double etp_z = (c.etr - p->G_E_mu) / p->G_E_sig;
				double ws_z = (c.ws - p->G_W_mu) / p->G_W_sig;
				double ndep_z = (c.ndep - p->G_N_mu) / p->G_N_sig;
				double ca_z = (c.ca - p->G_C_mu) / p->G_C_sig;

				double G_fun_ext =
					p->G_Tmin * tmin_z +
					p->G_T * tmean_z +
					p->G_E * etp_z +
					p->G_W * ws_z +
					p->G_N * ndep_z +
					p->G_C * ca_z;

				// Standardization
				double LnB_z = (std::log(B) - p->G_LnB_mu) / p->G_LnB_sig;
				double B_z = (B - p->G_B_mu) / p->G_B_sig;

				double BS_z = (BS - p->G_BS_mu) / p->G_BS_sig;
				double NS_z = (NS - p->G_NS_mu) / p->G_NS_sig;

				for (auto li : s.iLive(species)) 
				{
					double AS_z = (_A(s, li) - p->G_AS_mu) / p->G_AS_sig;

					// Summarize intrinsic factors
					double G_fun_int = p->G_LnB * LnB_z + p->G_B * B_z + p->G_AS *
						AS_z + p->G_BS * BS_z + p->G_NS * NS_z + p->G_LnBxBS *
						LnB_z * BS_z;

					// Add all effects to intercept
					double yhat = p->G_Int + G_fun_int + G_fun_ext;

					// Back - transform and apply log correction
					yhat = p->G_LogCorrection * std::exp(yhat);

					// Cap unrealistic growth
					yhat = std::min(yhat, constants.G_Max);

					result[li] = yhat;
				}
			}
			return result;
		}

		std::vector<double> ComputeGrowthES2(
			const Parameter::SpatialVariable& c, const Stand& s)
		{
			std::vector<double> result(s.MaxDensity());
			std::vector<double> B_Larger = _B_Larger(s);
			

			//Aboveground biomass of individual trees from t - 1 (kg C tree - 1)
			double B = _B(s);

			//Stand-level biomass from t-1 (Mg C ha-1)
			double BS = _BS(s);

			//Stand density from t-1 (stems ha-1)
			double NS = _NS(s);
			for (auto species : s.UniqueSpecies()) {

				const auto p = Parameters.GetParameterGrowthES2(species);

				// Standardization
				double tmin_z = (c.tmin - p->G_Tm_mu) / p->G_Tm_sig;
				double tmean_z = (c.tmean - p->G_T_mu) / p->G_T_sig;
				double eeq_z = (c.eeq - p->G_E_mu) / p->G_E_sig;
				double ws_z = (c.ws - p->G_W_mu) / p->G_W_sig;
				double ndep_z = (c.ndep - p->G_N_mu) / p->G_N_sig;
				double ca_z = (c.ca - p->G_C_mu) / p->G_C_sig;

				// Summarize extrinsic factors
				double G_fun_ext =
					p->G_Tm * tmin_z +
					p->G_T * tmean_z +
					p->G_E * eeq_z +
					p->G_W * ws_z +
					p->G_N * ndep_z +
					p->G_C * ca_z +
					p->G_NxT * ndep_z * tmean_z +
					p->G_NxT2 * ndep_z * std::pow(tmean_z, 2.0) +
					p->G_NxE * ndep_z * eeq_z +
					p->G_NxE2 * ndep_z * std::pow(eeq_z, 2.0) +
					p->G_NxW * ndep_z * ws_z +
					p->G_NxW2 * ndep_z * std::pow(ws_z, 2.0) +
					p->G_CxT * ca_z * tmean_z +
					p->G_CxT2 * ca_z * std::pow(tmean_z, 2.0) +
					p->G_CxE * ca_z * eeq_z +
					p->G_CxE2 * ca_z * std::pow(eeq_z, 2.0) +
					p->G_CxW * ca_z * ws_z +
					p->G_CxW2 * ca_z * std::pow(ws_z, 2.0) +
					p->G_CxN * ca_z * ndep_z +
					p->G_CxN2 * ca_z * std::pow(ndep_z, 2.0);

				// Standardization
				double LnB_z = (log(B) - p->G_LnB_mu) / p->G_LnB_sig;
				double B_z = (B - p->G_B_mu) / p->G_B_sig;
				//BS_z = (BS - p->G_BS_mu). / p->G_BS_sig;

				double NS_z = (NS - p->G_NS_mu) / p->G_NS_sig;

				for (auto li : s.iLive(species))
				{
					double A = _A(s, li);
					double BS_z = (B_Larger[li] - p->G_BS_mu) / p->G_BS_sig;
					double AS_z = (A - p->G_AS_mu) / p->G_AS_sig;

					// Summarize intrinsic factors
					double G_fun_int = p->G_LnB * LnB_z + p->G_B * B_z + p->G_AS * AS_z + p->G_BS * BS_z + p->G_NS * NS_z;

					// Summarize interactions between intrinsic factors
					double G_fun_int_x_int = 0;

					// Summarize interactions between intrinsic and extrinsic factors
					double G_fun_int_x_ext = 0;

					// Add all effects to intercept
					double yhat = p->G_Int + G_fun_int + G_fun_int_x_int + G_fun_int_x_ext + G_fun_ext;

					// Back - transform and apply log correction
					yhat = p->G_LogCorrection * std::exp(yhat);

					// Cap unrealistic growth
					yhat = std::min(yhat, constants.G_Max);
					result[li] = yhat;
				}
			}
			return result;
		}

		std::vector<double> ComputeGrowthES3(
			const Parameter::SpatialVariable& c, const Stand& s) {

			std::vector<double> result(s.MaxDensity());

			std::vector<double> B_Larger = _B_Larger(s);

			double B = _B(s);

			//Stand-level biomass from t-1 (Mg C ha-1)
			double BS = _BS(s);

			for (auto species : s.UniqueSpecies()) {
				const auto p = Parameters.GetParameterGrowthES3(species);

				double LnB_z = (std::log(B) - p->G_LnB_mu) / p->G_LnB_sig;
				double B_z = (B - p->G_B_mu) / p->G_B_sig;
				double BS_z = (BS - p->G_BS_mu) / p->G_BS_sig;

				double DAI_z = 0;
				double DAP_z = 0;
				double tmin_z = (c.tmin - p->G_Tc_mu) / p->G_Tc_sig;
				double tmean_z = (c.tmean - p->G_T_mu) / p->G_T_sig;
				double etp_z = (c.etr - p->G_E_mu) / p->G_E_sig;
				double ws_z = (c.ws - p->G_W1_mu) / p->G_W1_sig;
				double ws2_z = (std::pow(c.ws, 2) - p->G_W2_mu) / p->G_W2_sig;
				double ws3_z = (std::pow(c.ws, 3) - p->G_W3_mu) / p->G_W3_sig;
				double ndep_z = (c.ndep - p->G_N_mu) / p->G_N_sig;

				// Standard experimental response to carbon dioxide
				double ca_ser = 0.339 * std::log(c.ca) - 1.257 / 0.339 * std::log(300) - 1.257;
				double ca_z = (ca_ser - p->G_C_mu) / p->G_C_sig;

				double BIOL = p->G_DAI * DAI_z + p->G_DAP * DAP_z;

				double ENVI = p->G_Tc * tmin_z + p->G_T * tmean_z + p->G_E * etp_z + p->G_W1 * ws_z +
					p->G_W2 * ws2_z + p->G_W3 * ws3_z + p->G_N * ndep_z + p->G_C * ca_z;
				
				double BIOLxENVI = p->G_DAIxT * DAI_z * tmean_z + p->G_DAIxE * DAI_z * etp_z + p->G_DAIxW * DAI_z * ws_z +
					p->G_DAIxN * DAI_z * ndep_z + p->G_DAIxC * DAI_z * ca_z +
					p->G_DAPxT * DAP_z * tmean_z + p->G_DAPxE * DAP_z * etp_z + p->G_DAPxW * DAP_z * ws_z +
					p->G_DAPxN * DAP_z * ndep_z + p->G_DAPxC * DAP_z * ca_z;
				
				double ENVIxENVI = p->G_TxE * tmean_z * etp_z + p->G_TxW * tmean_z * ws_z +
					p->G_NxT * ndep_z * tmean_z + p->G_NxT2 * ndep_z * std::pow(tmean_z, 2) +
					p->G_NxE * ndep_z * etp_z + p->G_NxE2 * ndep_z * std::pow(etp_z, 2) +
					p->G_NxW * ndep_z * ws_z + p->G_NxW2 * ndep_z * std::pow(ws_z, 2) +
					p->G_CxT * ca_z * tmean_z + p->G_CxT2 * ca_z * std::pow(tmean_z, 2) +
					p->G_CxE * ca_z * etp_z + p->G_CxE2 * ca_z * std::pow(etp_z, 2) +
					p->G_CxW * ca_z * ws_z + p->G_CxW2 * ca_z * std::pow(ws_z, 2) +
					p->G_CxN * ca_z * ndep_z + p->G_CxN2 * ca_z * std::pow(ndep_z, 2);

				
				for (auto li : s.iLive(species))
				{
					double SL1_z = (c.SL[li] - p->G_SL1_mu) / p->G_SL1_sig;
					double SL2_z = (std::pow(c.SL[li], 2) - p->G_SL2_mu) / p->G_SL2_sig;
					double CASL_z = (c.CASL[li] - p->G_CASL_mu) / p->G_CASL_sig;
					double TWI_z = (c.TWI[li] - p->G_TWI_mu) / p->G_TWI_sig;

					double SITE = p->G_SL1 * SL1_z + p->G_SL2 * SL2_z +
						p->G_CASL * CASL_z + p->G_TWI * TWI_z;

					double SITExBIOL = p->G_SLxDAI * SL1_z * DAI_z + p->G_SLxDAP * SL1_z * DAP_z +
						p->G_CASLxDAI * CASL_z * DAI_z + p->G_CASLxDAP * CASL_z * DAP_z +
						p->G_TWIxDAI * TWI_z * DAI_z + p->G_TWIxDAP * TWI_z * DAP_z;

					double SITExENVI = p->G_SLxT * SL1_z * tmean_z + p->G_SLxE * SL1_z * etp_z +
						p->G_SLxW * SL1_z * ws_z + p->G_SLxN * SL1_z * ndep_z +
						p->G_SLxC * SL1_z * ca_z + p->G_CASLxT * CASL_z * tmean_z + p->G_CASLxE * CASL_z * etp_z +
						p->G_CASLxW * CASL_z * ws_z + p->G_CASLxN * CASL_z * ndep_z +
						p->G_CASLxC * CASL_z * ca_z + p->G_TWIxT * TWI_z * tmean_z + p->G_TWIxE * TWI_z * etp_z +
						p->G_TWIxW * TWI_z * ws_z + p->G_TWIxN * TWI_z * ndep_z +
						p->G_TWIxC * TWI_z * ca_z;

					double SITExBIOLxENVI =
						p->G_SLxDAIxT * SL1_z * DAI_z * tmean_z +
						p->G_SLxDAPxT * SL1_z * DAP_z * tmean_z +
						p->G_CASLxDAIxT * CASL_z * DAI_z * tmean_z +
						p->G_CASLxDAPxT * CASL_z * DAP_z * tmean_z +
						p->G_TWIxDAIxT * TWI_z * DAI_z * tmean_z +
						p->G_TWIxDAPxT * TWI_z * DAP_z * tmean_z +
						p->G_SLxDAIxE * SL1_z * DAI_z * etp_z +
						p->G_SLxDAPxE * SL1_z * DAP_z * etp_z +
						p->G_CASLxDAIxE * CASL_z * DAI_z * etp_z +
						p->G_CASLxDAPxE * CASL_z * DAP_z * etp_z +
						p->G_TWIxDAIxE * TWI_z * DAI_z * etp_z +
						p->G_TWIxDAPxE * TWI_z * DAP_z * etp_z +
						p->G_SLxDAIxW * SL1_z * DAI_z * ws_z +
						p->G_SLxDAPxW * SL1_z * DAP_z * ws_z +
						p->G_CASLxDAIxW * CASL_z * DAI_z * ws_z +
						p->G_CASLxDAPxW * CASL_z * DAP_z * ws_z +
						p->G_TWIxDAIxW * TWI_z * DAI_z * ws_z +
						p->G_TWIxDAPxW * TWI_z * DAP_z * ws_z +
						p->G_SLxDAIxN * SL1_z * DAI_z * ndep_z +
						p->G_SLxDAPxN * SL1_z * DAP_z * ndep_z +
						p->G_CASLxDAIxN * CASL_z * DAI_z * ndep_z +
						p->G_CASLxDAPxN * CASL_z * DAP_z * ndep_z +
						p->G_TWIxDAIxN * TWI_z * DAI_z * ndep_z +
						p->G_TWIxDAPxN * TWI_z * DAP_z * ndep_z +
						p->G_SLxDAIxC * SL1_z * DAI_z * ca_z +
						p->G_SLxDAPxC * SL1_z * DAP_z * ca_z +
						p->G_CASLxDAIxC * CASL_z * DAI_z * ca_z +
						p->G_CASLxDAPxC * CASL_z * DAP_z * ca_z +
						p->G_TWIxDAIxC * TWI_z * DAI_z * ca_z +
						p->G_TWIxDAPxC * TWI_z * DAP_z * ca_z;

					double BLS_z = (B_Larger[li] - p->G_BS_mu) / p->G_BS_sig;

					int A = _A(s, li);
					double AS_z = (A - p->G_AS_mu) / p->G_AS_sig;

					double SIZE = p->G_LnB * LnB_z + p->G_B * B_z + p->G_AS * AS_z;

					double COMP = p->G_BLS * BLS_z + p->G_BS * BS_z;

					double COMPxSITE = p->G_BLSxSL * BLS_z * SL1_z +
						p->G_BLSxCASL * BLS_z * CASL_z + p->G_BLSxTWI * BLS_z * TWI_z;

					double COMPxBIOL = p->G_BLSxDAI * BLS_z * DAI_z + p->G_BLSxDAP * BLS_z * DAP_z;

					double COMPxENVI = p->G_BLSxT * BLS_z * tmean_z + p->G_BLSxE * BLS_z * etp_z +
						p->G_BLSxW * BLS_z * ws_z + p->G_BLSxN * BLS_z * ndep_z +
						p->G_BLSxC * BLS_z * ca_z;

					double COMPxSITExBIOL = p->G_BLSxSLxDAI * BLS_z * SL1_z * DAI_z + 
						p->G_BLSxCASLxDAI * BLS_z * CASL_z * DAI_z + 
						p->G_BLSxTWIxDAI * BLS_z * TWI_z * DAI_z +
						p->G_BLSxSLxDAP * BLS_z * SL1_z * DAP_z + 
						p->G_BLSxCASLxDAP * BLS_z * CASL_z * DAP_z + 
						p->G_BLSxTWIxDAP * BLS_z * TWI_z * DAP_z;

					double COMPxSITExBIOLxENVI = 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAI_z * tmean_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAI_z * tmean_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAI_z * tmean_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAP_z * tmean_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAP_z * tmean_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAP_z * tmean_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAI_z * etp_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAI_z * etp_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAI_z * etp_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAP_z * etp_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAP_z * etp_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAP_z * etp_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAI_z * ws_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAI_z * ws_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAI_z * ws_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAP_z * ws_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAP_z * ws_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAP_z * ws_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAI_z * ndep_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAI_z * ndep_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAI_z * ndep_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAP_z * ndep_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAP_z * ndep_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAP_z * ndep_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAI_z * ca_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAI_z * ca_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAI_z * ca_z + 
						p->G_BLSxSLxDAIxT * BLS_z * SL1_z * DAP_z * ca_z + 
						p->G_BLSxCASLxDAIxT * BLS_z * CASL_z * DAP_z * ca_z + 
						p->G_BLSxTWIxDAIxT * BLS_z * TWI_z * DAP_z * ca_z;

					// Add all effects to intercept
					double yhat = p->G_Int + SIZE + COMP + SITE + BIOL + ENVI + COMPxSITE + COMPxBIOL + 
						COMPxENVI + SITExBIOL + SITExENVI + BIOLxENVI + ENVIxENVI +
						COMPxSITExBIOL + SITExBIOLxENVI + COMPxSITExBIOLxENVI;

					// Back - transform and apply log correction
					yhat = p->G_LogCorrection * exp(yhat);

					// Cap unrealistic growth
					yhat = std::min(yhat, constants.G_Max);

					result[li] = yhat;

				}
			}
			return result;
		}


		void ComputeMortalityD1(const Stand& s,
			MortalityProbability& p_m)
		{

			double B = _B(s);

			double B2 = std::pow(B, 2.0);

			double BS = _BS(s);

			for (auto species : s.UniqueSpecies()) {
				
				const auto p = Parameters.GetParameterMortalityD1(species);

			std::vector<double> B_Larger = _B_Larger(s);

				double B_z = (B - p->M_B_mu) / p->M_B_sig;
				double B2_z = (B2 - p->M_B2_mu) / p->M_B2_sig;
				double BS_z = (BS - p->M_BS_mu) / p->M_BS_sig;

				for (auto ilive : s.iLive(species)) {

					double AS = s.Age(ilive);

					double AS_z = (AS - p->M_AS_mu) / p->M_AS_sig;

					double B_Larger_z = (B_Larger[ilive] - p->M_SBLT_mu) / p->M_SBLT_sig;

					double lgit = p->M_Int + p->M_B * B_z + p->M_B2 * B2_z + 
						p->M_SA * AS_z + p->M_SBLT * B_Larger_z + p->M_SB * BS_z;

					double Pm = std::exp(lgit) / (1 + std::exp(lgit));
					p_m.P_Regular[ilive] = Pm;
				}
			}
		}


		void ComputeMortalityD2(const Stand& s,
			MortalityProbability& p_m)
		{

			double BS = _BS(s);

			double B = _B(s);

			double B2 = std::pow(B, 2.0);

			for (auto species : s.UniqueSpecies()) {
				
				const auto p = Parameters.GetParameterMortalityD2(species);
				double B_z = (B - p->M_B_mu) / p->M_B_sig;
				double B2_z = (B2 - p->M_B2_mu) / p->M_B2_sig;
				double BS_z = (BS - p->M_BS_mu) / p->M_BS_sig;

				for (auto ilive : s.iLive(species)) {

					double AS = s.Age(ilive);

					double AS_z = (AS - p->M_AS_mu) / p->M_AS_sig;


					double lgit = p->M_Int + p->M_B * B_z + p->M_B2 * B2_z + 
						p->M_AS * AS_z + p->M_BS * BS_z + p->M_BxBS * B_z * BS_z;

					double Pm = std::exp(lgit) / (1 + std::exp(lgit));
					p_m.P_Regular[ilive] = Pm;
				}
			}
		}


		void ComputeMortalityES1(const Stand& s,
			const Parameter::SpatialVariable& c,
			MortalityProbability& p_m) {
			
			double BS = _BS(s);
			
			double B = _B(s);

			double B2 = std::pow(B, 2.0);
			for (auto species : s.UniqueSpecies()) {

				const auto e = Parameters.GetParameterMortalityES1(species);

				double tmin_z = (c.tmin - e->M_Tm_mu) / e->M_Tm_sig;
				double tmean_z = (c.tmean - e->M_T_mu) / e->M_T_sig;
				double etp_z = (c.etr - e->M_E_mu) / e->M_E_sig;
				double ws_z = (c.ws - e->M_W_mu) / e->M_W_sig;
				double ndep_z = (c.ndep - e->M_N_mu) / e->M_N_sig;

				double M_fun_ext =
					e->M_Tm * tmin_z +
					e->M_T * tmean_z +
					e->M_E * etp_z +
					e->M_W * ws_z +
					e->M_N * ndep_z;

				// Standardize
				double B_z = (B - e->M_B_mu) / e->M_B_sig;
				double B2_z = (B2 - e->M_B2_mu) / e->M_B2_sig;

				double BS_z = (BS - e->M_BS_mu) / e->M_BS_sig;

				// *** Special order ***
				double M_BxBS = -0.02;

				for (auto ilive : s.iLive(species)) {
					// Stand age
					double A = _A(s, ilive);
					double AS_z = (A - e->M_AS_mu) / e->M_AS_sig;
					double lgit = e->M_Int + e->M_B * B_z + e->M_B2 * B2_z +
						e->M_AS * AS_z + e->M_BS * BS_z + M_BxBS * B_z * BS_z + M_fun_ext;

					double Pm = std::exp(lgit) / (1 + std::exp(lgit));
					p_m.P_Regular[ilive] = Pm;
				}
			}
		}

		void ComputeMortalityES2(const Stand& s,
			const Parameter::SpatialVariable& c,
			MortalityProbability& p_m) {

			std::vector<double> B_Larger = _B_Larger(s);
			for (auto species : s.UniqueSpecies()) {

				const auto e = Parameters.GetParameterMortalityES2(species);

				double W1 = c.ws_mjjas_z; //-e.M_W1_mu). / e.M_W1_sig;
				double W2 = std::pow(c.ws_mjjas_z, 2); //-e.M_W2_mu). / e.M_W2_sig;
				double E1 = c.etr_mjjas_z; //-e->M_E1_mu). / e->M_E1_sig;
				double E2 = std::pow(c.etr_mjjas_z, 2); //-e->M_E1_mu). / e->M_E2_sig;
				double NW = (c.ws_mjjas_n - e->M_NW_mu) / e->M_NW_sig;
				double NE = (c.etr_mjjas_n - e->M_NE_mu) / e->M_NE_sig;

				for (auto ilive : s.iLive(species)) {

					double H = _H(s, ilive);
					double H1 = (H - e->M_H1_mu) / e->M_H1_sig;
					double H2 = (std::pow(H, 2) - e->M_H2_mu) / e->M_H2_sig;
					double CI = (B_Larger[ilive] - e->M_CI_mu) / e->M_CI_sig;
					
					double lgit = e->M_Int +
						e->M_H1 * H1 +
						e->M_H2 * H2 +
						e->M_CI * CI +
						e->M_W1 * W1 +
						e->M_W2 * W2 +
						e->M_E1 * E1 +
						e->M_E2 * E2 +
						e->M_H1xW1 * H1 * W1 +
						e->M_H1xW2 * H1 * W2 +
						e->M_H2xW1 * H2 * W1 +
						e->M_H2xW2 * H2 * W2 +
						e->M_H1xE1 * H1 * E1 +
						e->M_H1xE2 * H1 * E2 +
						e->M_H2xE1 * H2 * E1 +
						e->M_H2xE2 * H2 * E2 +
						e->M_NWxW1 * NW * W1 +
						e->M_NWxW2 * NW * W2 +
						e->M_NExE1 * NE * E1 +
						e->M_NExE2 * NE * E2;

					double Pm = e->M_BiasAdj * (std::exp(lgit) / (1 + std::exp(lgit)));
					p_m.P_Regular[ilive] = Pm;
				}
			}
		}

		void ComputeMortalityMLR35(const Stand& s,
			const Parameter::SpatialVariable& c,
			MortalityProbability& p_m) {

			std::vector<double> B_Larger = _B_Larger(s);
			for (auto species : s.UniqueSpecies()) {

				const auto e = Parameters.GetParameterMortalityMLR35(species);

				double T1 = (c.tmin - e->M_tmin1_mu) / e->M_tmin1_sig;
				double T2 = (std::pow(c.tmin, 2) - e->M_tmin2_mu) / e->M_tmin2_sig;
				double N1 = (c.ndep - e->M_n1_mu) / e->M_n1_sig;
				double N2 = (std::pow(c.ndep, 2) - e->M_n2_mu) / e->M_n2_sig;
				double W1 = (c.ws_mjjas_z - e->M_ws1_mu) / e->M_ws1_sig;
				double W2 = (std::pow(c.ws_mjjas_z, 2) - e->M_ws2_mu) / e->M_ws2_sig;
				double E1 = (c.etr_mjjas_z - e->M_etp1_mu) / e->M_etp1_sig;
				double E2 = (std::pow(c.etr_mjjas_z, 2) - e->M_etp2_mu) / e->M_etp2_sig;
				double WN = (c.ws_mjjas_n - e->M_nw_mu) / e->M_nw_sig;
				double EN = (c.etr_mjjas_n - e->M_ne_mu) / e->M_ne_sig;

				for (auto ilive : s.iLive(species)) {

					double H = _H(s, ilive);

					double H1 = (H - e->M_h1_mu) / e->M_h1_sig;
					double H2 = (std::pow(H, 2) - e->M_h2_mu) / e->M_h2_sig;
					double CI = (B_Larger[ilive] - e->M_ci_mu) / e->M_ci_sig;

					double b1 = std::exp(e->M_1_int + H1 * e->M_1_h1 + H2 * e->M_1_h2 +
						CI * e->M_1_ci + T1 * e->M_1_tmin1 + T2 * e->M_1_tmin2 +
						N1 * e->M_1_ndep1 + N2 * e->M_1_ndep2 + W1 * e->M_1_ws1 +
						W2 * e->M_1_ws2 + E1 * e->M_1_etp1 + E2 * e->M_1_etp2 +
						WN*W1 * e->M_1_nw_w1 + WN*W2 * e->M_1_nw_x_w2 +
						H1*W1 * e->M_1_h1_x_w1 + H1*W2 * e->M_1_h1_x_w2 +
						H2*W1 * e->M_1_h2_x_w1 + H2*W2 * e->M_1_h2_x_w2 +
						CI*W1 * e->M_1_ci_x_w1 + CI*W2 * e->M_1_ci_x_w2 +
						N1*W1 * e->M_1_n1_x_w1 + N1*W2 * e->M_1_n1_x_w2 +
						N2*W1 * e->M_1_n2_x_w1 + N2*W2 * e->M_1_n2_x_w2 +
						EN*E1 * e->M_1_ne_e1 + EN*E2 * e->M_1_ne_x_e2 +
						H1*E1 * e->M_1_h1_x_e1 + H1*E2 * e->M_1_h1_x_e2 +
						H2*E1 * e->M_1_h2_x_e1 + H2*E2 * e->M_1_h2_x_e2 +
						CI*E1 * e->M_1_ci_x_e1 + CI*E2 * e->M_1_ci_x_e2 +
						N1*E1 * e->M_1_n1_x_e1 + N1*E2 * e->M_1_n1_x_e2 +
						N2*E1 * e->M_1_n2_x_e1 + N2*E2 * e->M_1_n2_x_e2);

					double b2 = std::exp(e->M_2_int + H1 * e->M_2_h1 + H2 * e->M_2_h2 +
						CI * e->M_2_ci + T1 * e->M_2_tmin1 + T2 * e->M_2_tmin2 +
						N1 * e->M_2_ndep1 + N2 * e->M_2_ndep2 + W1 * e->M_2_ws1 +
						W2 * e->M_2_ws2 + E1 * e->M_2_etp1 + E2 * e->M_2_etp2 +
						WN*W1 * e->M_2_nw_w1 + WN*W2 * e->M_2_nw_x_w2 +
						H1*W1 * e->M_2_h1_x_w1 + H1*W2 * e->M_2_h1_x_w2 +
						H2*W1 * e->M_2_h2_x_w1 + H2*W2 * e->M_2_h2_x_w2 +
						CI*W1 * e->M_2_ci_x_w1 + CI*W2 * e->M_2_ci_x_w2 +
						N1*W1 * e->M_2_n1_x_w1 + N1*W2 * e->M_2_n1_x_w2 +
						N2*W1 * e->M_2_n2_x_w1 + N2*W2 * e->M_2_n2_x_w2 +
						EN*E1 * e->M_2_ne_e1 + EN*E2 * e->M_2_ne_x_e2 +
						H1*E1 * e->M_2_h1_x_e1 + H1*E2 * e->M_2_h1_x_e2 +
						H2*E1 * e->M_2_h2_x_e1 + H2*E2 * e->M_2_h2_x_e2 +
						CI*E1 * e->M_2_ci_x_e1 + CI*E2 * e->M_2_ci_x_e2 +
						N1*E1 * e->M_2_n1_x_e1 + N1*E2 * e->M_2_n1_x_e2 +
						N2*E1 * e->M_2_n2_x_e1 + N2*E2 * e->M_2_n2_x_e2);

					double b3 = std::exp(e->M_3_int + H1 * e->M_3_h1 + H2 * e->M_3_h2 +
						CI * e->M_3_ci + T1 * e->M_3_tmin1 + T2 * e->M_3_tmin2 +
						N1 * e->M_3_ndep1 + N2 * e->M_3_ndep2 + W1 * e->M_3_ws1 +
						W2 * e->M_3_ws2 + E1 * e->M_3_etp1 + E2 * e->M_3_etp2 +
						WN*W1 * e->M_3_nw_w1 + WN*W2 * e->M_3_nw_x_w2 +
						H1*W1 * e->M_3_h1_x_w1 + H1*W2 * e->M_3_h1_x_w2 +
						H2*W1 * e->M_3_h2_x_w1 + H2*W2 * e->M_3_h2_x_w2 +
						CI*W1 * e->M_3_ci_x_w1 + CI*W2 * e->M_3_ci_x_w2 +
						N1*W1 * e->M_3_n1_x_w1 + N1*W2 * e->M_3_n1_x_w2 +
						N2*W1 * e->M_3_n2_x_w1 + N2*W2 * e->M_3_n2_x_w2 +
						EN*E1 * e->M_3_ne_e1 + EN*E2 * e->M_3_ne_x_e2 +
						H1*E1 * e->M_3_h1_x_e1 + H1*E2 * e->M_3_h1_x_e2 +
						H2*E1 * e->M_3_h2_x_e1 + H2*E2 * e->M_3_h2_x_e2 +
						CI*E1 * e->M_3_ci_x_e1 + CI*E2 * e->M_3_ci_x_e2 +
						N1*E1 * e->M_3_n1_x_e1 + N1*E2 * e->M_3_n1_x_e2 +
						N2*E1 * e->M_3_n2_x_e1 + N2*E2 * e->M_3_n2_x_e2);

					double b4 = std::exp(e->M_4_int + H1 * e->M_4_h1 + H2 * e->M_4_h2 +
						CI * e->M_4_ci + T1 * e->M_4_tmin1 + T2 * e->M_4_tmin2 +
						N1 * e->M_4_ndep1 + N2 * e->M_4_ndep2 + W1 * e->M_4_ws1 +
						W2 * e->M_4_ws2 + E1 * e->M_4_etp1 + E2 * e->M_4_etp2 +
						WN*W1 * e->M_4_nw_w1 + WN*W2 * e->M_4_nw_x_w2 +
						H1*W1 * e->M_4_h1_x_w1 + H1*W2 * e->M_4_h1_x_w2 +
						H2*W1 * e->M_4_h2_x_w1 + H2*W2 * e->M_4_h2_x_w2 +
						CI*W1 * e->M_4_ci_x_w1 + CI*W2 * e->M_4_ci_x_w2 +
						N1*W1 * e->M_4_n1_x_w1 + N1*W2 * e->M_4_n1_x_w2 +
						N2*W1 * e->M_4_n2_x_w1 + N2*W2 * e->M_4_n2_x_w2 +
						EN*E1 * e->M_4_ne_e1 + EN*E2 * e->M_4_ne_x_e2 +
						H1*E1 * e->M_4_h1_x_e1 + H1*E2 * e->M_4_h1_x_e2 +
						H2*E1 * e->M_4_h2_x_e1 + H2*E2 * e->M_4_h2_x_e2 +
						CI*E1 * e->M_4_ci_x_e1 + CI*E2 * e->M_4_ci_x_e2 +
						N1*E1 * e->M_4_n1_x_e1 + N1*E2 * e->M_4_n1_x_e2 +
						N2*E1 * e->M_4_n2_x_e1 + N2*E2 * e->M_4_n2_x_e2);

					double b5 = std::exp(e->M_5_int + H1 * e->M_5_h1 + H2 * e->M_5_h2 +
						CI * e->M_5_ci + T1 * e->M_5_tmin1 + T2 * e->M_5_tmin2 +
						N1 * e->M_5_ndep1 + N2 * e->M_5_ndep2 + W1 * e->M_5_ws1 +
						W2 * e->M_5_ws2 + E1 * e->M_5_etp1 + E2 * e->M_5_etp2 +
						WN*W1 * e->M_5_nw_w1 + WN*W2 * e->M_5_nw_x_w2 +
						H1*W1 * e->M_5_h1_x_w1 + H1*W2 * e->M_5_h1_x_w2 +
						H2*W1 * e->M_5_h2_x_w1 + H2*W2 * e->M_5_h2_x_w2 +
						CI*W1 * e->M_5_ci_x_w1 + CI*W2 * e->M_5_ci_x_w2 +
						N1*W1 * e->M_5_n1_x_w1 + N1*W2 * e->M_5_n1_x_w2 +
						N2*W1 * e->M_5_n2_x_w1 + N2*W2 * e->M_5_n2_x_w2 +
						EN*E1 * e->M_5_ne_e1 + EN*E2 * e->M_5_ne_x_e2 +
						H1*E1 * e->M_5_h1_x_e1 + H1*E2 * e->M_5_h1_x_e2 +
						H2*E1 * e->M_5_h2_x_e1 + H2*E2 * e->M_5_h2_x_e2 +
						CI*E1 * e->M_5_ci_x_e1 + CI*E2 * e->M_5_ci_x_e2 +
						N1*E1 * e->M_5_n1_x_e1 + N1*E2 * e->M_5_n1_x_e2 +
						N2*E1 * e->M_5_n2_x_e1 + N2*E2 * e->M_5_n2_x_e2);

					// Correct for variable number of trials per observation assuming
					// median number of years in interval is 10 and assuming that the
					// correction approach zero as multiannual probability approaches
					// 1.0
					double MedNumTrial = 10.0;

					// Annual probability of mortality due to regular process
					double Pm_r = b1 / (1.0 + b1 + b2 + b3 + b4 + b5);
					p_m.P_Regular[ilive] = 1.0 - std::pow((1.0 - Pm_r), (1.0 / MedNumTrial));

					// Annual probability of mortality due to insects
					double Pm_i = b3 / (1.0 + b1 + b2 + b3 + b4 + b5);
					p_m.P_Insect[ilive] = 1.0 - std::pow((1.0 - Pm_i), (1.0 / MedNumTrial));

					// Annual probability of mortality due to pathogens
					double Pm_p = b5 / (1.0 + b1 + b2 + b3 + b4 + b5);
					p_m.P_Pathogen[ilive] = 1.0 - std::pow((1.0 - Pm_p), (1.0 / MedNumTrial));
					
				}
			}
		}

		std::vector<double> ComputeHeight(const Stand& s) {
			std::vector<double> height(s.MaxDensity(), 0.0);
			
			for (auto species : s.UniqueSpecies()) {
				const auto sp =
					Parameters.GetParameterCore(species);
				for (auto t : s.iLive(species)) {
					height[t] = std::pow(sp->Cag2H1 *
						((1 - std::exp(-sp->Cag2H2 * s.C_ag(t)))),
						(1.0 / (1.0 - sp->Cag2H3)));
				}
			}
			return height;
		}

		void Disturbance(Stand& s, int distype) {
			
			const auto dist = Parameters.GetDisturbanceType(distype);
			double p_mortality = dist->P_Mortality();
			if (p_mortality == 0.0) {
				return;
			}

			if (p_mortality == 1.0 && !dist->HasFilter()) {
				//disturb everything
				s.KillAllTrees(Sawtooth_Disturbance);
			}
			else {
				int nLive = s.NLive();
				std::vector<double> rn = random.rand(nLive);
				int k = 0;
				for (auto iLive : s.iLive()) {
					if (dist->IsFiltered(s.SpeciesId(iLive))) {
						continue; //do not allow non-eligible species to be killed
					}
					if (rn[k++] < p_mortality) {
						s.KillTree(iLive, Sawtooth_Disturbance);
					}
				}
			}
		}

		void Mortality(Stand &s, const MortalityProbability& Pm) {
			
			//random sequence the length of live trees in the stand
			std::vector<double> rLive = random.rand(s.NLive());
			int counter = 0;
			for (auto i_live : s.iLive()) {
				if (rLive[counter] <= Pm.P_Regular[i_live]) {
					s.KillTree(i_live, Sawtooth_RegularMortality);
				}
				else if (rLive[counter] <= Pm.P_Insect[i_live]) {
					s.KillTree(i_live, Sawtooth_InsectAttack);
				}
				else if (rLive[counter] <= Pm.P_Pathogen[i_live]) {
					s.KillTree(i_live, Sawtooth_Pathogen);
				}
				counter++;
			}
		}

		void Recruitment(Stand& s, const std::vector<double>& Pr, double initial_C_ag,
			double initial_height) {

			std::vector<double> rDead = random.rand(s.NDead());
			// Establish trees based on probability of recruitment
			int counter = 0;
			for (auto i_d : s.iDead()) {
				if (Pr[i_d] >= rDead[counter++]) {
					s.EstablishTree(i_d, initial_C_ag, initial_height);
				}
			}
		}
	};
}
#endif // !sawtooth_h
