#include "pch.h"
#include "ArgParser.h"


ArgParser::Scheme::Scheme(const vec& t, const vec& v)
{
	for (const auto& i : t)
		_toggles.insert(i);

	for (const auto& i : v)
		_wvalue.insert(i);
}

//ArgParser::Scheme::Scheme(std::initializer_list<std::pair<vec, vec>> l)
//{
//	for (auto it = l.begin(); it != l.end(); it++) {
//
//	}
//}


std::map<std::string, std::string> ArgParser::parse(std::vector<std::string>& args) const
{
	std::map<std::string, std::string> result;
	std::vector<std::string> untouched;

	args.push_back("");

	for (size_t i = 0; i < args.size() - 1; ++i) {
		const auto& arg = args[i];
		const auto& narg = args[i + 1];

		if (s.toggles().count(arg) > 0) {
			result.insert({arg, ""});
		}
		else if (s.wvalue().count(arg) > 0) {
			result.insert({ arg, narg });
			i++;
		}
		else {
			untouched.push_back(arg);
		}
	}

	args = untouched;

	return result;
}
