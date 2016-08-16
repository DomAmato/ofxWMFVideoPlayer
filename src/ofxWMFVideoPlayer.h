#pragma once
//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//Based upon Windows SDK samples
//MIT Licensing


#include "ofMain.h"
#include "ofxWMFVideoPlayerUtils.h"
#include "EVRPresenter.h"

class CPlayer;
class ofxWMFVideoPlayer : public ofBaseVideoPlayer {

public:
	ofxWMFVideoPlayer();
	~ofxWMFVideoPlayer();

	bool				load(string name);
	void				loadAsync(string name);

	void				closeMovie() { close(); }
	void				close();

	void				update();
	void				play();
	void				stop();

	void				setPaused(bool bPause);

	float				getPosition();
	float				getPositionInSeconds();
	float				getDuration();
	float				getFrameRate();

	bool				canRewind() { return _player->canRewind(); }

	void				setPosition(float pct);

	void				setVolume(float vol);
	float				getVolume();

	float				getHeight() const { return _player->getHeight(); }
	float				getWidth() const { return _player->getWidth(); }

	bool				isLooping() { return _isLooping; }

	void				setLoopState(ofLoopType loopType);
	ofLoopType			getLoopState() { return _isLooping ? OF_LOOP_NORMAL : OF_LOOP_NONE; }
	bool				getIsMovieDone();

	void				setSpeed(float speed) { setSpeed(speed, false); }
	bool				setSpeed(float speed, bool useThinning = false); //thinning drops delta frames for faster playback though appears to be choppy, default is false
	float				getSpeed();

	bool				isLoaded() const;

	const ofPixels &    getPixels() const;
	ofPixels &          getPixels();

	ofPixelFormat		getPixelFormat() const;
	bool				isFrameNew() const;

	PlayerState			getState() { return _player->GetState(); }
	bool				isPlaying() const { return _player->GetState() == Started; }
	bool				isStopped() const { return (_player->GetState() == Stopped || _player->GetState() == Paused); }
	bool				isPaused() const { return _player->GetState() == Paused; }

	bool				setPixelFormat(ofPixelFormat pixelFormat);

	bool				lockSharedTexture();
	void				unlockSharedTexture();

	void				draw(int x, int y, int w, int h);
	void				draw(int x, int y) { draw(x, y, getWidth(), getHeight()); }

	void                OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr);

	HWND				getHandle() { return _hwndPlayer; }
	LRESULT				WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


	ofTexture&	getTexture() {
		if (playerTex == NULL) {
			return _tex;
		}
		else {
			return *playerTex;
		}
	};

	void setAnchorPercent(float xPct, float yPct) {
		if (lockSharedTexture()) {
			_tex.setAnchorPercent(xPct, yPct);
			unlockSharedTexture();
		}
	}
	void setAnchorPoint(float x, float y) {
		if (lockSharedTexture()) {
			_tex.setAnchorPoint(x, y);
			unlockSharedTexture();
		}
	}
	void resetAnchor() {
		if (lockSharedTexture()) {
			_tex.resetAnchor();
			unlockSharedTexture();
		}
	}

private:
	CPlayer*	_player;

	int _id;

	static int  _instanceCount;

	HWND		_hwndPlayer;

	BOOL bRepaintClient;

	int _width;
	int _height;

	bool _waitingForLoad;
	bool _waitForLoadedToPlay;
	bool _isLoaded;
	bool _isLooping;
	bool _wantToSetVolume;
	float _currentVolume;

	bool _sharedTextureCreated;

	ofTexture _tex;
	ofTexture * playerTex;
	ofPixels _pixels;

	BOOL InitInstance();
	bool endLoad();

	float _frameRate;

	void setLoop(bool isLooping) { _isLooping = isLooping; _player->setLooping(isLooping); }
	static void forceExit();
};