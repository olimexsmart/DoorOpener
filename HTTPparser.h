/*
	HTTPparser for Arduino
*/
#ifndef HTTP_parser
#define HTTP_parser

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>

class HTTPparser {
	
	public:	
	HTTPparser(unsigned int pathAllocation, unsigned int messageAllocation);
	~HTTPparser();
	void ParseChar(char c);
	void Reset();
	bool IsValid();
	void AllSheWrote();
	const char * MethodString();
	char * Path;
	char * Message;	
	
	typedef enum {
		GET,
		POST,
		NR
	} MethodType;
	MethodType Method;
	
	private:
	HTTPparser();
	char method[6];
	unsigned int index;
	unsigned int pathAllocation;
	unsigned int messageAllocation;
	
	typedef enum {
		ERROR,
		DONE,
		METHOD,
		PATH,
		HEADERS,
		BLANK,
		MESSAGE,
		FULL
	} Status;
	
	Status status;
	
	const char * methodStrings[3] = { 
			"GET", 
			"POST", 
			"NR" 
			};
	
};

#endif