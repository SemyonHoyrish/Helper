#pragma once
#include <unordered_set>
#include <map>
#include <vector>
#include <initializer_list>


class ArgParser
{
	template <typename T>
	using set_t = typename std::unordered_set<T>;

	using set = set_t<std::string>;
	using vec = std::vector<std::string>;

public:
	class Scheme
	{
	public:
		Scheme(const vec& t, const vec& v);
		//Scheme(std::initializer_list<std::pair<vec, vec>> l);

		const set& toggles() const
		{
			return _toggles;
		}

		const set& wvalue() const
		{
			return _wvalue;
		}

	private:
		set _toggles;
		set _wvalue;
	};

	ArgParser(Scheme s) : s(s) {}

	std::map<std::string, std::string> parse(std::vector<std::string>& args) const;
	//std::map<std::string, std::string> parse(int argc, std::string argv[]);

private:

private:
	const Scheme s;

};



