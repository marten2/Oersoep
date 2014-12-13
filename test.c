

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 300
#define WIDTH 600

// radius of ball in pixels
#define RADIUS 7

// amount of balls
#define BALLS 10
typedef struct
{
	GOval ball;
	double vx;
	double vy;
}BALL;
// prototypes
void initBall(GWindow window, BALL ballen[]);
BALL detectCollision(GWindow window, BALL ball);

int main(void)
{
	BALL ballen[BALLS]; 
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    GWindow window = newGWindow(WIDTH, HEIGHT);

    // instantiate ball, centered in middle of window
    initBall(window, ballen);
	
	waitForClick();
	
    while (true)
    {
    	for(int i = 0; i < BALLS; i++)
    	{
    		move(ballen[i].ball, ballen[i].vx, ballen[i].vy);
    	
    		double xball = getX(ballen[i].ball);
    		double yball = getY(ballen[i].ball);
    	
    		if((xball <= 0 && ballen[i].vx < 0) || (xball + 2 * RADIUS >= WIDTH && ballen[i].vx > 0))
    		{
   				ballen[i].vx = -ballen[i].vx;
    		}
    	
    		if((yball <= 0 && ballen[i].vy < 0) || (yball >= HEIGHT - RADIUS*2 && ballen[i].vy > 0))
    		{
    			ballen[i].vy = -ballen[i].vy;
    		}
    		
    		// detect collisions reverse speed
    		ballen[i] = detectCollision(window, ballen[i]);
    	}
    	pause(10);
    }

	closeGWindow(window);
    return 0;
}


/**
 * Instantiates ball in center of window.  Returns ball.
 */
void initBall(GWindow window, BALL ballen[])
{
	srand48(time(NULL));
	for(int i = 0; i < BALLS; i++)
	{
		// set ballx compensate if ball is out of the screen
		double ballx = WIDTH * drand48();
		if(ballx >= WIDTH - 2 * RADIUS)
		{
			ballx-= 2 * RADIUS;
		}
		
		// set bally compensate if ball is out of the screen
		double bally = HEIGHT * drand48();
		if(bally >= WIDTH - 2 * RADIUS)
		{
			bally-= 2 * RADIUS;
		}
		
		// place balls and initialise speed
		ballen[i].ball = newGOval(ballx, bally, 2 * RADIUS, 2 * RADIUS);
		ballen[i].vx = drand48() * 2 + 1;
		if(drand48() <= 0.5)
		{
			ballen[i].vx = -ballen[i].vx;
		}
		ballen[i].vy = drand48() * 2 + 1;
		if(drand48() <= 0.5)
		{
			ballen[i].vy = -ballen[i].vy;
		}
		add(window, ballen[i].ball);
		setFilled(ballen[i].ball, true); 
	}
}

/**
 * Detects whether ball has collided with some object in window
 * by checking the four corners of its bounding box (which are
 * outside the ball's GOval, and so the ball can't collide with
 * itself).  Returns object if so, else NULL.
 */
BALL detectCollision(GWindow window, BALL ball)
{
    // get location of object tot check collision
    double x = getX(ball.ball);
    double y = getY(ball.ball);
	
	srand48(time(NULL));
	
    // for checking for collisions
    GObject object;

    // check for collision at detectopject's top-left corner
    object = getGObjectAt(window, x + RADIUS, y);
    if (object != NULL && object != ball.ball)
    {      
        TotVy = ball.vy + object.vy; 
        ball.vy = drand48() * TotVy;
    	object.vy = TotVy - ball.vy;
    }

    // check for collision at ball's top-right corner
    object = getGObjectAt(window, x , y + RADIUS);
    if (object != NULL && object != ball.ball)
    {   	
        ball.vx *= -1;
    }

    // check for collision at ball's bottom-left corner
    object = getGObjectAt(window, x + RADIUS, y + 2 * RADIUS );
    if (object != NULL && object != ball.ball)
    {
    	ball.vy *= -1;
    }

    // check for collision at ball's bottom-right corner
    object = getGObjectAt(window, x + 2 * RADIUS , y + RADIUS);
    if (object != NULL && object != ball.ball)
    {
        ball.vx *= -1;
    }
    return ball;
}


