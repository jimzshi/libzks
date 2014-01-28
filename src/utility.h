#ifndef _ZKS_UTILITY_H
#define _ZKS_UTILITY_H

#include <algorithm>

namespace zks
{
	template<typename T>
	class LocalBackup {
		T old_;
		T& ref_;
	public:
		LocalBackup(T& v) : old_(std::move(v)), ref_(v) {}
		~LocalBackup() {
			ref_ = std::move(old_);
		}
	};
}

#endif // !_ZKS_UTILITY_H
