#pragma once


//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//Based upon Windows SDK samples
//MIT Licensing


#include "ofMain.h"

#include "ofxWMFVideoPlayerUtils.h"

#include "EVRPresenter.h"



//class ofxWMFVideoPlayer;


class CPlayer;
class ofxWMFVideoPlayer : public ofBaseVideoPlayer {

	private:

		static int  _instanceCount;

		HWND		_hwndPlayer;

		BOOL bRepaintClient;


		int _width;
		int _height;


		bool _waitForLoadedToPlay;
		bool _isLooping;
		bool _wantToSetVolume;
		float _currentVolume;

		bool _sharedTextureCreated;

		ofTexture _tex;
		ofPixels _pixels;

		BOOL InitInstance();


		void	OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);

		bool	loadEventSent;
		bool	bLoaded;


		float	_frameRate;


	public:

	CPlayer*	_player;

	int _id;

	ofxWMFVideoPlayer();
	 ~ofxWMFVideoPlayer();

	 bool			load(string name) { return loadMovie(name); }
	 bool			loadMovie(string name);

	 void			close();
	 void			update();

	 void			play();
	 void			stop();
	 void			pause();
	 void			setPaused( bool bPause ) ;

	 float			getPosition();
	 float			getDuration();
	 float			getFrameRate();

	 void			setPosition(float pos);

	 void			setVolume(float vol);
	 float			getVolume() const;

	 float			getHeight() const;
	 float			getWidth()  const;

	 bool			isPlaying() const;
	 bool			isStopped() const;
	 bool			isPaused()  const;

	 bool			isLoaded() const;

	 void			setLoop(bool isLooping);
	 bool			isLooping() { return _isLooping; }

	 void			bind();
	 void			unbind();

	ofEvent<bool>   videoLoadEvent;

	 void			setLoopState( ofLoopType loopType ) ;
	 bool			getIsMovieDone( ) ;

	 ofPixels&		getPixelsRef() { return _pixels; }
	 ofPixels&		getPixels() { return _pixels; }
	const ofPixels& getPixels() const { return _pixels; };
	 ofTexture * 	getTexture(){ return &_tex; };
	 bool 			setPixelFormat(ofPixelFormat pixelFormat);
	 ofPixelFormat 	getPixelFormat() const;

	 bool			isFrameNew() const;

	 void			draw(int x, int y , int w, int h);
	 void			draw(int x, int y) { draw(x,y,getWidth(),getHeight()); }

	 HWND			getHandle() { return _hwndPlayer;}
	 LRESULT		WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	 static void	forceExit();


};
