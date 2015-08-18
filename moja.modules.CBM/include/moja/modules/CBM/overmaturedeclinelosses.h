#ifndef CBM_OvermatureDeclineLosses_H_
#define CBM_OvermatureDeclineLosses_H_

#include "moja/modules/cbm/_modules.cbm_exports.h"
#include "moja/flint/modulebase.h"

namespace moja {
namespace modules {
namespace CBM {
	class CBM_API OvermatureDeclineLosses : public moja::flint::ModuleBase {
	public:
		OvermatureDeclineLosses() :
			_lossesPresent(false), _merchToStemSnags(0.0), _foliageToAGVeryFast(0.0), _otherToBranchSnag(0.0), _otherToAGFast(0.0), _coarseRootToAGFast(0.0), _coarseRootToBGFast(0.0), _fineRootToAGVeryFast(0.0), _fineRootToBGVeryFast(0.0) {};

		virtual ~OvermatureDeclineLosses() {};	
		
		bool lossesPresent () const;             
		double merchToStemSnags () const;        
		double foliageToAGVeryFast () const;     
		double otherToBranchSnag () const;       
		double otherToAGFast () const;           
		double coarseRootToAGFast () const;      
		double coarseRootToBGFast () const;      
		double fineRootToAGVeryFast () const;    
		double fineRootToBGVeryFast () const;
		
		void setLossesPresent(bool value);       
		void setMerchToStemSnags(double value);    
		void setFoliageToAGVeryFast(double value); 
		void setOtherToBranchSnag(double value);   
		void setOtherToAGFast(double value);       
		void setCoarseRootToAGFast(double value);  
		void setCoarseRootToBGFast(double value);  
		void setFineRootToAGVeryFast(double value);
		void setFineRootToBGVeryFast(double value);
		
	private:
		bool _lossesPresent;         
		double _merchToStemSnags;    
		double _foliageToAGVeryFast; 
		double _otherToBranchSnag;   
		double _otherToAGFast;       
		double _coarseRootToAGFast;  
		double _coarseRootToBGFast;  
		double _fineRootToAGVeryFast;
		double _fineRootToBGVeryFast;
	};
	
	inline bool   OvermatureDeclineLosses::lossesPresent () const{ return _lossesPresent; }              
	inline double OvermatureDeclineLosses::merchToStemSnags () const{ return _merchToStemSnags; }        
	inline double OvermatureDeclineLosses::foliageToAGVeryFast () const{ return _foliageToAGVeryFast; }  
	inline double OvermatureDeclineLosses::otherToBranchSnag () const{ return _otherToBranchSnag; }      
	inline double OvermatureDeclineLosses::otherToAGFast () const{ return _otherToAGFast; }              
	inline double OvermatureDeclineLosses::coarseRootToAGFast () const{ return _coarseRootToAGFast; }    
	inline double OvermatureDeclineLosses::coarseRootToBGFast () const{ return _coarseRootToBGFast; }    
	inline double OvermatureDeclineLosses::fineRootToAGVeryFast () const{ return _fineRootToAGVeryFast; }
	inline double OvermatureDeclineLosses::fineRootToBGVeryFast () const{ return _fineRootToBGVeryFast;}

	inline void OvermatureDeclineLosses::setLossesPresent(bool value){ _lossesPresent = value; }
	inline void OvermatureDeclineLosses::setMerchToStemSnags(double value){ _merchToStemSnags = value; }
	inline void OvermatureDeclineLosses::setFoliageToAGVeryFast(double value){ _foliageToAGVeryFast = value; }
	inline void OvermatureDeclineLosses::setOtherToBranchSnag(double value){ _otherToBranchSnag = value; }
	inline void OvermatureDeclineLosses::setOtherToAGFast(double value){ _otherToAGFast = value; }
	inline void OvermatureDeclineLosses::setCoarseRootToAGFast(double value){ _coarseRootToAGFast = value; }
	inline void OvermatureDeclineLosses::setCoarseRootToBGFast(double value){ _coarseRootToBGFast = value; }
	inline void OvermatureDeclineLosses::setFineRootToAGVeryFast(double value){ _fineRootToAGVeryFast = value; }
	inline void OvermatureDeclineLosses::setFineRootToBGVeryFast(double value){ _fineRootToBGVeryFast = value; }

}}}
#endif