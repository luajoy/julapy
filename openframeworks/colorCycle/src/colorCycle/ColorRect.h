/*
 *  ColorRect.h
 *  emptyExample
 *
 *  Created by lukasz karluk on 10/08/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"

class ColorRect
{
	
public:
	
	 ColorRect();
	 ColorRect( float w, float h );
	~ColorRect();
	
	void init			( float w, float h );
	void setCornerColor	( const ofColor& c, int cornerIndex );
	void draw			();
	
	GLfloat*	rectPoints;
	GLfloat*	rectColors;

};