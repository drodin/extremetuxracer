/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
 *
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef WINSYS_H
#define WINSYS_H 1

#include "SDL.h"

void setup_sdl_video_mode();
void winsys_post_redisplay();

void winsys_swap_buffers();
void winsys_enable_key_repeat( bool enabled );
void winsys_warp_pointer( int x, int y );
void winsys_show_cursor( bool visible );

void winsys_init( int *argc, char **argv, char *window_title,
		  char *icon_title );
void winsys_shutdown();

void winsys_process_events(); /* Never returns */

void winsys_exit( int code );

#endif /* WINSYS_H */
