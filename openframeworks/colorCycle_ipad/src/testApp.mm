#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup()
{	
	ofRegisterTouchEvents(this);
	ofxAccelerometer.setup();
	ofxiPhoneAlerts.addListener(this);
	ofBackground( 127, 127, 127 );
	
	ofxiPhoneSetOrientation( OFXIPHONE_ORIENTATION_LANDSCAPE_LEFT );
	
	ofSetFrameRate( frameRate = 60 );
//	ofSetVerticalSync( true );
	
	lastTouchId = -1;
	
	splashScreen = NULL;
	splashScreen = new SplashScreen();		// comment out to remove.
	if( splashScreen != NULL )
	{
		splashScreen->setup();
		splashScreen->draw();
	}
}

void testApp :: init ()
{
	footer = NULL;
	footer = new FooterBar();				// comment out to remove.
	if( footer != NULL )
	{
		footer->setup();
	}
	
	infoScreen = NULL;
	infoScreen = new InfoScreen();			// comment out to remove.
	if( infoScreen != NULL )
	{
		infoScreen->setup();
	}
	
	popupScreen = NULL;
	popupScreen = new PopupScreen();
	if( popupScreen != NULL )
	{
		popupScreen->setup();
	}
	
	sounds = NULL;
	sounds = new ColorSound();				// comment out to remove.
	if( sounds != NULL )
	{
		sounds->setup();
	}
	
	box2d.init();
	
	cc = NULL;
	cc = new ColorCycle();					// comment out to remove.
	if( cc != NULL )
	{
		cc->setBox2d( &box2d );
		cc->setSounds( sounds );
		cc->setScreenSize( ofGetWidth(), ofGetHeight() );
		cc->setup();
	}
}

//--------------------------------------------------------------
void testApp::update()
{
	int t = 2;
	int f = ofGetFrameNum();
	if( f < t )		return;
	if( f == t )	init();
		
	checkLastTouch();
	
	if( footer != NULL )
	{
		if( footer->isShuffleSelected() )
		{
			if( cc != NULL )
			{
				cc->shuffle();
			}
		}
	}
	
	if( footer != NULL )
	{
		if( footer->isColorSelected() )
		{
			if( cc != NULL )
			{
				cc->startColorSelectMode();
			}
		}
	}

	if( footer != NULL )
	{
		if( footer->isAddSelected() )
		{
			if( cc != NULL )
			{
				cc->addCircle();
			}
		}
	}
	
	if( footer != NULL )
	{
		if( footer->isRemoveSelected() )
		{
			if( cc != NULL )
			{
				cc->removeCircle();
			}
		}
	}
	
	if( footer != NULL )
	{
		if( footer->isInfoSelected() )
		{
			if( infoScreen != NULL )
			{
				infoScreen->show();
			}
		}
	}
	
	if( footer != NULL )
	{
		if( footer->isHideSelected() )				// doesn't work, touch needs to pass through EAGLView to work.
		{
			ofPoint p = footer->getHidePoint();
			
			if( cc != NULL )
			{
				cc->down( p.x, p.y, 0 );
			}
		}
	}
	
	if( footer != NULL )
	{
		if( footer->isPhotoSaved() )
		{
			if( popupScreen != NULL )
			{
				popupScreen->show();
			}
		}
	}
	
	float gx;
	gx = ofxAccelerometer.getForce().y;
	gx *= 2;									// increase the reaction to tilt.
	gx = MIN( 1.0, MAX( -1.0, gx ) );			// between -1 and 1.
	
	float gy;
	gy = ofxAccelerometer.getForce().x;
	gy *= 2;									// increase the reaction to tilt.
	gy = MIN( 1.0, MAX( -1.0, gy ) );			// between -1 and 1.

	if( cc != NULL )
	{
		cc->setGravity( gx, gy );
		cc->update();
	}
}

//--------------------------------------------------------------
void testApp::draw()
{
	if( cc != NULL )
		cc->draw();
	
	if( splashScreen != NULL )
		splashScreen->draw();
	
	if( upsideDown )
	{
		int cx = (int)( ofGetScreenWidth()  * 0.5 );
		int cy = (int)( ofGetScreenHeight() * 0.5 );
		
		glPushMatrix();
		glTranslatef( cx, cy, 0 );
		glRotatef( 180, 0, 0, 1 );
		glTranslatef( -cx, -cy, 0 );
	}
	
	if( cc != NULL )
		cc->drawColorPanel();
	
	if( infoScreen != NULL )
		infoScreen->draw();
	
	if( popupScreen != NULL )
		popupScreen->draw();
	
	ofSetColor( 0, 0, 0 );
//	ofDrawBitmapString( ofToString( ofGetFrameRate(),  0 ), ofGetScreenWidth() - 30, 20 );
	
	if( upsideDown )
	{
		glPopMatrix();
	}
}

//--------------------------------------------------------------
void testApp::exit()
{
	delete popupScreen;
	delete infoScreen;
	delete splashScreen;
	delete footer;
	delete sounds;
	delete cc;
}

//--------------------------------------------------------------
void testApp::touchDown(ofTouchEventArgs &touch)
{
	if( infoScreen )
		infoScreen->mouseDown( touch.x, touch.y );
	
	if( lastTouchId != touch.id )
	{
		lastTouchId			= touch.id;
		lastTouch.x			= touch.x;
		lastTouch.y			= touch.y;
		lastTouchMoved.x	= touch.x;
		lastTouchMoved.y	= touch.y;
		lastTouchCount		= 0;
	}
	
	if( cc != NULL )
		cc->down( touch.x, touch.y, touch.id );
}

//--------------------------------------------------------------
void testApp::touchMoved(ofTouchEventArgs &touch)
{
	if( lastTouchId == touch.id )
	{
		lastTouchMoved.x	= touch.x;
		lastTouchMoved.y	= touch.y;
	}
	
	if( cc != NULL )
		cc->drag( touch.x, touch.y, touch.id );
}

//--------------------------------------------------------------
void testApp::touchUp(ofTouchEventArgs &touch)
{
	if( lastTouchId == touch.id )
	{
		lastTouchId = -1;
	}
	
	if( cc != NULL )
		cc->up( touch.x, touch.y, touch.id );
}

void testApp :: checkLastTouch ()
{
	if( lastTouchId == -1 )
		return;
	
	if( ++lastTouchCount > 30 )
	{
		lastTouchId = -1;
		
		float d = ofDist( lastTouch.x, lastTouch.y, lastTouchMoved.x, lastTouchMoved.y );
		
		if( d < 20 )
		{
			if( footer != NULL )
			{
				if( !footer->isShowing() )
				{
					footer->show();
					
					if( cc != NULL )
						cc->stopColorSelectMode();
				}
			}
		}
	}
}

//--------------------------------------------------------------
void testApp::touchDoubleTap(ofTouchEventArgs &touch)
{
	if( infoScreen )
		infoScreen->hide();
	
	if( footer == NULL )
		return;
		
	footer->toggleShow();
	
	if( footer->isShowing() )
	{
		if( cc != NULL )
			cc->stopColorSelectMode();
	}
}

//--------------------------------------------------------------
void testApp::lostFocus()
{
	//
}

//--------------------------------------------------------------
void testApp::gotFocus(){

}

//--------------------------------------------------------------
void testApp::gotMemoryWarning()
{
	cout << "memory warning" << endl;
}

//--------------------------------------------------------------
void testApp::deviceOrientationChanged(int newOrientation)
{
	if( newOrientation == 3 )
		upsideDown = true;
	
	if( newOrientation == 4 )
		upsideDown = false;
	
	if( cc != NULL )
		cc->setUpsideDown( upsideDown );
}

