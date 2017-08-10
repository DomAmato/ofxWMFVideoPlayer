//ofxWMFVideoPlayer addon written by Philippe Laulheret for Second Story (secondstory.com)
//MIT Licensing

#include "ofxWMFVideoPlayerUtils.h"
#include "ofxWMFVideoPlayer.h"

typedef std::pair<HWND, ofxWMFVideoPlayer*> PlayerItem;
list<PlayerItem> g_WMFVideoPlayers;

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
// Message handlers

ofxWMFVideoPlayer* findPlayers(HWND hwnd)
{
	for (PlayerItem e : g_WMFVideoPlayers)
	{
		if (e.first == hwnd) return e.second;
	}

	return NULL;
}

int  ofxWMFVideoPlayer::_instanceCount = 0;

ofxWMFVideoPlayer::ofxWMFVideoPlayer() 
	: _player(NULL)
	, playerTex(NULL)
{

	if (_instanceCount == 0) {
		if (!ofIsGLProgrammableRenderer()) {
			if (wglewIsSupported("WGL_NV_DX_interop")) {
				ofLogVerbose("ofxWMFVideoPlayer") << "WGL_NV_DX_interop supported";
			}
			else {
				ofLogError("ofxWMFVideoPlayer") << "WGL_NV_DX_interop not supported. Upgrade your graphc drivers and try again.";
				return;
			}
		}


		HRESULT hr = MFStartup(MF_VERSION);
		if (!SUCCEEDED(hr))
		{
			ofLog(OF_LOG_ERROR, "ofxWMFVideoPlayer: Error while loading MF");
		}
	}

	_id = _instanceCount;
	_instanceCount++;
	this->InitInstance();

	_waitingForLoad = false;
	_waitForLoadedToPlay = false;
	_sharedTextureCreated = false;
	_wantToSetVolume = false;
	_currentVolume = 1.0;
	_frameRate = 0.0f;
}

ofxWMFVideoPlayer::~ofxWMFVideoPlayer() {
	//forceExit();
	if (_player)
	{
		_player->Shutdown();
		//if (_sharedTextureCreated) _player->m_pEVRPresenter->releaseSharedTexture();
		SafeRelease(&_player);
	}

	cout << "Player " << _id << " Terminated" << endl;
	_instanceCount--;
	if (_instanceCount == 0) {
		MFShutdown();

		cout << "Shutting down MF" << endl;
	}
	int i = 1;
}

void ofxWMFVideoPlayer::forceExit()
{
	if (_instanceCount != 0)
	{
		cout << "Shutting down MF some ofxWMFVideoPlayer remains" << endl;
		MFShutdown();
	}
}

bool ofxWMFVideoPlayer::load(string name) {
	_isLoaded = false;
	if (!_player) {
		ofLogError("ofxWMFVideoPlayer") << "Player not created. Can't open the movie.";
		return false;
	}

	HRESULT hr = S_OK;
	if (name.find("http") == string::npos) {
		DWORD fileAttr = GetFileAttributesA(ofToDataPath(name).c_str());
		if (fileAttr == INVALID_FILE_ATTRIBUTES) {
			stringstream s;
			s << "The video file '" << name << "'is missing.";
			ofLog(OF_LOG_ERROR, "ofxWMFVideoPlayer:" + s.str());
			return false;
		}
		string s = ofToDataPath(name);
		std::wstring w(s.length(), L' ');
		std::copy(s.begin(), s.end(), w.begin());
		hr = _player->OpenURL(w.c_str());
	}
	else {
		string s = name;
		std::wstring w(s.length(), L' ');
		std::copy(s.begin(), s.end(), w.begin());
		hr = _player->OpenURL(w.c_str());
	}
	_waitForLoadedToPlay = false;
	return endLoad();
}

void ofxWMFVideoPlayer::loadAsync(string name) {
	_isLoaded = false;
	if (!_player) {
		ofLogError("ofxWMFVideoPlayer") << "Player not created. Can't open the movie.";
		return;
	}

	HRESULT hr = S_OK;
	if (name.find("http") == string::npos) {
		DWORD fileAttr = GetFileAttributesA(ofToDataPath(name).c_str());
		if (fileAttr == INVALID_FILE_ATTRIBUTES) {
			stringstream s;
			s << "The video file '" << name << "'is missing.";
			ofLog(OF_LOG_ERROR, "ofxWMFVideoPlayer:" + s.str());
			return;
		}
		string s = ofToDataPath(name);
		std::wstring w(s.length(), L' ');
		std::copy(s.begin(), s.end(), w.begin());
		hr = _player->OpenURLAsync(w.c_str());
	}
	else {
		string s = name;
		std::wstring w(s.length(), L' ');
		std::copy(s.begin(), s.end(), w.begin());
		hr = _player->OpenURLAsync(w.c_str());
	}
	_waitingForLoad = true;
}

bool ofxWMFVideoPlayer::endLoad() {
	_frameRate = 0.0; //reset frameRate as the new movie loaded might have a different value than previous one
	if (!_sharedTextureCreated)
	{

		_width = _player->getWidth();
		_height = _player->getHeight();

		_tex.allocate(_width, _height, GL_RGBA, true);

		_player->m_pEVRPresenter->createSharedTexture(_width, _height, _tex.texData.textureID);
		_sharedTextureCreated = true;
	}
	else
	{
		if ((_width != _player->getWidth()) || (_height != _player->getHeight()))
		{

			_player->m_pEVRPresenter->releaseSharedTexture();

			_width = _player->getWidth();
			_height = _player->getHeight();

			_tex.allocate(_width, _height, GL_RGBA, true);
			_player->m_pEVRPresenter->createSharedTexture(_width, _height, _tex.texData.textureID);

		}
	}
	_isLoaded = true;
	return true;
}

bool ofxWMFVideoPlayer::lockSharedTexture() {
	if (_isLoaded) {
		_player->m_pEVRPresenter->lockSharedTexture();
		return true;
	}
	return false;
}

void ofxWMFVideoPlayer::unlockSharedTexture() {
	_player->m_pEVRPresenter->unlockSharedTexture();
}

void ofxWMFVideoPlayer::draw(int x, int y, int w, int h) {
	if (lockSharedTexture()) {
		_tex.draw(x, y, w, h);
		unlockSharedTexture();
	}
}

void ofxWMFVideoPlayer::close() {
	_player->Shutdown();
	_currentVolume = 1.0;
	_wantToSetVolume = false;

}

void ofxWMFVideoPlayer::update() {
	if (!_player) return;

	if (_waitingForLoad && _player->GetState() == OpenAsyncComplete) {
		_player->EndOpenURL();
		endLoad();
		_waitingForLoad = false;
	}

	if ((_waitForLoadedToPlay) && _player->GetState() == Paused)
	{
		_waitForLoadedToPlay = false;
		_player->Play();

	}

	DWORD d;
	_player->GetBufferProgress(&d);

	if ((_wantToSetVolume))
	{
		_player->setVolume(_currentVolume);

	}

	if ((_wantToSetVolume))
	{
		_player->setVolume(_currentVolume);

	}
	return;
}

bool	ofxWMFVideoPlayer::getIsMovieDone()
{
	return _player->getIsDone();
}

bool ofxWMFVideoPlayer::isLoaded() const {
	if (_player == NULL) { return false; }
	PlayerState ps = _player->GetState();
	return ps == PlayerState::Paused || ps == PlayerState::Stopped || ps == PlayerState::Started;
}

ofPixels& ofxWMFVideoPlayer::getPixels() {
	if (_tex.isAllocated()) {
		_tex.readToPixels(_pixels);
	}
	return _pixels;
}

const ofPixels& ofxWMFVideoPlayer::getPixels() const {
	/*if (_tex.isAllocated()) {
		_tex.readToPixels(_pixels);
	}*/
	return _pixels;
}

bool ofxWMFVideoPlayer::setPixelFormat(ofPixelFormat pixelFormat) {
	return (pixelFormat == OF_PIXELS_RGB);
}

ofPixelFormat ofxWMFVideoPlayer::getPixelFormat() const {
	return OF_PIXELS_RGB;
}

bool ofxWMFVideoPlayer::isFrameNew() const {
	return true;//TODO fix this
}

void ofxWMFVideoPlayer::setPaused(bool bPause) 
{
	if (bPause == true)
		_player->Pause();
	else
		play();
}

void ofxWMFVideoPlayer::play()
{

	if (!_player) return;
	if (_player->GetState() == OpenAsyncPending || _player->GetState() == OpenAsyncComplete || _player->GetState() == OpenPending)
		_waitForLoadedToPlay = true;
	else
		_player->Play();
}

void	ofxWMFVideoPlayer::stop()
{
	_player->Stop();
}

void ofxWMFVideoPlayer::setLoopState(ofLoopType loopType)
{
	switch (loopType)
	{
	case OF_LOOP_NONE:
		setLoop(false);
		break;
	case OF_LOOP_NORMAL:
		setLoop(true);
		break;
	default:
		ofLogError("ofxWMFVideoPlayer::setLoopState LOOP TYPE NOT SUPPORTED") << loopType << endl;
		break;
	}
}

float ofxWMFVideoPlayer::getPosition()
{
	return (_player->getPosition() / getDuration());
}

float ofxWMFVideoPlayer::getPositionInSeconds()
{
	//this returns it in seconds
	return _player->getPosition();
}

float 			ofxWMFVideoPlayer::getDuration() {
	return _player->getDuration();
}

void ofxWMFVideoPlayer::setPosition(float pos)
{
	pos = ofClamp(pos, 0, 1);
	_player->setPosition(pos*getDuration());
}

void ofxWMFVideoPlayer::setVolume(float vol)
{

	if ((_player) && (_player->GetState() != OpenPending) && (_player->GetState() != Closing) && (_player->GetState() != Closed)) {
		_player->setVolume(vol);
		_wantToSetVolume = false;
	}
	else {
		_wantToSetVolume = true;
	}
	_currentVolume = vol;

}

float ofxWMFVideoPlayer::getVolume()
{
	return _player->getVolume();
}

float ofxWMFVideoPlayer::getFrameRate()
{
	if (!_player) return 0.0f;
	if (_frameRate == 0.0f) _frameRate = _player->getFrameRate();
	return _frameRate;
}

float ofxWMFVideoPlayer::getSpeed() {
	return _player->GetPlaybackRate();
}

bool ofxWMFVideoPlayer::setSpeed(float speed, bool useThinning) {
	//according to MSDN playback must be stopped to change between forward and reverse playback and vice versa
	//but is only required to pause in order to shift between forward rates
	float curRate = getSpeed();
	HRESULT hr = S_OK;
	bool resume = isPlaying();
	float position = getPosition();
	hr = _player->SetPlaybackRate(useThinning, speed);

	if (resume && !isPlaying()) {
		_player->setPosition(position);
		//_player->Play();
	}
	return hr == S_OK ? true : false;
}


//-----------------------------------
// Prvate Functions
//-----------------------------------

// Handler for Media Session events.
void ofxWMFVideoPlayer::OnPlayerEvent(HWND hwnd, WPARAM pUnkPtr)
{
	HRESULT hr = _player->HandleEvent(pUnkPtr);
	if (FAILED(hr))
	{
		LPTSTR errorText = NULL;

		FormatMessageW(
			// use system message tables to retrieve error text
			FORMAT_MESSAGE_FROM_SYSTEM
			// allocate buffer on local heap for error text
			| FORMAT_MESSAGE_ALLOCATE_BUFFER
			// Important! will fail otherwise, since we're not 
			// (and CANNOT) pass insertion parameters
			| FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
			hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorText,  // output 
			0, // minimum size for output buffer
			NULL);   // arguments - see note 
		wstring ws = errorText;
		string error(ws.begin(), ws.end());

		ofLogError("ofxWMFVideoPlayer") << error;
		// release memory allocated by FormatMessage()
		LocalFree(errorText);
		errorText = NULL;
	}
}

LRESULT CALLBACK WndProcDummy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{


	switch (message)
	{

	case WM_CREATE:
	{
		return DefWindowProc(hwnd, message, wParam, lParam);

	}
	default:
	{
		ofxWMFVideoPlayer*   myPlayer = findPlayers(hwnd);
		if (!myPlayer) return DefWindowProc(hwnd, message, wParam, lParam);
		return myPlayer->WndProc(hwnd, message, wParam, lParam);
	}
	}
	return 0;
}

LRESULT  ofxWMFVideoPlayer::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{

	case WM_DESTROY:
		PostQuitMessage(0);
		break;


	case WM_APP_PLAYER_EVENT:
		OnPlayerEvent(hwnd, wParam);
		break;

	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

//  Create the application window.
BOOL ofxWMFVideoPlayer::InitInstance()
{
	PCWSTR szWindowClass = L"MFBASICPLAYBACK";
	HWND hwnd;
	WNDCLASSEX wcex;

	//   g_hInstance = hInst; // Store the instance handle.

	// Register the window class.
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;

	wcex.lpfnWndProc = WndProcDummy;
	//  wcex.hInstance      = hInst;
	wcex.hbrBackground = (HBRUSH)(BLACK_BRUSH);
	// wcex.lpszMenuName   = MAKEINTRESOURCE(IDC_MFPLAYBACK);
	wcex.lpszClassName = szWindowClass;

	if (RegisterClassEx(&wcex) == 0)
	{
		// return FALSE;
	}


	// Create the application window.
	hwnd = CreateWindow(szWindowClass, L"", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, NULL, NULL);

	if (hwnd == 0)
	{
		return FALSE;
	}


	g_WMFVideoPlayers.push_back(std::pair<HWND, ofxWMFVideoPlayer*>(hwnd, this));
	HRESULT hr = CPlayer::CreateInstance(hwnd, hwnd, &_player);



	LONG style2 = ::GetWindowLong(hwnd, GWL_STYLE);
	style2 &= ~WS_DLGFRAME;
	style2 &= ~WS_CAPTION;
	style2 &= ~WS_BORDER;
	style2 &= WS_POPUP;
	LONG exstyle2 = ::GetWindowLong(hwnd, GWL_EXSTYLE);
	exstyle2 &= ~WS_EX_DLGMODALFRAME;
	::SetWindowLong(hwnd, GWL_STYLE, style2);
	::SetWindowLong(hwnd, GWL_EXSTYLE, exstyle2);



	_hwndPlayer = hwnd;



	UpdateWindow(hwnd);


	return TRUE;
}