#include "Common.hpp"
#include "GUI.h"
#include "Editor.h"

int myargc;
char **myargv;

void Exit(void)
{
    Printf("Exiting app (code : 1)");
    exit(1);
}

void Error(const char *fmt, ...)
{
    va_list argptr;
    char buffer[4096];

    va_start(argptr, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, argptr);
    va_end(argptr);

    Editor::Get()->getGUI()->Print("ERROR: %s", buffer);
    Editor::Get()->getGUI()->Print("Exiting app (code : -1)");
    spdlog::critical("ERROR: {}", buffer);
    spdlog::critical("Exiting app (code : -1)");

    exit(-1);
}

void Printf(const char *fmt, ...)
{
    va_list argptr;
    char buffer[4096];

    va_start(argptr, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, argptr);
    va_end(argptr);

    spdlog::info("{}", buffer);
    Editor::Get()->getGUI()->Print("%s", buffer);
}

int GetParm(const char *parm)
{
    int i;

    for (i = 1; i < myargc - 1; i++) {
        if (!N_stricmp(myargv[i], parm))
            return i;
    }
    return -1;
}

#ifdef _WIN32
/*
=============
N_vsnprintf
 
Special wrapper function for Microsoft's broken _vsnprintf() function. mingw-w64
however, uses Microsoft's broken _vsnprintf() function.
=============
*/
int N_vsnprintf( char *str, size_t size, const char *format, va_list ap )
{
	int retval;
	
	retval = _vsnprintf( str, size, format, ap );

	if ( retval < 0 || (size_t)retval == size ) {
		// Microsoft doesn't adhere to the C99 standard of vsnprintf,
		// which states that the return value must be the number of
		// bytes written if the output string had sufficient length.
		//
		// Obviously we cannot determine that value from Microsoft's
		// implementation, so we have no choice but to return size.
		
		str[size - 1] = '\0';
		return size;
	}
	
	return retval;
}
#endif

int N_isprint( int c )
{
	if ( c >= 0x20 && c <= 0x7E )
		return ( 1 );
	return ( 0 );
}


int N_islower( int c )
{
	if (c >= 'a' && c <= 'z')
		return ( 1 );
	return ( 0 );
}


int N_isupper( int c )
{
	if (c >= 'A' && c <= 'Z')
		return ( 1 );
	return ( 0 );
}


int N_isalpha( int c )
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return ( 1 );
	return ( 0 );
}

bool N_isintegral(float f)
{
	return (int)f == f;
}


bool N_isanumber( const char *s )
{
#ifdef Q3_VM
    //FIXME: implement
    return qfalse;
#else
    char *p;

	if( *s == '\0' )
        return false;

	strtod( s, &p );

    return *p == '\0';
#endif
}

void N_itoa(char *buf, uint64_t bufsize, int i)
{
	snprintf(buf, bufsize, "%i", i);
}

void N_ftoa(char *buf, uint64_t bufsize, float f)
{
	snprintf(buf, bufsize, "%f", f);
}

void N_strcpy (char *dest, const char *src)
{
	char *d = dest;
	const char *s = src;
	while (*s)
		*d++ = *s++;
	
	*d++ = 0;
}

void N_strncpyz (char *dest, const char *src, size_t count)
{
	if (!dest)
		Error( "N_strncpyz: NULL dest");
	if (!src)
		Error( "N_strncpyz: NULL src");
	if (count < 1)
		Error( "N_strncpyz: bad count");
	
#if 1 
	// do not fill whole remaining buffer with zeros
	// this is obvious behavior change but actually it may affect only buggy QVMs
	// which passes overlapping or short buffers to cvar reading routines
	// what is rather good than bad because it will no longer cause overwrites, maybe
	while ( --count > 0 && (*dest++ = *src++) != '\0' );
	*dest = '\0';
#else
	strncpy( dest, src, count-1 );
	dest[ count-1 ] = '\0';
#endif
}

void N_strncpy (char *dest, const char *src, size_t count)
{
	while (*src && count--)
		*dest++ = *src++;

	if (count)
		*dest++ = 0;
}


char *N_strupr(char *s1)
{
	char *s;

	s = s1;
	while (*s) {
		if (*s >= 'a' && *s <= 'z')
			*s = *s - 'a' + 'A';
		s++;
	}
	return s1;
}

// never goes past bounds or leaves without a terminating 0
void N_strcat(char *dest, size_t size, const char *src)
{
	size_t l1;

	l1 = strlen(dest);
	if (l1 >= size)
		Error( "N_strcat: already overflowed" );

	N_strncpy( dest + l1, src, size - l1 );
}

char *N_stradd(char *dst, const char *src)
{
	char c;
	while ( (c = *src++) != '\0' )
		*dst++ = c;
	*dst = '\0';
	return dst;
}


/*
* Find the first occurrence of find in s.
*/
const char *N_stristr(const char *s, const char *find)
{
	char c, sc;
	size_t len;

	if ((c = *find++) != 0) {
		if (c >= 'a' && c <= 'z') {
	    	c -= ('a' - 'A');
		}
 	   	len = strlen(find);
    	do {
    		do {
        		if ((sc = *s++) == 0)
          			return NULL;
        		if (sc >= 'a' && sc <= 'z') {
          			sc -= ('a' - 'A');
        		}
      		} while (sc != c);
    	} while (N_stricmpn(s, find, len) != 0);
   		s--;
  	}
  	return s;
}

int N_replace(const char *str1, const char *str2, char *src, size_t max_len)
{
	size_t len1, len2, count;
	ssize_t d;
	const char *s0, *s1, *s2, *max;
	char *match, *dst;

	match = strstr(src, str1);

	if (!match)
		return 0;

	count = 0; // replace count

    len1 = strlen(str1);
    len2 = strlen(str2);
    d = len2 - len1;

    if (d > 0) { // expand and replace mode
        max = src + max_len;
        src += strlen(src);

        do { // expand source string
			s1 = src;
            src += d;
            if (src >= max)
                return count;
            dst = src;
            
            s0 = match + len1;

            while (s1 >= s0)
                *dst-- = *s1--;
			
			// replace match
            s2 = str2;
			while (*s2)
                *match++ = *s2++;
			
            match = strstr(match, str1);

            count++;
		} while (match);

        return count;
    } 
    else if (d < 0) { // shrink and replace mode
        do  { // shrink source string
            s1 = match + len1;
            dst = match + len2;
            while ( (*dst++ = *s1++) != '\0' );
			
			//replace match
            s2 = str2;
			while ( *s2 ) {
				*match++ = *s2++;
			}

            match = strstr( match, str1 );

            count++;
        } 
        while ( match );

        return count;
    }
    else {
	    do { // just replace match
    	    s2 = str2;
			while (*s2)
				*match++ = *s2++;

    	    match = strstr(match, str1);
    	    count++;
		}  while (match);
	}

	return count;
}

size_t N_strlen (const char *str)
{
	size_t count = 0;
    while (str[count]) {
        ++count;
    }
	return count;
}

char *N_strrchr(char *str, char c)
{
    char *s = str;
    size_t len = N_strlen(s);
    s += len;
    while (len--)
    	if (*--s == c) return s;
    return 0;
}

int N_strcmp (const char *str1, const char *str2)
{
    const char *s1 = str1;
    const char *s2 = str2;
	while (1) {
		if (*s1 != *s2)
			return -1;              // strings not equal    
		if (!*s1)
			return 1;               // strings are equal
		s1++;
		s2++;
	}
	
	return 0;
}

bool N_streq(const char *str1, const char *str2)
{
	const char *s1 = str1;
	const char *s2 = str2;
	
	while (*s2 && *s1) {
		if (*s1++ != *s2++)
			return false;
	}
	return true;
}

bool N_strneq(const char *str1, const char *str2, size_t n)
{
	const char *s1 = str1;
	const char *s2 = str2;

	while (*s1 && n) {
		if (*s1++ != *s2++)
			return false;
		n--;
	}
	return true;
}

int N_strncmp( const char *s1, const char *s2, size_t n )
{
	int c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int N_stricmpn (const char *str1, const char *str2, size_t n)
{
	int c1, c2;

	// bk001129 - moved in 1.17 fix not in id codebase
    if (str1 == NULL) {
    	if (str2 == NULL )
            return 0;
        else
            return -1;
    }
    else if (str2 == NULL)
        return 1;


	
	do {
		c1 = *str1++;
		c2 = *str2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int N_stricmp( const char *s1, const char *s2 ) 
{
	unsigned char c1, c2;

	if (s1 == NULL)  {
		if (s2 == NULL)
			return 0;
		else
			return -1;
	}
	else if (s2 == NULL)
		return 1;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 != c2) {
			if ( c1 <= 'Z' && c1 >= 'A' )
				c1 += ('a' - 'A');

			if ( c2 <= 'Z' && c2 >= 'A' )
				c2 += ('a' - 'A');

			if ( c1 != c2 ) 
				return c1 < c2 ? -1 : 1;
		}
	} while ( c1 != '\0' );

	return 0;
}

#ifdef PATH_MAX
#define MAX_OSPATH PATH_MAX
#else
#define MAX_OSPATH 256
#endif
#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

char* BuildOSPath(const std::filesystem::path& curPath, const eastl::string& gamepath, const char *npath)
{
	static char ospath[MAX_OSPATH*2+1];
	char temp[MAX_OSPATH];

	if (npath)
		snprintf(temp, sizeof(temp), "%c%s%c%s", PATH_SEP, gamepath.c_str(), PATH_SEP, npath);
	else
		snprintf(temp, sizeof(temp), "%c%s%c", PATH_SEP, gamepath.c_str(), PATH_SEP);

	snprintf(ospath, sizeof(ospath), "%s%s", curPath.c_str(), temp);
	return ospath;
}

void *SafeMalloc(size_t size)
{
	void *p;

	Printf("Allocating %lu bytes with malloc()", size);

	p = malloc(size);
	if (!p) {
		Error("malloc() failure on %lu bytes, strerror: %s", size, strerror(errno));
	}

	return p;
}


void SafeWrite(const void *buffer, size_t size, FILE *fp)
{
    Printf("Writing %lu bytes to file", size);
    if (!fwrite(buffer, size, 1, fp)) {
        Error("Failed to write %lu bytes to file", size);
    }
}

void SafeRead(void *buffer, size_t size, FILE *fp)
{
    Printf("Reading %lu bytes from file", size);
    if (!fread(buffer, size, 1, fp)) {
        Error("Failed to read %lu bytes from file", size);
    }
}

FILE *SafeOpenRead(const char *path)
{
    FILE *fp;
    
    Printf("Opening '%s' in read mode", path);
    fp = fopen(path, "rb");
    if (!fp) {
        Error("Failed to open file %s in read mode", path);
    }
    return fp;
}

FILE *SafeOpenWrite(const char *path)
{
    FILE *fp;
    
	Printf("Opening '%s' in write mode", path);
    fp = fopen(path, "wb");
    if (!fp) {
        Error("Failed to open file %s in write mode", path);
    }
    return fp;
}

bool FileExists(const char *path)
{
    FILE *fp;

    fp = fopen(path, "r");
    if (!fp)
        return false;
    
    fclose(fp);
    return true;
}

uint64_t FileLength(FILE *fp)
{
    uint64_t pos, end;

    pos = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    end = ftell(fp);
    fseek(fp, pos, SEEK_SET);

    return end;
}

#define BIG_INFO_STRING 8192
#define MAX_STRING_TOKENS 1024
#define MAX_HISTORY 32
#define MAX_CMD_BUFFER  65536

#define arraylen(x) (sizeof((x))/sizeof((*x)))

static uint32_t numCommands = 0;
static uint32_t cmd_argc;
static char cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];
static char *cmd_argv[MAX_STRING_TOKENS];
static char cmd_cmd[BIG_INFO_STRING];

static char cmd_history[MAX_HISTORY][BIG_INFO_STRING];
static uint32_t cmd_historyused;

void *operator new[](size_t n, const char *, int, unsigned int, const char *, int)
{
    return new char[n];
}

const char *Argv(uint32_t index)
{
    if (index >= cmd_argc)
        return "";
    
    return cmd_argv[index];
}

uint32_t Argc(void)
{
    return cmd_argc;
}

void TokenizeString(const char *str, bool ignoreQuotes)
{
	const char *p;
	char *tok;

    memset(cmd_cmd, 0, sizeof(cmd_cmd));
    memset(cmd_tokenized, 0, sizeof(cmd_tokenized));
    cmd_argc = 0;

    strncpy(cmd_cmd, str, sizeof(cmd_cmd));
	p = str;
	tok = cmd_tokenized;

	while (1) {
		if (cmd_argc >= arraylen(cmd_argv)) {
			return; // usually something malicious
		}
		while (*p && *p <= ' ') {
			p++; // skip whitespace
		}
		if (!*p) {
			break; // end of string
		}
		// handle quoted strings
		if (!ignoreQuotes && *p == '\"') {
			cmd_argv[cmd_argc] = tok;
            cmd_argc++;
			p++;
			while (*p && *p != '\"') {
				*tok++ = *p++;
			}
			if (!*p) {
				return; // end of string
			}
			p++;
			continue;
		}

		// regular stuff
		cmd_argv[cmd_argc] = tok;
        cmd_argc++;

		// skip until whitespace, quote, or command
		while (*p > ' ') {
			if (!ignoreQuotes && p[0] == '\"') {
				break;
			}

			if (p[0] == '/' && p[1] == '/') {
				// accept protocol headers (e.g. http://) in command lines that match "*?[a-z]://" pattern
				if (p < cmd_cmd + 3 || p[-1] != ':' || p[-2] < 'a' || p[-2] > 'z') {
					break;
				}
			}

			// skip /* */ comments
			if (p[0] == '/' && p[1] == '*') {
				break;
			}

			*tok++ = *p++;
		}
		*tok++ = '\0';
		if (!*p) {
			return; // end of string
		}
	}
}
