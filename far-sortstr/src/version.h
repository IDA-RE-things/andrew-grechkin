#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "26";
	static const char MONTH[] = "11";
	static const char YEAR[] = "2010";
	static const char UBUNTU_VERSION_STYLE[] = "10.11";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 1;
	static const long MINOR = 4;
	static const long BUILD = 13;
	static const long REVISION = 53;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1;
	#define RC_FILEVERSION 1,4,13,53
	#define RC_FILEVERSION_STRING "1, 4, 13, 53\0"
	static const char FULLVERSION_STRING[] = "1.4.13.53";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 10;
	

}
#endif //VERSION_H
