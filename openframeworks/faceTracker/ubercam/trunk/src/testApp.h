#ifndef _TEST_APP
#define _TEST_APP


#define WIDTH				640
#define HEIGHT				480
#define SAMPLE_WIDTH		320
#define SAMPLE_HEIGHT		240

//#define	DEBUG_MODE

#include "ofMain.h"
#include "ofxCvHaarTracker.h"
#include "ofxFrameAnimation.h"
#include "HeadphoneAnimation.h"
#include "KraftwerkAnimation.h"
#include "DaftpunkAnimation.h"

class Face
	{
		public :
		int faceID;
		int x, y, w, h;
		int currentFrame;
		int idleCount;
		int idleCountLimit;
		bool found;
		ofxFrameAnimation *animation;
	};

class testApp : public ofSimpleApp{
	
public:
	
	void setup();
	void update();
	void draw();
	
	void addFace( int faceID, int x, int y, int w, int h );
	bool hasFace( int faceID );
	void updateFace( int faceID, int x, int y, int w, int h );
	void cullFaces();
	void renderFaces();
	
	void keyPressed(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased();
	
	ofVideoGrabber			vidGrabber;
	
	ofxCvColorImage			colorLargeImage;
	ofxCvColorImage			colorSmallImage;
	ofxCvGrayscaleImage		grayLargeImage;
	ofxCvGrayscaleImage 	graySmallImage;
	
	ofxCvHaarFinder			haarFinder;
	ofxCvHaarTracker		haarTracker;
	
	float					sourceToSampleScale;
	float					sampleToSourceScale;
	
	int						animationsTotal;
	ofxFrameAnimation		**animations;
	vector<Face>			faces;
};

#endif
