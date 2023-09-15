#include "gln.h"

static CCommand *cmdlist;

static char *G_CopyString(const char *str)
{
    uint64_t len;
    char *out;

    len = strlen(str);
    out = (char *)GetMemory(len + 1);
    memset(out, 0, len + 1);
    memcpy(out, str, len);

    return out;
}

static void Cmd_List_f(void)
{
    uint64_t numCommands;

    Printf("Command list:");
    numCommands = 0;
    for (CCommand *cmd = cmdlist; cmd; cmd = cmd->mNext) {
        if (!cmd->mName)
            continue;

        Printf("%s", cmd->mName);
        numCommands++;
    }
    Printf("Number of commands: %lu", numCommands);
}


/*
Cmd_Init: initializes all the common commands that can be found in the editor
*/
void Cmd_Init(void)
{
    cmdlist = (CCommand *)GetMemory(sizeof(*cmdlist));

    Cmd_AddCommand("list", Cmd_List_f);
}

static CCommand *Cmd_FindCommand(const char *name)
{
    CCommand *cmd;

    for (cmd = cmdlist; cmd; cmd = cmd->mNext) {
        if (!N_stricmp(name, cmd->mName)) {
            return cmd;
        }
    }
    return NULL;
}

void Cmd_Shutdown(void)
{
}

#define BIG_INFO_STRING 8192
#define MAX_STRING_TOKENS 1024
#define MAX_HISTORY 32
#define MAX_CMD_BUFFER  65536

static uint32_t cmd_argc;
static char CCommandokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];
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
    memset(CCommandokenized, 0, sizeof(CCommandokenized));
    cmd_argc = 0;

    strncpy(cmd_cmd, str, sizeof(cmd_cmd));
	p = str;
	tok = CCommandokenized;

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

void Cmd_ExecuteText(const char *text)
{
    CCommand *cmd;
    const char *cmdname;

    TokenizeString(text, false);
    cmdname = Argv(0);

    cmd = Cmd_FindCommand(cmdname);
    if (!cmd) {
        Printf("No such command '%s'", cmdname);
        return;
    }
    
    cmd->mFunc();
}

void Cmd_AddCommand(const char *name, cmdfunc_t func)
{
    CCommand *cmd;

    cmd = Cmd_FindCommand(name);
    if (cmd) {
        Printf("Command '%s' already registered", cmd->mName);
        return;
    }

    cmd = (CCommand *)Allocate<CCommand>(name, func);
    cmd->mName = name;
    cmd->mFunc = func;
    cmd->mNext = cmdlist;
    cmdlist = cmd;
}

#if 0
typedef struct alloc_s
{
    struct alloc_s *next;
    uint32_t size;
    uint32_t used;
    uint32_t padding;
    byte base[4];
} alloc_t;

static alloc_t *memorylist, *rover;
static uint32_t memoryHighwater;

#define MEMORY_BLOCK_SIZE 0x1000000

void G_ShutdownMemory(void)
{
    alloc_t *alloc, *next;

    //
    // free any current data
    //
    
    for (alloc = memorylist; alloc; alloc = next) {
        next = alloc->next;
        Free(alloc);
    }
}

void G_InitMemory(void)
{
    alloc_t *alloc;

    G_ShutdownMemory();

    alloc = (alloc_t *)Malloc(MEMORY_BLOCK_SIZE + sizeof(*alloc));
    if (!alloc) { 
        Error("G_InitMemory: failed to allocate memory");
    }

    alloc->size = MEMORY_BLOCK_SIZE;
    alloc->used = 0;
    alloc->next = NULL;

    memorylist = alloc;
    rover = alloc;
    memoryHighwater = 0;
}

void *G_AllocMem(uint32_t size)
{
    alloc_t *alloc;
    void *buf;

    size = PAD(size, 16);

    // see if it can be satisfied in the current block
    alloc = rover;
    if (alloc->used + size <= alloc->size) {
        buf = alloc->base + alloc->used;
        alloc->used += size;
        return buf;
    }

    // check the other blocks for available space
    alloc = alloc->next;
    
    // we're at the end of the chain, allocate a new block
    if (!alloc) {
        uint32_t blocksize;

        blocksize = MEMORY_BLOCK_SIZE;
        alloc = (alloc_t *)Malloc(blocksize + sizeof(*alloc));
        if (!alloc) {
            Error("G_AllocMem: failed to allocate memory");
        }
        alloc->size = blocksize;
        alloc->used = 0;
        alloc->next = NULL;
        rover->next = alloc;
    }

    if (size > alloc->size) {
        Error("G_AllocMem: overflow of %i bytes", size);
    }

    rover = alloc;
    alloc->used = size;

    return alloc->base;
}

#endif
