#include "Common.hpp"
#include "Editor.h"

typedef enum {
  // bk001129 - make sure SE_NONE is zero
	SE_NONE = 0,		// evTime is still valid
	SE_KEY,				// evValue is a key code, evValue2 is whether its pressed or not
	SE_MOUSE,	    	// evValue and evValue2 are relative signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE,			// evPtr is a char*
	SE_WINDOW,			// really only used by the rendering engine for window resisizing
	SE_MAX,
} sysEventType_t;

typedef struct
{
	unsigned int		evTime;
	sysEventType_t	evType;
	unsigned int		evValue, evValue2;
	unsigned int		evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
} sysEvent_t;

void QueueEvent(unsigned int evTime, sysEventType_t evType, int evValue, int evValue2, unsigned int ptrLength, void *ptr);
void SendKeyEvents(void);
void KeyEvent(int key, bool down, unsigned int time);
void MouseEvent(int x, int y);

#define KEYCATCH_SGAME	0x2000
#define KEYCATCH_SCRIPT	0x0400
#define KEYCATCH_UI		0x0080

void Key_KeynameCompletion( void(*callback)(const char *s) );
unsigned int Key_StringToKeynum( const char *str );
const char *Key_KeynumToString(unsigned int keynum);
const char *Key_GetBinding(unsigned int keynum);
void Key_ClearStates(void);

#define MAX_EVENT_QUEUE 256
#define MASK_QUEUED_EVENTS (MAX_EVENT_QUEUE - 1)
#define MAX_PUSHED_EVENTS 256


static sysEvent_t eventQueue[MAX_EVENT_QUEUE];
static sysEvent_t *lastEvent = eventQueue + MAX_EVENT_QUEUE - 1;
static unsigned int eventHead = 0;
static unsigned int eventTail = 0;
static unsigned int com_pushedEventsHead;
static unsigned int com_pushedEventsTail;
static sysEvent_t com_pushedEvents[MAX_PUSHED_EVENTS];


typedef struct
{
	const char *name;
	int keynum;
} keyname_t;

static const keyname_t keynames[] = {
	{"MOUSE_LEFT", KEY_MOUSE_LEFT},
	{"MOUSE_MIDDLE", KEY_MOUSE_MIDDLE},
	{"MOUSE_RIGHT", KEY_MOUSE_RIGHT},
	{"a", KEY_A},
	{"b", KEY_B},
	{"c", KEY_C},
	{"d", KEY_D},
	{"e", KEY_E},
	{"f", KEY_F},
	{"g", KEY_G},
	{"h", KEY_H},
	{"i", KEY_I},
	{"j", KEY_J},
	{"k", KEY_K},
	{"l", KEY_L},
	{"m", KEY_M},
	{"n", KEY_N},
	{"o", KEY_O},
	{"p", KEY_P},
	{"q", KEY_Q},
	{"r", KEY_R},
	{"s", KEY_S},
	{"t", KEY_T},
	{"u", KEY_U},
	{"v", KEY_V},
	{"w", KEY_W},
	{"x", KEY_X},
	{"y", KEY_Y},
	{"z", KEY_Z},

	{"LSHIFT", KEY_LSHIFT},
	{"RSHIFT", KEY_RSHIFT},
	{"LCTRL", KEY_LCTRL},
	{"RCTRL", KEY_RCTRL},
	{"TAB", KEY_TAB},
	{"ENTER", KEY_ENTER},
	{"BACKSPACE", KEY_BACKSPACE},

	{"CONSOLE", KEY_CONSOLE},
	{"PRINTSCREEN", KEY_SCREENSHOT},

	{"UPARROW", KEY_UP},
	{"DOWNARROW", KEY_DOWN},
	{"RIGHTARROW", KEY_RIGHT},
	{"LEFTARROW", KEY_LEFT},

	{"F1", KEY_F1},
	{"F2", KEY_F2},
	{"F3", KEY_F3},
	{"F4", KEY_F4},
	{"F5", KEY_F5},
	{"F6", KEY_F6},
	{"F7", KEY_F7},
	{"F8", KEY_F8},
	{"F9", KEY_F9},
	{"F10", KEY_F10},
	{"F11", KEY_F11},
	{"F12", KEY_F12},

    {"WHEELUP", KEY_WHEEL_UP},
    {"WHEELDOWN", KEY_WHEEL_DOWN},

	{NULL, 0}
};


nkey_t keys[NUMKEYS];

bool Key_IsDown(int keynum)
{
    if (keynum >= NUMKEYS)
        return false;
    
    return keys[keynum].down;
}

void InitEvents(void)
{
	// clear the static buffer array
	// this requires SE_NONE to be accepted as a valid but NOP event
	memset( com_pushedEvents, 0, sizeof(com_pushedEvents) );
	// reset counters while we are at it
	// beware: GetEvent might still return an SE_NONE from the buffer
	com_pushedEventsHead = 0;
	com_pushedEventsTail = 0;
}

static const char *EventName(sysEventType_t evType)
{
	static const char *evNames[SE_MAX] = {
		"SE_NONE",
		"SE_KEY",
		"SE_MOUSE",
		"SE_JOYSTICK_AXIS",
		"SE_CONSOLE",
		"SE_WINDOW"
	};

	if ((unsigned)evType >= arraylen(evNames))
		return "SE_UNKOWN";
	else
		return evNames[evType];
}

static void PushEvent(const sysEvent_t *event)
{
	sysEvent_t *ev;
	static bool printedWarning = false;

	ev = &com_pushedEvents[com_pushedEventsTail & (MAX_EVENT_QUEUE - 1)];

	if (com_pushedEventsHead - com_pushedEventsTail >= MAX_EVENT_QUEUE) {
		// don't print the warning constantly, or it can give time for more...
		if (!printedWarning) {
			printedWarning = true;
			Printf("WARNING: Com_PushEvent: overflow");
		}

		if (ev->evPtr) {
			free(ev->evPtr);
		}
		com_pushedEventsTail++;
	}
	else {
		printedWarning = false;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

static void KeyDownEvent(int key, unsigned int time)
{
	keys[key].down = true;
	keys[key].bound = false;
	keys[key].repeats++;

	// not alpanumerical, scancode to keycode conversion needed
//	if (key < 'a' && key > 'z') {
//		key = SDL_SCANCODE_TO_KEYCODE(key);
//	}

	// console key is hardcoded, so the user can never unbind it
	if (key == KEY_CONSOLE || (keys[KEY_LSHIFT].down && key == KEY_ESCAPE)) {
		Editor::Get()->getGUI()->setConsoleActive(true);
		return;
	}

	// only let the console process the event if its open
	if (Editor::Get()->getGUI()->isConsoleActive()) {
		keys[key].down = false;
		keys[key].repeats--;
		return;
	}
}

static void KeyUpEvent(int key, unsigned int time)
{
	const bool bound = keys[key].bound;

	keys[key].repeats = 0;
	keys[key].down = false;
	keys[key].bound = false;

	// don't process key-up events for the console key
	if (key == KEY_CONSOLE || (key == KEY_ESCAPE && keys[KEY_LSHIFT].down)) {
		return;
	}

	// hardcoded screenshot key
	if (key == KEY_SCREENSHOT) {
		return;
	}
}

void KeyEvent(int key, bool down, unsigned int time)
{
	if (down)
		KeyDownEvent(key, time);
	else
		KeyUpEvent(key, time);
}

mouse_t mouse;

static inline float clampYaw(float f)
{
    float temp = (f + 180.0f) / 360.0f;
    return f - ((int)temp - (temp < 0.0f ? 1 : 0)) * 360.0f;
}

static inline float clampPitch(float f)
{
    return f > 89.0f ? 89.0f : (f < -89.0f ? -89.0f : f);
}

void MouseEvent(int x, int y)
{
    float deltaX, deltaY;

    mouse.x = x;
    mouse.y = y;

    mouse.angle = atan2(mouse.y - (1080/2), mouse.x - (1920/2));
    
    if (mouse.angle > 180.0f)
        mouse.angle -= 360.0f;
    else if (mouse.angle <= -180.0f)
        mouse.angle += 360.0f;

    deltaX = cos(glm::radians(mouse.angle));
    deltaY = sin(glm::radians(mouse.angle));

    mouse.deltaX = deltaX;
    mouse.deltaY = deltaY;
    
    mouse.moving = true;
}

bool Mouse_WheelUp(void)
{
    return mouse.wheelUp;
}

bool Mouse_WheelDown(void)
{
    return mouse.wheelDown;
}


static void PumpKeyEvents(void)
{
	SDL_Event event;

	SDL_PumpEvents();

    mouse.wheelDown = false;
    mouse.wheelUp = false;

	while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
		case SDL_KEYDOWN:
            QueueEvent(0, SE_KEY, event.key.keysym.scancode, 1, 0, NULL);
			break;
		case SDL_KEYUP:
			QueueEvent(0, SE_KEY, event.key.keysym.scancode, 0, 0, NULL);
			break;
		case SDL_QUIT:
			QueueEvent(0, SE_WINDOW, event.type, 0, 0, NULL);
			break;
		case SDL_MOUSEMOTION:
            QueueEvent(0, SE_MOUSE, event.motion.x, event.motion.y, 0, NULL);
            break;
		case SDL_MOUSEBUTTONUP:
            QueueEvent(0, SE_KEY, event.button.button, 0, 0, NULL);
            break;
		case SDL_MOUSEBUTTONDOWN:
            QueueEvent(0, SE_KEY, event.button.button, 1, 0, NULL);
            break;
		case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0)
                mouse.wheelUp = true;
            if (event.wheel.y < 0)
                mouse.wheelDown = true;
			break;
		};
	}
}

static sysEvent_t GetSystemEvent(void)
{
	sysEvent_t ev;
	const char *s;
	unsigned int evTime;

	// return if we have data
	if (eventHead - eventTail > 0)
		return eventQueue[(eventTail++) & MASK_QUEUED_EVENTS];
	
	PumpKeyEvents();

//	evTime = Sys_Milliseconds();

	// check for console commands

	// return if we have data
	if (eventHead - eventTail > 0)
		return eventQueue[(eventTail++) & MASK_QUEUED_EVENTS];
	
	// create a new empty event to return
	memset(&ev, 0, sizeof(ev));
	ev.evTime = evTime;

	return ev;
}

static sysEvent_t GetRealEvent(void)
{
	return GetSystemEvent();
}

void QueueEvent(unsigned int evTime, sysEventType_t evType, int evValue, int evValue2, unsigned int ptrLength, void *ptr)
{
	sysEvent_t *ev;

	// try to combine all sequential mouse moves in one event
	if (evType == SE_MOUSE && lastEvent->evType == SE_MOUSE && eventHead != eventTail) {
		lastEvent->evValue += evValue;
		lastEvent->evValue2 += evValue2;
		lastEvent->evTime = evTime;
		return;
	}

	ev = &eventQueue[eventHead & MASK_QUEUED_EVENTS];

	if (eventHead - eventTail >= MAX_EVENT_QUEUE) {
		Printf("%s(type=%s,keys=(%i,%i),time=%i): overflow", __func__, EventName(evType), evValue, evValue2, evTime);
		// we are discarding an event, but avoid leaking memory
		if (ev->evPtr) {
			free(ev->evPtr);
		}
		eventTail++;
	}

	eventHead++;

	ev->evTime = evTime;
	ev->evType = evType;
	ev->evValue = evValue;
	ev->evValue2 = evValue2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;

	lastEvent = ev;
}

static sysEvent_t GetEvent(void)
{
	if (com_pushedEventsHead - com_pushedEventsTail > 0) {
		return com_pushedEvents[(com_pushedEventsTail++) & (MAX_EVENT_QUEUE - 1)];
	}

	return GetRealEvent();
}

static void WindowEvent(unsigned int value)
{
	if (value == SDL_QUIT) {
        Exit();
	}
}

uint64_t EventLoop(void)
{
	sysEvent_t ev;

	while (1) {
		ev = GetEvent();

		// no more events are available
		if (ev.evType == SE_NONE) {
			return ev.evTime;
		}

		switch (ev.evType) {
		case SE_KEY:
			KeyEvent(ev.evValue, ev.evValue2, ev.evTime);
			break;
		case SE_WINDOW:
			WindowEvent(ev.evValue);
			break;
		case SE_MOUSE:
			MouseEvent(ev.evValue, ev.evValue2);
			break;
		case SE_CONSOLE:
            Editor::Get()->getGUI()->setConsoleActive(true);
			break;
		default:
			Error("Com_EventLoop: bad event type %i", ev.evType);
		};

		// free any block data
		if (ev.evPtr) {
			free(ev.evPtr);
			ev.evPtr = NULL;
		}
	}

	return 0; // never reached
}