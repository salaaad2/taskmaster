#pragma once

template <typename T>
std::string JoinStrings(
		const T & source,
		const std::string & separator = " ",
		const std::string & terminator = "")
{
	std::string out;
	for (auto it = source.begin(); it != source.end(); ++it)
	{
		out.append(separator);
		out.append(*it);
	}
	out.append(terminator);
	return out;
}
