#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>
using namespace std;
enum tokenType {tID, tCLASS, tIDENTIFIER, tLWING, tRWING, tCOLON, tSEMICOLON, tUNDEF};
enum parserState {pSelector,pAfterSelector,pInBlock,pAfterBlock,pUndef};

std::string& trim(std::string& s)
{
	if (s.empty())
	{
		return s;
	}

	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}

bool IsNameStartCodePoint(char c) {
	//from Chromium third_party/blink/renderer/core/css/parser/css_parser_idioms.h line 54
	return ((c | 0x20) >= 'a' && (c | 0x20) <= 'z') || c == '_' || (c & ~0x7F);
}

bool IsNonPrintableCodePoint(char cc) {
	//from Chromium third_party/blink/renderer/core/css/parser/css_tokenizer.cc line 464
	return (cc >= '\0' && cc <= '\x8') || cc == '\xb' ||
		(cc >= '\xe' && cc <= '\x1f') || cc == '\x7f';
}

bool TwoCharsAreValidEscape(char first, char second) {
	//from Chromium third_party/blink/renderer/core/css/parser/css_parser_idioms.h line 65
	return first == '\\' && !(second == '\r' || second == '\n' || second == '\f');
}



class Token {
public:
	Token(tokenType type) { type_ = type; }
	Token(tokenType type, std::string value) { type_ = type; value_ = value; }

	void set_value(const std::string value) {
		value_ = value;
	};
	std::string get_value() {
		return value_;
	};
	void set_type(const tokenType type) {
		type_ = type;
	};
	tokenType get_type() {
		return type_;
	};
private:
	std::string value_;
	tokenType type_;
};

class Attribute {
public:
	Attribute(std::string name, std::string value) { name_ = name; value_ = value; }
	void set_value(const std::string value) {
		value_ = value;
	};
	std::string get_value() {
		return value_;
	};
	void set_name(const std::string name) {
		name_ = name;
	};
	std::string get_name() {
		return name_;
	};
private:
	std::string name_;
	std::string value_;
};

class Element {
public:
	Element() {}
	Element(Token token) { selector_token_.push_back(token); }

	void add_tag(const Token token) {
		selector_token_.push_back(token);
	};
	std::vector<Token> get_tag() {
		return selector_token_;
	};
	void add_attribute(const Attribute attribute) {
		attributes_.push_back(attribute);
	};
	std::vector<Attribute> get_attributes() {
		return attributes_;
	};
private:
	std::vector<Token> selector_token_;
	std::vector<Attribute> attributes_;
};

class CPPParser {
public:
	CPPParser(std::string filename) {
		filename_ = filename;
		state_ = pSelector;
	}

	void loadFileToParse() {
		// Read the file specified by filename_ and save in a std::string data structure
		std::ifstream CPPFile(filename_.c_str());
		std::ostringstream buf;
		char ch;
		while (buf && CPPFile.get(ch)) {
			buf.put(ch);
		}
		input_ = buf.str();
		length_ = input_.length();

		parse();
	}

	void parse() {
		// Keep parsing until no token left
		while (has_next_token()) {
			parse_rule();

			/*
			cout << "elements size: " << elements_.size() << "\n";
			cout << "offset: " << offset_ << "\n";
			cout << "state: " << state_ << "\n";
			*/

		}
	}

	void print();

private:
	std::string filename_;
	std::string input_;
	std::vector<Element> elements_;
	parserState state_;
	int offset_;
	int length_;

	bool has_next_token();
	bool has_next_token(tokenType type);
	Token expect_token(tokenType type);
	Token next_token();
	Token parse_next_token();
	bool nextCharsAreIdentifier();
	std::string consumeIdentifier();
	void parse_rule();
	Token parse_selector();
	bool isSpaceOrNewLine();
	bool consumeSpaceAndNewline();
	std::vector<std::string> split(const std::string& str, const std::string& delim);
	bool isValidURL(std::string& str);
	bool checkEOF();
}; 







