#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "05";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2011";
	static const char UBUNTU_VERSION_STYLE[] = "11.11";
	
	//Software Status
	static const char STATUS[] = "Release";
	static const char STATUS_SHORT[] = "r";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 6;
	static const long BUILD = 16;
	static const long REVISION = 55;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,6,16,55
	#define RC_FILEVERSION_STRING "1, 6, 16, 55\0"
	static const char FULLVERSION_STRING[] = "1.6.16.55";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 10;
	

}
#endif //VERSION_H
