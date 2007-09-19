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

#include "event_race_select.h"

#include "game_config.h"

#include "ppgltk/font.h"
#include "ppgltk/audio/audio.h"

#include "course_load.h"
#include "textures.h"
#include "joystick.h"

#include "stuff.h"
#include "game_mgr.h"
#include "player.h"

#include <iostream>

static std::list<CourseData>::iterator curElem;

EventRaceSelect::EventRaceSelect()
{
    pp::Vec2d pos(0,0);

	mp_titleLbl = new pp::Label(pos, "heading", _("Select a race"));
	mp_titleLbl->alignment.center();
	mp_titleLbl->alignment.middle();	
	
	
    mp_backBtn = new pp::Button(pos,
			      pp::Vec2d(150, 40), 
			      "button_label", 
			      _("Back") );
    mp_backBtn->setHilitFontBinding( "button_label_hilit" );
    mp_backBtn->signalClicked.Connect(pp::CreateSlot(this,&EventRaceSelect::back));

    mp_startBtn = new pp::Button(pos,
			       pp::Vec2d(150, 40),
			       "button_label",
			       _("Race!") );
    mp_startBtn->setHilitFontBinding( "button_label_hilit" );
    mp_startBtn->setDisabledFontBinding( "button_label_disabled" );
	mp_startBtn->signalClicked.Connect(pp::CreateSlot(this,&EventRaceSelect::start));

	if ( GameMode::prevmode != GAME_OVER ) {
		curElem = gameMgr->getCurrentCup().raceList.begin();
    } else {
		if( gameMgr->wasRaceWon() ){
			if(curElem != --gameMgr->getCurrentCup().raceList.end()){
				curElem++;
			}
		}
    }

	mp_raceListbox = new pp::Listbox<CourseData>(pos,
				   pp::Vec2d(460, 44),
				   "listbox_item",
				   gameMgr->getCurrentCup().raceList);
    mp_raceListbox->setCurrentItem( curElem );
	mp_raceListbox->signalChange.Connect(pp::CreateSlot(this,&EventRaceSelect::listboxItemChange));
    // Create text area 
     
    mp_descTa = new pp::Textarea( pos,
			       pp::Vec2d(312, 107),
			       "race_description",
			       "" );
	mp_descTa->setText( (*curElem).description.c_str() );
    // Create state buttons - only if practicing or if cup_complete
    
  	mp_conditionsSSBtn = NULL;
	mp_windSSBtn = NULL;
	mp_snowSSBtn = NULL;
	mp_mirrorSSBtn = NULL;
    
    updateStates();

    play_music( "start_screen" );
}

EventRaceSelect::~EventRaceSelect()
{
	delete mp_titleLbl;
	
	delete mp_backBtn;
	delete mp_startBtn;
	delete mp_raceListbox;
	delete mp_conditionsSSBtn;
	delete mp_snowSSBtn;
	delete mp_windSSBtn;
	delete mp_mirrorSSBtn;
    delete mp_descTa;	
}

void
EventRaceSelect::loop(float timeStep)
{
	update_audio();

    set_gl_options( GUI );

    clear_rendering_context();

    UIMgr.setupDisplay();

	// todo: detect wether the cours is windy
	// in the meantime we always set windy to true
	drawSnow(timeStep, true);
	
    theme.drawMenuDecorations();

    setWidgetPositionsAndDrawDecorations();

    UIMgr.draw();

    reshape( getparam_x_resolution(), getparam_y_resolution() );

    winsys_swap_buffers();
}

void
EventRaceSelect::setWidgetPositionsAndDrawDecorations()
{
    int w = getparam_x_resolution();
    int h = getparam_y_resolution();
    int box_width, box_height, box_max_y;
    int x_org, y_org;
    GLuint texobj;

    // set the dimensions of the box in which all widgets should fit
    box_width = 460;
    box_height = 310;
    box_max_y = h - 128;

    x_org = w/2 - box_width/2;
    y_org = h/2 - box_height/2;
    if ( y_org + box_height > box_max_y ) {
	y_org = box_max_y - box_height;
    }

    mp_backBtn->setPosition(pp::Vec2d( x_org + 131 - mp_backBtn->getWidth()/2.0,
		      42 ) );

    mp_startBtn->setPosition(pp::Vec2d( x_org + 343 - mp_startBtn->getWidth()/2.0,
		      42 ) );

	mp_raceListbox->setPosition(pp::Vec2d( x_org,y_org + 221 ) );

	mp_descTa->setPosition(pp::Vec2d( x_org,y_org + 66 ) );
    
	// Draw tux life icons
	int i;
	
	glPushMatrix();
	{

	    glTranslatef( x_org + box_width - 4*36 + 4,
			  y_org + 181,
			  0 );
	    
	    check_assertion( INIT_NUM_LIVES == 4, 
			     "Assumption about number of lives invalid -- "
			     "need to recode this part" );

	    if ( !get_texture_binding( "tux_life", &texobj ) ) {
		texobj = 0;
	    }

	    glBindTexture( GL_TEXTURE_2D, texobj );

	    for ( i=0; i<4; i++ ) {
		pp::Vec2d ll, ur;
		if ( players[0].getLives() > i ) {
		    ll = pp::Vec2d( 0, 0.5 );
		    ur = pp::Vec2d( 1, 1 );
		} else {
		    ll = pp::Vec2d( 0, 0 );
		    ur = pp::Vec2d( 1, 0.5 );
		}
		glBegin( GL_QUADS );
		{
		    glTexCoord2f( ll.x, ll.y );
		    glVertex2f( 0, 0 );

		    glTexCoord2f( ur.x, ll.y );
		    glVertex2f( 32, 0 );

		    glTexCoord2f( ur.x, ur.y );
		    glVertex2f( 32, 32 );

		    glTexCoord2f( ll.x, ur.y );
		    glVertex2f( 0, 32 );
		}
		glEnd();

		glTranslatef( 36, 0, 0 );
	    }
	}
	glPopMatrix();
    
    // Draw other stuff
	
	mp_titleLbl->setPosition(pp::Vec2d(x_org + box_width/2.0,y_org + 310));
	
	// Draw text indicating race requirements (if race not completed), 
    //   or results in race if race completed.
    drawStatusMsg( x_org, y_org, box_width, box_height );

    // Draw preview
	std::list<CourseData>::iterator elem;
	elem = mp_raceListbox->getCurrentItem();

    glDisable( GL_TEXTURE_2D );

    glColor4f( 1.0, 1.0, 1.0, 1.0 );
    glBegin( GL_QUADS );
    {
		glVertex2f( x_org+box_width-140, y_org+66 );
		glVertex2f( x_org+box_width, y_org+66 );
		glVertex2f( x_org+box_width, y_org+66+107 );
		glVertex2f( x_org+box_width-140, y_org+66+107 );
    }
    glEnd();

    glEnable( GL_TEXTURE_2D );

    if ( !get_texture_binding( (*elem).course.c_str(), &texobj ) ) {
	if ( !get_texture_binding( "no_preview", &texobj ) ) {
	    texobj = 0;
	}
    }

    glBindTexture( GL_TEXTURE_2D, texobj );

    glBegin( GL_QUADS );
    {
	glTexCoord2d( 0, 0);
	glVertex2f( x_org+box_width-136, y_org+70 );

	glTexCoord2d( 1, 0);
	glVertex2f( x_org+box_width-4, y_org+70 );

	glTexCoord2d( 1, 1);
	glVertex2f( x_org+box_width-4, y_org+70+99 );

	glTexCoord2d( 0, 1);
	glVertex2f( x_org+box_width-136, y_org+70+99 );
    }
    glEnd();
}

void
EventRaceSelect::drawStatusMsg( int x_org, int y_org, int box_width, int box_height )
{
    const char *msg;
    char buff[BUFF_LEN];
    bool draw_stats = true;	
	
	switch(state){
		case DEAD:
			msg = _("You don't have any lives left.");
	    	draw_stats = false;
			break;
		
		case RACEWON:
			msg = _("Race won! Your result:");	
			break;
		
		case PLAYABLE:
			msg = _("Needed to advance:");
			break;
		
		case UNPLAYABLE:
			msg = _("You can't enter this race yet.");
			draw_stats = false;
			break;
		default:
			msg = "";
			draw_stats = false;
			break;
	}
		
	pp::Font *font = pp::Font::get("race_requirements");
	pp::Font *labelfont = pp::Font::get("race_requirements_label");
	
	labelfont->draw(msg,pp::Vec2d(x_org,y_org + 200));
	
	if ( draw_stats ) {
		int minutes;
		int seconds;
		int hundredths;

		pp::Vec2d pos(x_org,y_org + 184);
		
		getTimeComponents( m_data.time, minutes, seconds, hundredths );
		
		const char* string = _("Time:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;
		
		sprintf( buff, "%02d:%02d.%02d",minutes, seconds, hundredths);
		font->draw(buff, pos);
		
		pos.x+=75;
		string = _("Herring:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;

		sprintf( buff, "%03d", m_data.herring ); 
		font->draw(buff, pos);
		
		pos.x+=40;	
		string = _("Score:");
		labelfont->draw(string, pos);
		pos.x+=labelfont->advance(string) + 5;
	
		sprintf( buff, "%06d", m_data.score );
		font->draw(buff, pos);
	
	}
		
	const char* string = _("Contributed by:");
	pp::Vec2d pos(x_org,y_org+50);
	labelfont->draw(string,pos);
	pos.x += labelfont->advance(string) + 5;
		
	if(!(*curElem).contributed.empty()){
		font->draw((*curElem).contributed.c_str(),pos);		
	}else{
		font->draw(_("Unknown"),pos);
	}	
}


void
EventRaceSelect::updateStates()
{
	//mp_startBtn->setSensitive( true );
	if ( players[0].getLives() <= 0 ) {
		state=DEAD;
		mp_startBtn->setSensitive( false );
	} else {
		m_data.won=false;
		players[0].getCupCourseData(
				std::string(gameMgr->getCurrentEvent().name),
				std::string(gameMgr->getCurrentCup().name),
				std::string((*curElem).name),
				m_data);
	
		if(m_data.won){
			state=RACEWON;
			mp_startBtn->setSensitive( false );	
		}else{
			
			difficulty_level_t d = gameMgr->difficulty;
			m_data.time = (*curElem).time_req[d];
			m_data.herring = (*curElem).herring_req[d];
			m_data.score = int((*curElem).score_req[d]);

			if(curElem==gameMgr->getCurrentCup().raceList.begin()){
				state=PLAYABLE;
				mp_startBtn->setSensitive( true );
			}else{
				PlayerCourseData prev;
				curElem--;
				players[0].getCupCourseData(
					std::string(gameMgr->getCurrentEvent().name),
					std::string(gameMgr->getCurrentCup().name),
					std::string((*curElem).name),
					prev);
				curElem++;
				
				if(prev.won){
					state=PLAYABLE;
					mp_startBtn->setSensitive( true );
				} else {
					state=UNPLAYABLE;
					mp_startBtn->setSensitive( false );
				}
			}
		}
	}
}

void
EventRaceSelect::back()
{
	set_game_mode( EVENT_SELECT );
    UIMgr.setDirty();
}

void
EventRaceSelect::start()
{
    if(mp_startBtn->getSensitive()) {
	    mp_startBtn->setHighlight( true );
	    loop( 0 );
		gameMgr->setCurrentRace(curElem);

	    set_game_mode( LOADING );
    }
}


void
EventRaceSelect::listboxItemChange()
{
	curElem = mp_raceListbox->getCurrentItem();
	mp_descTa->setText( (*curElem).description.c_str() );
	updateStates();
	UIMgr.setDirty();
}

bool
EventRaceSelect::keyPressEvent(SDLKey key)
{
	switch (key){
		case SDLK_UP:
		case SDLK_LEFT:
			mp_raceListbox->gotoPrevItem();
	    	return true;
		case SDLK_RIGHT:
		case SDLK_DOWN:
			mp_raceListbox->gotoNextItem();
	    	return true;
		case SDLK_RETURN:
	    	mp_startBtn->simulateMouseClick();
			UIMgr.setDirty();
			return true;
		case SDLK_ESCAPE:
	    	mp_backBtn->simulateMouseClick();
			UIMgr.setDirty();
			return true;
		case 'c': 
	    	mp_conditionsSSBtn->simulateMouseClick();
	    	return true;
		case 'w': 
	    	mp_windSSBtn->simulateMouseClick();
	    	return true;
		case 'm':
	    	mp_mirrorSSBtn->simulateMouseClick();
	    	return true;
		default:
			return false;
	}	
}
