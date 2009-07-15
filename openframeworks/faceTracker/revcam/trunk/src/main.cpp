#include "ofMain.h"
#include "testApp.h"

//#define	FULL_SCREEN

//========================================================================
int main( ){

	#ifdef FULL_SCREEN
	
		ofSetupOpenGL( 1280, 768, OF_FULLSCREEN );
	
	#else
	
		ofSetupOpenGL( 1280, 768, OF_WINDOW );
//		ofSetWindowPosition( -1600, 0 );
	
	#endif
	
	ofRunApp( new testApp() );
}
