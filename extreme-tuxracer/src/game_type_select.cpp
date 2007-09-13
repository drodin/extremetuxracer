/* 
 * PPRacer 
 * Copyright (C) 2004-2005 Volker Stroebel <volker@planetpenguin.de>
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


#include "game_type_select.h"

#include "ppgltk/audio/audio.h"
#include "ppgltk/alg/defs.h"

#include "game_mgr.h"


GameTypeSelect::GameTypeSelect()
{
	pp::Vec2d pos(0,0);

    mp_enterEventBtn = new pp::Button(pos,
				     pp::Vec2d(300, 40), 
				     "button_label", 
				     _("Enter an event") );
    mp_enterEventBtn->setHilitFontBinding( "button_label_hilit" );
    mp_enterEventBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::eventSelect));

    mp_practiceBtn = new pp::Button(pos,
				  pp::Vec2d(300, 40),
				  "button_label",
				  _("Practice")	);
    mp_practiceBtn->setHilitFontBinding( "button_label_hilit" );
    mp_practiceBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::practice));

    mp_highscoreBtn = new pp::Button(pos,
				  pp::Vec2d(300, 40),
				  "button_label",
				  _("Highscore")	);
    mp_highscoreBtn->setHilitFontBinding( "button_label_hilit" );
    mp_highscoreBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::highscore));

	mp_configureBtn = new pp::Button(pos,
				  pp::Vec2d(300, 40),
				  "button_label",
				  _("Configuration") );
    mp_configureBtn->setHilitFontBinding( "button_label_hilit" );
    mp_configureBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::configuration));

	mp_creditsBtn = new pp::Button(pos,
				  pp::Vec2d(300, 40),
				  "button_label",
				  _("Credits") );
    mp_creditsBtn->setHilitFontBinding( "button_label_hilit" );
	mp_creditsBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::credits));

	mp_quitBtn = new pp::Button(pos,
			      pp::Vec2d(300, 40),
			      "button_label",
			      _("Quit") );

	mp_quitBtn->setHilitFontBinding( "button_label_hilit" );
    mp_quitBtn->signalClicked.Connect(pp::CreateSlot(this,&GameTypeSelect::quit));

    play_music( "start_screen" );	
}

GameTypeSelect::~GameTypeSelect()
{
	delete mp_enterEventBtn;
	delete mp_practiceBtn;
	delete mp_highscoreBtn;
	delete mp_configureBtn;
	delete mp_creditsBtn;
	delete mp_quitBtn;	
}

void
GameTypeSelect::loop(float timeStep)
{
	update_audio();

    set_gl_options( GUI );

    clear_rendering_context();

    setWidgetPositions();

    UIMgr.setupDisplay();

	drawSnow(timeStep);

    theme.drawMenuDecorations();

    UIMgr.draw();
	
    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
}


void
GameTypeSelect::setWidgetPositions()
{
    pp::Button* button_list[] = { mp_enterEventBtn,
				  mp_practiceBtn,
				  mp_highscoreBtn,
				  mp_configureBtn,
				  mp_creditsBtn,
				  mp_quitBtn };
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int box_height;
    int box_max_y;
    int top_y;
    int bottom_y;
    int num_buttons = sizeof( button_list ) / sizeof( button_list[0] );
    int i;
    int tot_button_height = 0;
    int button_sep =0;
    int cur_y_pos;

    box_height = 210;
    box_max_y = h - 128;

    bottom_y = int(0.4*h - box_height/2);

    if ( bottom_y + box_height > box_max_y ) {
	bottom_y = box_max_y - box_height;
    }

    top_y = bottom_y + box_height;

    for (i=0; i<num_buttons; i++) {
		tot_button_height += int(button_list[i]->getHeight());
    }

    if ( num_buttons > 1 ) {
	button_sep = ( top_y - bottom_y - tot_button_height ) / 
	    ( num_buttons - 1 );
	button_sep = MAX( 0, button_sep );
    }

    cur_y_pos = top_y;
    for (i=0; i<num_buttons; i++) {
	cur_y_pos -= int(button_list[i]->getHeight());
	button_list[i]->setPosition(pp::Vec2d( w/2.0 - button_list[i]->getWidth()/2.0,
			  cur_y_pos ) );
	cur_y_pos -= button_sep;
    }
}

bool
GameTypeSelect::keyPressEvent(SDLKey key)
{
	switch (key){
		case 'q':
		case 27: // Esc 
			winsys_exit(0);
	    	return true;;
		case 'e':
		case 13: // Enter
			mp_enterEventBtn->simulateMouseClick();
	    	return true;
		case 'p':
			mp_practiceBtn->simulateMouseClick();
			return true;
		case 'c':
			mp_creditsBtn->simulateMouseClick();
			return true;
		default:
			return false;
	}	
}

void
GameTypeSelect::eventSelect()
{
	gameMgr->reset(GameMgr::EVENT);

	set_game_mode( EVENT_SELECT );
	UIMgr.setDirty();
}

void
GameTypeSelect::credits()
{
	set_game_mode( CREDITS );
	UIMgr.setDirty();
}

void
GameTypeSelect::practice()
{
	gameMgr->reset(GameMgr::PRACTICING);

	set_game_mode( RACE_SELECT );
	UIMgr.setDirty();
}

void
GameTypeSelect::highscore()
{
	set_game_mode( HIGHSCORE );
	UIMgr.setDirty();
}

void
GameTypeSelect::configuration()
{
	set_game_mode( CONFIGURATION );
	UIMgr.setDirty();
}

void
GameTypeSelect::quit()
{
	winsys_exit( 0 );
}
