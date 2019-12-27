#pragma once
#include <string>


// ToDo: Add functionality to convert string to Arduino String
namespace EmotiBit
{
	class String
	{
	public:

		std::string str;

		String()
		{
			str = "";
		}

		String(std::string s) :
			str(s) {}

		size_t indexOf(char val, size_t from) const
		{
			return str.find_first_of(val, from);
		}

		String substring(size_t from, size_t to) const
		{
			return String(str.substr(from, to - from));
		}

		bool equals(String s) const
		{
			return str.compare(s.str) == 0;
		}

		size_t length() const
		{
			return str.length();
		}

	private:
	};
}