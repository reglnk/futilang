#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>
#include <functional>
#include <cassert>
#include <filesystem>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define SL_SKIP 2
#define SL_OK 0
#define SL_ERROR 1
#define SL_BREAK 3
#define SL_CONTINUE 4
#define SL_RETURN 5
#define SL_UNDEFINED 0xffffffff

namespace fs = std::filesystem;

class slContext;

class Variable
{
public:
	enum class Type
	{
		String,
		Bool,
		Number,
		Pointer,
		Function
	};

	enum class Rel
	{
		Equal,
		NotEqual,
		Less,
		Greater,
		Undefined
	};

	Type type;
	union {
		void *p;
		int8_t i8;
		int16_t i16;
		int32_t i32;
		int64_t i64;
		uint8_t u8;
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
		bool b;
		float f;
		double d;
	} value;

	static constexpr unsigned f_signed = 0x1000;
	static constexpr unsigned f_float = 0x2000;
	static constexpr unsigned f_flags = 0x3000;
	unsigned meta = 0u;

	Variable(): type(Type::Pointer) {
		value.p = nullptr;
	}

	Variable(const Variable &v): type(v.type), meta(v.meta)
	{
		if (type != Type::String)
			value = v.value;
		else value.p = new std::string(*reinterpret_cast<std::string *>(v.value.p));
	}

	Variable(Variable &&rv): type(rv.type), meta(rv.meta)
	{
		if (type != Type::String)
			value = rv.value;
		else {
			value.p = rv.value.p;
			rv.type = Type::Pointer;
			rv.value.p = nullptr;
		}
	}

	Variable &operator =(const Variable &v)
	{
		type = v.type;
		meta = v.meta;

		if (type != Type::String)
			value = v.value;
		else value.p = new std::string(*reinterpret_cast<std::string *>(v.value.p));

		return *this;
	}

	Variable &operator =(Variable &&rv)
	{
		type = rv.type;
		meta = rv.meta;

		if (type != Type::String)
			value = rv.value;
		else {
			value.p = rv.value.p;
			rv.type = Type::Pointer;
			rv.value.p = nullptr;
		}
		return *this;
	}

	Variable(std::string const &val, Type tp = Type::String): type(tp) {
		value.p = new std::string(val);
	}

	Variable(int16_t val, Type tp = Type::Number): type(tp) {
		value.i16 = val;
		meta = f_signed | 2;
	}

	Variable(int32_t val, Type tp = Type::Number): type(tp) {
		value.i32 = val;
		meta = f_signed | 4;
	}

	Variable(int64_t val, Type tp = Type::Number): type(tp) {
		value.i64 = val;
		meta = f_signed | 8;
	}

	Variable(uint16_t val, Type tp = Type::Number): type(tp) {
		value.u16 = val;
		meta = 2;
	}

	Variable(uint32_t val, Type tp = Type::Number): type(tp) {
		value.u32 = val;
		meta = 4;
	}

	Variable(uint64_t val, Type tp = Type::Number): type(tp) {
		value.u64 = val;
		meta = 8;
	}

	Variable(bool val, Type tp = Type::Bool): type(tp) {
		value.b = val;
	}

	Variable(float val, Type tp = Type::Number): type(tp) {
		value.f = val;
		meta = f_float | sizeof(float);
	}

	Variable(double val, Type tp = Type::Number): type(tp) {
		value.d = val;
		meta = f_float | sizeof(double);
	}

	Variable(void const *val, Type tp = Type::Pointer): type(tp) {
		value.p = const_cast<void *>(val);
	}

	~Variable() {
		if (type == Type::String) {
			delete reinterpret_cast<std::string *>(value.p);
		}
	}

	void get(bool &v) const
	{
		if (type == Type::Number)
		{
			unsigned cmeta = meta & (~f_flags);
			if (meta & f_float)
			{
				if (cmeta == sizeof(float))
					v = (bool)value.f;
				else v = (bool)value.d;
				return;
			}
			switch (cmeta)
			{
				case 1:
					v = (bool)value.u8;
					break;
				case 2:
					v = (bool)value.u16;
					break;
				case 4:
					v = (bool)value.u32;
					break;
				case 8:
					v = (bool)value.u64;
					break;
			}
		}
		else if (type == Type::Bool)
			v = value.b;
		else if (type == Type::String)
			v = value.p && reinterpret_cast<std::string *>(value.p)->size();
		else if (type == Type::Pointer)
			v = value.p;
		else v = true;
	}

	void get(uint8_t &v) const
	{
		uint64_t vv;
		get(vv);
		v = (uint8_t)vv;
	}
	void get(uint16_t &v) const
	{
		uint64_t vv;
		get(vv);
		v = (uint16_t)vv;
	}
	void get(uint32_t &v) const
	{
		uint64_t vv;
		get(vv);
		v = (uint32_t)vv;
	}
	void get(uint64_t &v) const
	{
		if (type == Type::Number || type == Type::Function)
		{
			unsigned cmeta = meta & (~f_flags);
			if (meta & f_float)
			{
				if (cmeta == sizeof(float))
					v = (uint64_t)value.f;
				else v = (uint64_t)value.d;
				return;
			}
			switch (cmeta)
			{
				default:
				case 1:
					v = value.u8;
					break;
				case 2:
					v = value.u16;
					break;
				case 4:
					v = value.u32;
					break;
				case 8:
					v = value.u64;
					break;
			}
			return;
		}
		if (type == Type::Bool) {
			v = value.b;
			return;
		}
		if (type == Type::String) {
			std::stringstream(*(std::string *)value.p) >> v;
			return;
		}
		v = 0;
	}
	void get(int8_t &v) const
	{
		int64_t vv;
		get(vv);
		v = (int8_t)vv;
	}
	void get(int16_t &v) const
	{
		int64_t vv;
		get(vv);
		v = (int16_t)vv;
	}
	void get(int32_t &v) const
	{
		int64_t vv;
		get(vv);
		v = (int32_t)vv;
	}
	void get(int64_t &v) const
	{
		if (type == Type::Number || type == Type::Function)
		{
			unsigned cmeta = meta & (~f_flags);
			if (meta & f_float)
			{
				if (cmeta == sizeof(float))
					v = (int64_t)value.f;
				else v = (int64_t)value.d;
				return;
			}
			switch (cmeta)
			{
				default:
				case 1:
					v = value.i8;
					break;
				case 2:
					v = value.i16;
					break;
				case 4:
					v = value.i32;
					break;
				case 8:
					v = value.i64;
					break;
			}
			return;
		}
		if (type == Type::Bool) {
			v = value.b;
			return;
		}
		if (type == Type::String) {
			std::stringstream(*(std::string *)value.p) >> v;
			return;
		}
		v = 0;
	}
	void get(double &v) const {
		if (type == Type::Number)
		{
			unsigned cmeta = meta & (~f_flags);
			if (meta & f_float)
			{
				if (cmeta == sizeof(float))
					v = value.f;
				else v = value.d;
				return;
			}
			switch (cmeta)
			{
				default:
				case 1:
					v = value.u8;
					break;
				case 2:
					v = value.u16;
					break;
				case 4:
					v = value.u32;
					break;
				case 8:
					v = value.u64;
					break;
			}
			return;
		}
		if (type == Type::Bool) {
			v = (int)value.b;
			return;
		}
		if (type == Type::String) {
			std::stringstream(*(std::string *)value.p) >> v;
			return;
		}
		v = 0.0;
	}
	void get(float &v) const {
		double vv;
		get(vv);
		v = vv;
	}
	void get(std::string &v) const {
		if (type == Type::String) {
			v = *reinterpret_cast<std::string *>(value.p);
		}
		else if (type == Type::Number || type == Type::Function)
		{
			unsigned cmeta = meta & (~f_flags);
			std::stringstream sst;
			if (meta & f_float)
			{
				if (cmeta == sizeof(float))
					sst << value.f;
				else sst << value.d;
			}
			else if (meta & f_signed)
			{
				switch (cmeta)
				{
					default:
					case 1:
						sst << value.i8;
						break;
					case 2:
						sst << value.i16;
						break;
					case 4:
						sst << value.i32;
						break;
					case 8:
						sst << value.i64;
						break;
				}
			}
			else
			{
				switch (cmeta)
				{
					default:
					case 1:
						sst << value.u8;
						break;
					case 2:
						sst << value.u16;
						break;
					case 4:
						sst << value.u32;
						break;
					case 8:
						sst << value.u64;
						break;
				}
			}
			v = sst.str();
		}
		else if (type == Type::Bool) {
			if (value.b)
				v = "bullshit";
			else v = "batshit";
		}
		else {
			size_t num = (size_t)(value.p);
			v = (std::stringstream() << std::hex << num).str();
		}
	}

	void get(void *&v) const {
		v = value.p;
	}

	void get(void **&v) const {
		void ***vv = &v;
		void **vp = reinterpret_cast<void **>(vv);
		get(*vp);
	}

	template<typename T>
	void get(T *&v) const {
		size_t vp = (size_t)&v;
		get(*(void **)(vp));
	}

	constexpr bool isFloat() const {
		return meta & f_float;
	}

	Rel compare(Variable const &oth) const
	{
		if (type != oth.type)
			return Rel::NotEqual;

		if (type == Type::Bool)
			return value.b == oth.value.b ? Rel::Equal : Rel::NotEqual;

		// @todo more precise comparison (uint64_t to int64_t, double to uint64_t, ...)
		// when comparing big unsigned numbers, there may be bugs

		if (type == Type::Number || type == Type::Function)
		{
			if (meta & Variable::f_float)
			{
				double v1, v2;
				get(v1);
				oth.get(v2);
				if (v1 == v2)
					return Rel::Equal;
				if (v1 < v2)
					return Rel::Less;
				if (v1 > v2)
					return Rel::Greater;
				return Rel::NotEqual;
			}

			int64_t v1, v2;
			get(v1);
			oth.get(v2);
			if (v1 == v2)
				return Rel::Equal;
			if (v1 < v2)
				return Rel::Less;
			if (v1 > v2)
				return Rel::Greater;
			return Rel::NotEqual;
		}

		if (type == Type::Pointer)
			return value.p == oth.value.p ? Rel::Equal : Rel::NotEqual;

		assert(value.p && oth.value.p);
		return *reinterpret_cast<std::string *>(value.p)
		    == *reinterpret_cast<std::string *>(oth.value.p) ?
				Rel::Equal : Rel::NotEqual;
	}
};

bool isText(char s)
{
	if (s == '_')
		return true;
	if (s >= 'a' && s <= 'z')
		return true;
	if (s >= 'A' && s <= 'Z')
		return true;
	return false;
}

bool isWhitespace(char s)
{
	return s == ' ' || s == '\t' || s == '\r' || s == '\n' || s == '\v' || s == '\f';
}

bool isDigit(char s)
{
	return s >= '0' && s <= '9';
}

bool isOper(char s)
{
	switch (s)
	{
		case '=':
		case '+':
		case '-':
		case '*':
		case '/':
		case '~':
		case '<':
		case '>':
		case '!':
		case '&':
		case '|':
		case '^':
			return true;
		default:
			return false;
	}
}

bool isNumber(std::string const &s, bool *isfloat = nullptr)
{
	bool isnumber = (bool)s.size();
	for (char ch: s) {
		if (ch == '.') {
			if (isfloat) *isfloat = true;
		}
		else if (!isDigit(ch)) {
			isnumber = false;
			break;
		}
	}
	return isnumber;
}

struct slParseData
{
	std::string part1;
	std::string part2;
	std::string part3;

	const char *beg1;
	const char *end1;
	const char *beg2;
	const char *end2;
	const char *beg3;
	const char *end3;

	bool oper;
	int action = SL_UNDEFINED;
	int metadata;

	// literal: 1
	int flags1 = 0;
	int flags3 = 0;
};

class slScopeData
{
public:
	std::unordered_map<std::string, Variable> locals;
};

class slContext
{
public:
	struct CacheEntry
	{
		std::vector<slParseData> pdata;
		int status = SL_UNDEFINED;
	};

	int line;
	fs::path mainpath;
	std::vector<std::string> code;
	std::vector<CacheEntry> codecache; // stores parsed code
	std::vector<slScopeData> callStack;
	std::vector<Variable> valueStack;
	std::unordered_map<std::string, Variable> vars;
	std::unordered_map<std::string, Variable> literals;
	std::unordered_map<std::string, int> callbacks;
	std::vector<std::function<int(slContext *)>> nativeFunctions;
	std::vector<std::string> stackTraceback;

	slContext(): line(0), code(), vars() {}

	Variable *findvar(std::string const &name)
	{
		if (vars.count(name))
			return &vars[name];

		if (callStack.size() && callStack.back().locals.count(name))
			return &callStack.back().locals[name];

		return nullptr;
	}

	// create a variable on top level of call stack
	Variable &createvar(std::string const &part)
	{
		if (!callStack.size())
			return vars[part];
		return callStack.back().locals[part];
	}

	Variable &assign(std::string const &part)
	{
		Variable *ex = findvar(part);
		if (ex)
			return *ex;
		return createvar(part);
	}

	Variable evaluate(std::string const &part)
	{
		if (!part.size())
			return Variable();

		if (part.size() >= 2 && part[0] == '"' && part[part.size() - 1] == '"') {
			char *mem = new char[part.size() - 2];
			int mempos = 0;
			for (int i = 1; i + 1 < part.size(); ++i)
			{
				char ch = part[i];
				if (ch != '\\')
					mem[mempos++] = ch;
				else {
					char sc = part[++i];
					if (sc == 'n')
						mem[mempos++] = '\n';
					else if (sc == 'r')
						mem[mempos++] = '\r';
					else if (sc == 't')
						mem[mempos++] = '\t';
					else mem[mempos++] = sc;
				}
			}
			auto vr = Variable(std::string(mem, mempos));
			delete[] mem;
			return vr;
		}

		bool isfloat = false;
		if (isNumber(part, &isfloat))
		{
			std::stringstream sst; sst << part;
			if (isfloat)
			{
				double d; sst >> d;
				return Variable(d);
			}
			int64_t n; sst >> n;
			return Variable(n);
		}
		if (part == "bullshit")
			return Variable(true);
		if (part == "batshit")
			return Variable(false);

		Variable *vrf = findvar(part);
		if (vrf)
			return *vrf;

		return Variable();
	}

	Variable *evaluate(std::string const &part, int &rtflags)
	{
		if (rtflags || literals.count(part))
			return &literals[part];

		Variable *vrf = findvar(part);
		if (vrf)
			return vrf;

		bool isfloat = false;
		if (isNumber(part, &isfloat))
		{
			rtflags = 1;
			std::stringstream sst(part);
			if (isfloat)
			{
				double d; sst >> d;
				auto &lp = literals[part] = Variable(d);
				return &lp;
			}
			int64_t n; sst >> n;
			auto &lp = literals[part] = Variable(n);
			return &lp;
		}

		if (part.size() >= 2 && part[0] == '"' && part[part.size() - 1] == '"') {
			char *mem = new char[part.size() - 2];
			int mempos = 0;
			for (int i = 1; i + 1 < part.size(); ++i)
			{
				char ch = part[i];
				if (ch != '\\')
					mem[mempos++] = ch;
				else {
					char sc = part[++i];
					if (sc == 'n')
						mem[mempos++] = '\n';
					else if (sc == 'r')
						mem[mempos++] = '\r';
					else if (sc == 't')
						mem[mempos++] = '\t';
					else mem[mempos++] = sc;
				}
			}
			auto &lp = literals[part] = Variable(std::string(mem, mempos));
			delete[] mem;
			rtflags = 1;
			return &lp;
		}
		if (part == "bullshit") {
			auto &lp = literals[part] = Variable(true);
			rtflags = 1;
			return &lp;
		}
		if (part == "batshit") {
			auto &lp = literals[part] = Variable(false);
			rtflags = 1;
			return &lp;
		}
		return nullptr;
	}
};

int slDoFile(slContext &slCtx, const char *filename, bool fullpath = true);

std::string &slPushError(slContext &slCtx, const char *text, int pos, int line) {
	slCtx.stackTraceback.push_back (
		(std::stringstream() << "at " << line + 1 << " (" << pos <<
		"): " << text).str()
	);
	auto &bk = slCtx.stackTraceback.back();
	return bk;
};

int slParseBlock(const char *buf, const char *end, slParseData &dat)
{
	if (buf == end)
		return SL_SKIP;

	dat.beg1 = buf;
	while (dat.beg1 != end && isWhitespace(*dat.beg1))
		++dat.beg1;
	if (*dat.beg1 == '#')
		return SL_SKIP;

	dat.end1 = dat.beg1;
	if (*dat.beg1 == '"') {
		++dat.end1;
		while (dat.end1 != end && *dat.end1 != '"')
			++dat.end1;
		if (dat.end1 == end)
			return SL_ERROR;
		++dat.end1;
	} else {
		while (dat.end1 != end && !isOper(*dat.end1) && !isWhitespace(*dat.end1))
			++dat.end1;
	}
	if (dat.beg1 == dat.end1)
		return SL_SKIP;

	dat.part1 = std::string(dat.beg1, dat.end1 - dat.beg1);

	dat.beg2 = dat.end1;
	while (dat.beg2 != end && isWhitespace(*dat.beg2))
		++dat.beg2;

	dat.oper = false;
	dat.end2 = dat.beg2;
	if (isOper(*dat.beg2)) {
		dat.oper = true; // @fixme: part2 can be oper and number at same time
		while (dat.end2 != end && isOper(*dat.end2))
			++dat.end2;
	} else if (*dat.beg2 == '"') {
		++dat.end2;
		while (dat.end2 != end && *dat.end2 != '"')
			++dat.end2;
		if (dat.end2 == end)
			return SL_ERROR;
		++dat.end2;
	} else {
		while (dat.end2 != end && !isWhitespace(*dat.end2))
			++dat.end2;
	}
	dat.part2 = std::string(dat.beg2, dat.end2 - dat.beg2);

	dat.part3 = "";
	dat.beg3 = dat.end2;
	while (dat.beg3 != end && isWhitespace(*dat.beg3))
		++dat.beg3;
	dat.end3 = dat.beg3;
	if (*dat.beg3 == '"') {
		++dat.end3;
		while (dat.end3 != end && *dat.end3 != '"')
			++dat.end3;
		if (dat.end3 == end)
			return SL_ERROR;
		++dat.end3;
	} else {
		while (dat.end3 != end && !isWhitespace(*dat.end3))
			++dat.end3;
	}
	if (dat.beg3 != dat.end3)
		dat.part3 = std::string(dat.beg3, dat.end3 - dat.beg3);

	return SL_OK;
}

int slParseString(slContext &slCtx, int lineNum, std::vector<slParseData> *&dat)
{
	auto &ccache = slCtx.codecache;
	auto &ccode = slCtx.code;
	if (ccache.size() != ccode.size())
		ccache.resize(ccode.size());

	auto &entry = ccache[lineNum];
	dat = &entry.pdata;
	if (entry.status != SL_UNDEFINED)
		return entry.status;

	const char *buf = slCtx.code[lineNum].c_str();

	bool quotes = false;
	const char *end = buf;
	while (*end && (quotes || *end != '#')) {
		if (*end == '"')
			quotes = !quotes;
		++end;
	}
	if (buf == end) {
		entry.status = SL_SKIP;
		return entry.status;
	}

	const char *prev = end;

	for (const char *iter = end - 1; iter >= buf; --iter)
	{
		if (*iter == '"')
			quotes = !quotes;

		if (iter == buf || (*iter == ';' && !quotes))
		{
			slParseData pdata;
			int res = slParseBlock(iter + (*iter == ';'), prev, pdata);
			if (res == SL_OK)
				dat->push_back(std::move(pdata));
			else if (res != SL_SKIP) {
				entry.status = res;
				return res;
			}
			prev = iter;
		}
	}
	entry.status = SL_OK;
	return entry.status;
}

int slPreloadString(slContext &slCtx, int lineNum, int *change = nullptr)
{
	std::vector<slParseData> *dat;
	int pars = slParseString(slCtx, lineNum, dat);
	if (pars == SL_ERROR)
		return SL_ERROR;

	if (!dat->size()) {
		if (change) *change = lineNum + 1;
		return SL_OK;
	}

	auto &p = dat->back();
	if (p.oper)
	{
		if (p.part2 == "~")
		{
			// put the line number into new variable
			if (!p.part3.size())
				slCtx.vars[p.part1] = Variable (lineNum + 1);
		}
	}

	if (change) *change = lineNum + 1;
	return SL_OK;
}

int slDoString(slContext &slCtx, int lineNum, int *change = nullptr);

int slCallFunction(slContext &slCtx, int newln)
{
	slScopeData scdat{};
	slCtx.callStack.push_back(std::move(scdat));

	int sr;
	for (int ln = newln - 1; ln < slCtx.code.size(); )
	{
		int next;
		sr = slDoString(slCtx, ln, &next);
		if (sr == SL_ERROR || sr == SL_RETURN)
			break;
		ln = next;
	}
	slCtx.callStack.pop_back();
	return sr;
}

int slCallFunc (
	slContext &slCtx,
	Variable const &v,
	const char *buf,
	int lineNum,
	slParseData const &p,
	int *change = nullptr
) {
	auto gv = slCtx.evaluate(p.part2);

	if (gv.type == Variable::Type::Function) {
		size_t fn_id;
		gv.get(fn_id);
		if (fn_id >= slCtx.nativeFunctions.size()) {
			slPushError(slCtx, "invalid native function", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}
		int res = slCtx.nativeFunctions[fn_id](&slCtx);
		if (res != SL_OK) {
			slPushError(slCtx, (std::string("calling native function ") + p.part2).c_str(), p.beg2 - buf, lineNum);
		}
		return res;
	}

	if (gv.type != Variable::Type::Number) {
		slPushError(slCtx, "invalid type", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	int newln;
	gv.get(newln);
	if (newln == 0) {
		slPushError(slCtx, "invalid label", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	int res = slCallFunction(slCtx, newln);
	if (res != SL_OK)
		return res == SL_RETURN ? SL_OK : res;

	return SL_OK;
}

template<typename T>
inline int s_ExecOperFA(T &l, const Variable &vr, int type)
{
	T r;
	vr.get(r);

	switch (type)
	{
		case 0:
			l += r;
			break;
		case 1:
			l -= r;
			break;
		case 2:
			l *= r;
			break;
		case 3:
			l /= r;
			break;
		default:
			return SL_ERROR;
	}
	return SL_OK;
}

template<typename T>
inline int s_ExecOperA(T &l, const Variable &vr, int type)
{
	T r;
	vr.get(r);

	switch (type)
	{
		default:
		case 0:
			l += r;
			break;
		case 1:
			l -= r;
			break;
		case 2:
			l *= r;
			break;
		case 3:
			l /= r;
			break;
		case 4:
			l |= r;
			break;
		case 5:
			l = l || r;
			break;
		case 6:
			l &= r;
			break;
		case 7:
			l = l && r;
			break;
		case 8:
			l ^= r;
			break;
	}
	return SL_OK;
}

static int execOper(Variable &left, Variable const &right, int type, const char **errDesc)
{
	int res = SL_OK;
	if (left.type == Variable::Type::Number)
	{
		switch (left.meta)
		{
			default:
			case 8:
				res = s_ExecOperA(left.value.u64, right, type);
				break;
			case 4:
				res = s_ExecOperA(left.value.u32, right, type);
				break;
			case 2:
				res = s_ExecOperA(left.value.u16, right, type);
				break;
			case 1:
				res = s_ExecOperA(left.value.u8, right, type);
				break;
			case 8 | Variable::f_signed:
				res = s_ExecOperA(left.value.i64, right, type);
				break;
			case 4 | Variable::f_signed:
				res = s_ExecOperA(left.value.i32, right, type);
				break;
			case 2 | Variable::f_signed:
				res = s_ExecOperA(left.value.i16, right, type);
				break;
			case 1 | Variable::f_signed:
				res = s_ExecOperA(left.value.i8, right, type);
				break;
			case Variable::f_float | sizeof(float):
				res = s_ExecOperFA(left.value.f, right, type);
			case Variable::f_float | sizeof(double):
				res = s_ExecOperFA(left.value.d, right, type);
		}
		if (res) {
			*errDesc = "unsupported operand type";
			return SL_ERROR;
		}
	}
	else if (left.type == Variable::Type::Bool)
	{
		bool &gf = left.value.b;
		bool gs;
		right.get(gs);

		switch (type)
		{
			case 4:
			case 5:
				gf |= gs;
				break;
			case 6:
			case 7:
				gf &= gs;
				break;
			case 8:
				gf ^= gs;
				break;
			default:
				*errDesc = "unsupported operand type";
				return SL_ERROR;
		}
	}
	else if (left.type == Variable::Type::Pointer)
	{
		Variable **gfv = reinterpret_cast<Variable **>(&left.value.p);

		if (type == 0) {
			if (right.type != Variable::Type::Number) {
				*errDesc = "unsupported operand type";
				return SL_ERROR;
			}
			int64_t gs;
			right.get(gs);
			*gfv += gs;
		}
		else if (type == 1) {
			if (right.type == Variable::Type::Number) {
				int gs;
				right.get(gs);
				*gfv -= gs;
			}
			else if (right.type == Variable::Type::Pointer) {
				Variable *gsv = reinterpret_cast<Variable *>(right.value.p);
				int64_t diff = (int64_t)(*gfv - gsv);
				left = Variable(diff);
				return SL_ERROR;
			}
			else {
				*errDesc = "unsupported operand type";
				return SL_ERROR;
			}
		}
		else {
			*errDesc = "unsupported operand type";
			return SL_ERROR;
		}
	}
	else if (left.type == Variable::Type::String)
	{
		if (right.type != Variable::Type::String) {
			*errDesc = "unsupported operand type";
			return SL_ERROR;
		}
		std::string &s1 = *reinterpret_cast<std::string *>(left.value.p);
		std::string &s2 = *reinterpret_cast<std::string *>(right.value.p);

		if (type == 0)
			s1 += s2;
		else if (type == 1) {
			if (s1.size() < s2.size()) {
				*errDesc = "substraction of longer string from another";
				return SL_ERROR;
			}
			s1.resize(s1.size() - s2.size());
		}
		else {
			*errDesc = "unsupported operand type";
			return SL_ERROR;
		}
	}
	else {
		*errDesc = "unsupported operand type";
		return SL_ERROR;
	}
	return res;
}

typedef int (*slBlockExecFn)(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change);

// ok
int sl_action0(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &right = slCtx.assign(p.part1);
	if (p.part3.size()) {
		Variable *vrf = slCtx.evaluate(p.part3, p.flags3);
		if (vrf != nullptr)
			right = *vrf;
		else return SL_ERROR;
	}
	else if (slCtx.valueStack.size()) {
		right = std::move(slCtx.valueStack.back());
		slCtx.valueStack.pop_back();
	}
	return SL_OK;
}

// ok
int sl_action1(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	// put the line number into new variable
	slCtx.assign(p.part1) = std::move(Variable(lineNum + 1));
	return SL_OK;
}

// in
int sl_action2(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	std::string inp;
	std::cin >> inp;
	slCtx.assign(p.part3) = Variable(inp);
	return SL_OK;
}

// out
int sl_action3(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	auto gv = slCtx.evaluate(p.part3);
	std::string outp;
	gv.get(outp);
	std::cout << outp;
	return SL_OK;
}

// split
int sl_action4(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type != Variable::Type::String) {
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "string required", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}
	std::string vstr;
	vref.get(vstr);
	Variable *arr = new Variable[vstr.size()];
	for (size_t i = 0; i < vstr.size(); ++i)
		arr[i] = Variable(std::string(&vstr[i], 1));

	Variable res((void *)arr);
	slCtx.valueStack.push_back(std::move(res));
	return SL_OK;
}

// length
int sl_action5(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type != Variable::Type::String) {
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "string required", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}
	std::string vstr;
	vref.get(vstr);
	Variable res(vstr.size());
	slCtx.valueStack.push_back(std::move(res));
	return SL_OK;
}

// type
int sl_action6(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);

	const char *resp =
		vref.type == Variable::Type::Pointer ? "pointer" :
		vref.type == Variable::Type::Number ? "number" :
		vref.type == Variable::Type::String ? "string" :
		vref.type == Variable::Type::Bool ? "bool" :
		vref.type == Variable::Type::Function ? "function" :
		"undefined";

	auto res = Variable(std::string(resp));
	slCtx.valueStack.push_back(std::move(res));
	return SL_OK;
}

// bool
int sl_action7(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type == Variable::Type::Bool)
		return SL_OK;

	bool v;
	vref.get(v);
	vref = Variable(v);
	return SL_OK;
}

// number
int sl_action8(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type == Variable::Type::Number)
		return SL_OK;

	if (vref.type == Variable::Type::String)
	{
		std::string st;
		vref.get(st);
		std::stringstream sst {st};
		if (st.find('.') != std::string::npos ||
			st.find('e') != std::string::npos
		) {
			double vd;
			sst >> vd;
			vref = Variable(vd);
		}
		else
		{
			int64_t vi;
			sst >> vi;
			vref = Variable(vi);
		}
	}
	else if (vref.type == Variable::Type::Pointer)
	{
		vref.type = Variable::Type::Number;
		vref.meta = sizeof(size_t);
	}
	else {
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "cannot convert to number", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}
	return SL_OK;
}

// string
int sl_action9(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type == Variable::Type::String)
		return SL_OK;

	std::string st;
	vref.get(st);
	vref = Variable(st);
	return SL_OK;
}

// pointer
int sl_action10(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type == Variable::Type::Pointer)
		return SL_OK;

	if (vref.type == Variable::Type::Number)
	{
		size_t p;
		vref.get(p);
		vref.type = Variable::Type::Pointer;
		vref.value.p = (void *)p;
	}
	else {
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "cannot convert to pointer", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}
	return SL_OK;
}

// function
int sl_action11(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &vref = slCtx.assign(p.part3);
	if (vref.type == Variable::Type::Function)
		return SL_OK;

	if (vref.type == Variable::Type::Number)
	{
		uint64_t fn_num;
		vref.get(fn_num);
		vref = Variable(fn_num, Variable::Type::Function);
		return SL_OK;
	}

	const char *buf = slCtx.code[lineNum].c_str();
	slPushError(slCtx, "cannot convert to number", p.beg3 - buf, lineNum);
	return SL_ERROR;
}

int sl_action12(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	Variable &left = slCtx.assign(p.part1);

	Variable right;
	if (p.part3.size())
		right = slCtx.evaluate(p.part3);
	else if (slCtx.valueStack.size()) {
		right = slCtx.valueStack.back();
		slCtx.valueStack.pop_back();
	}
	else {
		slPushError(slCtx, "stack is empty", p.beg2 - buf, lineNum);
		return SL_ERROR;
	}

	if (right.type != Variable::Type::Pointer) {
		slPushError(slCtx, "pointer required", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}

	void *vr;
	right.get(vr);
	left = *reinterpret_cast<Variable *>(vr);
	return SL_OK;
}

int sl_action13(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	Variable left = slCtx.evaluate(p.part1);

	Variable right;
	if (p.part3.size())
		right = slCtx.evaluate(p.part3);
	else if (slCtx.valueStack.size()) {
		right = slCtx.valueStack.back();
		slCtx.valueStack.pop_back();
	}
	else {
		slPushError(slCtx, "stack is empty", p.beg2 - buf, lineNum);
		return SL_ERROR;
	}

	if (right.type != Variable::Type::Pointer) {
		slPushError(slCtx, "pointer required", p.beg3 - buf, lineNum);
		return SL_ERROR;
	}

	void *vr;
	right.get(vr);
	*reinterpret_cast<Variable *>(vr) = left;
	return SL_OK;
}

// comparison operators
int sl_action14(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable *left = slCtx.evaluate(p.part1, p.flags1);
	Variable *right = slCtx.evaluate(p.part3, p.flags3);
	if (left == nullptr || right == nullptr)
		return SL_ERROR;

	auto rel = left->compare(*right);

	bool result;
	switch (subaction)
	{
		default:
		case 0: result = rel == Variable::Rel::Equal; break;
		case 1: result = rel != Variable::Rel::Equal; break;
		case 2: result = rel == Variable::Rel::Less; break;
		case 3: result = rel == Variable::Rel::Greater; break;
		case 4: result = rel == Variable::Rel::Less || rel == Variable::Rel::Equal; break;
		case 5: result = rel == Variable::Rel::Greater || rel == Variable::Rel::Equal; break;
	}

	slCtx.valueStack.emplace_back(result);
	return SL_OK;
}

// arith operators
int sl_action15(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	Variable left = std::move(slCtx.evaluate(p.part1));

	const char *errMsg {""};
	int res;
	if (p.part3.size())
	{
		Variable *right = slCtx.evaluate(p.part3, p.flags3);
		res = execOper(left, *right, subaction, &errMsg);
	}
	else if (slCtx.valueStack.size())
	{
		Variable &right = slCtx.valueStack.back();
		res = execOper(left, right, subaction, &errMsg);
		slCtx.valueStack.pop_back();
	}
	else {
		slPushError(slCtx, "stack is empty", p.beg2 - buf, lineNum);
		return SL_ERROR;
	}
	if (res != SL_OK) {
		slPushError(slCtx, errMsg, p.beg1 - buf, lineNum);
		return res;
	}

	slCtx.valueStack.push_back(std::move(left));
	return res;
}

// assignment operators
int sl_action16(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	Variable *lft = slCtx.findvar(p.part1);

	if (!lft) {
		slPushError(slCtx, "undefined variable ", p.beg1 - buf, lineNum) += p.part1;
		return SL_ERROR;
	}
	Variable &left = *lft;

	const char *errMsg {""};
	int res;
	if (p.part3.size())
	{
		Variable *right = slCtx.evaluate(p.part3, p.flags3);
		res = execOper(left, *right, subaction, &errMsg);
	}
	else if (slCtx.valueStack.size())
	{
		Variable &right = slCtx.valueStack.back();
		res = execOper(left, right, subaction, &errMsg);
		slCtx.valueStack.pop_back();
	}
	else {
		slPushError(slCtx, "stack is empty", p.beg2 - buf, lineNum);
		return SL_ERROR;
	}
	if (res != SL_OK)
		slPushError(slCtx, errMsg, p.beg1 - buf, lineNum);

	return res;
}

// ===================

int sl_action17(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	int l;
	if (p.flags1)
		l = p.flags1;
	else
	{
		int lvl = 1;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			std::vector<slParseData> *pd;
			int lr = slParseString(slCtx, l, pd);
			if (lr == SL_ERROR)
				return lr;

			bool brk = false;
			for (auto const &p: *pd)
			{
				// @todo optimize: pick ready slParseData from slCtx instead of this
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "while" || p.part1 == "if" || p.part1 == "try")
					++lvl;
				if (!lvl) {
					brk = true;
					break;
				}
			}
			if (brk) break;
		}
		if (lvl) {
			slPushError(slCtx, "unclosed 'while'", p.beg1 - buf, l);
			return SL_ERROR;
		}
		p.flags1 = l;
	}

	bool ev = true;
	if (slCtx.valueStack.size()) {
		slCtx.valueStack.back().get(ev);
		slCtx.valueStack.pop_back();
	}
	if (p.part2 == "!")
		ev = !ev;

	if (!ev) {
		if (change) *change = l + 1;
		return SL_OK;
	}

	bool brk = false;
	for (int ll = lineNum + 1; ll < l; )
	{
		int lr = slDoString(slCtx, ll, &ll);

		if (lr == SL_BREAK) {
			brk = true;
			break;
		}
		if (lr == SL_ERROR)
			return SL_ERROR;
		if (lr == SL_RETURN)
			return SL_RETURN;
		if (lr == SL_CONTINUE)
			break;
	}

	if (change) *change = brk ? l + 1 : lineNum;
	return SL_OK;
}

int sl_action18(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	return SL_BREAK;
}

int sl_action19(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	return SL_CONTINUE;
}

int sl_action20(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	return SL_RETURN;
}

int sl_action21(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();

	// @todo test
	Variable v = slCtx.evaluate(p.part2);
	if (v.type != Variable::Type::Pointer)
	{
		slPushError(slCtx, "pointer required", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	if (slCtx.valueStack.size() < 1)
	{
		slPushError(slCtx, "stack is empty", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}
	Variable &len = slCtx.valueStack.back();
	if (len.type != Variable::Type::Number)
	{
		slPushError(slCtx, "number required", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	size_t arrlen;
	len.get(arrlen);
	Variable *ptr;
	v.get(ptr);

	// [0]: allocation function
	// [1]: size
	// [2]: constructor function
	// [3]: destructor function
	int res = slCallFunc(slCtx, ptr[0], buf, lineNum, p, change);
	if (res != SL_OK)
		return SL_ERROR;

	size_t objsize;
	ptr[1].get(objsize);
	char *pptr;

	Variable &obj = slCtx.valueStack.back();
	obj.get(pptr);

	for (size_t i = 0; i != arrlen; ++i)
	{
		Variable pt((void *)(pptr + i * objsize));
		slCtx.valueStack.push_back(std::move(pt));
		int rr = slCallFunc(slCtx, ptr[2], buf, lineNum, p, change);
		if (res != SL_OK)
			return SL_ERROR;
	}
	return SL_OK;
}

int sl_action22(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	if (p.part2.size()) {
		Variable v = slCtx.evaluate(p.part2);
		int vv;
		v.get(vv);
		int stacksize = slCtx.valueStack.size();
		if (vv + 1 > stacksize || vv < 0)
		{
			const char *buf = slCtx.code[lineNum].c_str();
			slPushError(slCtx, "invalid stack index", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}
		slCtx.valueStack.push_back(slCtx.valueStack[stacksize - 1 - vv]);
	}
	else slCtx.valueStack.clear();
	return SL_OK;
}

int sl_action23(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();

	int lvl = 1;
	int l;
	int sep = 0;
	for (l = lineNum + 1; l != slCtx.code.size(); ++l)
	{
		std::vector<slParseData> *pd;
		int lr = slParseString(slCtx, l, pd);
		if (lr == SL_ERROR)
			return lr;

		bool brk = false;
		for (auto const &p: *pd)
		{
			if (p.part1 == "end")
				--lvl;
			else if (p.part1 == "if" || p.part1 == "while" || p.part1 == "try")
				++lvl;
			else if (!sep && p.part1 == "else")
				sep = l;
			if (!lvl) {
				brk = true;
				break;
			}
		}
		if (brk)
			break;
	}
	if (lvl) {
		slPushError(slCtx, "unclosed 'if'", p.beg1 - buf, l);
		return SL_ERROR;
	}

	bool ev = true;
	if (slCtx.valueStack.size()) {
		slCtx.valueStack.back().get(ev);
		slCtx.valueStack.pop_back();
	}
	if (p.part2 == "!")
		ev = !ev;

	if (ev)
	{
		int stopl = sep ? sep : l;
		for (int ll = lineNum + 1; ll < stopl; )
		{
			int lr = slDoString(slCtx, ll, &ll);
			if (lr == SL_ERROR)
				return SL_ERROR;
			if (lr == SL_BREAK)
				return SL_BREAK;
			if (lr == SL_CONTINUE)
				return SL_CONTINUE;
			if (lr == SL_RETURN)
				return SL_RETURN;
		}
	}
	else if (sep)
	{
		for (int ll = sep + 1; ll < l; )
		{
			int lr = slDoString(slCtx, ll, &ll);
			if (lr == SL_ERROR)
				return SL_ERROR;
			if (lr == SL_BREAK)
				return SL_BREAK;
			if (lr == SL_CONTINUE)
				return SL_CONTINUE;
			if (lr == SL_RETURN)
				return SL_RETURN;
		}
	}
	if (change) *change = l + 1;
	return SL_OK;
}

int sl_action24(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	if (slCtx.valueStack.size() < 2)
	{
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "not enough stack size", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	bool v1, v2;
	slCtx.valueStack.back().get(v1);
	slCtx.valueStack.pop_back();
	slCtx.valueStack.back().get(v2);
	slCtx.valueStack.pop_back();

	bool res = subaction ? (v1 && v2) : (v1 || v2);
	Variable v(res);
	slCtx.valueStack.push_back(std::move(v));

	return SL_OK;
}

int sl_action25(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	auto gv = slCtx.evaluate(p.part2);
	int newln;
	gv.get(newln);
	if (newln == 0) {
		const char *buf = slCtx.code[lineNum].c_str();
		slPushError(slCtx, "invalid label", p.beg1 - buf, lineNum);
		return SL_ERROR;
	}

	if (change) *change = newln - 1;
	return SL_OK;
}

int sl_action26(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	// @todo remove extra argument buf
	const char *buf = slCtx.code[lineNum].c_str();

	int res = slCallFunc(slCtx, slCtx.evaluate(p.part2), buf, lineNum, p);
	return res == SL_RETURN ? SL_OK : res;
}

// try - catch
int sl_action27(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();

	int l;
	int lvl = 1;
	int sep = 0; // position of catch
	for (l = lineNum + 1; l != slCtx.code.size(); ++l)
	{
		std::vector<slParseData> *pd;
		int lr = slParseString(slCtx, l, pd);
		if (lr == SL_ERROR)
			return lr;

		bool brk = false;
		for (auto const &p: *pd)
		{
			if (p.part1 == "end")
				--lvl;
			else if (p.part1 == "if" || p.part1 == "while" || p.part1 == "try")
				++lvl;
			else if (!sep && p.part1 == "catch")
				sep = l;
			if (!lvl) {
				brk = true;
				break;
			}
		}
		if (brk)
			break;
	}
	if (lvl) {
		slPushError(slCtx, "unclosed 'try'", p.beg1 - buf, l);
		return SL_ERROR;
	}
	if (!sep) {
		slPushError(slCtx, "missing 'catch' block", p.beg1 - buf, l);
		return SL_ERROR;
	}

	// execute 'try' block
	bool ev = false;
	for (int ll = lineNum + 1; ll < sep; )
	{
		int lr = slDoString(slCtx, ll, &ll);
		if (lr == SL_ERROR) {
			ev = true;
			slCtx.stackTraceback.clear();
			break;
		}
		else {
			if (lr == SL_BREAK)
				return SL_BREAK;
			if (lr == SL_CONTINUE)
				return SL_CONTINUE;
			if (lr == SL_RETURN)
				return SL_RETURN;
		}
	}

	// catch block
	if (ev)
	{
		for (int ll = sep + 1; ll < l; )
		{
			int lr = slDoString(slCtx, ll, &ll);
			if (lr == SL_ERROR)
				return SL_ERROR;
			if (lr == SL_BREAK)
				return SL_BREAK;
			if (lr == SL_CONTINUE)
				return SL_CONTINUE;
			if (lr == SL_RETURN)
				return SL_RETURN;
		}
	}
	if (change) *change = l + 1;
	return SL_OK;
}

int sl_action28(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &left = slCtx.vars[p.part2];
	int arrlen;
	slCtx.evaluate(p.part3).get(arrlen);

	if (arrlen)
	{
		Variable *arr = new Variable[arrlen];
		left = Variable((void *)arr);
	}
	return SL_OK;
}

int sl_action29(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable &left = slCtx.vars[p.part2];
	void *arr;
	left.get(arr);
	delete[] reinterpret_cast<Variable *>(arr);
	return SL_OK;
}

int sl_action30(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable filename = slCtx.evaluate(p.part2);
	std::string fname;
	filename.get(fname);
	int res = slDoFile(slCtx, fname.c_str(), false);
	return res == SL_RETURN ? SL_OK : res;
}

int sl_action31(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	Variable filename = slCtx.evaluate(p.part2);
	std::string e;
	filename.get(e);
	slPushError(slCtx, e.c_str(), p.beg2 - buf, lineNum);
	return SL_ERROR;
}

int sl_action32(slContext &slCtx, int lineNum, slParseData &p, int subaction, int *change)
{
	Variable v = slCtx.evaluate(p.part1);
	slCtx.valueStack.push_back(std::move(v));
	return SL_OK;
}

static slBlockExecFn sl_actions[] = {
	sl_action0,
	sl_action1,
	sl_action2,
	sl_action3,
	sl_action4,
	sl_action5,
	sl_action6,
	sl_action7,
	sl_action8,
	sl_action9,
	sl_action10,
	sl_action11,
	sl_action12,
	sl_action13,
	sl_action14,
	sl_action15,
	sl_action16,
	sl_action17,
	sl_action18,
	sl_action19,
	sl_action20,
	sl_action21,
	sl_action22,
	sl_action23,
	sl_action24,
	sl_action25,
	sl_action26,
	sl_action27,
	sl_action28,
	sl_action29,
	sl_action30,
	sl_action31,
	sl_action32
};

int slDoBlock(slContext &slCtx, int lineNum, slParseData &p, int *change = nullptr)
{
	const char *buf = slCtx.code[lineNum].c_str();

	if (p.action == SL_UNDEFINED)
	{
		int action;
		int subaction;

		if (p.oper && p.part2 != "!")
		{
			if (p.part2 == "=") action = 0;
			else if (p.part2 == "~")
			{
				if (!p.part3.size()) action = 1;
				else if (p.part1 == "in") action = 2;
				else if (p.part1 == "out") action = 3;
				else if (p.part1 == "split") action = 4;
				else if (p.part1 == "length") action = 5;
				else if (p.part1 == "type") action = 6;
				else {
					if (p.part1 == "bool") action = 7;
					else if (p.part1 == "number") action = 8;
					else if (p.part1 == "string") action = 9;
					else if (p.part1 == "pointer") action = 10;
					else if (p.part1 == "function") action = 11;
					else {
						slPushError(slCtx, "invalid type", p.beg1 - buf, lineNum);
						return SL_ERROR;
					}
				}
			}
			else if (p.part2 == "<-") action = 12;
			else if (p.part2 == "->") action = 13;
			else if (
				p.part2 == "==" ||
				p.part2 == "!=" ||
				p.part2 == "<" ||
				p.part2 == ">" ||
				p.part2 == "<=" ||
				p.part2 == ">="
			) {
				action = 14;
				subaction =
					p.part2 == "==" ? 0 :
					p.part2 == "!=" ? 1 :
					p.part2 == "<" ? 2 :
					p.part2 == ">" ? 3 :
					p.part2 == "<=" ? 4 : 5;
			}
			else if (
				p.part2 == "+" ||
				p.part2 == "-" ||
				p.part2 == "*" ||
				p.part2 == "/" ||
				p.part2 == "|" ||
				p.part2 == "||" ||
				p.part2 == "&" ||
				p.part2 == "&&" ||
				p.part2 == "^"
			) {
				action = 15;
				subaction =
					p.part2 == "+" ? 0 :
					p.part2 == "-" ? 1 :
					p.part2 == "*" ? 2 :
					p.part2 == "/" ? 3 :
					p.part2 == "|" ? 4 :
					p.part2 == "||" ? 5 :
					p.part2 == "&" ? 6 :
					p.part2 == "&&" ? 7 : 8;
			}
			else if (
				p.part2 == "+=" ||
				p.part2 == "-=" ||
				p.part2 == "*=" ||
				p.part2 == "/=" ||
				p.part2 == "|=" ||
				p.part2 == "||=" ||
				p.part2 == "&=" ||
				p.part2 == "&&=" ||
				p.part2 == "^="
			) {
				action = 16;
				subaction =
					p.part2 == "+=" ? 0 :
					p.part2 == "-=" ? 1 :
					p.part2 == "*=" ? 2 :
					p.part2 == "/=" ? 3 :
					p.part2 == "|=" ? 4 :
					p.part2 == "||=" ? 5 :
					p.part2 == "&=" ? 6 :
					p.part2 == "&&=" ? 7 : 8;
			}
			else {
				slPushError(slCtx, "invalid operator", p.beg2 - buf, lineNum);
				return SL_ERROR;
			}
		}
		else if (p.part1 == "while") action = 17;
		else if (p.part1 == "break") action = 18;
		else if (p.part1 == "continue") action = 19;
		else if (p.part1 == "return") action = 20;
		else if (p.part1 == "new") action = 21;
		else if (p.part1 == "stack") action = 22;
		else if (p.part1 == "if") action = 23;
		else if (p.part1 == "or" || p.part1 == "and") {
			action = 24;
			subaction = p.part1 == "and";
		}
		else if (p.part1 == "goto") action = 25;
		else if (p.part1 == "call") action = 26;
		else if (p.part1 == "try") action = 27;
		else if (p.part1 == "alloc") action = 28;
		else if (p.part1 == "free") action = 29;
		else if (p.part1 == "require") action = 30;
		else if (p.part1 == "throw") action = 31;
		else if (p.part1.size()) action = 32;
		else {
			slPushError(slCtx, "syntax error", 0, lineNum);
			return SL_ERROR;
		}

		p.action = action;
		p.metadata = subaction;
	}

	return sl_actions[p.action](slCtx, lineNum, p, p.metadata, change);
/*
	if (p.oper && p.part2 != "!")
	{
		if (p.part2 == "=")
		{
			// action 0
			Variable &right = slCtx.assign(p.part1);
			if (p.part3.size())
				right = std::move(slCtx.evaluate(p.part3));
			else if (slCtx.valueStack.size()) {
				right = slCtx.valueStack.back();
				slCtx.valueStack.pop_back();
			}
		}
		else if (p.part2 == "~")
		{
			if (!p.part3.size())
			{
				// put the line number into new variable
				slCtx.assign(p.part1) = Variable(lineNum + 1);
			}
			else if (p.part1 == "in")
			{
				std::string inp;
				std::cin >> inp;
				slCtx.assign(p.part3) = Variable(inp);
			}
			else if (p.part1 == "out")
			{
				auto gv = slCtx.evaluate(p.part3);
				std::string outp;
				gv.get(outp);
				std::cout << outp;
			}
			else if (p.part1 == "split")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					slPushError(slCtx, "string required", p.beg3 - buf, lineNum);
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				Variable *arr = new Variable[vstr.size()];
				for (size_t i = 0; i < vstr.size(); ++i)
					arr[i] = Variable(std::string(&vstr[i], 1));
				
				Variable res((void *)arr);
				slCtx.valueStack.push_back(std::move(res));
			}
			else if (p.part1 == "length")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					slPushError(slCtx, "string required", p.beg3 - buf, lineNum);
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				Variable res(vstr.size());
				slCtx.valueStack.push_back(std::move(res));
			}
			else if (p.part1 == "type")
			{
				Variable &vref = slCtx.assign(p.part3);
				
				const char *resp =
					vref.type == Variable::Type::Pointer ? "pointer" :
					vref.type == Variable::Type::Number ? "number" :
					vref.type == Variable::Type::String ? "string" :
					vref.type == Variable::Type::Bool ? "bool" :
					vref.type == Variable::Type::Function ? "function" :
					"undefined";

				auto res = Variable(std::string(resp));
				slCtx.valueStack.push_back(std::move(res));
			}
			else
			{
				Variable &vref = slCtx.assign(p.part3);
				if (p.part1 == "bool")
				{
					if (vref.type == Variable::Type::Bool)
						return SL_OK;

					bool v;
					vref.get(v);
					vref = Variable(v);
				}
				else if (p.part1 == "number")
				{
					if (vref.type == Variable::Type::Number)
						return SL_OK;

					if (vref.type == Variable::Type::String)
					{
						std::string st;
						vref.get(st);
						std::stringstream sst {st};
						if (st.find('.') != std::string::npos ||
							st.find('e') != std::string::npos
						) {
							double vd;
							sst >> vd;
							vref = Variable(vd);
						}
						else
						{
							int64_t vi;
							sst >> vi;
							vref = Variable(vi);
						}
					}
					else if (vref.type == Variable::Type::Pointer)
					{
						vref.type = Variable::Type::Number;
						vref.meta = sizeof(size_t);
					}
					else {
						slPushError(slCtx, "cannot convert to number", p.beg3 - buf, lineNum);
						return SL_ERROR;
					}
				}
				else if (p.part1 == "string")
				{
					if (vref.type == Variable::Type::String)
						return SL_OK;

					std::string st;
					vref.get(st);
					vref = Variable(st);
				}
				else if (p.part1 == "pointer")
				{
					if (vref.type == Variable::Type::Pointer)
						return SL_OK;

					if (vref.type == Variable::Type::Number)
					{
						size_t p;
						vref.get(p);
						vref.type = Variable::Type::Pointer;
						vref.value.p = (void *)p;
					}
					else {
						slPushError(slCtx, "cannot convert to pointer", p.beg3 - buf, lineNum);
						return SL_ERROR;
					}
				}
				else if (p.part1 == "function")
				{
					if (vref.type == Variable::Type::Function)
						return SL_OK;

					if (vref.type == Variable::Type::Number)
					{
						uint64_t fn_num;
						vref.get(fn_num);
						vref = Variable(fn_num, Variable::Type::Function);
						return SL_OK;
					}
					slPushError(slCtx, "cannot convert to number", p.beg3 - buf, lineNum);
					return SL_ERROR;
				}
				else {
					slPushError(slCtx, "invalid type", p.beg1 - buf, lineNum);
					return SL_ERROR;
				}
			}
		}
		else if (p.part2 == "<-")
		{
			Variable &left = slCtx.assign(p.part1);
			
			Variable right;
			if (p.part3.size())
				right = slCtx.evaluate(p.part3);
			else if (slCtx.valueStack.size()) {
				right = slCtx.valueStack.back();
				slCtx.valueStack.pop_back();
			}
			else {
				errStackEmpty(p.beg2 - buf, lineNum);
				return SL_ERROR;
			}

			if (right.type != Variable::Type::Pointer) {
				slPushError(slCtx, "pointer required", p.beg3 - buf, lineNum);
				return SL_ERROR;
			}

			void *vr;
			right.get(vr);
			left = *reinterpret_cast<Variable *>(vr);
		}
		else if (p.part2 == "->")
		{
			Variable left = slCtx.evaluate(p.part1);
			
			Variable right;
			if (p.part3.size())
				right = slCtx.evaluate(p.part3);
			else if (slCtx.valueStack.size()) {
				right = slCtx.valueStack.back();
				slCtx.valueStack.pop_back();
			}
			else {
				errStackEmpty(p.beg2 - buf, lineNum);
				return SL_ERROR;
			}

			if (right.type != Variable::Type::Pointer) {
				slPushError(slCtx, "pointer required", p.beg3 - buf, lineNum);
				return SL_ERROR;
			}

			void *vr;
			right.get(vr);
			*reinterpret_cast<Variable *>(vr) = left;
		}
		else if (p.part2 == "==" || p.part2 == "!=" || p.part2 == "<" || p.part2 == ">" || p.part2 == "<=" || p.part2 == ">=")
		{
			Variable left = slCtx.evaluate(p.part1);
			Variable right = slCtx.evaluate(p.part3);
			auto rel = left.compare(right);

			bool result;
			if (p.part2 == "==")
				result = rel == Variable::Rel::Equal;
			else if (p.part2 == "!=")
				result = rel == Variable::Rel::NotEqual;
			else if (p.part2 == "<")
				result = rel == Variable::Rel::Less;
			else if (p.part2 == ">")
				result = rel == Variable::Rel::Greater;
			else if (p.part2 == "<=")
				result = rel == Variable::Rel::Less || rel == Variable::Rel::Equal;
			else result = rel == Variable::Rel::Greater || rel == Variable::Rel::Equal;
			
			Variable v(result);
			slCtx.valueStack.push_back(std::move(v));
		}
		else if (
			p.part2 == "+" ||
			p.part2 == "-" ||
			p.part2 == "*" ||
			p.part2 == "/" ||
			p.part2 == "|" ||
			p.part2 == "||" ||
			p.part2 == "&" ||
			p.part2 == "&&" ||
			p.part2 == "^"
		) {
			Variable left = slCtx.evaluate(p.part1);
			Variable right;
			if (p.part3.size())
				right = slCtx.evaluate(p.part3);
			else if (slCtx.valueStack.size()) {
				right = slCtx.valueStack.back();
				slCtx.valueStack.pop_back();
			}
			else {
				errStackEmpty(p.beg2 - buf, lineNum);
				return SL_ERROR;
			}
			
			int type =
				p.part2 == "+" ? 0 :
				p.part2 == "-" ? 1 :
				p.part2 == "*" ? 2 :
				p.part2 == "/" ? 3 :
				p.part2 == "|" ? 4 :
				p.part2 == "||" ? 5 :
				p.part2 == "&" ? 6 :
				p.part2 == "&&" ? 7 :
				8;

			const char *errMsg {""};
			int res = execOper(left, right, type, &errMsg);
			if (res != SL_OK) {
				slPushError(slCtx, errMsg, p.beg1 - buf);
				return res;
			}
			slCtx.valueStack.push_back(std::move(left));
			return SL_OK;
		}
		else if (
			p.part2 == "+=" ||
			p.part2 == "-=" ||
			p.part2 == "*=" ||
			p.part2 == "/=" ||
			p.part2 == "|=" ||
			p.part2 == "||=" ||
			p.part2 == "&=" ||
			p.part2 == "&&=" ||
			p.part2 == "^="
		) {
			Variable right = slCtx.evaluate(p.part3);
			Variable *lft = slCtx.findvar(p.part1);
			if (!lft) {
				slPushError(slCtx, "undefined variable ", p.beg1 - buf) += p.part1;
				return SL_ERROR;
			}
			Variable &left = *lft;

			int type =
				p.part2 == "+=" ? 0 :
				p.part2 == "-=" ? 1 :
				p.part2 == "*=" ? 2 :
				p.part2 == "/=" ? 3 :
				p.part2 == "|=" ? 4 :
				p.part2 == "||=" ? 5 :
				p.part2 == "&=" ? 6 :
				p.part2 == "&&=" ? 7 :
				8;

			const char *errMsg {""};
			int res = execOper(left, right, type, &errMsg);
			if (res != SL_OK) {
				slPushError(slCtx, errMsg, p.beg1 - buf);
				return res;
			}
			return SL_OK;
		}
		else {
			slPushError(slCtx, "invalid operator", p.beg2 - buf, lineNum);
			return SL_ERROR;
		}
	}
	else if (p.part1 == "while")
	{
		int lvl = 1;
		int l;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			std::vector<slParseData> *pd;
			int lr = slParseString(slCtx, l, pd);
			if (lr == SL_ERROR)
				return lr;

			bool brk = false;
			for (auto const &p: *pd)
			{
				// @todo optimize: pick ready slParseData from slCtx instead of this
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "while" || p.part1 == "if" || p.part1 == "try")
					++lvl;
				if (!lvl) {
					brk = true;
					break;
				}
			}
			if (brk) break;
		}
		if (lvl) {
			slPushError(slCtx, "unclosed 'while'", p.beg1 - buf, l);
			return SL_ERROR;
		}

		bool ev = true;
		if (slCtx.valueStack.size()) {
			slCtx.valueStack.back().get(ev);
			slCtx.valueStack.pop_back();
		}
		if (p.part2 == "!")
			ev = !ev;

		if (!ev) {
			if (change) *change = l + 1;
			return SL_OK;
		}

		bool brk = false;
		for (int ll = lineNum + 1; ll < l; )
		{
			int lr = slDoString(slCtx, ll, &ll);
			if (lr == SL_ERROR)
				return SL_ERROR;
			if (lr == SL_BREAK) {
				brk = true;
				break;
			}
			if (lr == SL_CONTINUE)
				break;
			if (lr == SL_RETURN)
				return SL_RETURN;
		}

		if (change) *change = brk ? l + 1 : lineNum;
		return SL_OK;
	}
	else if (p.part1 == "break")
	{
		return SL_BREAK;
	}
	else if (p.part1 == "continue")
	{
		return SL_CONTINUE;
	}
	else if (p.part1 == "return")
	{
		return SL_RETURN;
	}
	else if (p.part1 == "new")
	{
		// @todo test
		Variable v = slCtx.evaluate(p.part2);
		if (v.type != Variable::Type::Pointer)
		{
			slPushError(slCtx, "pointer required", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}

		if (slCtx.valueStack.size() < 1)
		{
			slPushError(slCtx, "stack is empty", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}
		Variable &len = slCtx.valueStack.back();
		if (len.type != Variable::Type::Number)
		{
			slPushError(slCtx, "number required", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}

		size_t arrlen;
		len.get(arrlen);
		Variable *ptr;
		v.get(ptr);

		// [0]: allocation function
		// [1]: size
		// [2]: constructor function
		// [3]: destructor function
		int res = slCallFunc(slCtx, ptr[0], buf, lineNum, p, change);
		if (res != SL_OK)
			return SL_ERROR;

		size_t objsize;
		ptr[1].get(objsize);
		char *pptr;

		Variable &obj = slCtx.valueStack.back();
		obj.get(pptr);

		for (size_t i = 0; i != arrlen; ++i)
		{
			Variable pt((void *)(pptr + i * objsize));
			slCtx.valueStack.push_back(std::move(pt));
			int rr = slCallFunc(slCtx, ptr[2], buf, lineNum, p, change);
			if (res != SL_OK)
				return SL_ERROR;
		}
		return SL_OK;
	}
	else if (p.part1 == "stack")
	{
		if (p.part2.size()) {
			Variable v = slCtx.evaluate(p.part2);
			int vv;
			v.get(vv);
			int stacksize = slCtx.valueStack.size();
			if (vv + 1 > stacksize || vv < 0)
			{
				slPushError(slCtx, "invalid stack index", p.beg1 - buf, lineNum);
				return SL_ERROR;
			}
			slCtx.valueStack.push_back(slCtx.valueStack[stacksize - 1 - vv]);
		}
		else slCtx.valueStack.clear();
	}
	else if (p.part1 == "if")
	{
		int lvl = 1;
		int l;
		int sep = 0;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			std::vector<slParseData> *pd;
			int lr = slParseString(slCtx, l, pd);
			if (lr == SL_ERROR)
				return lr;

			bool brk = false;
			for (auto const &p: *pd)
			{
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "if" || p.part1 == "while" || p.part1 == "try")
					++lvl;
				else if (!sep && p.part1 == "else")
					sep = l;
				if (!lvl) {
					brk = true;
					break;
				}
			}
			if (brk)
				break;
		}
		if (lvl) {
			slPushError(slCtx, "unclosed 'if'", p.beg1 - buf, l);
			return SL_ERROR;
		}

		bool ev = true;
		if (slCtx.valueStack.size()) {
			slCtx.valueStack.back().get(ev);
			slCtx.valueStack.pop_back();
		}
		if (p.part2 == "!")
			ev = !ev;
		
		if (ev)
		{
			int stopl = sep ? sep : l;
			for (int ll = lineNum + 1; ll < stopl; )
			{
				int lr = slDoString(slCtx, ll, &ll);
				if (lr == SL_ERROR)
					return SL_ERROR;
				if (lr == SL_BREAK)
					return SL_BREAK;
				if (lr == SL_CONTINUE)
					return SL_CONTINUE;
				if (lr == SL_RETURN)
					return SL_RETURN;
			}
		}
		else if (sep)
		{
			for (int ll = sep + 1; ll < l; )
			{
				int lr = slDoString(slCtx, ll, &ll);
				if (lr == SL_ERROR)
					return SL_ERROR;
				if (lr == SL_BREAK)
					return SL_BREAK;
				if (lr == SL_CONTINUE)
					return SL_CONTINUE;
				if (lr == SL_RETURN)
					return SL_RETURN;
			}
		}
		if (change) *change = l + 1;
		return SL_OK;
	}
	else if (p.part1 == "or" || p.part1 == "and")
	{
		if (slCtx.valueStack.size() < 2)
		{
			slPushError(slCtx, "not enough stack size", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}

		bool v1, v2;
		slCtx.valueStack.back().get(v1);
		slCtx.valueStack.pop_back();
		slCtx.valueStack.back().get(v2);
		slCtx.valueStack.pop_back();

		bool res = (p.part1 == "or") ? (v1 || v2) : (v1 && v2);
		Variable v(res);
		slCtx.valueStack.push_back(std::move(v));

		return SL_OK;
	}
	else if (p.part1 == "goto")
	{
		auto gv = slCtx.evaluate(p.part2);
		int newln;
		gv.get(newln);
		if (newln == 0) {
			slPushError(slCtx, "invalid label", p.beg1 - buf, lineNum);
			return SL_ERROR;
		}

		if (change) *change = newln - 1;
		return SL_OK;
	}
	else if (p.part1 == "call")
	{
		int res = slCallFunc(slCtx, slCtx.evaluate(p.part2), buf, lineNum, p);
		return res == SL_RETURN ? SL_OK : res;
	}
	else if (p.part1 == "try")
	{
		int l;
		int lvl = 1;
		int sep = 0; // position of catch
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			std::vector<slParseData> *pd;
			int lr = slParseString(slCtx, l, pd);
			if (lr == SL_ERROR)
				return lr;

			bool brk = false;
			for (auto const &p: *pd)
			{
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "if" || p.part1 == "while" || p.part1 == "try")
					++lvl;
				else if (!sep && p.part1 == "catch")
					sep = l;
				if (!lvl) {
					brk = true;
					break;
				}
			}
			if (brk)
				break;
		}
		if (lvl) {
			slPushError(slCtx, "unclosed 'try'", p.beg1 - buf, l);
			return SL_ERROR;
		}
		if (!sep) {
			slPushError(slCtx, "missing 'catch' block", p.beg1 - buf, l);
			return SL_ERROR;
		}

		// execute 'try' block
		bool ev = false;
		for (int ll = lineNum + 1; ll < sep; )
		{
			int lr = slDoString(slCtx, ll, &ll);
			if (lr == SL_ERROR) {
				ev = true;
				slCtx.stackTraceback.clear();
				break;
			}
			else {
				if (lr == SL_BREAK)
					return SL_BREAK;
				if (lr == SL_CONTINUE)
					return SL_CONTINUE;
				if (lr == SL_RETURN)
					return SL_RETURN;
			}
		}

		// catch block
		if (ev)
		{
			for (int ll = sep + 1; ll < l; )
			{
				int lr = slDoString(slCtx, ll, &ll);
				if (lr == SL_ERROR)
					return SL_ERROR;
				if (lr == SL_BREAK)
					return SL_BREAK;
				if (lr == SL_CONTINUE)
					return SL_CONTINUE;
				if (lr == SL_RETURN)
					return SL_RETURN;
			}
		}
		if (change) *change = l + 1;
		return SL_OK;
	}
	else if (p.part1 == "alloc")
	{
		Variable &left = slCtx.vars[p.part2];
		int arrlen;
		slCtx.evaluate(p.part3).get(arrlen);

		if (arrlen)
		{
			Variable *arr = new Variable[arrlen];
			left = Variable((void *)arr);
		}
	}
	else if (p.part1 == "free")
	{
		Variable &left = slCtx.vars[p.part2];
		void *arr;
		left.get(arr);
		delete[] reinterpret_cast<Variable *>(arr);
	}
	else if (p.part1 == "require")
	{
		Variable filename = slCtx.evaluate(p.part2);
		std::string fname;
		filename.get(fname);
		int res = slDoFile(slCtx, fname.c_str(), false);
		return res == SL_RETURN ? SL_OK : res;
	}
	else if (p.part1 == "throw")
	{
		Variable filename = slCtx.evaluate(p.part2);
		std::string e;
		filename.get(e);
		slPushError(slCtx, e.c_str(), p.beg2 - buf, lineNum);
		return SL_ERROR;
	}
	else if (p.part1.size())
	{
		Variable v = slCtx.evaluate(p.part1);
		slCtx.valueStack.push_back(std::move(v));
	}
	else {
		slPushError(slCtx, "syntax error", 0, lineNum);
		return SL_ERROR;
	}
	return SL_OK;
	*/
}

int slDoString(slContext &slCtx, int lineNum, int *change)
{
	std::vector<slParseData> *p;
	int pars = slParseString(slCtx, lineNum, p);
	if (pars == SL_ERROR)
		return pars;
	
	size_t psize = p->size();
	for (size_t i = 0u; i != psize; ++i)
	{
		// update possibly invalidated pointer
		p = &slCtx.codecache[lineNum].pdata;

		int chg = lineNum + 1;
		int res = slDoBlock(slCtx, lineNum, (*p)[i], &chg);
		if (res == SL_SKIP)
			continue;
		if (res != SL_OK)
			return res;

		if (chg != lineNum + 1) {
			if (i + 1 == psize) {
				if (change) *change = chg;
				return SL_OK;
			}
			else {
				slCtx.stackTraceback.push_back (
					(std::stringstream() << "at " << lineNum + 1 <<
					": [core] slDoString - only last statement may change the line").str()
				);
				return SL_ERROR;
			}
		}
	}
	
	if (change) *change = lineNum + 1;
	return SL_OK;
}

void loadlibs(slContext &ctx)
{
	ctx.vars["malloc"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		size_t v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		void *mem = ::malloc(v1);

		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["free"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		void *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		::free(v1);

		return SL_OK;
	});

	ctx.vars["memcpy"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		void *v1, *v2;
		size_t v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		::memcpy(v1, v2, v3);
		return SL_OK;
	});

	ctx.vars["strlen"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		auto v = ctx->valueStack.back();
		ctx->valueStack.pop_back();

		Variable res;
		if (v.type == Variable::Type::String)
			res = Variable(reinterpret_cast<std::string *>(v.value.p)->size());
		else if (v.type == Variable::Type::Pointer) {
			char *s;
			v.get(s);
			res = Variable(::strlen(s));
		}

		ctx->valueStack.push_back(std::move(res));
		return SL_OK;
	});

	ctx.vars["to_cstr"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		auto v = ctx->valueStack.back();
		ctx->valueStack.pop_back();

		if (v.type != Variable::Type::String)
			return SL_ERROR;

		std::string *stp = reinterpret_cast<std::string *>(v.value.p);
		size_t slen = stp->size();
		int8_t *cstr = new int8_t[slen + 1];
		cstr[slen] = '\0';
		std::memcpy(cstr, stp->c_str(), slen);

		Variable res(cstr);
		ctx->valueStack.push_back(std::move(res));

		return SL_OK;
	});

	ctx.vars["to_string"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		Variable res;
		if (ctx->valueStack.back().type == Variable::Type::Number)
		{
			int8_t ch[2];
			ctx->valueStack.back().get(ch[0]);
			ctx->valueStack.pop_back();
			res = Variable(std::string((const char *)&ch[0]));
		}
		else
		{
			if (ctx->valueStack.size() < 1)
				return SL_ERROR;

			char const *ptr;
			size_t slen;
			ctx->valueStack.back().get(ptr);
			ctx->valueStack.pop_back();
			ctx->valueStack.back().get(slen);
			ctx->valueStack.pop_back();
		}
		ctx->valueStack.push_back(std::move(res));
		return SL_OK;
	});

	ctx.vars["SEEK_SET"] = Variable((int)SEEK_SET);
	ctx.vars["SEEK_END"] = Variable((int)SEEK_END);
	ctx.vars["SEEK_CUR"] = Variable((int)SEEK_CUR);

	ctx.vars["fopen"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		auto source = ctx->valueStack.back();
		ctx->valueStack.pop_back();
		auto modes = ctx->valueStack.back();
		ctx->valueStack.pop_back();

		if (source.type != Variable::Type::String || modes.type != source.type)
			return SL_ERROR;

		std::string *p1 = reinterpret_cast<std::string *>(source.value.p);
		std::string *p2 = reinterpret_cast<std::string *>(modes.value.p);
		FILE *f = fopen(p1->c_str(), p2->c_str());
		ctx->valueStack.push_back(Variable(f));
		return SL_OK;
	});

	ctx.vars["fclose"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		FILE *f;
		ctx->valueStack.back().get((void *&)f);
		ctx->valueStack.pop_back();

		fclose(f);
		return SL_OK;
	});

	ctx.vars["fseek"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		FILE *f;
		int off;
		int w;
		ctx->valueStack.back().get((void *&)f);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(off);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(w);
		ctx->valueStack.pop_back();

		int res = fseek(f, off, w);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["ftell"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		FILE *fp;
		ctx->valueStack.back().get((void *&)fp);
		ctx->valueStack.pop_back();

		long res = ftell(fp);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["fread"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		void *mem;
		size_t objsize;
		size_t nmemb;
		FILE *fp;
		ctx->valueStack.back().get(mem);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(objsize);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(nmemb);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get((void *&)fp);
		ctx->valueStack.pop_back();

		size_t res = fread(mem, objsize, nmemb, fp);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["fwrite"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		void *mem;
		size_t objsize;
		size_t nmemb;
		FILE *fp;
		ctx->valueStack.back().get(mem);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(objsize);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(nmemb);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get((void *&)fp);
		ctx->valueStack.pop_back();

		size_t res = fwrite(mem, objsize, nmemb, fp);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["newi8"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		int8_t *mem = new int8_t[v1];
		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["deli8"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int8_t *v1;
		ctx->valueStack.back().get((void *&)v1);
		ctx->valueStack.pop_back();

		delete[] v1;
		return SL_OK;
	});

	ctx.vars["readi8"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int8_t *v1;
		int v2;
		ctx->valueStack.back().get((void *&)v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		int8_t val = v1[v2];
		ctx->valueStack.push_back(Variable(val));
		return SL_OK;
	});

	ctx.vars["writei8"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		int8_t *v1;
		int v2;
		int8_t v3;
		ctx->valueStack.back().get((void *&)v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		v1[v2] = v3;
		return SL_OK;
	});

	ctx.vars["newi32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		int32_t *mem = new int32_t[v1];
		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["deli32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int32_t *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		delete[] v1;
		return SL_OK;
	});

	ctx.vars["readi32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int32_t *v1;
		int v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		int32_t val = v1[v2];
		ctx->valueStack.push_back(Variable(val));
		return SL_OK;
	});

	ctx.vars["writei32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		int32_t *v1;
		int v2;
		int32_t v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		v1[v2] = v3;
		return SL_OK;
	});

	ctx.vars["newi64"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		int64_t *mem = new int64_t[v1];
		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["deli64"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int64_t *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		delete[] v1;
		return SL_OK;
	});

	ctx.vars["readi64"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int64_t *v1;
		int v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		int64_t val = v1[v2];
		ctx->valueStack.push_back(Variable(val));
		return SL_OK;
	});

	ctx.vars["writei64"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		int64_t *v1;
		int v2;
		int64_t v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		v1[v2] = v3;
		return SL_OK;
	});

	ctx.vars["newvp"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		void **mem = new void*[v1];
		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["delvp"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		void **v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		delete[] v1;
		return SL_OK;
	});

	ctx.vars["readvp"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		void **v1;
		int v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		void *val = v1[v2];
		ctx->valueStack.push_back(Variable(val));
		return SL_OK;
	});

	ctx.vars["writevp"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		void **v1;
		int v2;
		void *v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		v1[v2] = v3;
		return SL_OK;
	});

	ctx.vars["newf32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		float *mem = new float[v1];
		ctx->valueStack.push_back(Variable(mem));
		return SL_OK;
	});

	ctx.vars["delf32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		float *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		delete[] v1;
		return SL_OK;
	});

	ctx.vars["readf32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		float *v1;
		int v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		float val = v1[v2];
		ctx->valueStack.push_back(Variable(val));
		return SL_OK;
	});

	ctx.vars["writef32"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		float *v1;
		int v2;
		float v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		v1[v2] = v3;
		return SL_OK;
	});

	ctx.vars["sin"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		double v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		Variable res(std::sin(v1));
		ctx->valueStack.push_back(std::move(res));
		return SL_OK;
	});

	ctx.vars["cos"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		double v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		Variable res(std::cos(v1));
		ctx->valueStack.push_back(std::move(res));
		return SL_OK;
	});

	ctx.vars["sqrt"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		double v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		Variable res(std::sqrt(v1));
		ctx->valueStack.push_back(std::move(res));
		return SL_OK;
	});

	ctx.vars["GLFW_CONTEXT_VERSION_MAJOR"] = Variable((int)GLFW_CONTEXT_VERSION_MAJOR);
	ctx.vars["GLFW_CONTEXT_VERSION_MINOR"] = Variable((int)GLFW_CONTEXT_VERSION_MINOR);
	ctx.vars["GLFW_OPENGL_PROFILE"] = Variable((int)GLFW_OPENGL_PROFILE);
	ctx.vars["GLFW_OPENGL_COMPAT_PROFILE"] = Variable((int)GLFW_OPENGL_COMPAT_PROFILE);
	ctx.vars["GLFW_OPENGL_CORE_PROFILE"] = Variable((int)GLFW_OPENGL_CORE_PROFILE);
	ctx.vars["GLFW_REFRESH_RATE"] = Variable((int)GLFW_REFRESH_RATE);
	ctx.vars["GLFW_CURSOR"] = Variable((int)GLFW_CURSOR);
	ctx.vars["GLFW_CURSOR_NORMAL"] = Variable((int)GLFW_CURSOR_NORMAL);
	ctx.vars["GLFW_CURSOR_DISABLED"] = Variable((int)GLFW_CURSOR_DISABLED);

	ctx.vars["GLFW_PRESS"] = Variable((int)GLFW_PRESS);
	ctx.vars["GLFW_RELEASE"] = Variable((int)GLFW_RELEASE);

	ctx.vars["GLFW_KEY_A"] = Variable((int)GLFW_KEY_A);
	ctx.vars["GLFW_KEY_B"] = Variable((int)GLFW_KEY_B);
	ctx.vars["GLFW_KEY_C"] = Variable((int)GLFW_KEY_C);
	ctx.vars["GLFW_KEY_D"] = Variable((int)GLFW_KEY_D);
	ctx.vars["GLFW_KEY_E"] = Variable((int)GLFW_KEY_E);
	ctx.vars["GLFW_KEY_F"] = Variable((int)GLFW_KEY_F);
	ctx.vars["GLFW_KEY_G"] = Variable((int)GLFW_KEY_G);
	ctx.vars["GLFW_KEY_H"] = Variable((int)GLFW_KEY_H);
	ctx.vars["GLFW_KEY_I"] = Variable((int)GLFW_KEY_I);
	ctx.vars["GLFW_KEY_J"] = Variable((int)GLFW_KEY_J);
	ctx.vars["GLFW_KEY_K"] = Variable((int)GLFW_KEY_K);
	ctx.vars["GLFW_KEY_L"] = Variable((int)GLFW_KEY_L);
	ctx.vars["GLFW_KEY_M"] = Variable((int)GLFW_KEY_M);
	ctx.vars["GLFW_KEY_N"] = Variable((int)GLFW_KEY_N);
	ctx.vars["GLFW_KEY_O"] = Variable((int)GLFW_KEY_O);
	ctx.vars["GLFW_KEY_P"] = Variable((int)GLFW_KEY_P);
	ctx.vars["GLFW_KEY_Q"] = Variable((int)GLFW_KEY_Q);
	ctx.vars["GLFW_KEY_R"] = Variable((int)GLFW_KEY_R);
	ctx.vars["GLFW_KEY_S"] = Variable((int)GLFW_KEY_S);
	ctx.vars["GLFW_KEY_T"] = Variable((int)GLFW_KEY_T);
	ctx.vars["GLFW_KEY_U"] = Variable((int)GLFW_KEY_U);
	ctx.vars["GLFW_KEY_V"] = Variable((int)GLFW_KEY_V);
	ctx.vars["GLFW_KEY_W"] = Variable((int)GLFW_KEY_W);
	ctx.vars["GLFW_KEY_X"] = Variable((int)GLFW_KEY_X);
	ctx.vars["GLFW_KEY_Y"] = Variable((int)GLFW_KEY_Y);
	ctx.vars["GLFW_KEY_Z"] = Variable((int)GLFW_KEY_Z);

	ctx.vars["GLFW_KEY_ESCAPE"] = Variable((int)GLFW_KEY_ESCAPE);
	ctx.vars["GLFW_KEY_LEFT_ALT"] = Variable((int)GLFW_KEY_LEFT_ALT);
	ctx.vars["GLFW_KEY_RIGHT_ALT"] = Variable((int)GLFW_KEY_RIGHT_ALT);
	ctx.vars["GLFW_KEY_LEFT_SHIFT"] = Variable((int)GLFW_KEY_LEFT_SHIFT);
	ctx.vars["GLFW_KEY_RIGHT_SHIFT"] = Variable((int)GLFW_KEY_RIGHT_SHIFT);
	ctx.vars["GLFW_KEY_LEFT_CONTROL"] = Variable((int)GLFW_KEY_LEFT_CONTROL);
	ctx.vars["GLFW_KEY_RIGHT_CONTROL"] = Variable((int)GLFW_KEY_RIGHT_CONTROL);

	ctx.vars["glfwInit"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		int res = glfwInit();
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["glfwTerminate"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		glfwTerminate();
		return SL_OK;
	});

	ctx.vars["glfwWindowHint"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int v1, v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		glfwWindowHint(v1, v2);
		return SL_OK;
	});

	ctx.vars["glfwCreateWindow"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		int v1, v2;
		std::string v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		GLFWwindow *wnd = glfwCreateWindow(v1, v2, v3.c_str(), NULL, NULL);
		if (wnd != NULL)
			glfwSetWindowUserPointer(wnd, ctx);

		ctx->valueStack.push_back(Variable(wnd));
		return SL_OK;
	});

	ctx.vars["glfwDestroyWindow"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		void *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		glfwDestroyWindow((GLFWwindow *)v1);

		return SL_OK;
	});

	ctx.vars["glfwMakeContextCurrent"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLFWwindow *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		glfwMakeContextCurrent(v1);

		return SL_OK;
	});

	ctx.vars["glfwSetKeyCallback"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		void *v1;
		int callbk;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(callbk);
		ctx->valueStack.pop_back();
		ctx->callbacks["GLFWkeyfun"] = callbk;

		auto kbcallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			auto slctx = reinterpret_cast<slContext *>(glfwGetWindowUserPointer(window));
			slctx->valueStack.push_back(Variable(mods));
			slctx->valueStack.push_back(Variable(action));
			slctx->valueStack.push_back(Variable(scancode));
			slctx->valueStack.push_back(Variable(key));
			slctx->valueStack.push_back(Variable(window));
			slCallFunction(*slctx, slctx->callbacks["GLFWkeyfun"]);
		};
		glfwSetKeyCallback((GLFWwindow *)v1, kbcallback);
		return SL_OK;
	});

	ctx.vars["glfwSetFramebufferSizeCallback"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		void *v1;
		int callbk;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(callbk);
		ctx->valueStack.pop_back();
		ctx->callbacks["GLFWfbsizefun"] = callbk;

		auto fbcallback = [](GLFWwindow* window, int width, int height)
		{
			auto slctx = reinterpret_cast<slContext *>(glfwGetWindowUserPointer(window));
			slctx->valueStack.push_back(Variable(height));
			slctx->valueStack.push_back(Variable(width));
			slctx->valueStack.push_back(Variable(window));
			slCallFunction(*slctx, slctx->callbacks["GLFWfbsizefun"]);
		};
		glfwSetFramebufferSizeCallback((GLFWwindow *)v1, fbcallback);
		return SL_OK;
	});

	ctx.vars["glfwSwapInterval"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		glfwSwapInterval(v1);

		return SL_OK;
	});

	ctx.vars["glfwWindowShouldClose"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		void *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		bool s = glfwWindowShouldClose((GLFWwindow *)v1);
		ctx->valueStack.push_back(Variable(s));

		return SL_OK;
	});

	ctx.vars["glfwGetTime"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx)
	{
		double time = glfwGetTime();
		ctx->valueStack.push_back(Variable((float)time));
		return SL_OK;
	});

	ctx.vars["glfwGetKey"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx)
	{
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		GLFWwindow *v1;
		int v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		int res = glfwGetKey(v1, v2);
		ctx->valueStack.push_back(Variable(res));

		return SL_OK;
	});

	ctx.vars["glfwPollEvents"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx)
	{
		glfwPollEvents();
		return SL_OK;
	});

	ctx.vars["glfwSwapBuffers"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx)
	{
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLFWwindow *v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		glfwSwapBuffers(v1);
		return SL_OK;
	});

	ctx.vars["glfwSetInputMode"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		void *v1;
		int v2, v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();
		glfwSetInputMode((GLFWwindow *)v1, v2, v3);

		return SL_OK;
	});

	ctx.vars["GL_FLOAT"] = Variable(GL_FLOAT);
	ctx.vars["GL_UNSIGNED_INT"] = Variable(GL_UNSIGNED_INT);
	ctx.vars["GL_FALSE"] = Variable(GL_FALSE);
	ctx.vars["GL_RED"] = Variable(GL_RED);
	ctx.vars["GL_RGB"] = Variable(GL_RGB);
	ctx.vars["GL_RGBA"] = Variable(GL_RGBA);
	ctx.vars["GL_ARRAY_BUFFER"] = Variable(GL_ARRAY_BUFFER);
	ctx.vars["GL_ELEMENT_ARRAY_BUFFER"] = Variable(GL_ELEMENT_ARRAY_BUFFER);
	ctx.vars["GL_STATIC_DRAW"] = Variable(GL_STATIC_DRAW);
	ctx.vars["GL_DYNAMIC_DRAW"] = Variable(GL_DYNAMIC_DRAW);
	ctx.vars["GL_TRIANGLES"] = Variable(GL_TRIANGLES);
	ctx.vars["GL_TRIANGLE_STRIP"] = Variable(GL_TRIANGLE_STRIP);
	ctx.vars["GL_COLOR_BUFFER_BIT"] = Variable(GL_COLOR_BUFFER_BIT);
	ctx.vars["GL_DEPTH_BUFFER_BIT"] = Variable(GL_DEPTH_BUFFER_BIT);

	ctx.vars["GL_VERTEX_SHADER"] = Variable(GL_VERTEX_SHADER);
	ctx.vars["GL_FRAGMENT_SHADER"] = Variable(GL_FRAGMENT_SHADER);
	ctx.vars["GL_GEOMETRY_SHADER"] = Variable(GL_GEOMETRY_SHADER);

	ctx.vars["GL_COMPILE_STATUS"] = Variable(GL_COMPILE_STATUS);

	ctx.vars["gladLoadGL"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		int res = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["gladLoaderLoadGL"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		gladLoaderLoadGL();
		return SL_OK;
	});

	ctx.vars["glClear"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		uint64_t mask;
		ctx->valueStack.back().get(mask);
		ctx->valueStack.pop_back();

		glClear((GLbitfield)mask);
		return SL_OK;
	});

	ctx.vars["glClearColor"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		float v1, v2, v3, v4;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v4);
		ctx->valueStack.pop_back();

		glClearColor(v1, v2, v3, v4);
		return SL_OK;
	});

	ctx.vars["glGenVertexArrays"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int v1;
		GLuint *v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		glGenVertexArrays(v1, v2);
		return SL_OK;
	});

	ctx.vars["glGenBuffers"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int v1;
		GLuint *v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get((void *&)v2);
		ctx->valueStack.pop_back();

		glGenBuffers(v1, v2);
		return SL_OK;
	});

	ctx.vars["glBindVertexArray"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLuint v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		glBindVertexArray(v1);
		return SL_OK;
	});

	ctx.vars["glBindBuffer"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		int v1;
		GLuint v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		glBindBuffer(v1, v2);
		return SL_OK;
	});

	ctx.vars["glBufferData"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		int v1, v4;
		size_t v2;
		void *v3;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v4);
		ctx->valueStack.pop_back();

		glBufferData(v1, v2, v3, v4);
		return SL_OK;
	});

	ctx.vars["glViewport"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		int v1, v2, v3, v4;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v4);
		ctx->valueStack.pop_back();

		glViewport(v1, v2, v3, v4);
		return SL_OK;
	});

	ctx.vars["glVertexAttribPointer"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 6)
			return SL_ERROR;

		int v1, v2, v3, v4;
		size_t v5, v6;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v4);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v5);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v6);
		ctx->valueStack.pop_back();

		glVertexAttribPointer(v1, v2, v3, v4, v5, (void *)v6);
		return SL_OK;
	});

	ctx.vars["glEnableVertexAttribArray"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		glEnableVertexAttribArray(v1);
		return SL_OK;
	});

	ctx.vars["glCreateShader"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		int v1;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();

		GLuint res = glCreateShader(v1);
		ctx->valueStack.push_back(Variable(res));
		return SL_OK;
	});

	ctx.vars["glShaderSource"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		GLuint shader;
		GLsizei count;
		GLchar **string;
		GLint const *length;
		ctx->valueStack.back().get(shader);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(count);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(string);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(length);
		ctx->valueStack.pop_back();

		glShaderSource(shader, count, string, length);
		return SL_OK;
	});

	ctx.vars["glCompileShader"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLuint shader;
		ctx->valueStack.back().get(shader);
		ctx->valueStack.pop_back();

		glCompileShader(shader);
		return SL_OK;
	});

	ctx.vars["glGetShaderiv"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		GLuint shader;
		int v2;
		GLint *v3;
		ctx->valueStack.back().get(shader);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v3);
		ctx->valueStack.pop_back();

		glGetShaderiv(shader, v2, v3);
		return SL_OK;
	});

	ctx.vars["glGetShaderInfoLog"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		GLuint shader;
		GLsizei maxLength;
		GLsizei *length;
		GLchar *infoLog;
		ctx->valueStack.back().get(shader);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(maxLength);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(length);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(infoLog);
		ctx->valueStack.pop_back();

		glGetShaderInfoLog(shader, maxLength, length, infoLog);
		return SL_OK;
	});

	ctx.vars["glCreateProgram"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		GLuint prog = glCreateProgram();
		ctx->valueStack.push_back(Variable(prog));
		return SL_OK;
	});

	ctx.vars["glAttachShader"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 2)
			return SL_ERROR;

		GLuint v1, v2;
		ctx->valueStack.back().get(v1);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(v2);
		ctx->valueStack.pop_back();

		glAttachShader(v1, v2);
		return SL_OK;
	});

	ctx.vars["glLinkProgram"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLuint prog;
		ctx->valueStack.back().get(prog);
		ctx->valueStack.pop_back();

		glLinkProgram(prog);
		return SL_OK;
	});

	ctx.vars["glUseProgram"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLuint prog;
		ctx->valueStack.back().get(prog);
		ctx->valueStack.pop_back();

		glUseProgram(prog);
		return SL_OK;
	});

	ctx.vars["glDeleteShader"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		GLuint shader;
		ctx->valueStack.back().get(shader);
		ctx->valueStack.pop_back();

		glDeleteShader(shader);
		return SL_OK;
	});

	ctx.vars["glDrawArrays"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 3)
			return SL_ERROR;

		GLenum mode;
		GLuint first;
		GLsizei count;
		ctx->valueStack.back().get(mode);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(first);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(count);
		ctx->valueStack.pop_back();

		glDrawArrays(mode, first, count);
		return SL_OK;
	});

	ctx.vars["glDrawElements"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 4)
			return SL_ERROR;

		GLenum mode, type;
		GLsizei count;
		GLvoid const *indices;

		ctx->valueStack.back().get(mode);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(count);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(type);
		ctx->valueStack.pop_back();
		ctx->valueStack.back().get(indices);
		ctx->valueStack.pop_back();

		glDrawElements(mode, count, type, indices);
		return SL_OK;
	});
}

int slDoFile(slContext &slCtx, const char *filename, bool fullpath)
{
	FILE *fin;
	if (fullpath) {
		fin = fopen(filename, "r");
	}
	else {
		fs::path fpath = slCtx.mainpath / filename;
		fin = fopen(fpath.c_str(), "r");
	}
	if (!fin)
		return SL_ERROR;

	int ln = slCtx.code.size();
	const int newcode_begin = ln;
	
	char buf[1024];
	while (fgets(buf, 1023, fin))
		slCtx.code.push_back(std::string(buf));
	fclose(fin);
	
	// preload labels
	while (ln < slCtx.code.size())
	{
		int next;
		int sr = slPreloadString(slCtx, ln, &next);
		if (sr == SL_ERROR)
			return sr;
		ln = next;
	}
	
	ln = slCtx.line; // backup line
	slCtx.line = newcode_begin; // start executing new code
	const size_t current_end = slCtx.code.size(); // dont forget that executed code may include other files and increase code size

	while (slCtx.line < current_end)
	{
		int next;
		int sr = slDoString(slCtx, slCtx.line, &next);
		if (sr == SL_ERROR) {
			std::cout << "[Exception encountered]\n";
			for (auto const &e: slCtx.stackTraceback) {
				std::cout << "    " << e << '\n';
			}
			return sr;
		}
		if (sr == SL_RETURN)
			break;
		slCtx.line = next;
	}

	slCtx.line = ln;
	return SL_OK;
}

int slDoFile(const char *filename)
{
	slContext slCtx;
	loadlibs(slCtx);
	slCtx.mainpath = fs::path(filename).parent_path();
	return slDoFile(slCtx, filename);
}

int main(int argc, char **argv)
{
	if (argc >= 2)
		return slDoFile(argv[1]);

	slContext slCtx;
	loadlibs(slCtx);

	while (slCtx.line < slCtx.code.size())
	{
		int next;
		int sr = slDoString(slCtx, slCtx.line, &next);
		if (sr == SL_ERROR) {
			std::cout << "[Exception encountered]\n";
			for (auto const &e: slCtx.stackTraceback) {
				std::cout << "    " << e << '\n';
			}
			return sr;
		}
		if (sr == SL_RETURN)
			break;
		slCtx.line = next;
	}

	std::string codeline;
	bool ignore = false;
	for (;;)
	{
		std::cout << (slCtx.code.size() + 1);
		if (ignore)
			std::cout << " i";
		std::cout << "> ";
		std::getline(std::cin, codeline, '\n');

		if (!codeline.size())
			ignore = false;

		slCtx.code.push_back(codeline);
		std::vector<slParseData> *pdata;
		int res = slParseString(slCtx, slCtx.code.size() - 1, pdata);
		if (res == SL_ERROR)
		{
			std::cout << "Error parsing string\n";
			continue;
		}

		if (ignore)
			continue;

		slCtx.line = slCtx.code.size() - 1;
		int sr = slDoString(slCtx, slCtx.line, &slCtx.line);
		if (sr == SL_ERROR) {
			std::cout << "[Exception encountered]\n";
			for (auto const &e: slCtx.stackTraceback) {
				std::cout << "    " << e << '\n';
			}
		}
		if (sr == SL_RETURN)
			break;

		if (pdata->size() && (*pdata)[0].part2 == "~" && !(*pdata)[0].part3.size())
			ignore = true;
	}
	return SL_OK;
}
