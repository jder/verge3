/// The VERGE 3 Project is originally by Ben Eirich and is made available via
///  the BSD License.
///
/// Please see LICENSE in the project's root directory for the text of the
/// licensing agreement.  The CREDITS file in the same directory enumerates the
/// folks involved in this public build.
///
/// If you have altered this source file, please log your name, date, and what
/// changes you made below this line.


/****************************************************************
	xerxes engine
	mac_system.cpp
 ****************************************************************/

#include <sys/types.h>
#include <dirent.h>

#include "xerxes.h"


/***************************** data *****************************/


/****************************************************************/

bool AppIsForeground;
int DesktopBPP;
bool IgnoreEvents = false;
/***************************** code *****************************/


// internal use, in mac_cocoa_util.mm
void doMessageBox(string msg);

// internal use
int getCurrentBpp();

int main(int argc, char **argv)
{
	srand(timeGetTime());
	log_Init(true);

	// we must init SDL before any other sdl stuff
	if(SDL_Init(SDL_INIT_TIMER |
			 SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0)
		err("Couldn't start up SDL.");

	// create video window
	sdl_video_init();

	// hide the cursor while we're open -
	// SDL shows it when it moves outside of window
	SDL_ShowCursor(SDL_DISABLE);

	DesktopBPP = getCurrentBpp(); // needs video inited already

	xmain(argc,argv);
	err("");
	return 0;
}

/* returns time in s since 1970 */
/* not suitable for movie timing */
unsigned int timeGetTime()
{
	return time(NULL);
}

/* Get the bits per pixel of the screen currently */
int getCurrentBpp()
{
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	return info->vfmt->BitsPerPixel;
}

// clipboard stuff unimplemented
char *clipboard_getText()
{
	return "";
}

void clipboard_setText(char *text)
{
}

image *clipboard_getImage()
{
	return 0;
}

void clipboard_putImage(image *img)
{
}

// called to update state variables based
// on a mouse event
void handleMouseButton(SDL_MouseButtonEvent e)
{
	bool state = (e.type == SDL_MOUSEBUTTONDOWN ? true : false);
	switch(e.button)
	{
		case SDL_BUTTON_LEFT:
			mouse_l = state;
			break;
		case SDL_BUTTON_RIGHT:
			mouse_r = state;
			break;
		case SDL_BUTTON_MIDDLE:
			mouse_m = state;
			break;
			/* XXX These are changed by 120 each time
			because that seems to be what happens
			in Windows. */
		case SDL_BUTTON_WHEELUP:
			mwheel += 120;
			break;
		case SDL_BUTTON_WHEELDOWN:
			mwheel -= 120;
			break;
	}
}

/* Run the SDL event loop to get waiting messages */
void HandleMessages(void)
{
	SDL_Event event;
	while ( SDL_PollEvent(&event) ) {
		if(IgnoreEvents) {
			// ignore everything but quit
			if(event.type == SDL_QUIT)
				err("");
			continue;
		}
		switch (event.type) {
			case SDL_VIDEORESIZE:
				if(!vid_window)
					break; // not in windowed mode; what's a resize?
				handleResize(event.resize);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				handleMouseButton(event.button);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				ParseKeyEvent(event.key);
				break;
			case SDL_QUIT:
				err("");
				break;
			default:
				break;
		}
	}
}

void writeToConsole(char *str)
{
	printf("%s", str);
}

void initConsole()
{
	/* nothing to do, using stdio */
}

// helper function for get* below
struct tm *getTime()
{
	time_t theTime = time(NULL);
	return localtime(&theTime);
}

int getYear()
{
	return getTime()->tm_year + 1900;
}

int getMonth()
{
	return getTime()->tm_mon;
}

int getDay()
{
	return getTime()->tm_yday;
}

int getDayOfWeek()
{
	return getTime()->tm_wday;
}

int getHour()
{
	return getTime()->tm_hour;
}

int getMinute()
{
	return getTime()->tm_min;
}

int getSecond()
{
	return getTime()->tm_sec;
}

// returns a vector of filenames (in CWD) that match the
// given pattern. The pattern may be either a raw
// string (looking for a given file) or it may be
// "*something", in which case it looks for files ending
// with "something"
// Matches case-insensitively (to mimic windows behaviour)
std::vector<string> listFilePattern(string pattern)
{
	struct dirent *ent;
	std::vector<string> res;
	DIR *d = opendir(".");

	bool matchEnd = false;
	if(pattern[0] == '*')
	{
		matchEnd = true; // just match last piece of name
		pattern = pattern.right(pattern.length()-1);
	}

	// lowercase it and all read names for case-insensitive
	strlwr(pattern.c_str());

	// get each entry, look at the name
	while((ent = readdir(d)) != NULL)
	{
		// lowercase name for case-insensitive search
		strlwr(ent->d_name);
		string name = ent->d_name;
		bool matches = false;
		if(matchEnd)
		{
			// just check end
			if(name.right(pattern.length()) == pattern)
			{
				matches = true;
			}
		}
		else
		{
			if(name == pattern)
				matches = true;
		}
		if(matches)
			res.push_back(name);
	}

	closedir(d);

	return res;
}

// replacement for windows string functions
void strupr(char *c)
{
	while(*c)
	{
		*c = toupper(*c);
		c++;
	}
}
void strlwr(char *c)
{
	while(*c)
	{
		*c = tolower(*c);
		c++;
	}
}

// set main window title, if we have one
void setWindowTitle(char *str)
{
	if(gameWindow) {
		gameWindow->setTitle(str);
	}
}

void Sleep(unsigned int msec)
{
	SDL_Delay(msec);
}

void err(char *str, ...)
{
	va_list argptr;
	char msg[256];

	va_start(argptr, str);
	vsprintf(msg, str, argptr);
	va_end(argptr);

	if(vid_Close)
 	{
 		vid_Close();
 	}

	if (strlen(msg))
	{
		showMessageBox(msg);
		log("Exiting: %s", msg);
	}

	exit(strlen(msg)==0?0:-1);
}

// show a message box to the user. Handles
// mouse showing (and re-hiding if you ask for it)
// and turns full screen off if it's on.
void showMessageBox(string msg)
{
	if(!vid_window)
		sdl_toggleFullscreen();

	// see what the cursor is now
	int cursorState = SDL_ShowCursor(SDL_QUERY);

	// need to show cursor so they can click on it
	SDL_ShowCursor(SDL_ENABLE);

	doMessageBox(msg);

	// now restore it back to the old state
	SDL_ShowCursor(cursorState);
}
