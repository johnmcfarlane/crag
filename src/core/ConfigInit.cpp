//
//  ConfigInit.cpp
//  crag
//
//  Created by john on 6/26/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ConfigEntry.h"

#include "core/app.h"

#if defined(ENABLE_CONFIG)

using namespace ::crag::core::config;

namespace
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// types

	// a line from a config file
	struct Line
	{
		int number;
		bool is_default;
		char * key;
		char * value;
	};

	// complete set of lines from a config file
	using Lines = std::vector<Line>;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// constants

	// loaded from working directory
	constexpr auto config_filename = "crag.cfg";

	// parser constants
	constexpr auto comment = '#';
	constexpr auto separator = '=';
	constexpr auto new_line = '\n';
	constexpr auto terminator = '\0';
	constexpr char breakset[3] = {separator, new_line, terminator};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// functions

	// sorts lines alphabetically by key; returns false if keys arent unique
	bool Sort(Lines & lines)
	{
		auto begin = std::begin(lines), end = std::end(lines);

		// sort
		std::sort(begin, end, [] (Line const & lhs, Line const & rhs)
		{
			return std::strcmp(lhs.key, rhs.key) < 0;
		});

		// look for duplicates
		auto duplicate = std::adjacent_find(begin, end, [] (Line const & lhs, Line const & rhs)
		{
			return std::strcmp(lhs.key, rhs.key) == 0;
		});

		// if there's a duplicate
		if (duplicate != end)
		{
			// report first duplicate
			auto match = duplicate + 1;
			auto key = duplicate->key;
			int numbers[2] = { duplicate->number, match->number };

			ASSERT(numbers[0] != numbers[1]);
			ASSERT(! std::strcmp(key, match->key));

			ERROR_MESSAGE(
				"Duplicate entry, '%s', found on lines %d and %d of '%s'",
				key,
				std::min(numbers[0], numbers[1]),
				std::max(numbers[0], numbers[1]),
				config_filename);

			return false;
		}

		return true;
	}

	// parses/tokenizes contents of config file contents, in_buffer and writes to lines
	bool Read(app::FileResource & in_buffer, Lines & lines)
	{
		auto line_begin = in_buffer.data();

		// for each line
		for (auto line_number = 1; *line_begin != terminator; ++ line_number)
		{
			// locate and return the far end of the given token;
			// null-terminate the token; report error and return null iff error detected
			auto delimit = [&] (char * begin, char delimiter) -> char *
			{
				// find the end of the token
				ASSERT(strchr(breakset, delimiter));
				auto end = std::strpbrk(begin, breakset);

				// print out detailed diagnostic error message
				auto report = [line_number, line_begin](char const * message, char const * error_pos)
				{
					ERROR_MESSAGE("%s:%d:%d: %s", config_filename, line_number, int(1 + error_pos - line_begin), message);
				};

				// error checking
				if (! end)
				{
					report("missing new-line character", std::strchr(begin, terminator));
					return nullptr;
				}
				if (end == begin)
				{
					report((delimiter == separator) ? "missing key" : "missing value", end);
					return nullptr;
				}
				if (* end != delimiter)
				{
					ASSERT(delimiter == separator);
					report("missing '='", end);
					return nullptr;
				}

				// terminate the token
				* end = terminator;

				// return end of token
				return end;
 			};

			// is_default
			auto is_default = ((*line_begin) == comment);

			// key
			auto key_begin = line_begin + std::size_t(is_default);
			auto key_end = delimit(key_begin, separator);
			if (! key_end)
			{
				return false;
			}

			// value
			auto value_begin = key_end + 1;
			auto value_end = delimit(value_begin, new_line);
			if (! value_end)
			{
				return false;
			}

			// add the new entry to the list
			lines.push_back({line_number, is_default, key_begin, value_begin});

			// point to the start of the next line
			line_begin = value_end + 1;
		}

		return true;
	}

	// writes a combination of lines and config entry values to out_buffer
	bool Write(app::FileResource & out_buffer, Lines const & lines)
	{
		auto entry_iterator = Entry::begin(), entry_end = Entry::end();
		auto line_iterator = std::begin(lines), line_end = std::end(lines);

		for (auto line_number = 1; ; ++ line_number)
		{
			auto has_entry = entry_iterator != entry_end;
			auto has_line = line_iterator != line_end;

			// choose the next config value to write
			int comparison = int(has_line) - int(has_entry);
			if (comparison == 0)
			{
				if (! has_entry)
				{
					// all done
					break;
				}

				comparison = std::strcmp(entry_iterator->GetKey(), line_iterator->key);
			}

			// determine what to write back to the config file
			bool is_default;
			char const * key, * value;

			constexpr int max_buffer_size = 1024;
			char buffer[max_buffer_size + 1];

			// if no line OR the line is commented out
			if (comparison < 0 || (comparison == 0 && line_iterator->is_default))
			{
				// use the hard-coded default value
				auto & entry = static_cast<Entry const &>(* entry_iterator);

				// will be commented out to denote it's the default
				is_default = true;

				// key
				key = entry.GetKey();

				// value (held in temp buffer)
				value = buffer;

				// populate the buffer
				if (! entry.Get(buffer, max_buffer_size))
				{
					ERROR_MESSAGE("Error serializing %s", key);
					return false;
				}
			}
			else
			{
				// use the line from the config file
				is_default = line_iterator->is_default;

				// key
				key = line_iterator->key;

				// value
				value = line_iterator->value;

				if (comparison == 0)
				{
					ASSERT(! is_default);
					auto & entry = static_cast<Entry &>(* entry_iterator);
					entry.Set(line_iterator->value);
				}
				else
				{
					ERROR_MESSAGE("Unrecognized config key, '%s', on line %d of %s",
							key, line_number, config_filename);
				}
			}

			if (is_default)
			{
				out_buffer.push_back(comment);
			}

			auto append_string = [& out_buffer] (char const * string)
			{
				auto l = strlen(string);
				auto s = out_buffer.size();
				out_buffer.resize(l + s);
				std::memcpy(& out_buffer[s], string, l);
			};

			append_string(key);
			out_buffer.push_back(separator);
			append_string(value);
			out_buffer.push_back(new_line);

			if (comparison <= 0)
			{
				++ entry_iterator;
			}

			if (comparison >= 0)
			{
				++ line_iterator;
			}
		}

		return true;
	}

	// loads config file
	bool LoadSave()
	{
		Lines lines;

		// load
		auto in_buffer = app::LoadFile(config_filename, true), tmp_buffer = in_buffer;
		if (! in_buffer.data())
		{
			ERROR_MESSAGE("Failed to load config file, \"%s\". Generating fresh copy.", config_filename);
		}
		else if (! (Read(tmp_buffer, lines) && Sort(lines)))	// read
		{
			ERROR_MESSAGE("Failed to parse config file, \"%s\". (Hint: delete it for a fresh copy.)", config_filename);
			return false;
		}

		// write
		decltype(in_buffer) out_buffer;
		out_buffer.reserve((in_buffer.size() * 0x100) >> 8);
		if (! Write(out_buffer, lines))
		{
			ERROR_MESSAGE("Error generating config file, \"%s\".", config_filename);
			return false;
		}

		// save
		auto has_changed = [] (app::FileResource const & in_buffer, app::FileResource const & out_buffer)
		{
			if (in_buffer.empty())
			{
				return true;
			}

			ASSERT (in_buffer.back() == terminator);
			auto in_size = in_buffer.size() - 1;
			auto out_size = out_buffer.size();
			return out_size != in_size
					|| ! std::equal(std::begin(in_buffer), std::begin(in_buffer) + in_size, std::begin(out_buffer));
		};

		if (has_changed(in_buffer, out_buffer))
		{
			// only save if there's any change
			if (! app::SaveFile(config_filename, out_buffer))
			{
				return false;
			}
		}

		return true;
	}

	// apply any changes to config entries as a result of command-line
	bool ParseCommandLine(int argc, char * const * argv)
	{
		auto get_value = [] (char * const argument) -> char const *
		{
			auto found = strchr(argument, '=');
			if (found == nullptr)
			{
				return "1";
			}

			* found = '\0';
			return found + 1;
		};

		for (; argc > 0; ++ argv, -- argc)
		{
			auto key_string = * argv;
			auto value_string = get_value(* argv);

			// Get the parameter in question, given its key.
			auto parameter = Entry::find(key_string);
			if (parameter == nullptr)
			{
				ERROR_MESSAGE("unrecognised command line parameter \"%s\"", key_string, * argv);
				return false;
			}

			parameter->Set(value_string);
		}

		return true;
	}
}

bool crag::core::config::Init(int argc, char * const * argv)
{
	return LoadSave() && ParseCommandLine(argc, argv);
}

#else

bool crag::core::config::Init(int, char * const *)
{
	return true;
}

#endif	// ENABLE_CONFIG
