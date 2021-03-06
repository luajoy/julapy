#include "clockApp.h"

///////////////////////////////////////////
//	SETUP.
///////////////////////////////////////////

void clockApp :: setup()
{	
	ofxAccelerometer.setup();
	ofxiPhoneAlerts.addListener(this);
	ofxiPhoneSetOrientation( OFXIPHONE_ORIENTATION_LANDSCAPE_RIGHT );
	
	//-------------------
	
	splashScreen = NULL;
	splashScreen = new SplashScreen();		// comment out to remove.
	if( splashScreen != NULL )
	{
		splashScreen->setup();
		splashScreen->draw();
	}
	
	//-------------------
	
	rot				= 0;
	rotTime			= 0;
	rotTimeTotal	= 20;
	bFlipLeft		= false;
	bFlipRight		= false;

	ofBackground( 0, 0, 0 );
	ofSetFrameRate( frameRate = 60 );
	ofSetCircleResolution( 100 );
	
	//-- size.

	screenSize.width	= ofGetScreenWidth();
	screenSize.height	= ofGetScreenHeight();

	deviceType = DEVICE_IPAD;	// default.
	
	if( screenSize.width == 480 && screenSize.height == 320 )
		deviceType = DEVICE_IPHONE;

	if( screenSize.width == 1024 && screenSize.height == 768 )
		deviceType = DEVICE_IPAD;
	
	//-- sound.
	
	secTwoSound.loadSound( ofToDataPath( "sound/beep_600hz_0_70.caf" ) );
	secTwoSound.setVolume( 0.5 );
	secOneSound.loadSound( ofToDataPath( "sound/beep_400hz_1_40.caf" ) );
	
	//-- images.
	
	texFsSize.width		= screenSize.width  * 1.0;
	texFsSize.height	= screenSize.height * 1.0;
	
	if( deviceType == DEVICE_IPHONE )
	{
//		loadImageToTexture( ofToDataPath( "image/background_480x320.png" ), texBg );
	}
	else if( deviceType == DEVICE_IPAD )
	{
//		loadImageToTexture( ofToDataPath( "image/background_1024x768.png" ), texBg );
	}
	
	if( deviceType == DEVICE_IPHONE )
	{
		loadImageToTexture( ofToDataPath( "image/info_480x320.png" ), texInfo );
	}
	else if( deviceType == DEVICE_IPAD )
	{
		loadImageToTexture( ofToDataPath( "image/info_1024x768.png" ), texInfo );
	}

	if( deviceType == DEVICE_IPHONE )
	{
		loadImageToTexture( ofToDataPath( "image/membrane2_512x25.png" ), texMembrane );
	}
	else if( deviceType == DEVICE_IPAD )
	{
		loadImageToTexture( ofToDataPath( "image/membrane2_512x25.png" ), texMembrane );
	}
	
	vector<string> cellNames;
	if( deviceType == DEVICE_IPHONE )
	{
		cellNames.push_back( "image/cell01_64x64.png" );
//		cellNames.push_back( "image/cell02_64x64.png" );
//		cellNames.push_back( "image/cell03_64x64.png" );
	}
	else if( deviceType == DEVICE_IPAD )
	{
		cellNames.push_back( "image/cell01_128x128.png" );
//		cellNames.push_back( "image/cell02_128x128.png" );
//		cellNames.push_back( "image/cell03_128x128.png" );
	}
	
	texCellsNum = cellNames.size();
	texCells	= new ofTexture[ texCellsNum ];
	
	for( int i=0; i<cellNames.size(); i++ )
	{
		loadImageToTexture( ofToDataPath( cellNames[ i ] ), texCells[ i ] );
	}
	
	vector<string> digitNames;
	digitNames.push_back( "image/digits/0_64x64.png" );
	digitNames.push_back( "image/digits/1_64x64.png" );
	digitNames.push_back( "image/digits/2_64x64.png" );
	digitNames.push_back( "image/digits/3_64x64.png" );
	digitNames.push_back( "image/digits/4_64x64.png" );
	digitNames.push_back( "image/digits/5_64x64.png" );
	digitNames.push_back( "image/digits/6_64x64.png" );
	digitNames.push_back( "image/digits/7_64x64.png" );
	digitNames.push_back( "image/digits/8_64x64.png" );
	digitNames.push_back( "image/digits/9_64x64.png" );
	
	texDigitsNum	= digitNames.size();
	texDigits		= new ofTexture[ texDigitsNum ];
	
	for( int i=0; i<digitNames.size(); i++ )
	{
		loadImageToTexture( ofToDataPath( digitNames[ i ] ), texDigits[ i ] );
	}
	
	initClock();
	
	//-- flash.
	
	stage = ofxFlashStage :: getInstance();			// ofxFlash setup.
	
	
	xfl.loadFile( "assets/assets_ipad_1024x768/DOMDocument.xml" );		// load XFL flash file.
	xfl.build();
	
	bg = new Background( (ofxFlashMovieClip*)stage->root()->getChildByName( "bg" ) );
	
	ofxFlashMovieClip* clockHolder;
	clockHolder = (ofxFlashMovieClip*)bg->asset->getChildByName( "clock" );
	clockHolder->addChild( &clock );
}

void clockApp :: initClock ()
{
	box2d.init();
	box2d.setGravity( 0, 0 );
	box2d.setFPS( 30.0 );
//	box2d.registerGrabbing();
	box2d.setIterations( 20, 10 );
	box2d.getWorld()->SetContactListener( &contactListener );
	
	contactListener.addReceiver( &clock );

	int w = screenSize.width;
	int h = screenSize.height;
	
	float a1 = w * h;
	float a2 = 1280 * 720;
	float fs = a1 / (float)a2;
	
	clock.setBox2d( &box2d );
	clock.setSize( w, h );
	clock.setScreenScale( fs );
	clock.setSound( &secTwoSound, &secOneSound );
//	clock.setBgTexture( &texBg );
	clock.setCellTexture( texCells, texCellsNum );
	clock.setInfoTexture( &texInfo );
	clock.setMembraneTex( &texMembrane );
	clock.setDigitTexture( texDigits, texDigitsNum );
	clock.setup();
}

void clockApp :: loadImageToTexture	( string path, ofTexture& tex )
{
	NSString*	imagePath;
	UIImage*	image;
	
	imagePath	= ofxStringToNSString( path );
	image		= [ [ UIImage alloc ] initWithContentsOfFile : imagePath ];
	
	ofxiPhoneUIImageToOFTexture( image, tex );
	
	[ image release ];
}

///////////////////////////////////////////
//	UPDATE.
///////////////////////////////////////////

void clockApp :: update()
{
	if( splashScreen )
		if( !splashScreen->isFading() )			// splash screen is not fading yet... don't update yet.
			return;
	
	int hrs;
	int min;
	int sec;
	
	if( false )
	{
		int frameNum = ofGetFrameNum();
		
		int s = 00 + frameNum / frameRate;		// start time is add at front.
		int m = 48 + s / 60;					// start time is add at front.
		int h = 12 + m / 60;					// start time is add at front.
		
		hrs = h % 24;
		min = m % 60;
		sec = s % 60;
	}
	else
	{
		hrs = ofGetHours();
		min = ofGetMinutes();
		sec = ofGetSeconds();
	}
	
	//-- orientation.
	
	float ax = ofxAccelerometer.getForce().x;
	float ot = 0.6;								// orientation threshold.
	
	if( ax > ot )
	{
		if( ofxiPhoneGetOrientation() != OFXIPHONE_ORIENTATION_LANDSCAPE_LEFT && !bFlipLeft )
		{
			rotTime		= 0;
			bFlipLeft	= true;
			bFlipRight	= false;
		}
	}
	else if( ax < -ot )
	{
		if( ofxiPhoneGetOrientation() != OFXIPHONE_ORIENTATION_LANDSCAPE_RIGHT && !bFlipRight )
		{
			rotTime		= 0;
			bFlipLeft	= false;
			bFlipRight	= true;
		}
	}
	
	if( bFlipLeft )
	{
		flipLeft();
	}
	else if( bFlipRight )
	{
		flipRight();
	}
	
	//-- gravity slant.
	
	float gx;
	gx = ofxAccelerometer.getForce().y;
	gx *= 2;									// increase the reaction to tilt.
	gx = MIN( 1.0, MAX( -1.0, gx ) );			// between -1 and 1.
	gx *= ( ofxiPhoneGetOrientation() == OFXIPHONE_ORIENTATION_LANDSCAPE_RIGHT ) ? -1 : 1;

	float gy;
	gy = ofxAccelerometer.getForce().x;
	gy *= 2;									// increase the reaction to tilt.
	gy = MIN( 1.0, MAX( -1.0, gy ) );			// between -1 and 1.
	gy *= ( ofxiPhoneGetOrientation() == OFXIPHONE_ORIENTATION_LANDSCAPE_RIGHT ) ? -1 : 1;
	
	clock.setGravity( gx, gy );
	clock.update( hrs, min, sec );
	
	//-- update.

	bg->update();
	stage->update();
}

void clockApp :: flipLeft ()
{
	if( ++rotTime >= rotTimeTotal )
	{
		ofxiPhoneSetOrientation( OFXIPHONE_ORIENTATION_LANDSCAPE_LEFT );
		
		bFlipLeft	= false;
		rot			= 0;
		
		return;
	}
	
	rot = Quad :: easeOut( rotTime, 0, -180, rotTimeTotal );
}

void clockApp :: flipRight ()
{
	if( ++rotTime >= rotTimeTotal )
	{
		ofxiPhoneSetOrientation( OFXIPHONE_ORIENTATION_LANDSCAPE_RIGHT );
		
		bFlipRight	= false;
		rot			= 0;
		
		return;
	}
	
	rot = Quad :: easeOut( rotTime, 0, 180, rotTimeTotal );
}

///////////////////////////////////////////
//	DRAW.
///////////////////////////////////////////

void clockApp :: draw()
{
	if( bFlipLeft || bFlipRight )
	{
		glPushMatrix();
		glTranslatef( screenSize.width * 0.5, screenSize.height * 0.5, 0 );
		glRotatef( rot, 0, 0, 1 );
		glTranslatef( -screenSize.width * 0.5, -screenSize.height * 0.5, 0 );
	}
	
	bool bDrawStage = true;
	if( splashScreen )
	{
		if( !splashScreen->isFading() )
		{
			bDrawStage = false;
		}
	}
	
	if( bDrawStage )
	{
		stage->draw();
	}
	
	if( splashScreen )
	{
		if( splashScreen->isComplete() )
		{
			delete splashScreen;
			splashScreen = NULL;
		}
		else
		{
			splashScreen->draw();
		}
	}
	
//	ofSetColor( 0x000000 );
//	ofDrawBitmapString( ofToString( ofGetFrameRate(), 0 ), 15, 15 );
	
	if( bFlipLeft || bFlipRight )
	{
		glPopMatrix();
	}
}

void clockApp :: exit()
{

}

///////////////////////////////////////////
//	HANDLERS.
///////////////////////////////////////////

void clockApp::touchDown( int x, int y, int id )
{
	clock.touchDown( x, y, id );
	box2d.touchDown( x, y, id );
	
	bg->touchDown( x, y, id );
	stage->mousePressed( x, y, id );
}

void clockApp::touchMoved( int x, int y, int id )
{
	clock.touchMoved( x, y, id );
	box2d.touchMoved( x, y, id );
	
	bg->touchMoved( x, y, id );
	stage->mouseMoved( x, y, id );
}

void clockApp::touchUp( int x, int y, int id )
{
	clock.touchUp( x, y, id );
	box2d.touchUp( x, y, id );
	
	bg->touchUp( x, y, id );
	stage->mouseReleased( x, y, id );
}

void clockApp::touchDoubleTap( int x, int y, int id )
{
	clock.toggleClockMode();
}

void clockApp::lostFocus(){

}

void clockApp::gotFocus(){

}

void clockApp::gotMemoryWarning(){

}

void clockApp::deviceOrientationChanged(int newOrientation)
{
	//
}

