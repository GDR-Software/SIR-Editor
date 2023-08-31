CC		= distcc g++
CFLAGS	= -Og -g -I.
EXE		= mapeditor
O		= obj

COMPILE=$(CC) $(CFLAGS) -I/usr/local/include/spdlog/ -IDependencies/include -IDependencies/ -o $@ -c $<

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
	$(O)/Common.o \
	$(O)/GUI.o \
	$(O)/Map.o \
	$(O)/Texture.o \
	$(O)/Tileset.o \
	$(O)/MapFile.o \
	$(O)/Editor.o \
	$(O)/Project.o \
	$(O)/Events.o \
	$(O)/Zone.o \

$(O)/%.o: %.cpp
	$(COMPILE)
$(O)/%.o: Dependencies/src/%.cpp
	$(COMPILE)

$(EXE): $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(OBJS) $(DEPS) -o $(EXE) -lGL -lSDL2 /usr/local/lib/libspdlog.a

clean:
	rm $(O)/*