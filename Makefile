CC		= distcc g++
CFLAGS	= -Og -g -I.
EXE		= mapeditor
O		= obj

COMPILE=$(CC) $(CFLAGS) -Isrc -I/usr/local/include/spdlog/ -IDependencies/include -IDependencies/ -o $@ -c $< -Iinclude

DEPS=\
	$(O)/imgui_tables.o \
	$(O)/imgui_draw.o \
	$(O)/imgui_widgets.o \
	$(O)/imgui.o \
	$(O)/imgui_impl_sdl2.o \
	$(O)/imgui_impl_opengl3.o \
	$(O)/glad.o \

OBJS=\
	$(O)/main.o \
	$(O)/gln.o \
	$(O)/gui.o \
	$(O)/editor.o \
	$(O)/stream.o \
	$(O)/events.o \
	$(O)/command.o \
	$(O)/Heap.o \
	$(O)/ngl_proc.o \

$(O)/%.o: src/%.cpp
	$(COMPILE)

$(O)/%.o: Dependencies/src/%.cpp
	$(COMPILE)

$(EXE): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(OBJS) $(DEPS) -o $(EXE) -lGL -lSDL2 /usr/local/lib/libspdlog.a /usr/local/lib/libfoonathan_memory-0.7.3.a libEASTL.a -lbacktrace -lbz2 -lz

clean:
	rm $(O)/*