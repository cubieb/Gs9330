#ifdef _WIN32
#	pragma comment(lib, "Ws2_32.lib")
#	pragma comment(lib, "iconv.lib")
#	pragma comment(lib, "libxml2.lib")
#	pragma comment(lib, "zlib.lib")

#	ifdef _DEBUG
#		pragma comment(lib, "ACEd.lib")
#	else
#		pragma comment(lib, "ACE.lib")
#	endif
#endif