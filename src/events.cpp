#include "gln.h"
#include "imgui_impl_sdl2.h"

CEventQueue events;

typedef struct
{
	const char *name;
	uint32_t keynum;
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

CEventQueue::CEventQueue(void)
	: mLastEvent{ mEventQueue.data() + MAX_EVENT_QUEUE - 1 },
    mEventHead{ 0 }, mEventTail{ 0 },
	mPushedEventsHead{ 0 }, mPushedEventsTail{ 0 }
{
	memset(mEventQueue.data(), 0, sizeof(sysEvent_t) * mEventQueue.size());
	memset(mPushedEvents.data(), 0, sizeof(sysEvent_t) * mPushedEvents.size());
}

CEventQueue::~CEventQueue()
{
}

bool Key_IsDown(uint32_t keynum)
{
    if (keynum >= NUMKEYS)
        return false;
    
    return keys[keynum].down;
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

void CEventQueue::PushEvent(const sysEvent_t *event)
{
	sysEvent_t *ev;
	static bool printedWarning = false;

	ev = &mPushedEvents[mPushedEventsTail & (MAX_EVENT_QUEUE - 1)];

	if (mPushedEventsHead - mPushedEventsTail >= MAX_EVENT_QUEUE) {
		// don't print the warning constantly, or it can give time for more...
		if (!printedWarning) {
			printedWarning = true;
			Printf("WARNING: Com_PushEvent: overflow");
		}

		if (ev->evPtr) {
			FreeMemory(ev->evPtr);
		}
		mPushedEventsTail++;
	}
	else {
		printedWarning = false;
	}

	*ev = *event;
	mPushedEventsHead++;
}

static void KeyDownEvent(uint32_t key)
{
	keys[key].down = true;
	keys[key].bound = false;
	keys[key].repeats++;

	// console key is hardcoded, so the user can never unbind it
	if (key == KEY_CONSOLE || (keys[KEY_LSHIFT].down && key == KEY_ESCAPE)) {
		if (editor->mConsoleActive)
			editor->mConsoleActive = false;
		else
			editor->mConsoleActive = true;
		return;
	}

	// only let the console process the event if its open
	if (editor->mConsoleActive) {
		keys[key].down = false;
		keys[key].repeats--;
		return;
	}
}

static void KeyUpEvent(uint32_t key)
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

void KeyEvent(uint32_t key, bool down)
{
	if (down)
		KeyDownEvent(key);
	else
		KeyUpEvent(key);
}

typedef struct {
	int x, y;
	glm::ivec2 lastPos;
	glm::vec2 offset, delta;
} mouse_t;
mouse_t mouseState;

void CEventQueue::PumpKeyEvents(void)
{
	SDL_Event event;
	SDL_PumpEvents();

	while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

		switch (event.type) {
		case SDL_KEYDOWN:
            QueueEvent(SE_KEY, event.key.keysym.scancode, 1, 0, NULL);
			break;
		case SDL_KEYUP:
			QueueEvent(SE_KEY, event.key.keysym.scancode, 0, 0, NULL);
			break;
		case SDL_QUIT:
			QueueEvent(SE_WINDOW, event.type, 0, 0, NULL);
			break;
		case SDL_MOUSEMOTION:
			mouseState.x = event.motion.x;
			mouseState.y = event.motion.y;

			mouseState.offset.x = mouseState.x - mouseState.lastPos.x;
			mouseState.offset.y = mouseState.y - mouseState.lastPos.y;

			mouseState.lastPos.x = mouseState.x;
			mouseState.lastPos.y = mouseState.y;

			mouseState.delta.x = ((float)mouseState.offset.x) / gui->mWindowWidth * 2;
			mouseState.delta.y = ((float)mouseState.offset.y) / gui->mWindowHeight * 2;
			mouseState.delta = glm::normalize(mouseState.delta);

			if (Key_IsDown(KEY_MOUSE_LEFT) && editor->mode != MODE_EDIT) {
				gui->mCameraPos.x -= (mouseState.delta.x / 4);
				gui->mCameraPos.y += (mouseState.delta.y / 4);
			}
            QueueEvent(SE_MOUSE, event.motion.x, event.motion.y, 0, NULL);
            break;
		case SDL_MOUSEBUTTONUP:
			SDL_GetMouseState(&mouseState.lastPos.x, &mouseState.lastPos.y);
            QueueEvent(SE_KEY, event.button.button, 0, 0, NULL);
            break;
		case SDL_MOUSEBUTTONDOWN:
			SDL_GetMouseState(&mouseState.lastPos.x, &mouseState.lastPos.y);
            QueueEvent(SE_KEY, event.button.button, 1, 0, NULL);
            break;
		case SDL_MOUSEWHEEL:
            if (event.wheel.y > 0 && editor->mode != MODE_EDIT) // wheel up
				gui->mCameraZoom -= gameConfig->mCameraZoomSpeed;
            if (event.wheel.y < 0 && editor->mode != MODE_EDIT) // wheel down
				gui->mCameraZoom += gameConfig->mCameraZoomSpeed;
			break;
		};
	}
}

sysEvent_t CEventQueue::GetSystemEvent(void)
{
	sysEvent_t ev;
	const char *s;

	// return if we have data
	if (mEventHead - mEventTail > 0)
		return mEventQueue[(mEventTail++) & MASK_QUEUED_EVENTS];
	
	PumpKeyEvents();

	// check for console commands

	// return if we have data
	if (mEventHead - mEventTail > 0)
		return mEventQueue[(mEventTail++) & MASK_QUEUED_EVENTS];
	
	// create a new empty event to return
	memset(&ev, 0, sizeof(ev));

	return ev;
}

sysEvent_t CEventQueue::GetRealEvent(void)
{
	return GetSystemEvent();
}

void CEventQueue::QueueEvent(sysEventType_t evType, int evValue, int evValue2, uint32_t ptrLength, void *ptr)
{
	sysEvent_t *ev;

	// try to combine all sequential mouse moves in one event
	if (evType == SE_MOUSE && mLastEvent->evType == SE_MOUSE && mEventHead != mEventTail) {
		mLastEvent->evValue += evValue;
		mLastEvent->evValue2 += evValue2;
		return;
	}

	ev = &mEventQueue[mEventHead & MASK_QUEUED_EVENTS];

	if (mEventHead - mEventTail >= MAX_EVENT_QUEUE) {
		Printf("%s(type=%s,keys=(%i,%i)): overflow", __func__, EventName(evType), evValue, evValue2);
		// we are discarding an event, but avoid leaking memory
		if (ev->evPtr) {
			FreeMemory(ev->evPtr);
		}
		mEventTail++;
	}

	mEventHead++;

	ev->evType = evType;
	ev->evValue = evValue;
	ev->evValue2 = evValue2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;

	mLastEvent = ev;
}

sysEvent_t CEventQueue::GetEvent(void)
{
	if (mPushedEventsHead - mPushedEventsTail > 0) {
		return mPushedEvents[(mPushedEventsTail++) & (MAX_EVENT_QUEUE - 1)];
	}

	return GetRealEvent();
}

static void WindowEvent(uint32_t value)
{
	if (value == SDL_QUIT) {
        Exit();
	}
}

uint64_t CEventQueue::EventLoop(void)
{
	sysEvent_t ev;

	while (1) {
		ev = GetEvent();

		// no more events are available
		if (ev.evType == SE_NONE) {
			return 0;
		}

		switch (ev.evType) {
		case SE_KEY:
			KeyEvent(ev.evValue, ev.evValue2);
			break;
		case SE_WINDOW:
			WindowEvent(ev.evValue);
			break;
		case SE_MOUSE:
//			MouseEvent(ev.evValue, ev.evValue2);
			break;
		case SE_CONSOLE:
			if (editor->mConsoleActive)
				editor->mConsoleActive = false;
			else
				editor->mConsoleActive = true;
			break;
		default:
			Error("Com_EventLoop: bad event type %i", ev.evType);
		};

		// free any block data
		if (ev.evPtr) {
			FreeMemory(ev.evPtr);
			ev.evPtr = NULL;
		}
	}

	return 0; // never reached
}