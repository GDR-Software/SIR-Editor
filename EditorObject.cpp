#include "Common.hpp"
#include "EditorObject.h"
#include <foonathan_memory/foonathan/memory/memory_arena.hpp>
#include <foonathan_memory/foonathan/memory/tracking.hpp>
#include <foonathan_memory/foonathan/memory/threading.hpp>
#include <foonathan_memory/foonathan/memory/container.hpp>

CEditorObject CEditorObject::bigboi;

// probably the the time ill ever use this syntax
using namespace memory::literals;

CEditorObject::CEditorObject(void)
    : allocPool{128, 256_MiB}
{
}

CEditorObject::~CEditorObject()
{
}
