#ifndef _LIBBASE_UNCOPYABLE_HPP_
#define _LIBBASE_UNCOPYABLE_HPP_

namespace Base {

	///================================================================================== Uncopyable
	class Uncopyable {
		typedef Uncopyable this_type;

	protected:
		~Uncopyable() {
		}
		Uncopyable() {
		}

	private:
		Uncopyable(const this_type &);
		this_type & operator = (const this_type &);
	};

}

#endif