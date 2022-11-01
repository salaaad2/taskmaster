#pragma once

#include <iostream>
#include <vector>

template <typename T>
std::vector<const char*> JoinStrings(
		const std::string & name,
		const T & source,
		const std::string & separator = "",
		const std::string & terminator = "")
{
	(void)separator;
	(void)terminator;
	std::vector<const char*> out;
	out.push_back(name.c_str());
	for (auto it = source.begin(); it != source.end(); ++it)
	{
		out.push_back((*it).c_str());
	}
	out.push_back(NULL);
	return out;
}
