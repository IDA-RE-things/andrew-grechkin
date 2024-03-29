﻿#ifndef _LIBBASE_SINGLETON_P_HPP_
#define _LIBBASE_SINGLETON_P_HPP_

#include <libbase/std.hpp>

namespace Base {

	template <typename T>
	struct Singleton_p : private T {
		typedef T implementation_type;
		typedef Singleton_p<T> this_type;
		static T & instance() {
			static this_type instance;
			return instance;
		}

	private:
		~Singleton_p() {
		}

		Singleton_p() {
		}
	};

}

#endif
