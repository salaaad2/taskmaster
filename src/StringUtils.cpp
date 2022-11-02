#include "StringUtils.hpp"

std::vector<std::string> SplitString(
	std::string source,
	const std::string &separator)
{
	std::vector<std::string> out;
	int last_separator_pos = 0;

	while (source.find(separator) != std::string::npos)
	{
		out.push_back(source.substr(last_separator_pos, source.find(separator)));
		source.erase(0, source.find(separator) + separator.length());
	}
	return out;
}
