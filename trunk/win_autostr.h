﻿/**
	win_autostr
	@author		© 2010 Andrew Grechkin
	@link ()
**/
#ifndef WIN_AUTOSTR_HPP
#define WIN_AUTOSTR_HPP

template<typename Type, typename From>
class	AutoSTR {
protected:
	struct	Cont {
		size_t		m_capa;
		size_t		m_ref;
		size_t		m_size;
		Type		m_str;

		Cont(): m_capa(1), m_ref(1), m_size(0), m_str(0) {
		}
	} *m_data;

	void	delRef() {
		if (m_data && --m_data->m_ref == 0) {
			WinMem::Free(m_data);
		}
	}

	void	addRef() {
		++m_data->m_ref;
	}

	void	Init(Type in, size_t len) {
		m_data->m_size = len;
		if (in != (Type)0) {
			for (size_t i = 0; i < len; ++i) {
				*(&m_data->m_str + i) = in;
			}
		}
	}

	void	Init(const Type *in, size_t len) {
		WinMem::Copy(&m_data->m_str, in, len * sizeof(Type));
		m_data->m_size = len;
	}

	void	Alloc(size_t capa) {
		WinMem::Alloc(m_data, sizeof(*m_data) + capa * sizeof(Type));
		m_data->m_ref = 1;
		m_data->m_capa = capa;
	}

	AutoSTR(size_t capa, const AutoSTR *str = nullptr): m_data(nullptr) {
		Alloc(capa);
		if (str)
			Init(str->c_str(), str->size());
	}

	AutoSTR(const Type *str1, size_t size1, const Type *str2, size_t size2): m_data(nullptr) {
		Alloc(size1 + size2 + 1);
		if (size1)
			WinMem::Copy(&m_data->m_str, str1, size1 * sizeof(Type));
		if (size2)
			WinMem::Copy(&m_data->m_str + size1, str2, size2 * sizeof(Type));
		m_data->m_size = size1 + size2;
	}

public:
	typedef size_t size_type;
	static const	size_t	npos = (size_t) - 1;

	~AutoSTR() {
		delRef();
	}

	AutoSTR(): m_data(nullptr) {
		Alloc(1);
	}

	AutoSTR(size_t len, Type in): m_data(nullptr) {
		Alloc(len + 1);
		Init(in, len);
	}

	AutoSTR(const Type *in, size_t len = 0): m_data(nullptr) {
		if (in && len == 0)
			len = Len(in);
		Alloc(len + 1);
		Init(in, len);
	}

	AutoSTR(const AutoSTR &in): m_data(nullptr) {
		m_data = in.m_data;
		addRef();
	}

	AutoSTR(const From* in, UINT cp = DEFAULT_CHAR_CP): m_data(nullptr) {
		Alloc(Convert(in, cp));
		m_data->m_size = Convert(in, cp, buffer(), capacity()) - 1;
	}

	size_t			capacity() const {
		return	m_data->m_capa;
	}
	void			clear() {
		reserve();
		m_data->m_size = m_data->m_str = 0;
	}
	bool			empty() const {
		return	m_data->m_size == 0;
	}
	void			reserve(size_t capa = 0) {
		if (m_data->m_ref > 1) {
			AutoSTR tmp(std::max(capa, capacity()), this);
			swap(tmp);
		} else if (m_data->m_capa < capa) {
			WinMem::Realloc(m_data, sizeof(*m_data) + capa * sizeof(Type));
			m_data->m_capa = capa;
		}

	}
	size_t			size() const {
		return	m_data->m_size;
	}
	AutoSTR&		cp(const From* in, UINT cp = CP_UTF8) {
		reserve(convert(in, cp));
		convert(in, cp, buffer(), capacity());
		return	*this;
	}
	AutoSTR<From, Type>	utf8(const Type *tst = nullptr) const {
		return	AutoSTR<From, Type>(c_str(), CP_UTF8);
	}
	AutoSTR<From, Type>	utf16(const Type *tst = nullptr) const {
		return	AutoSTR<From, Type>(c_str(), CP_UTF16le);
	}

	const Type*		c_str() const {
		return	&m_data->m_str;
	}
	Type*			buffer() {
		reserve();
		return	&m_data->m_str;
	}
	void			swap(AutoSTR& in) {
		using std::swap;
		swap(m_data, in.m_data);
	}

	AutoSTR&		append(const Type *s, size_t n = npos) {
		if (n == npos)
			n = Len(s);
		reserve(size() + n + 1);
		WinMem::Copy(&m_data->m_str + size(), s, n * sizeof(Type));
		m_data->m_size += n;
		*(&m_data->m_str + size()) = (Type)0;
		return	*this;
	}
	AutoSTR&		append(const AutoSTR& str) {
		if (m_data != str.m_data) {
			append(str.c_str(), str.size());
		} else {
			AutoSTR tmp(size() + str.size() + 1, this);
			tmp.append(str.c_str(), str.size());
			swap(tmp);
		}
		return	*this;
	}
	AutoSTR&		append(const AutoSTR& str, size_t pos, size_t n = npos) {
		append(&str[pos], n);
		return	*this;
	}
	AutoSTR&		append(size_t n, Type c) {
		AutoSTR	add(n, c);
		append(add);
		return	*this;
	}

	AutoSTR& 		replace(size_t pos, size_t n1, const AutoSTR& str) {
		if (pos < size()) {
			AutoSTR	tmp(size() - n1 + str.size() + 1);
			tmp.append(&m_data->m_str, pos);
			tmp.append(str.c_str(), str.size());
			tmp.append(&m_data->m_str + (pos + n1), size() - n1 + pos);
			swap(tmp);
		}
		return	*this;
	}
	AutoSTR&		erase(size_t pos = 0, size_t n = npos) {
		if (pos < size()) {
			size_t size2 = n == npos ? size() : std::min(size(), pos + n);
			AutoSTR	tmp(c_str(), pos, c_str() + size2, size() - size2);
			swap(tmp);
		}
		return	*this;
	}
	AutoSTR			substr(size_t pos = 0, size_t n = npos) const {
		if (pos < size()) {
			if (n == npos) {
				return	AutoSTR(&m_data->m_str + pos);
			}
			return	AutoSTR(&m_data->m_str + pos, n);
		}
		return *this;
	}

	const AutoSTR&	operator=(const AutoSTR &in) {
		if (this != &in) {
			delRef();
			m_data = in.m_data;
			addRef();
		}
		return	*this;
	}
	AutoSTR&		operator+=(const AutoSTR &in) {
		append(in);
		return	*this;
	}
	AutoSTR&		operator+=(const Type *in) {
		append(in);
		return	*this;
	}
	AutoSTR&		operator+=(Type in) {
		Type tmp[] = {in, 0};
		append(tmp, 1);
		return	*this;
	}

	AutoSTR			operator+(const AutoSTR &in) const {
		AutoSTR	tmp(size() + in.size() + 1, this);
		return	tmp += in;
	}
	AutoSTR			operator+(const Type *in) const {
		AutoSTR	tmp(size() + Len(in) + 1, this);
		return	tmp += in;
	}
	AutoSTR			operator+(Type in) const {
		AutoSTR	tmp(size() + 1 + 1);
		return	tmp += in;
	}

	bool			operator==(const AutoSTR &in) const {
		return	Eq(c_str(), in.c_str());
	}
	bool			operator==(const Type *in) const {
		return	Eq(c_str(), in);
	}
	bool			operator!=(const AutoSTR &in) const {
		return	!operator==(in);
	}
	bool			operator!=(const Type *in) const {
		return	!operator==(in);
	}

	bool			operator<(const AutoSTR &in) const {
		return	Cmp(c_str(), in.c_str()) < 0;
	}
	bool			operator<(const Type *in) const {
		return	Cmp(c_str(), in) < 0;
	}
	bool			operator>(const AutoSTR &in) const {
		return	Cmp(c_str(), in.c_str()) > 0;
	}
	bool			operator>(const Type *in) const {
		return	Cmp(c_str(), in) > 0;
	}
	bool			operator<=(const AutoSTR &in) const {
		return	operator==(in) || operator<(in);
	}
	bool			operator<=(const Type *in) const {
		return	operator==(in) || operator<(in);
	}
	bool			operator>=(const AutoSTR &in) const {
		return	operator==(in) || operator>(in);
	}
	bool			operator>=(const Type *in) const {
		return	operator==(in) || operator>(in);
	}

	Type&			operator[](int in) {
		reserve();
		return	*(&m_data->m_str + in);
	}
	const Type&		operator[](int in) const {
		return	*(&m_data->m_str + in);
	}
	Type&			at(size_t in) {
		reserve();
		return	*(&m_data->m_str + in);
	}
	const Type&		at(size_t in) const {
		return	*(&m_data->m_str + in);
	}
	size_t			find(Type c, size_t p = 0) const {
		Type	what[] = {c, 0};
		return	find(what, p);
	}
	size_t			find(const AutoSTR &in, size_t p = 0) const {
		return	find(in.c_str(), p);
	}
	size_t			find(const Type *s, size_t p = 0) const {
		const Type *pos = ::Find(c_str() + std::min(p, size()), s);
		if (pos) {
			return	pos - c_str();
		}
		return	npos;
	}

	size_t			rfind(const AutoSTR &in) const {
		const Type *pos = RFind(c_str(), in.c_str());
		if (pos)
			return	pos - c_str();
		return	npos;
	}

	size_t			find_first_of(const AutoSTR &str, size_t pos = 0) const {
		size_t	Result = Span(c_str() + pos, str.c_str());
		return	(Result < size()) ? Result : npos;
	}
	size_t			find_last_of(const AutoSTR &str, size_t pos = npos) const {
		size_t	Result = Span(c_str() + pos, str.c_str());
		return	(Result < size()) ? Result : npos;
	}
	size_t			find_first_not_of(const AutoSTR &str, size_t pos = 0) const {
		for (; pos < size(); ++pos)
			if (::Find(str.c_str(), at(pos)))
				return	pos;
		return	npos;
	}
	size_t			find_last_not_of(const AutoSTR &str, size_t pos = npos) const {
		size_t	__size = size();
		if (__size) {
			if (--__size > pos)
				__size = pos;
			do {
				if (!::Find(str.c_str(), at(__size)))
					return __size;
			} while (__size--);
		}
		return	npos;
	}

	bool			Find(Type c, size_t pos = 0) const {
		return	find(c, pos) != npos;
	}
	bool			Find(const AutoSTR &sub) const {
		return	find(sub) != npos;
	}
	bool			Find(const AutoSTR &sub, size_t &pos) const {
		pos = find(sub);
		return	pos != npos;
	}
	AutoSTR&		Add(const Type add)  {
		size_t	pos = this->size() - 1;
		if (!(empty() || (at(pos) == add)))
			operator+=(add);
		return	*this;
	}
	AutoSTR&		Add(const AutoSTR &add)  {
		size_t	pos = this->size() - add.size();
		if (!(add.empty() || empty() || (rfind(add) == pos)))
			this->operator+=(add);
		return	*this;
	}
	AutoSTR&		Add(const AutoSTR &add, const AutoSTR &delim, bool chkEmpty = true) {
		size_t	pos = size() - delim.size();
		if (!(add.empty() || delim.empty() || (chkEmpty && empty()) || (rfind(delim) == pos) || (add.find(delim) == 0)))
			operator+=(delim);
		if (!add.empty())
			operator+=(add);
		return	*this;
	}
	AutoSTR&		Cut(const AutoSTR &sub) {
		size_t	pos;
		if (Find(sub, pos)) {
			erase(pos, sub.size());
		}
		return	*this;
	}
	bool			Cut(ssize_t &num, int base = 10) {
		size_t	pos1 = find_first_of(L"0123456789");
		if (pos1 == npos)
			return	false;
		size_t	pos2 = find_first_not_of(L"0123456789", pos1);
		if (pos1 > 0 && at(pos1 - 1) == L'-')
			--pos1;
		AutoSTR	tmp(substr(pos1, pos2));
		tmp.AsNum(num, base);
		erase(0, pos2);
		return	true;
	}

	bool			AsNum(size_t &num, int base = 10) const {
		PWSTR	end_ptr;
		num = ::wcstoll(c_str(), &end_ptr, base);
		return	end_ptr != c_str();
	}
	bool			AsNum(ssize_t &num, int base = 10) const {
		PWSTR	end_ptr;
		num = ::wcstoull(c_str(), &end_ptr, base);
		return	end_ptr != c_str();
	}
};

typedef AutoSTR<CHAR, WCHAR>	CStrA;
typedef AutoSTR<WCHAR, CHAR>	AutoUTF;

inline CStrA	w2cp(PCWSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	CHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}

inline AutoUTF	cp2w(PCSTR in, UINT cp) {
	size_t	size = Convert(in, cp);
	WCHAR	buf[size];
	Convert(in, cp, buf, size);
	return	buf;
}

#endif // WIN_AUTOSTR_HPP