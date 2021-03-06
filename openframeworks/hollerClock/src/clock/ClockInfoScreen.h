/*
 *  ClockInfoScreen.h
 *  emptyExample
 *
 *  Created by lukasz karluk on 31/07/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CLOCK_INFO_SCREEN_H
#define CLOCK_INFO_SCREEN_H

#include "ofMain.h"
#include "Quad.h"

class ClockInfoScreen
{
public :
	
	 ClockInfoScreen();
	~ClockInfoScreen();
	
	void setSize	( int w, int h );
	void setTexture	( ofTexture* tex );
	
	void setup		();
	void update		();
	void draw		();
	
	void show		();
	void hide		();
	
	bool	bShowing;
	bool	bVisible;
	int		screenWidth;
	int		screenHeight;
	
	ofTexture* tex;
	
	float	alphaBg;
	float	alphaTex;
	float	tweenTime;
	float	tweenTimeTotal;
};

#endif