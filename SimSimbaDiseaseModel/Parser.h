#ifndef PARSER_H
#define PARSER_H

#include "stdafx.h"
#include "Basics.h"

using namespace std;
namespace SimSimba {

	class Parser {
	public:
		Parser() {}
		~Parser();

		Param Parse(string fname);

	private:
		Param param;

		void SetGlobalVariable(string token, string value);
		void ParseFightTable(list<string> fightnums);
		void Prefill();
		void Error(string par, string val);
		void CalculateLifeExpectancy();


	};
}
#endif

