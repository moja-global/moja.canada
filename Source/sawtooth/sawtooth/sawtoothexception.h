#ifndef sawtooth_exception_h
#define sawtooth_exception_h
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "sawtootherror.h"

namespace Sawtooth {
	class SawtoothException : public std::runtime_error {
	private:
		Sawtooth_Error_Code Error;
		
	public:
		SawtoothException(Sawtooth_Error_Code error)
			: runtime_error("sawtooth error") {
			Error = error;
		}

		virtual const Sawtooth_Error_Code GetErrorCode() const {
			return Error;
		}

		std::ostringstream Message;

		void SetErrorStruct(Sawtooth_Error* err) const {
			err->Code = GetErrorCode();
			std::string message = Message.str();
			size_t len = std::min(maxErrLen-1, message.length());
			memcpy(err->Message, message.c_str(), len);
			err->Message[len] = '\0';
		}
	};
}
#endif
