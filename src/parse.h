#ifndef __PARSE__
#define __PARSE__

#pragma once

#define	MAX_STRING_CHARS	1024	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

uint64_t COM_Compress( char *data_p );
void COM_BeginParseSession( const char *name );
uint64_t COM_GetCurrentParseLine( void );
const char *COM_Parse( const char **data_p );
const char *COM_ParseExt( const char **data_p, qboolean allowLineBreak );
void COM_ParseError( const char *format, ... ) __attribute__((format(printf, 1, 2)));
void COM_ParseWarning( const char *format, ... ) __attribute__((format(printf, 1, 2)));

// md4 functions
uint32_t Com_BlockChecksum (const void *buffer, uint64_t length);

//int		COM_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] );

char *COM_ParseComplex( const char **data_p, qboolean allowLineBreak );

typedef enum {
	TK_GENEGIC = 0, // for single-char tokens
	TK_STRING,
	TK_QUOTED,
	TK_EQ,
	TK_NEQ,
	TK_GT,
	TK_GTE,
	TK_LT,
	TK_LTE,
	TK_MATCH,
	TK_OR,
	TK_AND,
	TK_SCOPE_OPEN,
	TK_SCOPE_CLOSE,
	TK_NEWLINE,
	TK_EOF,
} tokenType_t;

extern tokenType_t com_tokentype;

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

qboolean SkipBracedSection( const char **program, int depth );
void SkipRestOfLine( const char **data );

int ParseHex(const char* text);
bool Parse1DMatrix( const char **buf_p, int x, float *m);
bool Parse2DMatrix( const char **buf_p, int y, int x, float *m);
bool Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m);

#endif