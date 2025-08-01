#include "pch.h"
#include "ConverterApp.h"

std::ostream& ConverterApp::stream = std::cout;

Value parse(std::string v)
{
	// TOOD: auto-base?
	try {
		size_t pos;
		int iv = std::stoi(v, &pos);
		if (pos == v.size())
			return Value(iv);
	}
	catch (std::invalid_argument& ex) {
		// It is not a valid int, or not the whole string is a valid int.
	}

	try {
		size_t pos;
		double dv = std::stod(v, &pos);
		if (pos == v.size())
			return Value(dv);
	}
	catch (std::invalid_argument& ex) {
		// It is not a valid double, or not the whole string is a valid double.
		// At this point it is just a string
	}

	return Value(v);
}

std::string tostring(Value v)
{
	std::string res;

	using T = Value::Type;
	switch(v.getType())
	{
	case T::UINT:
		return std::to_string(v.getUInt());
	case T::INT:
		return std::to_string(v.getInt());
	case T::DOUBLE:
		return std::to_string(v.getDouble());
	
	case T::CHAR:
		return { v.getChar() };

	case T::STRING:
		return v.getString();

	default:
		assert(false);
	}

	return res;
}

//void outwithprec(Value v, std::ostream& s, int prec)
//{
//	//int pprec = s.precision(prec);
//
//	using T = Value::Type;
//	switch (v.getType())
//	{
//	case T::UINT:
//		s << v.getUInt();
//		break;
//	case T::INT:
//		s << v.getInt();
//		break;
//	case T::DOUBLE:
//		s << v.getDouble();
//		break;
//	case T::CHAR:
//		s << std::string{ v.getChar() };
//		break;
//	case T::STRING:
//		s << v.getString();
//		break;
//	default:
//		assert(false);
//	}
//
//	//s.precision(pprec);
//}

void outval(Value v, std::ostream& s)
{
	using T = Value::Type;
	switch (v.getType())
	{
	case T::UINT:
		s << v.getUInt();
		break;
	case T::INT:
		s << v.getInt();
		break;
	case T::DOUBLE:
		s << v.getDouble();
		break;
	case T::CHAR:
		s << std::string{ v.getChar() };
		break;
	case T::STRING:
		s << v.getString();
		break;
	default:
		assert(false);
	}
}


template <typename NodeT, typename EdgeT>
void ConversionGraph<NodeT, EdgeT>::insertNode(const NodeT* n)
{
	edges.insert({ n, {} });
}

template <typename NodeT, typename EdgeT>
void ConversionGraph<NodeT, EdgeT>::connect(const NodeT* a, const NodeT* b, const EdgeT edge)
{
	edges.at(a).push_back({b, edge});
}


template <typename NodeT, typename EdgeT>
vector<const EdgeT*> ConversionGraph<NodeT, EdgeT>::searchPath(const NodeT* a, const NodeT* b)
{ // BFS

	using std::deque;
	using Node = const NodeT*;
	using Edge = const EdgeT*;

	
	deque<Node> queue;
	
	queue.push_back(a);

	set<Node> visited;

	map<Node, Node> backPath;

	while (!queue.empty()) {
		Node current = queue.front();
		queue.pop_front();

		if (visited.count(current) > 0) continue;
		visited.insert(current);

		if (current == b) {
			break;
		}

		for (const auto& entry : edges[current]) {
			backPath.insert({ entry.first, current });

			if (entry.first == b) goto outer_break;

			queue.push_back(entry.first);
		}
	}
	outer_break:

	if (backPath.find(b) == backPath.end()) {
		return {};
		// We were not able to find any path between a and b.
	}

	vector<Edge> path;
	vector<Node> nodePath;
	
	auto current = b;
	nodePath.push_back(current);
	while (current != a) {
		current = backPath[current];
		nodePath.push_back(current);
	}

	std::reverse(nodePath.begin(), nodePath.end());

	for (size_t i = 0; i < nodePath.size() - 1; i ++) {

		auto& m = edges[nodePath[i]];

		for (auto& e : m) {
			if (e.first == nodePath[i + 1]) {
				path.push_back(&e.second);
				break;
			}
		}
	}

	return path;
}


// TODO: add American spelling???


// Length
static const Unit m = { "m", "metres" };
static const Unit dm = { "dm", "decimetres" };
static const Unit cm = { "cm", "cantimetres" };
static const Unit mm = { "mm", "millimetres" };
// Area
static const Unit m2 = { "m^2", "squared metres" };
static const Unit dm2 = { "dm^2", "squared decimetres" };
static const Unit cm2 = { "cm^2", "squared cantimetres" };
static const Unit mm2 = { "mm^2", "squared millimetres" };
// Volume
static const Unit m3 = { "m^3", "cubic metres" };
static const Unit dm3 = { "dm^3", "cubic decimetres" };
static const Unit cm3 = { "cm^3", "cubic cantimetres" };
static const Unit mm3 = { "mm^3", "cubic millimetres" };

static const Unit l = { "l", "litres" };
// Temperature
static const Unit dC = { "degC", "degrees Celsius" };
static const Unit dF = { "degF", "degrees Farenheit" };
static const Unit dK = { "degK", "degreees Kelvin" };


Value toDouble(Value v)
{
	using Type = Value::Type;
	switch (v.getType())
	{
	case Type::UINT:
		return (double)v.getUInt();
	case Type::INT:
		return (double)v.getInt();
	case Type::DOUBLE:
		return v.getDouble();

	default:
		return Value();
	}
}

void ConverterApp::init()
{

	// Length
	conversions.insertNode(&m);
	conversions.insertNode(&dm);
	conversions.insertNode(&cm);
	conversions.insertNode(&mm);

	conversions.connect(&m, &dm, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 10); });
	conversions.connect(&dm, &cm, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 10); });
	conversions.connect(&cm, &mm, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 10); });

	conversions.connect(&dm, &m, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 10); });
	conversions.connect(&cm, &dm, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 10); });
	conversions.connect(&mm, &cm, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 10); });

	// Area
	conversions.insertNode(&m2);
	conversions.insertNode(&dm2);
	conversions.insertNode(&cm2);
	conversions.insertNode(&mm2);

	conversions.connect(&m2, &dm2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 100); });
	conversions.connect(&dm2, &cm2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 100); });
	conversions.connect(&cm2, &mm2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 100); });

	conversions.connect(&dm2, &m2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 100); });
	conversions.connect(&cm2, &dm2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 100); });
	conversions.connect(&mm2, &cm2, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 100); });

	// Volume
	conversions.insertNode(&m3);
	conversions.insertNode(&dm3);
	conversions.insertNode(&cm3);
	conversions.insertNode(&mm3);
	conversions.insertNode(&l);

	conversions.connect(&m3, &dm3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 1000); });
	conversions.connect(&dm3, &cm3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 1000); });
	conversions.connect(&cm3, &mm3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 1000); });

	conversions.connect(&dm3, &m3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 1000); });
	conversions.connect(&cm3, &dm3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 1000); });
	conversions.connect(&mm3, &cm3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 1000); });
	
	conversions.connect(&m3, &l, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 1000); });
	conversions.connect(&l, &m3, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() / 1000); });

	// Temperature
	conversions.insertNode(&dC);
	conversions.insertNode(&dF);
	conversions.insertNode(&dK);

	conversions.connect(&dC, &dF, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() * 9/5 + 32); });
	conversions.connect(&dC, &dK, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() + 273.15); });

	conversions.connect(&dF, &dC, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value((toDouble(v).getDouble() - 32) * 5/9); });
	conversions.connect(&dK, &dC, [](Value v)->Value { return toDouble(v).isUndefined() ? Value() : Value(toDouble(v).getDouble() - 273.15); });

}

const Unit* const byName(std::string name)
{
	if (name == "m") return &m;
	if (name == "dm") return &dm;
	if (name == "cm") return &cm;
	if (name == "mm") return &mm;

	if (name == "m^2") return &m2;
	if (name == "dm^2") return &dm2;
	if (name == "cm^2") return &cm2;
	if (name == "mm^2") return &mm2;
	if (name == "m2") return &m2;
	if (name == "dm2") return &dm2;
	if (name == "cm2") return &cm2;
	if (name == "mm2") return &mm2;

	if (name == "m^3") return &m3;
	if (name == "dm^3") return &dm3;
	if (name == "cm^3") return &cm3;
	if (name == "mm^3") return &mm3;
	if (name == "m3") return &m3;
	if (name == "dm3") return &dm3;
	if (name == "cm3") return &cm3;
	if (name == "mm3") return &mm3;

	if (name == "l") return &l;

	if (name == "degC") return &dC;
	if (name == "degF") return &dF;
	if (name == "degK") return &dK;

	assert(false);
	return nullptr;
}

Status ConverterApp::run(const char* const _args[], int count)
{
	std::vector<std::string> args{};
	for (size_t i = 0; i < count; ++i) {
		args.push_back({ _args[i] });
	}

	const auto ap = ArgParser(ArgParser::Scheme({ "-v" }, {}));
	bool v = ap.parse(args).count("-v") > 0;


	// from to val...
	if (args.size() < 3) {
		return Status(StatusCode::SC_ERROR_INSUFFICIENT_DATA, "specify <from> <to> <val> [val ...]");
	}

	auto from = args[0];
	auto to = args[1];

	if (byName(from) == nullptr)
		tolowercase(from);

	if (byName(to) == nullptr)
		tolowercase(to);

	if (byName(from) == nullptr || byName(to) == nullptr) {
		return Status(StatusCode::SC_ERROR_NOT_FOUND, "Convertsion '" + from + " -> " + to + "' was not found");
	}

	auto p = conversions.searchPath(byName(from), byName(to));

	for (size_t i = 2; i < args.size(); ++i) {
		//auto val = c->f(parse(args[i]));
		Value val = parse(args[i]);

		for (const auto& c : p) {
			val = (*c)(val);
			if (val.isUndefined()) {
				return Status(StatusCode::SC_ERROR_INVALID, "Invalid conversion was performed?");
			}
		}

		if (v) {
			outval(parse(args[i]), ConverterApp::stream);
			ConverterApp::stream << " " << byName(from)->shortname << " -> ";
			outval(val, ConverterApp::stream);
			ConverterApp::stream << " " << byName(to)->shortname << std::endl;

		}
		else {
			outval(val, ConverterApp::stream);
			ConverterApp::stream << " ";
		}
	}

	if (!v) ConverterApp::stream << std::endl;	

	return Status::OK();
}
