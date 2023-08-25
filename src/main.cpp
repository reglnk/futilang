#include <iostream>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <cstring>
#include <sstream>
#include <vector>

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
		Pointer
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
	
	void get(bool &v) const {
		if (type == Type::Bool)
		{
			if (value == "bullshit" || std::atoi(value.c_str()))
				v = true;
			else v = false;
		}
		else if (type == Type::Number)
		{
			v = (bool)std::atoi(value.c_str());
		}
		else v = (bool)value.size();
	}
	void get(int &v) const {
		if (type == Type::Bool) {
			bool r;
			get(r);
			v = (int)r;
		}
		v = std::atoi(value.c_str());
	}
	void get(double &v) const {
		if (type == Type::Bool) {
			int r;
			get(r);
			v = (double)r;
		}
		v = std::atof(value.c_str());
	}
	void get(float &v) const {
		if (type == Type::Bool) {
			int r;
			get(r);
			v = (float)r;
		}
		v = (float)std::atof(value.c_str());
	}
	void get(std::string &v) const {
		v = value;
	}
	void get(void *&v) const {
		size_t ptr = (size_t)atoll(value.c_str());
		v = (void*)ptr;
	}
	
	Rel compare(Variable const &oth) const {
		if (type == Type::Bool || type == Type::Number || type == Type::Pointer) {
			if (oth.type != Type::Bool && oth.type != Type::Number && oth.type != Type::Pointer)
				return Rel::NotEqual;
			
			if (
				value.find('.') != std::string::npos ||
				oth.value.find('.') != std::string::npos ||
				value.find('e') != std::string::npos ||
				oth.value.find('e') != std::string::npos
			) {
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
	return s == '=' || s == '+' || s == '*' || s == '-' || s == '/' || s == '~' || s == '<' || s == '>' || s == '!';
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
	std::unordered_map<std::string, Variable> vars;
	
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
		if (part == "bullshit" || part == "dogshit")
			return Variable(Variable::Type::Bool, part);
		
		Variable *vrf = findvar(part);
		if (vrf)
			return *vrf;
		
		return Variable();
	}
};

int slParseString(const char *buf, slParseData &dat)
{
	int len = strlen(buf);
	if (len == 0 || buf[0] == '#')
		return SL_SKIP;
	
	dat.beg1 = buf;
	while (*dat.beg1 && isWhitespace(*dat.beg1))
		++dat.beg1;
	if (*dat.beg1 == '#')
		return SL_SKIP;
	
	dat.end1 = dat.beg1;
	while (*dat.end1 && !isOper(*dat.end1) && !isWhitespace(*dat.end1))
		++dat.end1;
	if (dat.beg1 == dat.end1)
		return SL_SKIP;
	
	dat.part1 = std::string(dat.beg1, dat.end1 - dat.beg1);
	
	dat.beg2 = dat.end1;
	while (*dat.beg2 && isWhitespace(*dat.beg2))
		++dat.beg2;
	
	dat.oper = false;
	dat.end2 = dat.beg2;
	if (isOper(*dat.beg2)) {
		dat.oper = true; // @fixme: part2 can be oper and number at same time
		while (*dat.end2 && isOper(*dat.end2))
			++dat.end2;
	} else if (*dat.beg2 == '"') {
		++dat.end2;
		while (*dat.end2 && *dat.end2++ != '"');
	} else {
		while (*dat.end2 && !isWhitespace(*dat.end2))
			++dat.end2;
	}
	dat.part2 = std::string(dat.beg2, dat.end2 - dat.beg2);
	
	dat.part3 = "";
	dat.beg3 = dat.end2;
	while (*dat.beg3 && isWhitespace(*dat.beg3))
		++dat.beg3;
	dat.end3 = dat.beg3;
	if (*dat.beg3 == '"') {
		++dat.end3;
		while (*dat.end3 && *dat.end3++ != '"');
	} else {
		while (*dat.end3 && !isWhitespace(*dat.end3))
			++dat.end3;
	}
	if (dat.beg3 != dat.end3)
		dat.part3 = std::string(dat.beg3, dat.end3 - dat.beg3);
	
	return SL_OK;
}

int slPreloadString(slContext &slCtx, int lineNum, int *change = nullptr)
{
	const char *buf = slCtx.code[lineNum].c_str();
	slParseData p;
	int pars = slParseString(buf, p);
	
	if (pars == SL_SKIP) {
		if (change) *change = lineNum + 1;
		return SL_OK;
	}
	
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

int slDoString(slContext &slCtx, int lineNum, int *change = nullptr)
{
	const char *buf = slCtx.code[lineNum].c_str();
	slParseData p;
	int pars = slParseString(buf, p);
	
	if (pars == SL_SKIP) {
		if (change) *change = lineNum + 1;
		return SL_OK;
	}
	
	if (p.oper)
	{
		if (p.part2 == "=") {
			Variable right = slCtx.evaluate(p.part3);
			slCtx.assign(p.part1) = right;
		}
		else if (p.part2 == "~")
		{
			// put the line number into new variable
			if (!p.part3.size())
			{
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
					std::cout << "<null>\n";
				else std::cout << gv.value;
			}
			else if (p.part1 == "split")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - string required\n";
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				Variable *arr = new Variable[vstr.size()];
				for (size_t i = 0; i < vstr.size(); ++i)
				{
					arr[i] = Variable(Variable::Type::String, std::string(&vstr[i], 1));
				}
				vref.type = Variable::Type::Pointer;
				vref.value = (std::stringstream() << (size_t)arr).str();
			}
			else if (p.part1 == "length")
			{
				Variable &vref = slCtx.assign(p.part3);
				if (vref.type != Variable::Type::String) {
					std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - string required\n";
					return SL_ERROR;
				}
				std::string vstr;
				vref.get(vstr);
				vref.type = Variable::Type::Number;
				vref.value = (std::stringstream() << vstr.size()).str();
			}
			else if (p.part1 == "type")
			{
				Variable &vref = slCtx.assign(p.part3);
				vref.value =
					vref.type == Variable::Type::Pointer ? "pointer" :
					vref.type == Variable::Type::Number ? "number" :
					vref.type == Variable::Type::String ? "string" :
					vref.type == Variable::Type::Bool ? "bool" :
					"undefined";
				vref.type = Variable::Type::String;
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
				else {
					std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - invalid type " << p.part3 << '\n';
					return SL_ERROR;
				}
			}		
		}
		else if (p.part2 == "<-")
		{
			Variable &left = slCtx.assign(p.part1);
			Variable right = slCtx.evaluate(p.part3);
			
			if (right.type != Variable::Type::Pointer) {
				std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - pointer required\n";
				return SL_ERROR;
			}
			
			void *vr;
			right.get(vr);
			left = *reinterpret_cast<Variable *>(vr);
		}
		else if (p.part2 == "->")
		{
			Variable left = slCtx.evaluate(p.part1);
			Variable right = slCtx.evaluate(p.part3);
			
			if (right.type != Variable::Type::Pointer) {
				std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - pointer required\n";
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
			
			slCtx.vars["sf"] = Variable(Variable::Type::Bool, result ? "bullshit" : "dogshit");
		}
		else if (p.part2 == "+=" || p.part2 == "-=" || p.part2 == "*=" || p.part2 == "/=")
		{
			Variable right = slCtx.evaluate(p.part3);
			Variable *lft = slCtx.findvar(p.part1);
			if (!lft) {
				std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - undefined variable " << p.part1 << '\n';
				return SL_ERROR;
			}
			Variable &left = *lft;
			if (left.type == Variable::Type::Number)
			{
				double gf, gs;
				left.get(gf);
				right.get(gs);
				
				if (p.part2 == "+=")
					gf += gs;
				else if (p.part2 == "-=")
					gf -= gs;
				else if (p.part2 == "*=")
					gf *= gs;
				else gf /= gs;
				
				std::stringstream ss;
				ss << gf;
				left.value = ss.str();
			}
			else if (left.type == Variable::Type::Bool)
			{
				bool gf, gs;
				left.get(gf);
				right.get(gs);
				
				if (p.part2 == "+=")
					gf |= gs;
				else if (p.part2 == "-=")
					gf ^= gs;
				else gf &= gs;
				
				left.value = gf ? "bullshit" : "dogshit";
			}
			else if (left.type == Variable::Type::Pointer)
			{
				void *gf;
				left.get(gf);
				Variable *gfv = reinterpret_cast<Variable *>(gf);
				
				if (p.part2 == "+=") {
					if (right.type != Variable::Type::Number) {
						std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - unsupported operand type\n";
						return SL_ERROR;
					}
					int gs;
					right.get(gs);
					gfv += gs;
					left.value = (std::stringstream() << (size_t)gfv).str();
				}
				else if (p.part2 == "-=") {
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
						std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - unsupported operand type\n";
						return SL_ERROR;
					}
				}
				else {
					std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - unsupported operand type\n";
					return SL_ERROR;
				}
			}
			else if (left.type == Variable::Type::String)
			{
				if (p.part2 == "+=")
					left.value += right.value;
				else if (p.part2 == "-=") {
					if (left.value.size() < right.value.size()) {
						std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - cannot substract a string from shorter one\n";
						return SL_ERROR;
					}
					left.value.resize(left.value.size() - right.value.size());
				}
				else {
					std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - unsupported operand type\n";
					return SL_ERROR;
				}
			}
			else {
				std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - unsupported operand type\n";
				return 1;
			}
		}
		else {
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg2 - buf << " - invalid operator " << p.part2 << '\n';
			return 1;
		}
	}
	else if (p.part1 == "while")
	{
		int lvl = 1;
		int l;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			slParseData pd;
			int lr = slParseString(slCtx.code[l].c_str(), pd);
			if (pd.part1 == "end")
				--lvl;
			else if (pd.part1 == "while" || pd.part1 == "if")
				++lvl;
			if (!lvl)
				break;
		}
		if (lvl) {
			std::cout << "<error> " << l + 1 << ": unclosed 'while' at level " << lvl << '\n';
			return SL_ERROR;
		}
		
		bool ev;
		for (;;)
		{
			if (!p.part2.size())
				slCtx.vars["sf"].get(ev);
			else slCtx.evaluate(p.part2).get(ev);
			if (!ev)
				break;
			
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
			if (brk)
				break;
		}
		if (change) *change = l + 1;
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
	else if (p.part1 == "if")
	{	
		int lvl = 1;
		int l;
		int sep = 0;
		for (l = lineNum + 1; l != slCtx.code.size(); ++l)
		{
			slParseData pd;
			int lr = slParseString(slCtx.code[l].c_str(), pd);
			if (pd.part1 == "end")
				--lvl;
			else if (pd.part1 == "if" || pd.part1 == "while")
				++lvl;
			else if (!sep && pd.part1 == "else")
				sep = l;
			if (!lvl)
				break;
		}
		if (lvl) {
			std::cout << "<error> " << l + 1 << ": unclosed 'if' at level " << lvl << '\n';
			return SL_ERROR;
		}
		
		bool ev;
		if (!p.part2.size())
			slCtx.vars["sf"].get(ev);
		else slCtx.evaluate(p.part2).get(ev);
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
	else if (p.part1 == "goto")
	{
		auto gv = slCtx.evaluate(p.part2);
		if (gv.type == Variable::Type::Pointer && gv.value == "") {
			std::cout << "<error> " << lineNum + 1 << ": cannot goto null pointer\n";
			return SL_ERROR;
		}
		int newln;
		gv.get(newln);
		if (newln == 0) {
			std::cout << "<error> " << lineNum + 1 << ": invalid label\n";
			return SL_ERROR;
		}
		
		if (change) *change = newln - 1;
		return SL_OK;
	}
	else if (p.part1 == "call")
	{
		auto gv = slCtx.evaluate(p.part2);
		if (gv.type == Variable::Type::Pointer && gv.value == "") {
			std::cout << "<error> " << lineNum + 1 << ": cannot call null pointer\n";
			return SL_ERROR;
		}
		int newln;
		gv.get(newln);
		if (newln == 0) {
			std::cout << "<error> " << lineNum + 1 << ": invalid label\n";
			return SL_ERROR;
		}
		
		slScopeData scdat{};
		slCtx.callStack.push_back(std::move(scdat));

		for (int ln = newln - 1; ln < slCtx.code.size(); )
		{
			int next;
			int sr = slDoString(slCtx, ln, &next);
			if (sr == SL_ERROR)
				return sr;
			if (sr == SL_RETURN)
				break;
			ln = next;
		}
		slCtx.callStack.pop_back();
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
	else if (p.part1 == "fsize")
	{
		Variable &dest = slCtx.vars[p.part2];
		Variable source = slCtx.evaluate(p.part3);
		if (source.type != Variable::Type::String) {
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - string required\n";
			return SL_ERROR;
		}
		
		dest.type = Variable::Type::Number;

		FILE *fin = fopen(source.value.c_str(), "rb");
		if (!fin) {
			dest.value = "-1";
			if (change) *change = lineNum + 1;
			return SL_OK;
		}
		if (fseek(fin, 0, SEEK_END)) {
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - fseek failed\n";
			return SL_ERROR;
		}
		int fs = (int)ftell(fin);
		if (fs < 0) {
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - ftell failed\n";
			return SL_ERROR;
		}
		fclose(fin);
		dest.value = (std::stringstream() << fs).str();
	}
	else if (p.part1 == "fread")
	{
		Variable &dest = slCtx.vars[p.part2];
		Variable source = slCtx.evaluate(p.part3);
		if (source.type != Variable::Type::String) {
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg3 - buf << " - string required\n";
			return SL_ERROR;
		}
		
		FILE *fin = fopen(source.value.c_str(), "rb");
		if (!fin) {
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - unable to open " << source.value << "\n";
			return SL_ERROR;
		}
		if (fseek(fin, 0, SEEK_END)) {
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - fseek failed\n";
			return SL_ERROR;
		}
		int fs = (int)ftell(fin);
		if (fs <= 0) {
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - ftell failed\n";
			return SL_ERROR;
		}
		if (fseek(fin, 0, SEEK_SET)) {
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - fseek failed\n";
			return SL_ERROR;
		}
		char *mem = new char[fs];
		if (fread(mem, 1, fs, fin) != fs)
		{
			fclose(fin);
			std::cout << "<error> " << lineNum + 1 << ": " << p.beg1 - buf << " - unable to read the file\n";
			return SL_ERROR;
		}
		fclose(fin);
		
		dest.type = Variable::Type::Pointer;
		Variable *varr = new Variable[fs];
		dest.value = (std::stringstream() << (size_t)varr).str();
		
		for (int i = 0; i != fs; ++i)
			varr[i] = Variable(Variable::Type::String, std::string(&mem[i], 1));
	}
	else if (p.part1 == "system")
	{
		auto gv = slCtx.evaluate(p.part2);
		if (gv.type != Variable::Type::String) {
			std::cout << "<error> " << lineNum + 1 << ": string value required\n";
			return SL_ERROR;
		}
		int res = system(gv.value.c_str());
		slCtx.vars["retcode"] = Variable (
			Variable::Type::Number,
			(std::stringstream() << res).str().c_str()
		);
	}
	else {
		std::cout << "<error> " << lineNum + 1 << ": syntax error\n";
		return SL_ERROR;
	}
	
	if (change) *change = lineNum + 1;
	return SL_OK;
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
		if (sr == SL_ERROR)
			return sr;
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
