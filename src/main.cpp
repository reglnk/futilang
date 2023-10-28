#include <iostream>
#include <cstdio>
#include <cmath>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>
#include <functional>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#define SL_SKIP 2
#define SL_OK 0
#define SL_ERROR 1
#define SL_BREAK 3
#define SL_CONTINUE 4
#define SL_RETURN 5

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
	std::string value;

	Variable(): type(Type::Pointer), value("") {}

	Variable(Type tp, std::string val = ""): type(tp), value(val) {}

	Variable(std::string const &val, Type tp = Type::String): type(tp), value(val) {}

	Variable(int16_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(int32_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(int64_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(uint16_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(uint32_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(uint64_t val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(float val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(double val, Type tp = Type::Number): type(tp) {
		value = (std::stringstream() << val).str();
	}

	Variable(void const *val, Type tp = Type::Pointer): type(tp) {
		value = (std::stringstream() << val).str();
	}

	void get(bool &v) const
	{
		if (type == Type::Bool)
		{
			if (value == "batshit") {
				v = false;
				return;
			}
			if (value == "bullshit") {
				v = true;
				return;
			}
			std::stringstream(value) >> v;
		}
		else if (type == Type::Number)
		{
			std::stringstream(value) >> v;
		}
		else if (type == Type::String)
			v = (bool)value.size();
		else if (type == Type::Pointer)
		{
			void *ptr;
			std::stringstream(value) >> ptr;
			v = (bool)ptr;
		}
		else v = true;
	}

	void get(int8_t &v) const
	{
		int32_t vv;
		get(vv);
		v = vv;
	}
	void get(int16_t &v) const
	{
		int32_t vv;
		get(vv);
		v = vv;
	}
	void get(int32_t &v) const
	{
		if (type == Type::Bool) {
			bool r;
			get(r);
			v = (int)r;
		}
		std::stringstream(value) >> v;
	}
	void get(int64_t &v) const
	{
		if (type == Type::Bool) {
			bool r;
			get(r);
			v = (int)r;
		}
		std::stringstream(value) >> v;
	}
	void get(uint8_t &v) const
	{
		uint32_t vv;
		get(vv);
		v = vv;
	}
	void get(uint16_t &v) const
	{
		uint32_t vv;
		get(vv);
		v = vv;
	}
	void get(uint32_t &v) const
	{
		if (type == Type::Bool) {
			bool r;
			get(r);
			v = (int)r;
		}
		std::stringstream(value) >> v;
	}
	void get(uint64_t &v) const
	{
		if (type == Type::Bool) {
			bool r;
			get(r);
			v = (int)r;
		}
		std::stringstream(value) >> v;
	}
	void get(double &v) const {
		if (type == Type::Bool) {
			int r;
			get(r);
			v = (double)r;
		}
		std::stringstream(value) >> v;
	}
	void get(float &v) const {
		if (type == Type::Bool) {
			int r;
			get(r);
			v = (float)r;
		}
		std::stringstream(value) >> v;
	}
	void get(std::string &v) const {
		v = value;
	}

	void get(void *&v) const {
		std::stringstream(value) >> v;
	}

	void get(void **&v) const {
		void ***vv = &v;
		void **vp = reinterpret_cast<void **>(vv);
		get(*vp);
	}

	template<typename T>
	void get(T *&v) const {
		std::stringstream(value) >> (void *&)v;
	}

	bool isFloat() const {
		return
			value.find('.') != std::string::npos ||
			value.find('e') != std::string::npos;
	}

	Rel compare(Variable const &oth) const {
		if (type == Type::Bool || type == Type::Number || type == Type::Pointer || type == Type::Function) {
			if (oth.type != Type::Bool && oth.type != Type::Number && oth.type != Type::Pointer && oth.type != Type::Function)
				return Rel::NotEqual;

			if (isFloat() || oth.isFloat())
			{
				double val1, val2;
				get(val1);
				oth.get(val2);
				if (val1 == val2)
					return Rel::Equal;
				if (val1 < val2)
					return Rel::Less;
				if (val1 > val2)
					return Rel::Greater;
				return Rel::Undefined;
			}

			int val1, val2;
			get(val1);
			oth.get(val2);
			if (val1 == val2)
				return Rel::Equal;
			if (val1 < val2)
				return Rel::Less;
			if (val1 > val2)
				return Rel::Greater;
			return Rel::Undefined;
		}
		return value == oth.value ? Rel::Equal : Rel::NotEqual;
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

bool isNumber(std::string const &s)
{
	bool isnumber = (bool)s.size();
	for (char ch: s) {
		if (ch != '.' && !isDigit(ch)) {
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
};

class slScopeData
{
public:
	std::unordered_map<std::string, Variable> locals;
};

class slContext
{
public:
	int line;
	std::vector<std::string> code;
	std::vector<slScopeData> callStack;
	std::vector<Variable> valueStack;
	std::unordered_map<std::string, Variable> vars;
	std::unordered_map<std::string, int> callbacks;
	std::vector<std::function<int(slContext *)>> nativeFunctions;
	std::vector<std::string> stackTraceback;

	slContext(): line(0), code(), vars() {
		vars["sf"] = Variable();
	}

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
			auto vr = Variable(Variable::Type::String, std::string(mem, mempos));
			delete[] mem;
			return vr;
		}
		if (isNumber(part))
			return Variable(Variable::Type::Number, part);
		if (part == "bullshit" || part == "batshit")
			return Variable(Variable::Type::Bool, part);

		Variable *vrf = findvar(part);
		if (vrf)
			return *vrf;

		return Variable();
	}
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
	while (dat.end1 != end && !isOper(*dat.end1) && !isWhitespace(*dat.end1))
		++dat.end1;
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
		while (dat.end2 != end && *dat.end2++ != '"');
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
		while (dat.end3 != end && *dat.end3++ != '"');
	} else {
		while (dat.end3 != end && !isWhitespace(*dat.end3))
			++dat.end3;
	}
	if (dat.beg3 != dat.end3)
		dat.part3 = std::string(dat.beg3, dat.end3 - dat.beg3);

	return SL_OK;
}

int slParseString(const char *buf, std::vector<slParseData> &dat)
{
	bool quotes = false;
	const char *end = buf;
	while (*end && (quotes || *end != '#')) {
		if (*end == '"')
			quotes = !quotes;
		++end;
	}
	if (buf == end)
		return SL_SKIP;

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
				dat.push_back(std::move(pdata));
			else if (res != SL_SKIP)
				return res;
			prev = iter;
		}
	}
	return SL_OK;
}

int slPreloadString(slContext &slCtx, int lineNum, int *change = nullptr)
{
	const char *buf = slCtx.code[lineNum].c_str();
	std::vector<slParseData> dat;
	int pars = slParseString(buf, dat);

	if (!dat.size()) {
		if (change) *change = lineNum + 1;
		return SL_OK;
	}

	auto &p = dat.back();
	if (p.oper)
	{
		Variable right = slCtx.evaluate(p.part3);
		if (p.part2 == "~")
		{
			// put the line number into new variable
			if (!p.part3.size())
			{
				slCtx.vars[p.part1] = Variable (
					Variable::Type::Number,
					(std::stringstream() << (lineNum + 1)).str().c_str()
				);
			}
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
	if (gv.type == Variable::Type::Pointer && gv.value == "") {
		pushError("calling null pointer", p.beg1 - buf);
		return SL_ERROR;
	}

	if (gv.type == Variable::Type::Function) {
		int fn_id;
		gv.get(fn_id);
		if (fn_id >= slCtx.nativeFunctions.size()) {
			pushError("invalid native function", p.beg1 - buf);
			return SL_ERROR;
		}
		int res = slCtx.nativeFunctions[fn_id](&slCtx);
		if (res != SL_OK) {
			pushError((std::string("calling native function ") + p.part2).c_str(), p.beg2 - buf);
		}
		return res;
	}
	int newln;
	gv.get(newln);
	if (newln == 0) {
		pushError("invalid label", p.beg1 - buf);
		return SL_ERROR;
	}

	int res = slCallFunction(slCtx, newln);
	if (res != SL_OK)
		return res == SL_RETURN ? SL_OK : res;

	return SL_OK;
}

int slDoBlock(slContext &slCtx, const char *buf, int lineNum, slParseData const &p, int *change = nullptr)
{
	const auto pushError = [&](const char *text, int pos, int line = -1) {
		if (line == -1)
			line = lineNum;
		slCtx.stackTraceback.push_back (
			(std::stringstream() << "at " << line + 1 << " (" << pos <<
			"): " << text).str()
		);
		auto &bk = slCtx.stackTraceback.back();
		return bk;
	};
	const auto errStackEmpty = [&](int pos, int line = -1) {
		pushError("stack is empty", pos, line);
	};

	// type - 0: add, 1: sub, 2: mul, 3: div
	const auto execOper = [&](Variable &left, const Variable &right, int type)
	{
		if (left.type == Variable::Type::Number)
		{
			std::stringstream ss;
			if (left.isFloat() || right.isFloat())
			{
				double gf, gs;
				left.get(gf);
				right.get(gs);

				switch (type)
				{
					case 0:
						gf += gs;
						break;
					case 1:
						gf -= gs;
						break;
					case 2:
						gf *= gs;
						break;
					case 3:
						gf /= gs;
						break;
					default:
						pushError("unsupported operand type", p.beg1 - buf);
						return SL_ERROR;
				}
				ss << gf;
			}
			else
			{
				int64_t gf, gs;
				left.get(gf);
				right.get(gs);

				switch (type)
				{
					case 0:
						gf += gs;
						break;
					case 1:
						gf -= gs;
						break;
					case 2:
						gf *= gs;
						break;
					case 3:
						gf /= gs;
						break;
					case 4:
						gf |= gs;
						break;
					case 5:
						gf = gf || gs;
						break;
					case 6:
						gf &= gs;
						break;
					case 7:
						gf = gf && gs;
						break;
					case 8:
						gf ^= gs;
					default:
						pushError("unsupported operand type", p.beg1 - buf);
						return SL_ERROR;
				}
				ss << gf;
			}
			left.value = ss.str();
		}
		else if (left.type == Variable::Type::Bool)
		{
			bool gf, gs;
			left.get(gf);
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
					pushError("unsupported operand type", p.beg1 - buf);
					return SL_ERROR;
			}
			left.value = gf ? "bullshit" : "batshit";
		}
		else if (left.type == Variable::Type::Pointer)
		{
			void *gf;
			left.get(gf);
			Variable *gfv = reinterpret_cast<Variable *>(gf);

			if (type == 0) {
				if (right.type != Variable::Type::Number) {
					pushError("unsupported operand type", p.beg3 - buf);
					return SL_ERROR;
				}
				int gs;
				right.get(gs);
				gfv += gs;
				left.value = (std::stringstream() << (size_t)gfv).str();
			}
			else if (type == 1) {
				if (right.type == Variable::Type::Number) {
					int gs;
					right.get(gs);
					gfv -= gs;
					left.value = (std::stringstream() << (size_t)gfv).str();
				}
				else if (right.type == Variable::Type::Pointer) {
					void *gs;
					right.get(gs);
					Variable *gsv = reinterpret_cast<Variable *>(gs);
					int diff = (int)(gfv - gsv);
					left.type = Variable::Type::Number;
					left.value = (std::stringstream() << diff).str();
					return SL_ERROR;
				}
				else {
					pushError("unsupported operand type", p.beg3 - buf);
					return SL_ERROR;
				}
			}
			else {
				pushError("unsupported operand type", p.beg1 - buf);
				return SL_ERROR;
			}
		}
		else if (left.type == Variable::Type::String)
		{
			if (type == 0)
				left.value += right.value;
			else if (type == 1) {
				if (left.value.size() < right.value.size()) {
					pushError("substraction of longer string from another", p.beg3 - buf);
					return SL_ERROR;
				}
				left.value.resize(left.value.size() - right.value.size());
			}
			else {
				pushError("unsupported operand type", p.beg3 - buf);
				return SL_ERROR;
			}
		}
		else {
			pushError("unsupported operand type", p.beg1 - buf);
			return SL_ERROR;
		}
		return SL_OK;
	};

	if (p.oper && p.part2 != "!")
	{
		if (p.part2 == "=")
		{
			Variable right;
			if (p.part3.size())
				right = std::move(slCtx.evaluate(p.part3));
			else if (slCtx.valueStack.size()) {
				right = slCtx.valueStack.back();
				slCtx.valueStack.pop_back();
			}
			slCtx.assign(p.part1) = right;
		}
		else if (p.part2 == "~")
		{
			if (!p.part3.size())
			{
				// put the line number into new variable
				slCtx.assign(p.part1) = Variable (
					Variable::Type::Number,
					(std::stringstream() << (lineNum + 1)).str().c_str()
				);
			}
			else if (p.part1 == "in")
			{
				Variable inputvar;
				std::cin >> inputvar.value;
				slCtx.assign(p.part3) = inputvar;
			}
			else if (p.part1 == "out")
			{
				auto gv = slCtx.evaluate(p.part3);
				if (gv.type == Variable::Type::Pointer && gv.value == "")
					std::cout << "<null>";
				else std::cout << gv.value;
			}
			else if (p.part1 == "split")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					pushError("string required", p.beg3 - buf);
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				Variable *arr = new Variable[vstr.size()];
				for (size_t i = 0; i < vstr.size(); ++i)
				{
					arr[i] = Variable(Variable::Type::String, std::string(&vstr[i], 1));
				}
				
				Variable res;
				res.type = Variable::Type::Pointer;
				res.value = (std::stringstream() << (size_t)arr).str();
				slCtx.valueStack.push_back(std::move(res));
			}
			else if (p.part1 == "length")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					pushError("string required", p.beg3 - buf);
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				
				Variable res;
				res.type = Variable::Type::Number;
				res.value = (std::stringstream() << vstr.size()).str();
				slCtx.valueStack.push_back(std::move(res));
			}
			else if (p.part1 == "type")
			{
				Variable &vref = slCtx.assign(p.part3);
				
				Variable res;
				res.value =
					vref.type == Variable::Type::Pointer ? "pointer" :
					vref.type == Variable::Type::Number ? "number" :
					vref.type == Variable::Type::String ? "string" :
					vref.type == Variable::Type::Bool ? "bool" :
					vref.type == Variable::Type::Function ? "function" :
					"undefined";
				res.type = Variable::Type::String;
				slCtx.valueStack.push_back(std::move(res));
			}
			else
			{
				Variable &vref = slCtx.assign(p.part3);
				if (p.part1 == "bool")
					vref.type = Variable::Type::Bool;
				else if (p.part1 == "number")
					vref.type = Variable::Type::Number;
				else if (p.part1 == "string")
					vref.type = Variable::Type::String;
				else if (p.part1 == "pointer")
					vref.type = Variable::Type::Pointer;
				else if (p.part1 == "function")
					vref.type = Variable::Type::Function;
				else {
					pushError("invalid type", p.beg1 - buf);
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
				errStackEmpty(p.beg2 - buf);
				return SL_ERROR;
			}

			if (right.type != Variable::Type::Pointer) {
				pushError("pointer required", p.beg3 - buf);
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
				errStackEmpty(p.beg2 - buf);
				return SL_ERROR;
			}

			if (right.type != Variable::Type::Pointer) {
				pushError("pointer required", p.beg3 - buf);
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
			
			Variable v(Variable::Type::Bool, result ? "bullshit" : "batshit");
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
				errStackEmpty(p.beg2 - buf);
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

			int res = execOper(left, right, type);
			if (res != SL_OK)
				return res;

			slCtx.valueStack.push_back(std::move(left));
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
				pushError("undefined variable ", p.beg1 - buf) += p.part1;
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

			int res = execOper(left, right, type);
			if (res != SL_OK)
				return res;

			slCtx.valueStack.push_back(left);
		}
		else {
			pushError("invalid operator", p.beg2 - buf);
			return SL_ERROR;
		}
	}
	else if (p.part1 == "while")
	{
		int lvl = 1;
		int l;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			std::vector<slParseData> pd;
			slParseString(slCtx.code[l].c_str(), pd);

			bool brk = false;
			for (auto const &p: pd)
			{
				// @todo optimize: pick ready slParseData from slCtx instead of this
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "while" || p.part1 == "if")
					++lvl;
				if (!lvl) {
					brk = true;
					break;
				}
			}
			if (brk) break;
		}
		if (lvl) {
			pushError("unclosed 'while'", p.beg1 - buf, l);
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
			pushError("pointer required", p.beg1 - buf);
			return SL_ERROR;
		}

		if (slCtx.valueStack.size() < 1)
		{
			pushError("stack is empty", p.beg1 - buf);
			return SL_ERROR;
		}
		Variable &len = slCtx.valueStack.back();
		if (len.type != Variable::Type::Number)
		{
			pushError("number required", p.beg1 - buf);
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
		int res = slCallFunction(slCtx, ptr[0], buf, lineNum, p, change);
		if (res != SL_OK)
			return SL_ERROR;

		size_t objsize;
		ptr[1].get(objsize);
		char *pptr;

		Variable &obj = slCtx.valueStack.back();
		obj.get(pptr);

		for (size_t i = 0; i != arrlen; ++i)
		{
			Variable pt((void *)(pptr + [i * objsize]));
			slCtx.valueStack.push_back(std::move(pt));
			int rr = slCallFunction(slCtx, ptr[2], buf, lineNum, p, change);
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
				pushError("invalid stack index", p.beg1 - buf);
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
			std::vector<slParseData> pd;
			int lr = slParseString(slCtx.code[l].c_str(), pd);
			bool brk = false;
			for (auto const &p: pd)
			{
				if (p.part1 == "end")
					--lvl;
				else if (p.part1 == "if" || p.part1 == "while")
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
			pushError("unclosed 'if'", p.beg1 - buf, l);
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
			pushError("not enough stack size", p.beg1 - buf);
			return SL_ERROR;
		}

		bool v1, v2;
		slCtx.valueStack.back().get(v1);
		slCtx.valueStack.pop_back();
		slCtx.valueStack.back().get(v2);
		slCtx.valueStack.pop_back();

		bool res = (p.part1 == "or") ? (v1 || v2) : (v1 && v2);
		Variable v(Variable::Type::Bool, res ? "bullshit" : "batshit");
		slCtx.valueStack.push_back(std::move(v));

		return SL_OK;
	}
	else if (p.part1 == "goto")
	{
		auto gv = slCtx.evaluate(p.part2);
		if (gv.type == Variable::Type::Pointer && gv.value == "") {
			pushError("cannot goto null pointer", p.beg1 - buf);
			return SL_ERROR;
		}
		int newln;
		gv.get(newln);
		if (newln == 0) {
			pushError("invalid label", p.beg1 - buf);
			return SL_ERROR;
		}

		if (change) *change = newln - 1;
		return SL_OK;
	}
	else if (p.part1 == "call")
	{

	}
	else if (p.part1 == "alloc")
	{
		Variable &left = slCtx.vars[p.part2];
		int arrlen;
		slCtx.evaluate(p.part3).get(arrlen);
		if (arrlen)
		{
			Variable *arr = new Variable[arrlen];
			left.value = (std::stringstream() << (size_t)arr).str();
		}
	}
	else if (p.part1 == "free")
	{
		Variable &left = slCtx.vars[p.part2];
		void *arr;
		left.get(arr);
		left.value = "";
		delete[] reinterpret_cast<Variable *>(arr);
	}
	else if (p.part1.size())
	{
		Variable v = slCtx.evaluate(p.part1);
		slCtx.valueStack.push_back(std::move(v));
	}
	else {
		pushError("syntax error", 0);
		return SL_ERROR;
	}
	return SL_OK;
}

int slDoString(slContext &slCtx, int lineNum, int *change)
{
	const char *buf = slCtx.code[lineNum].c_str();
	std::vector<slParseData> p;
	int pars = slParseString(buf, p);
	
	if (!p.size()) {
		if (change) *change = lineNum + 1;
		return SL_OK;
	}
	
	for (auto iter = p.begin(); iter != p.end(); ++iter)
	{
		int chg = lineNum + 1;
		int res = slDoBlock(slCtx, buf, lineNum, *iter, &chg);
		if (res == SL_SKIP)
			continue;
		if (res != SL_OK)
			return res;

		if (chg != lineNum + 1) {
			if (iter == p.end() - 1) {
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
			res = Variable(v.value.size());
		else if (v.type == Variable::Type::Pointer) {
			char *s;
			v.get(s);
			res = Variable(::strlen(s));
		}

		ctx->valueStack.push_back(res);
		return SL_OK;
	});

	ctx.vars["to_cstr"] = Variable((int)ctx.nativeFunctions.size(), Variable::Type::Function);
	ctx.nativeFunctions.push_back([](slContext *ctx) {
		if (ctx->valueStack.size() < 1)
			return SL_ERROR;

		auto v = ctx->valueStack.back();
		ctx->valueStack.pop_back();

		size_t slen = v.value.size();
		int8_t *cstr = new int8_t[slen + 1];
		cstr[slen] = '\0';
		std::memcpy(cstr, v.value.c_str(), slen);
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

		FILE *f = fopen(source.value.c_str(), modes.value.c_str());
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

int slDoFile(const char *filename)
{
	FILE *fin = fopen(filename, "r");
	if (!fin)
		return SL_ERROR;
	
	slContext slCtx;
	
	char buf[1024];
	while (fgets(buf, 1023, fin))
		slCtx.code.push_back(std::string(buf));
	fclose(fin);

	loadlibs(slCtx);
	
	// preload labels
	int ln = 0;
	while (ln < slCtx.code.size())
	{
		int next;
		int sr = slPreloadString(slCtx, ln, &next);
		if (sr == SL_ERROR)
			return sr;
		ln = next;
	}
	
	// execute code
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
	return SL_OK;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 1;
	
	return slDoFile(argv[1]);
}
