

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <cs50.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 1000

// radius of ball in pixels
#define RADIUS 4

// amount of balls
#define BALLS 50
typedef struct
{
	GOval ball;
	double vx;
	double vy;
	int mass;
	int counter;
	char type;
	bool exists;
	string color;
}BALL;

typedef struct node
{
	BALL* blueprint;

}node;

// prototypes
void initBall(GWindow window, BALL ballen[], int T);
void collision(BALL* ball1, BALL* ball2, double rand);
bool Eact(BALL* ball1, BALL* ball2, int i); 
int typereaction(BALL* ball1, BALL* ball2);
void react(BALL* ball1, BALL* ball2, int reaction_type);
bool decompose_time(int counter, char type, double random);
void decompose(BALL ball[], int i);
int getFreeSpot(BALL ballen[]);
void initDataStructure();
BALL DeepCopyBall(BALL blueprint);


int hashfunction(char type)
{
	return type - 'A';
}
GWindow window;
int T;
node hashtable[10];
int main(void)
{
	initDataStructure();
	
	// get temperature
	printf("Temp:");
 	int* temp = malloc(sizeof(int));
	scanf("%i", temp);
	
	T = *temp;
	free(temp);
	
	BALL ballen[BALLS]; 
    // seed pseudorandom number generator
    srand48(time(NULL));

    // instantiate window
    window = newGWindow(WIDTH, HEIGHT);

    // instantiate ball, centered in middle of window
    initBall(window, ballen, T);
	
	waitForClick();
	int ballcount = 0;
    while (true)
    {
    	for(int i = 0; i < BALLS; i++)
    	{
    		if (ballen[i].exists == true)
    		{
    			ballcount++;
				BALL* ball1 = &ballen[i];
				if (ballen[i].type != 'A')
				{
					ballen[i].counter++;
				}
				if (decompose_time(ballen[i].counter, ballen[i].type, drand48()))
				{
					decompose(ballen, i);
				}
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
				for (int j = 0; j < BALLS; j++)
				{
				    if (j != i && ballen[j].exists == true)
				    {
				    	BALL* ball2 = &ballen[j];
				        double xball2 = getX(ballen[j].ball);
				        double yball2 = getY(ballen[j].ball);
				        if ( // left upper corner
				            ((xball2 > xball && xball2 < xball + 2 * RADIUS) && 
				            (yball2 > yball && yball2 < yball + 2 * RADIUS)) ||
				            
				            // right upper corner
				            ((xball2 + 2 * RADIUS > xball && xball2 < xball) && 
				            (yball2 > yball && yball2 < yball + 2 * RADIUS)) ||
				            
				            // left lower corner
				            ((xball2 > xball && xball2 < xball + 2 * RADIUS) && 
				            (yball2 + 2 * RADIUS > yball && yball2 < yball)) ||
				            
				            // right lower corner
				            ((xball2 + 2 * RADIUS > xball && xball2 < xball) && 
				            (yball2 + 2 * RADIUS> yball && yball2 < yball)))
				            {
				                double rand = drand48();
				                collision(ball1, ball2, rand);
				            }
				            
				    }
				}
			}
		}
/*		printf("balls: %i\n", ballcount);*/
/*		ballcount = 0;*/
    	pause(10);
    }

	closeGWindow(window);
    return 0;
}

/**
 * Builds a data structure with all information about the system
 * in the form of a hashtable.
 */
void initDataStructure()
{
	int masses[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	char* colors[10] = {"BLUE", "RED", "CYAN", "ORANGE", "YELLOW", "GRAY", "PINK", "DARK_GRAY", "LIGHT_GRAY", "BLACK"};
	
	for(int i = 0; i < 10; i++)
	{
		
		BALL* new_blueprint = malloc(sizeof(BALL));
		new_blueprint->color = colors[i];
		new_blueprint->mass = masses[i];
		new_blueprint->type = 'A' + i;
		new_blueprint->exists = true;
		hashtable[i].blueprint = new_blueprint;
	}
}
/**
 * Instantiates ball in center of window.  Returns ball.
 */
void initBall(GWindow window, BALL ballen[], int T)
{
	srand48(time(NULL));
	for(int i = 0; i < BALLS; i++)
	{
		char type = 'A';
		int index = hashfunction(type);
		
		// make deepcopy from blueprint to balls 
		ballen[i] = DeepCopyBall(*hashtable[index].blueprint);
		printf("Colour = %f\n", getX(ballen[i].ball));
		
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
		ballen[i].vx = drand48() * T + 1;
		if(drand48() <= 0.5)
		{
			ballen[i].vx = -ballen[i].vx;
		}
		ballen[i].vy = drand48() * T + 1;
		if(drand48() <= 0.5)
		{
			ballen[i].vy = -ballen[i].vy;
		}
		addAt(window, ballen[i].ball, ballx, bally);
	}
}


// descrybes what happens when two balls collide, react or bounce 
void collision(BALL* ball1, BALL* ball2, double rand)
{
	// possible chance's on reaction dependend on reaction type
	double chance[2] = {0, 0.35};
	
	// get reactiontype 
	int reaction_type = typereaction(ball1, ball2);
	
	// get if ther is enough energy
	bool React = Eact(ball1, ball2, reaction_type);

	if (React && rand < chance[reaction_type])
	{
		// make reaction happen
		react(ball1, ball2, reaction_type);
	}
	else
	{	// change ball velocities
		if ((ball1->vx < 0 && ball2->vx > 0) || (ball2->vx < 0 && ball1->vx > 0))
		{
			ball1->vx *= -1;
			ball2->vx *= -1;
		}
		else
		{
			double temp = ball1->vx;
			ball1->vx = ball2->vx;
			ball2->vx = temp;
		}
		if ((ball1->vy < 0 && ball2->vy > 0) || (ball2->vy < 0 && ball1->vy > 0))
		{
			ball1->vy *= -1;
			ball2->vy *= -1;
		}
		
		else
		{
			double temp = ball1->vy;
			ball1->vy = ball2->vy;
			ball2->vy = temp;
		}
	}	  
}

// check what type of reaction it is depend on the types of the balls
int typereaction(BALL* ball1, BALL* ball2)
{
	if (ball1->type == 'A' && ball2->type == 'A')
	{
		return 1;
	}

	return 0;
}

// Check if there is enough energy to react 
bool Eact(BALL* ball1, BALL* ball2, int i)
{
	double Eact[2] = {0, 1};
	double E1in = 0.5 * ball1->mass * (ball1->vx*ball1->vx + ball1->vy*ball1->vy);
	double E2in = 0.5 * ball2->mass * (ball2->vx*ball2->vx + ball2->vy*ball2->vy);
	double Eout = 0.5 * (ball1->mass + ball2->mass) * ((ball1->vx + ball2->vx) * (ball1->vx + ball2->vx) + (ball1->vy + ball2->vy) * (ball1->vy + ball2->vy));
	//printf("Ein1 : %f, Ein2: %f, Eout: %f\n", E1in, E2in, Eout);
	if (E1in + E2in - Eout > Eact[i])
	{
		return true;
	}
	return false;
}

// make reaction between to balls, dependend on the type of reaction
void react(BALL* ball1, BALL* ball2, int reaction_type)
{
	// reaction betwee A and A
	if (reaction_type == 1)
	{
		// make two B balls 
		int temp = ball1->mass;
		ball1->type = 'B';
		ball1->counter = 0;
		setColor(ball1->ball, "RED");
		ball1->mass = ball1->mass + ball2->mass;
		
		ball1->vx = (ball1->vx * temp + ball2->vx * ball2->mass) / ball1->mass; 
		ball1->vy = (ball1->vy * temp + ball2->vy * ball2->mass) / ball1->mass; 
		
		setColor(ball2->ball, "GREEN");
		setVisible(ball2->ball, false);
		ball2->exists = false;
	}
}
// check if the ball should decompose
bool decompose_time(int counter, char type, double rand)
{
	// get chance on decomposition 
	double chance = counter * rand;
	
	// decompose depend on type and chance
	if (type == 'B' && chance > 80) return true;
	
	// or not
	return false;
}
// decompose the ball back to lower molecule form
void decompose(BALL ballen[], int i)
{
	srand48(time(NULL));
	// decompose Ball B into A
	int j = getFreeSpot(ballen);
	double impuls_x_in = ballen[i].vx * ballen[i].mass;
	double impuls_y_in = ballen[i].vy * ballen[i].mass;
	
	ballen[i].type = 'A';
	ballen[j].type = 'A';
	ballen[i].mass = 1;
	ballen[i].vx = drand48() * T + 1;
	ballen[i].vy = drand48() * T + 1;
	ballen[j].mass = 1;
	ballen[j].vx = impuls_x_in - ballen[i].vx;
	ballen[j].vy = impuls_y_in - ballen[j].vy;
	
	setColor(ballen[i].ball, "BLUE");	
	setColor(ballen[j].ball, "BLUE");
	
	setVisible(ballen[j].ball, true);
	ballen[j].exists = true;
}

int getFreeSpot(BALL ballen[])
{
	for (int i = 0; i < BALLS; i++)
	{
		if (!ballen[i].exists)
		{
			return i;
		}
	}
	return 0;
}
BALL DeepCopyBall(BALL blueprint)
{
	BALL output;
	output.ball = newGOval(0, 0, 2 * RADIUS, 2 * RADIUS);
	setColor(output.ball, blueprint.color);
	setFilled(output.ball, true);
	output.mass = blueprint.mass;
	output.counter = blueprint.counter;
	output.type = blueprint.type;
	output.exists = blueprint.exists;
	return output;	
}
