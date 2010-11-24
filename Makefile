
CC = g++
BIN = etr

OBJ = main.o game_config.o ogl.o tux.o audio.o winsys.o \
particles.o mathlib.o splash_screen.o intro.o racing.o \
game_over.o paused.o reset.o game_type_select.o event_select.o \
race_select.o credits.o loading.o course.o keyframe.o env.o event.o \
spx.o common.o course_render.o game_ctrl.o physics.o \
track_marks.o hud.o view.o gui.o translation.o tools.o \
quadtree.o font.o ft_font.o textures.o help.o regist.o tool_frame.o \
tool_char.o newplayer.o score.o

CFLAGS = -Wall -O2 -I/usr/include/freetype2
LDFLAGS = -lglut -lGL -lGLU -lSDL -lSDL_image -lSDL_mixer -lfreetype 

$(BIN) : $(OBJ)
	g++ -o $(BIN) $(OBJ) $(LDFLAGS) $(CFLAGS)

# mmmm.o : mmmm.cpp mmmm.h
#	$(CC) -c mmmm.cpp $(CFLAGS)

score.o : score.cpp score.h
	$(CC) -c score.cpp $(CFLAGS)

newplayer.o : newplayer.cpp newplayer.h
	$(CC) -c newplayer.cpp $(CFLAGS)

tool_char.o : tool_char.cpp tool_char.h
	$(CC) -c tool_char.cpp $(CFLAGS)

tool_frame.o : tool_frame.cpp tool_frame.h
	$(CC) -c tool_frame.cpp $(CFLAGS)

regist.o : regist.cpp regist.h
	$(CC) -c regist.cpp $(CFLAGS)

tools.o : tools.cpp tools.h
	$(CC) -c tools.cpp $(CFLAGS)

help.o : help.cpp help.h
	$(CC) -c help.cpp $(CFLAGS)

translation.o : translation.cpp translation.h
	$(CC) -c translation.cpp $(CFLAGS)

physics.o : physics.cpp physics.h
	$(CC) -c physics.cpp $(CFLAGS)

winsys.o : winsys.cpp winsys.h
	$(CC) -c winsys.cpp $(CFLAGS)

game_ctrl.o : game_ctrl.cpp game_ctrl.h
	$(CC) -c game_ctrl.cpp $(CFLAGS)

textures.o : textures.cpp textures.h
	$(CC) -c textures.cpp $(CFLAGS)

ft_font.o : ft_font.cpp ft_font.h
	$(CC) -c ft_font.cpp $(CFLAGS)

font.o : font.cpp font.h
	$(CC) -c font.cpp $(CFLAGS)

event.o : event.cpp event.h
	$(CC) -c event.cpp $(CFLAGS)

gui.o : gui.cpp gui.h
	$(CC) -c gui.cpp $(CFLAGS)

common.o : common.cpp common.h
	$(CC) -c common.cpp $(CFLAGS)

spx.o : spx.cpp spx.h
	$(CC) -c spx.cpp $(CFLAGS)

quadtree.o : quadtree.cpp quadtree.h
	$(CC) -c quadtree.cpp $(CFLAGS)

view.o : view.cpp view.h
	$(CC) -c view.cpp $(CFLAGS)

hud.o : hud.cpp hud.h
	$(CC) -c hud.cpp $(CFLAGS)

track_marks.o : track_marks.cpp track_marks.h
	$(CC) -c track_marks.cpp $(CFLAGS)

course_render.o : course_render.cpp course_render.h
	$(CC) -c course_render.cpp $(CFLAGS)

env.o : env.cpp env.h
	$(CC) -c env.cpp $(CFLAGS)

keyframe.o : keyframe.cpp keyframe.h
	$(CC) -c keyframe.cpp $(CFLAGS)

course.o : course.cpp course.h
	$(CC) -c course.cpp $(CFLAGS)

loading.o : loading.cpp loading.h
	$(CC) -c loading.cpp $(CFLAGS)

credits.o : credits.cpp credits.h
	$(CC) -c credits.cpp $(CFLAGS)

race_select.o : race_select.cpp race_select.h
	$(CC) -c race_select.cpp $(CFLAGS)

event_select.o : event_select.cpp event_select.h
	$(CC) -c event_select.cpp $(CFLAGS)

game_type_select.o : game_type_select.cpp game_type_select.h
	$(CC) -c game_type_select.cpp $(CFLAGS)

game_over.o : game_over.cpp game_over.h
	$(CC) -c game_over.cpp $(CFLAGS)

paused.o : paused.cpp paused.h
	$(CC) -c paused.cpp $(CFLAGS)

reset.o : reset.cpp reset.h
	$(CC) -c reset.cpp $(CFLAGS)

racing.o : racing.cpp racing.h
	$(CC) -c racing.cpp $(CFLAGS)

intro.o : intro.cpp intro.h
	$(CC) -c intro.cpp $(CFLAGS)

splash_screen.o : splash_screen.cpp splash_screen.h
	$(CC) -c splash_screen.cpp $(CFLAGS)

mathlib.o : mathlib.cpp mathlib.h
	$(CC) -c mathlib.cpp $(CFLAGS)

particles.o : particles.cpp particles.h
	$(CC) -c particles.cpp $(CFLAGS)

audio.o : audio.cpp audio.h
	$(CC) -c audio.cpp $(CFLAGS)

tux.o : tux.cpp tux.h
	$(CC) -c tux.cpp $(CFLAGS)

ogl.o : ogl.cpp ogl.h
	$(CC) -c ogl.cpp $(CFLAGS)

game_config.o : game_config.cpp game_config.h
	$(CC) -c game_config.cpp $(CFLAGS)

main.o : main.cpp bh.h etr_types.h
	$(CC) -c main.cpp $(CFLAGS)

