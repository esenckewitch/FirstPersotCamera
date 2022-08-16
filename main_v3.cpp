
#include <cmath>
#include <string>
#include <chrono>
#include <vector>
#include <utility>
#include <algorithm>

using namespace std;

#include <ncurses.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>



#define DELAY 100
#define M_HEIGHT 16
#define M_WIDTH 16

#define nShade1 L"\u2588"
#define nShade2 L"\u2593"
#define nShade3 L"\u2592"
#define nShade4 L"\u2591"




typedef enum{
	INIT = 0,
	DRAW,
	PROCESSING,
	EXIT
}eGameState;

typedef enum{
	WALL = 0,
	EMPTY
}eMapInfo;

struct sPlayer{
	double fPlayerX = 0.0;
	double fPlayerY = 0.0;
	double fPlayerA = 0.0;
};

static void reycast(sPlayer p, wstring field, int nScreenWidth, int nScreenHeight){
	double fFOV = 3.14159 / 4.0;
	double fDepth = 16.0;
	wchar_t *nShade;

	for(int x = 0; x < nScreenWidth; x++){
		double fRayAngle = (p.fPlayerA - fFOV / 2.0) + ((double)x / (double)nScreenWidth) *fFOV;
		double fDistanceToWall = 0;
		bool bHitWall = false;
		bool bBoundary = false;


		double fEyeX = sinf(fRayAngle);
		double fEyeY = cosf(fRayAngle);
	
		while(!bHitWall && fDistanceToWall < fDepth){

			fDistanceToWall += 0.01;

			int nTestX = (int)(p.fPlayerX + fEyeX * fDistanceToWall);
			int nTestY = (int)(p.fPlayerY + fEyeY * fDistanceToWall);


			// Test if rey is out of bounds
			if(nTestX < 0 || nTestX >= M_WIDTH || nTestY < 0 || nTestY >= M_HEIGHT){
				bHitWall = true;
				fDistanceToWall = fDepth;
			}
			else if(field[nTestY * M_WIDTH + nTestX] == '#'){
				bHitWall = true;
				vector<pair<float,float >> pa;

				for(int tx = 0; tx < 2; tx++)
					for(int ty = 0; ty < 2; ty++){
						float vy = (float)nTestY + ty - p.fPlayerY;
						float vx = (float)nTestX + tx - p.fPlayerX;
						float d = sqrt(vx*vx + vy*vy);
						float dot = (fEyeX * vx / d) + (fEyeY * vy /d);
						pa.push_back(make_pair(d, dot));
					}
				sort(pa.begin(), pa.end(), [](const pair<float, float> &left, const pair<float, float> &right){return left.first < right.first;});
				float fBound = 0.01;
				if(acos(pa.at(0).second) < fBound) bBoundary = true;
				if(acos(pa.at(1).second) < fBound) bBoundary = true;
				//if(acos(pa.at(2).second) < fBound) bBoundary = true;
			}
		}

		int nCeiling = (double)(nScreenHeight/2.0) - nScreenHeight / ((double)fDistanceToWall);
		int nFloor = nScreenHeight - nCeiling;
		
		for(int y = 0; y < nScreenHeight; y++){
			if(y < nCeiling) mvprintw(y,x, " ");
			else if(y > nCeiling && y <= nFloor){ 
				if(fDistanceToWall <= fDepth/4.0) mvaddwstr(y,x, nShade1);
				else if(fDistanceToWall < fDepth/3.0) mvaddwstr(y,x, nShade2);
				else if(fDistanceToWall < fDepth/2.0) mvaddwstr(y,x, nShade3);
				else if(fDistanceToWall < fDepth) mvaddwstr(y, x, nShade4);
				else mvaddwstr(y,x, L"\u0020");

				if(bBoundary) mvprintw(y,x, "|");


			}
			else{
				double b = 1.0f - (((float) y - nScreenHeight/ 2.0f)/((float)nScreenHeight /2));
				if(b < 0.25f) mvprintw(y,x, "#");
				else if (b < 0.5f) mvprintw(y,x, "x");
				else if(b < 0.75f) mvprintw(y,x, ".");
				else if(b < 0.9f) mvprintw(y,x,"-");
				else mvprintw(y,x, " ");
			}
		}
	}

}

int main(){
	setlocale(LC_ALL, "");
	int max_x, max_y, key;
	int is_run = true;
	sPlayer player;
	wstring field;
	field += L"################";
	field += L"#..............#";
	field += L"#..............#";
	field += L"#######........#";
	field += L"#..............#";
	field += L"#..#############";
	field += L"#..............#";
	field += L"#..............#";
	field += L"#..............#";
	field += L"#..............#";
	field += L"#......###...###";
	field += L"#......#.......#";
	field += L"#......#.......#";
	field += L"#......#.......#";
	field += L"#......#.......#";
	field += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();
	
	player.fPlayerX = 6;
	player.fPlayerY = 6;

	eGameState game_state = INIT;
	getmaxyx(stdscr, max_y, max_x);
	while(is_run){
		switch(game_state){
			case INIT:
				initscr();
				cbreak();
				timeout(DELAY);
				keypad(stdscr, 1);
				noecho();
				curs_set(0);
				//auto tp1 = chrono::system_clock::now();
				//auto tp2 = chrono::system_clock::now();
				game_state = DRAW;
				break;
			case DRAW:
				clear();
				reycast(player, field, max_x, max_y);
				refresh();
				game_state = PROCESSING;
				break;
			case PROCESSING:

				//tp2 = chrono::system_clock::now();
				//chrono::duration<float> eTime = tp2 - tp1;
				//tp1 = tp2;
				//float fElapsedTime = eTime.count();

				key = getch();
				getmaxyx(stdscr, max_y, max_x);
				
				if(key == KEY_LEFT) player.fPlayerA -= (0.05f);	
				else if(key == KEY_RIGHT) player.fPlayerA += (0.05f);
				
				if(key == KEY_UP){
					player.fPlayerX += sinf(player.fPlayerA) * 0.5f;
					player.fPlayerY += cosf(player.fPlayerA) * 0.5f;
				}
				if(key == KEY_DOWN){
					player.fPlayerX -= sinf(player.fPlayerA) * 0.5f;
					player.fPlayerY -= cosf(player.fPlayerA) * 0.5f;
				}

				if(key == 27){
					game_state = EXIT;
					break;
				}
				game_state = DRAW;
				break;
			case EXIT:
				is_run = false;
				endwin();
				return 0;
				break;
		}
	}
	endwin();
	return 0;
}
