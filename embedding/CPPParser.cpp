#include <stdio.h>
#include <vector>
#include<algorithm>
#include <string>
#include<sstream>
#include<iostream>
#include "CPPParser.h"
using namespace std;



bool CPPParser::has_next_token() {
	//check if there is any other token left
	CPPParser::consumeSpaceAndNewline();
	return (offset_ < length_);
};

bool CPPParser::has_next_token(tokenType type) {
	//check if the net token is of the given type
	if (!has_next_token()) {
		return false;
	}

	int originalOffset = offset_;
	Token nextToken = next_token();
	bool result = (nextToken.get_type() == type);
	offset_ = originalOffset;
	return result;
};

Token CPPParser::expect_token(tokenType type) {
	//return the next token if it is of the given type
	if (!has_next_token()) {
		return Token(tUNDEF);
	}

	if (has_next_token(type)) {
		return parse_next_token();
	}
	return Token(tUNDEF);
};

Token CPPParser::next_token() {
	//get the next token
	//assume that has_next_token() is true

	tokenType typeOfNextToken;
	if (input_[offset_] == '{') {
		return Token(tLWING);
	}

	if (input_[offset_] == '}') {
		return Token(tRWING);
	}

	if (input_[offset_] == ':') {
		return Token(tCOLON);
	}

	if (input_[offset_] == ';') {
		return Token(tSEMICOLON);
	}

	if (input_[offset_ == '#']) {
		if (isalpha(input_[offset_ + 1])) {
			return Token(tID);
		}
		return Token(tUNDEF);
	}

	if (input_[offset_ == '.']) {
		if (isalpha(input_[offset_ + 1])) {
			return Token(tCLASS);
		}
		return Token(tUNDEF);
	}

	if (IsNameStartCodePoint(input_[offset_])) {
		return Token(tIDENTIFIER, consumeIdentifier());
	}

	return Token(tUNDEF);
}

Token CPPParser::parse_next_token() {
	//get the next token
	//assume that has_next_token() is true
	if (!has_next_token()) {
		return Token(tUNDEF);
	}

	tokenType typeOfNextToken;
	if (input_[offset_] == '{') {
		offset_++;
		return Token(tLWING);
	}

	if (input_[offset_] == '}') {
		offset_++;
		return Token(tRWING);
	}

	if (input_[offset_] == ':') {
		offset_++;
		return Token(tCOLON);
	}

	if (input_[offset_] == ';') {
		offset_++;
		return Token(tSEMICOLON);
	}

	if (input_[offset_ == '#']) {
		offset_++;
		if (IsNameStartCodePoint(input_[offset_])) {
			std::string identifier = consumeIdentifier();
			return Token(tID, identifier);
		}
		return Token(tUNDEF);
	}

	if (input_[offset_ == '.']) {
		offset_++;
		if (IsNameStartCodePoint(input_[offset_])) {
			std::string identifier = consumeIdentifier();
			return Token(tCLASS, identifier);
		}
		return Token(tUNDEF);
	}

	if (isalpha(input_[offset_])) {
		std::string identifier = consumeIdentifier();
		return Token(tIDENTIFIER, identifier);
	}

	return Token(tUNDEF);
};

void CPPParser::parse_rule() {
	//parse a complete rule for a particular element
	switch (state_) {
		case pSelector: {
			// TODO: add detection of EOF and corresponding ending procedure
			consumeSpaceAndNewline();
			Element currElement = Element();
			Token firstSelector = parse_selector();
			if (firstSelector.get_type() != tUNDEF) {
				currElement.add_tag(firstSelector);
				elements_.push_back(Element(firstSelector));
				state_ = (state_ == pInBlock) ? pInBlock : pAfterSelector;
			}
			else {
				state_ = pUndef;
			}
			break;
		}
		case pAfterSelector: {

			// expect a '{' token to enter a block or a selector to further add to attribute list
			consumeSpaceAndNewline();
			if (has_next_token(tLWING)) {
				next_token();
				state_ = pInBlock;
			}
			else {
				Token nextSelector = parse_selector();
				if (nextSelector.get_type() != tUNDEF) {
					elements_.at(elements_.size() - 1).add_tag(nextSelector);
				}
				else {
					elements_.pop_back();
					state_ = pUndef;
				}
			}
			break;
		}
		case pInBlock: {
			// save all the content inside the block into a buffer and split the buffer first by ';' then by ':' to get all attribute directives.
			// On top of that we should address the problem that the ":" literal legitimately appears inside URLs.
			consumeSpaceAndNewline();
			int lookforwardOffset = offset_;
			while (lookforwardOffset < length_ && input_[lookforwardOffset] != '}') {
				lookforwardOffset++;
			}
			if (lookforwardOffset >= length_) {
				offset_ = lookforwardOffset;
				elements_.pop_back();
				return;
			}

			std::string blockContent = input_.substr(offset_, lookforwardOffset - offset_);
			//TODO: split blcokContent by ';' into attributes and parse attribute one by one
			//In the parsing of directive, use the mehod that we find the offset of the last ":" appearing in the directive and split it by that dilimeter(because ":" is valid in URL but not in attribute values)
			//And then we check if the name part of attribute is a valid URL possibly quoted. Add attribute only if this check is passed.
			std::vector<std::string> directives = split(blockContent, ";");
			int vSize = directives.size();
			for (int i = 0; i < vSize; i++) {
				std::string directive = directives.at(i);
				
				int offsetSearchForLastColon = directive.length() - 1;
				while (offsetSearchForLastColon > 0 && directive[offsetSearchForLastColon] != ':') {
					offsetSearchForLastColon--;
				}
				if (offsetSearchForLastColon == 0) {
					continue;
				}
				else {
					//TODO: shall we stripe newline and space in dirName and dirVal?
					std::string dirName = directive.substr(0, offsetSearchForLastColon);
					if (!isValidURL(dirName)) {
						continue;
					}
					for (int j = 0; j < dirName.length(); j++) {
						if (dirName[j] == ' ' || dirName[j] == '\n' || dirName[j] == '\r' || dirName[j] == '\t' || dirName[j] == '\f' || (int)dirName[j] == 10) {
							dirName.erase(j, 1);
						}
					}
					std::string dirVal = directive.substr(offsetSearchForLastColon + 1, vSize - offsetSearchForLastColon - 1);
					elements_.at(elements_.size() - 1).add_attribute(Attribute(trim(dirName), trim(dirVal)));
				}
			}
			offset_ = lookforwardOffset;
			state_ = pAfterBlock;
			break;
		}
		case pAfterBlock: {
			// end of parsing of the current element. Consume whitespace and check EOF. Quit at EOF and jump to pSelector otherwise.
			consumeSpaceAndNewline();
			if (checkEOF()) {
				return;
			}
			if (expect_token(tRWING).get_type() != tUNDEF) {
				state_ = pSelector;
				break;
			}
			else {
				state_ = pUndef;
				break;
			}
		}

		case pUndef: {
			// Error state. consume until finding a "}" token then jump to pAfterBlock.
			while (has_next_token() && input_[offset_] != '}') {
				offset_++;
			}
			state_ = pAfterBlock;
			break;
		}
			default:
				break;
	}
};

Token CPPParser::parse_selector() {
	//Parse the selector from the script (used in parse_rule())
	//Upon successfully parsed a legitimate selector, consume it and return its tokenized Token.
	//Upon failure, offset back to before the function has run and return Token(tUNDEF)
	switch (input_[offset_]) {
		case '#': {
			offset_++;
			if (nextCharsAreIdentifier()) {
				Token selector = Token(tID);
				selector.set_value(consumeIdentifier());
				return selector;
			}
			else {
				return Token(tUNDEF);
			}
			break;
		}
		case '.': {
			offset_++;
			if (nextCharsAreIdentifier()) {
				Token selector = Token(tCLASS);
				selector.set_value(consumeIdentifier());
				return selector;
			}
			else {
				return Token(tUNDEF);
			}
			break;
		}
		default: {
			if (nextCharsAreIdentifier()) {
				Token selector = Token(tIDENTIFIER);
				selector.set_value(consumeIdentifier());
				return selector;
			}
			else {
				return Token(tUNDEF);
			}
		}
	}
};

// check whether the next characters of input are a identifier
// borrowed from Chromium third_party / blink / renderer / core / css / parser / css_parser_idioms.cc  line 74
bool CPPParser::nextCharsAreIdentifier() {
	if (offset_ + 1 == length_) {
		return IsNameStartCodePoint(input_[length_]);
	}

	char first = input_[offset_];
	char second = input_[offset_ + 1];

	if (IsNameStartCodePoint(first) || TwoCharsAreValidEscape(first, second))
		return true;

	if (first == '-') {
		return IsNameStartCodePoint(second) || second == '-' ||
			(offset_ + 2 < length_ && TwoCharsAreValidEscape(second, input_[offset_ + 2]));
	}
}

// auxillary functions needed to complete the CPPParser class
std::string CPPParser::consumeIdentifier() {
	// consume one identifier-like literal sequence
	// ALWAYS used after checking method nextCharsAreIdentifier()
	// borrow from css/css_tokenizer.cc, to be implemented yet
	// TODO: consume until a non-valid char inside an identifier appears, and then record all chars consumed to be the identifier value.
	std::string name = "";
	if (IsNameStartCodePoint(input_[offset_])) {
		name = name + input_[offset_];
		offset_++;
	}
	else {
		return "";
	}
	while (has_next_token()) {
		if (IsNameStartCodePoint(input_[offset_])) {
			name = name + input_[offset_++];
		}
		else if (isdigit(input_[offset_]) || input_[offset_] == '-') {
			name = name + input_[offset_++];
		}
		else if (input_[offset_] == '{') {
			// meets a '{' character which indicates the parsing enters a block
			offset_++;
			state_ = pInBlock;
			return name;
		}
		else if (isSpaceOrNewLine()) {
			// meets a space or newline after a valid identifier. Return what consumed
			consumeSpaceAndNewline();
			return name;
		}
		else if (TwoCharsAreValidEscape(input_[offset_], input_[offset_ + 1])) {
			name = name + input_.substr(offset_, 2);
			offset_ = offset_ + 2;
		}
		else {
			// meets an invalid character in an identifier. Consume till space or new line and return a null string
			while (!isSpaceOrNewLine()) { offset_++; }
			consumeSpaceAndNewline();
			return "";
		}

		if (offset_ == length_) {
			return name;
		}
	}

}

bool CPPParser::isSpaceOrNewLine() {
	// consume until the next character is not space, \n, \t, \r or \f.
	// borrowed from Chromium third_party/blink/renderer/core/html/parser/html_parser_idioms.h Line 81
	if (input_[offset_] <= ' ' &&
		(input_[offset_] == ' ' || input_[offset_] == '\n' || input_[offset_] == '\t' ||
			input_[offset_] == '\r' || input_[offset_] == '\f')) {
		return true;
	}

	return false;
}

bool CPPParser::consumeSpaceAndNewline() {
	// consume until the next character is not space, \n, \t, \r or \f.
	// borrowed from Chromium third_party/blink/renderer/core/html/parser/html_parser_idioms.h Line 81
	// Adopted from a true-or-false method to a judge-and-consume method
	if (!isSpaceOrNewLine()) {
		return false;
	}

	while (isSpaceOrNewLine()) {
		offset_++;
	}

	return true;
}

std::vector<std::string> CPPParser::split(const std::string& str, const std::string& delim) {
	//split a string into tokens by the specified delimiter
	std::vector<std::string> tokens;
	size_t prev = 0, pos = 0;
	do {
		pos = str.find(delim, prev);
		if (pos == std::string::npos) {
			pos = str.length();
		}
		std::string token = str.substr(prev, pos - prev);
		if (!token.empty()) {
			tokens.push_back(token);
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

bool CPPParser::isValidURL(std::string& str) {
	//check if a given string is a syntactically valid URL string possibly quoted.
	//borrow code from the Chromium project, third_party/blink/renderer/core/css/parser/css_tokenizer.cc line 470

	//if the string is quoted, de-quote it first
	if (str[0] == '\'' && str[str.length() - 1] == '\'') {
		str.erase(0, 1);
		str.erase(str.length() - 1, 1);
	}
	else if (str[0] == '\"' && str[str.length() - 1] == '\"') {
		str.erase(0, 1);
		str.erase(str.length() - 1, 1);
	}

	consumeSpaceAndNewline();

	int offset = 0;
	char cc;
	while (offset < str.length()) {
		cc = str[offset];
		if (cc == '"' || cc == '\'' || cc == '(' || cc == ')' || IsNonPrintableCodePoint(cc)) {
			return false;
		}

		if (cc == '\\') {
			if (TwoCharsAreValidEscape(cc, str[offset + 1])) {
				continue;
			}
			return false;
		}

		offset++;
	}



	return true;
}

bool CPPParser::checkEOF() {
	//check end of file
	return input_[offset_] == EOF;
}

void CPPParser::print() {
	//TODO: print out all the information stored in every Element of the elements_ vector.
	//The specific form of information is: (Element.selector_token_[i].type, Element.selector_token_[i].value, Element.attributes_[i].name_, Element.attributes[i].value_)
	//Read them out by object method call and print out.
	int counter = 0;
	std::cout << input_ << "\n";
	for (std::vector<Element>::iterator eleIter = elements_.begin(); eleIter != elements_.end(); eleIter++) {
		printf("**********Element info for element #%d**********\n", counter);
		printf("List of selectors:\n");
		for (int i = 0; i < (*eleIter).get_tag().size(); i++) {
			std::cout << "Token type: " << (int)(*eleIter).get_tag()[i].get_type() << "; Token value:" << (*eleIter).get_tag()[i].get_value() << "\n";
		}
		printf("List of attributes:\n");
		for (int i = 0; i < (*eleIter).get_attributes().size(); i++) {
			std::cout << "Attribute name: " << (*eleIter).get_attributes()[i].get_name() << "; Attribut value: " << (*eleIter).get_attributes()[i].get_value() << "\n";
		}
		counter++;
	}
}

/*
int main(int argc, char** argv) {
	ofstream outfile;
	outfile.open("executed.txt");
	outfile << "foo" << endl;
	outfile.close();
	CPPParser parser = CPPParser(argv[1]);
	parser.loadFileToParse();
	parser.print();
}
*/