#include "bounce.hpp"
#include "assets.hpp"

using namespace blit;

struct GAME
{
    int state = 0;
	short miss[3]{10,10,10};
    int score = 0;
	short dscore;
	short skill;
	int best;
	int freq = 0;
};

struct PLAYER 
{
	short pos;
};

struct BALL
{	short place[2][6]{{10,10,10,10,10,10},{10,10,10,10,10,10}};
    short x;
	short pos;
};

struct BURST
{
	short pos;
	short step;
};

GAME game;
PLAYER p;
BURST b;
BALL ball;

Timer ani_timer;
Timer burst_timer;

void score();

void burst_update(Timer &t)
{
	b.step++;
	if (b.step > 5)
		burst_timer.stop();

   	channels[1].frequency = 100 - (b.step * 10);
	channels[1].trigger_attack();
}

void miss_ball(short pos, short ud, short p)
{
	b.pos = pos;
	b.step = 0;
	burst_timer.start();

	for (short i=0; i<3; i++)
	{
		if (game.miss[i] == 10)
		{
			game.miss[i] = ball.place[ud][p];
			ball.place[ud][p] = 10;
			if (i == 2)
			{
				if (game.score == game.best)
					write_save(game.best);
				game.state = 2;
			}
			break;
		}
	}
}		

void ani_update(Timer &t)
{
	ball.x++;
	game.freq = 0;

	if (game.state == 1)
	{	
		if (ball.x == 8)
		{
            game.freq = 250;
			if (ball.place[1][3] < 8)
				p.pos == 1?	score(): miss_ball(1, 1, 3);
		}
		else if (ball.x == 16)
		{
			game.freq = 200;
			if (ball.place[0][2] < 8)
				p.pos == 2? score(): miss_ball(2, 0 ,2);
		}
		else if (ball.x == 24)
		{
			game.freq = 250;
			if (ball.place [1][2] < 8)
				p.pos == 3? score(): miss_ball(3, 1, 2);
		}
		else if (ball.x == 32)
		{
			game.freq = 200;
			if (ball.place[0][4] < 8)
				p.pos == 0?	score(): miss_ball(0, 0, 4);
		}
	}
	else // CPU Control
	{	
		if (ball.x == 4 && ball.place[1][3] < 8)
			p.pos = 1;
		else if (ball.x == 12 && ball.place[0][2] < 8)
			p.pos = 2;
		else if (ball.x == 20 && ball.place[1][2] < 8)
			p.pos = 3;
		else if (ball.x == 28 && ball.place[0][4] < 8)
			p.pos = 0;
	}

	if (game.freq > 0)
	{
    	channels[0].frequency = game.freq;
		channels[0].trigger_attack();
	}

	if (ball.x == 32)
	{
		bool is_ball = false;
		short random = 12;

        for (short b=0; b<5; b++) //Ball
		{
			ball.place[0][b] = ball.place[0][b+1];
			ball.place[1][b] = ball.place[1][b+1];
			if (ball.place[0][b] < 8 || ball.place[1][b] < 8)
				is_ball = true;
		}

		if (is_ball && game.score <= 240) 
			random = 60 - int(game.score / 5);
		ball.place[0][5] = int(rand() %random);
		ball.place[1][5] = int(rand() %random);

		ball.x = 0;
	}
}	

void UpdateControl()
{
	// TASTATUR
/*    if (buttons & Button::B)
		p.pos = 0;
    else if (buttons & Button::DPAD_LEFT)
    	p.pos = 3;
    else if (buttons & Button::DPAD_RIGHT)
    	p.pos = 1;
    else if (buttons & Button::X)
		p.pos = 2;
*/  
	// 32BLIT
	if (buttons & Button::DPAD_UP)
		p.pos = 0;
    else if (buttons & Button::B)
    	p.pos = 1;
    else if (buttons & Button::X)
		p.pos = 2;
    else if (buttons & Button::DPAD_DOWN)
    	p.pos = 3;
}

void score()
{
	game.freq = 800;
	game.score++;
	if (game.score > game.best)
		game.best = game.score;

	game.dscore++;
	if (game.dscore == 50 && game.skill < 5)
	{
		game.dscore = 0;
		game.skill++;
		ani_timer.init(ani_update, 75 - (game.skill * 5), -1);
	    ani_timer.start();
	}
}

void start()
{
    p.pos = 0;
	
	for (short i=0;i<6;i++)
	{
		ball.place[0][i] = 10;
		ball.place[1][i] = 10;
	}

	for (short i=0;i<3;i++)
		game.miss[i] = 10;
    game.score = 0;
	game.dscore = 0;
	game.skill = 0;

    ani_timer.init(ani_update, 75, -1);
    ani_timer.start();

    game.state = 1;
}

void init() 
{
    set_screen_mode(ScreenMode::lores);

    screen.sprites = Surface::load(sprites);

    channels[0].waveforms = Waveform::SQUARE; 
    channels[0].attack_ms = 5;
    channels[0].decay_ms = 50;
    channels[0].release_ms = 50;
    channels[0].sustain = 0;
	channels[0].volume = 18000;

    channels[1].waveforms = Waveform::SQUARE; 
    channels[1].attack_ms = 5;
    channels[1].decay_ms = 50;
    channels[1].release_ms = 50;
    channels[1].sustain = 0;
	channels[1].volume = 12000;

    ani_timer.init(ani_update, 75, -1);
    ani_timer.start();

	burst_timer.init(burst_update,50, -1);
	burst_timer.stop();

    if (read_save(game.best) == false)
        game.best = 0;
}


// render(time)

void render(uint32_t time) 
{
    screen.pen = Pen(0, 0, 0);
    screen.clear();

    screen.alpha = 255;
    screen.mask = nullptr;

	short height[40]{39,28,19,12,7,4,2,1,0,1,2,4,7,12,19,28,39,28,19,12,7,4,2,1,0,1,2,4,7,12,19,28,39,28,19,12,7,4,2,1};
	short px[4]{46,94,94,46};
	short py[4]{57,115,57,115};

    for (short b=0; b<6; b++)
    {
		if (ball.place[0][b] < 8)
			screen.sprite(Rect((ball.place[0][b]) * 2, 0, 2, 2), Point(145 + ball.x  - (b * 32),5 + height[ball.x]));
		if (ball.place[1][b] < 8)
			screen.sprite(Rect((ball.place[1][b]) * 2, 0, 2, 2), Point(9 - ball.x  + (b * 32), 63 + height[ball.x + 8]));
    }

    screen.pen = Pen(128, 128, 128);
	screen.rectangle(Rect(0,57,45,2));
	screen.rectangle(Rect(67,57,26,2));
	screen.rectangle(Rect(115,57,45,2));
	screen.rectangle(Rect(0,115,45,2));
	screen.rectangle(Rect(67,115,26,2));
	screen.rectangle(Rect(115,115,45,2));

	screen.pen = Pen(255, 255, 255); // Player
	screen.rectangle(Rect(px[p.pos],py[p.pos],20,4));

	if (burst_timer.is_running())
		screen.sprite(Rect(b.step * 2, 3, 2, 2), Point(px[b.pos] + 3, py[b.pos] - 14));

	for (short i=0; i<3; i++)
		screen.sprite(32 + game.miss[i], Point(69 + (i * 7), 1));

	screen.pen = Pen(224, 224, 224);
	screen.text("score " + std::to_string(game.score), minimal_font, Point(1, 0), true, TextAlign::top_left);
	screen.text("best " + std::to_string(game.best), minimal_font, Point(160, 0), true, TextAlign::top_right);

   	if (game.state == 0) 
       screen.text("press a to start", minimal_font, Point(80, 70), true, TextAlign::center_center);
    else if (game.state == 2)
       screen.text("game over", minimal_font, Point(80, 70), true, TextAlign::center_center);
}

// update(time)

void update(uint32_t time) 
{
    if (game.state == 1) // game
        UpdateControl();
    else // title + game over
    {
        if (buttons.released & Button::A)
            start();        
    }
}

