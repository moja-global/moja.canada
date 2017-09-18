#ifndef sawtooth_exception_h
#define sawtooth_exception_h
#include <stdexcept>
#include <algorithm>
#include "sawtootherror.h"
namespace Sawtooth {
	class SawtoothException : public std::runtime_error {
	private:
		Sawtooth::Sawtooth_Error_Code Error;
		const std::string Message;
	public:
		SawtoothException(Sawtooth::Sawtooth_Error_Code error, const
			std::string& message)
			: runtime_error(message), Message(message){
			Error = error;
		}

		virtual const Sawtooth::Sawtooth_Error_Code GetErrorCode() const {
			return Error;
		}

		void SetErrorStruct(Sawtooth_Error* err) const {
			err->Code = GetErrorCode();
			size_t len = std::min(maxErrLen, Message.length());
			memcpy(err->Message, Message.c_str(), len);
			err->Message[len - 1] = '\0';
		}
	};
}
#endif
