////////////////////////////////////////////////////////////////////////////////
// Oersoep project
//
// Marten Folkertsma & Daniel Staal
// Script for model environment
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Libraries
////////////////////////////////////////////////////////////////////////////////

// standard libraries
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <cs50.h>
#include <math.h>

// Stanford Portable Library
#include "gevents.h"
#include "gobjects.h"
#include "gwindow.h"

////////////////////////////////////////////////////////////////////////////////
// Instance Variables
////////////////////////////////////////////////////////////////////////////////

// height and width of game's window in pixels
#define HEIGHT 600
#define WIDTH 1000

// radius of ball in pixels
#define RADIUS 5

// amount of balls
#define BALLS 100

// number of different particles
#define PARTICLES 10

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

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

typedef struct reaction
{
	bool react;
	char product;
	double Eact;
	double chance;	
}reaction;

typedef struct decomp
{
	bool possible;
	int time;
	char type1;
	char type2;
}decomp;

typedef struct node
{
	BALL* blueprint;
	reaction reactions[10];
	decomp decompinfo;
}node;

typedef struct datablok{

	int amount;
	struct datablok* next;
}datablok;

////////////////////////////////////////////////////////////////////////////////
// Methods declarations and instance variables
////////////////////////////////////////////////////////////////////////////////

void initBall(GWindow window, BALL ballen[], int T);
void collision(BALL* ball1, BALL* ball2, double rand);
bool Eact(BALL* ball1, BALL* ball2, int index, int index2); 
void react(BALL* ball1, BALL* ball2, int index, int index2);
bool decompose_time(int counter, int index, double random);
void decompose(BALL ball[], int i);
int getFreeSpot(BALL ballen[]);
void initDataStructure();
BALL DeepCopyBall(BALL input, BALL blueprint);
void UpdateDataArray(int data[]);
void PrintDataArray();

int hashfunction(char type)
{
	return type - 'A';
}
datablok DataArray[10];
GWindow window;
int T;
node hashtable[10];


////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////

int main(void)
{
	
	// keep track of number of particles
	int particles[PARTICLES];
	// initialize values to 0
	for(int i = 0; i<PARTICLES; i++){
		particles[i] = 0;
	}
	int iterationCounter = 0; 
	
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
    			int index = hashfunction(ballen[i].type);
    			
    			// count different kinds of particles present
    			particles[index]++;
    			// count iterations
    			
    			ballcount++;
				BALL* ball1 = &ballen[i];
				if (hashtable[index].decompinfo.possible)
				{
					ballen[i].counter++;
				}
				if (decompose_time(ballen[i].counter, index, drand48()) && hashtable[index].decompinfo.possible)
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
		ballcount = 0;
    	pause(10);
    	iterationCounter++;
    	if (iterationCounter == 100){
    		iterationCounter = 0;	
			for (int i = 0; i<PARTICLES;i++){
				printf("particles[%i] = %i\n", i, particles[i]);
			}
			updateDataArray(particles);
		}
		// reset particle number 
    	for (int j=0; j<PARTICLES; j++){
    		particles[j] = 0;
    	}
    }

	closeGWindow(window);
    return 0;
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
		ballen[i].ball = newGOval(0, 0, RADIUS * 2, RADIUS * 2);
		// make deepcopy from blueprint to balls 
		ballen[i] = DeepCopyBall(ballen[i], *hashtable[index].blueprint);
		
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
	int index = hashfunction(ball1->type);
	int index2 = hashfunction(ball2->type);
		
	// get if ther is enough energy
	if (hashtable[index].reactions[index2].react)
	{
		// check if there is enough energy
		bool EnergyOk = Eact(ball1, ball2, index, index2);
		
		// react if all conditions are met
		if (EnergyOk && rand < hashtable[index].reactions[index2].chance)
		{
			// make reaction happen
			react(ball1, ball2, index, index2);
			return;
		}
	}

	// change ball velocities
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

// Check if there is enough energy to react 
bool Eact(BALL* ball1, BALL* ball2, int index, int index2)
{
	double tempEact = hashtable[index].reactions[index2].Eact;
	double E1in = 0.5 * ball1->mass * (ball1->vx*ball1->vx + ball1->vy*ball1->vy);
	double E2in = 0.5 * ball2->mass * (ball2->vx*ball2->vx + ball2->vy*ball2->vy);
	double Eout = 0.5 * (ball1->mass + ball2->mass) * ((ball1->vx + ball2->vx) * (ball1->vx + ball2->vx) + (ball1->vy + ball2->vy) * (ball1->vy + ball2->vy));
	
	if (E1in + E2in - Eout > tempEact)
	{
		return true;
	}
	return false;
}

// make reaction between to balls, dependend on the type of reaction
void react(BALL* ball1, BALL* ball2, int index, int index2)
{
 
	int oldMass = ball1->mass;
	// getting index for constuction by looking up product type
	int NewIndex = hashfunction(hashtable[index].reactions[index2].product);
	*ball1 = DeepCopyBall(*ball1, *hashtable[NewIndex].blueprint);
		
	ball1->vx = (ball1->vx * oldMass + ball2->vx * ball2->mass) / ball1->mass; 
	ball1->vy = (ball1->vy * oldMass + ball2->vy * ball2->mass) / ball1->mass; 
	
	setVisible(ball2->ball, false);
	ball2->exists = false;
}
// check if the ball should decompose
bool decompose_time(int counter, int index, double rand)
{
	int time = hashtable[index].decompinfo.time;
	
	// get chance on decomposition 
	double chance = counter * rand;
	 
	// decompose depend on type and chance
	if (hashtable[index].decompinfo.possible && chance > time) return true;
	
	// or not
	return false;
}
// decompose the ball back to lower molecule form
void decompose(BALL ballen[], int i)
{
	// get index for look up types products
	int index = hashfunction(ballen[i].type);
	
	srand48(time(NULL));
	
	// get free spot to make ball and types of product balls
	int j = getFreeSpot(ballen);
/*	int ProductType1 = hashfunction(hashtable[index].decompinfo.type1); */
/*	int ProductType2 = hashfunction(hashtable[index].decompinfo.type1);*/
/*	*/
	double tempVx = ballen[i].vx;
	double tempVy = ballen[i].vy;
	
	int oldMass = ballen[i].mass;
	
	double impuls_x_in = ballen[i].vx * ballen[i].mass;
	double impuls_y_in = ballen[i].vy * ballen[i].mass;
	double Etot = 0.5 * oldMass * (tempVx*tempVx + tempVy*tempVy);
	
/*	ballen[i] = DeepCopyBall(ballen[i], *hashtable[ProductType1].blueprint);*/
/*	ballen[j] = DeepCopyBall(ballen[i], *hashtable[ProductType2].blueprint);*/
	
	ballen[i].type = 'A';
	ballen[j].type = 'A';
	ballen[i].mass = 1;
	ballen[i].vx = drand48() * tempVx;
	ballen[i].vy = sqrt((Etot/ballen[i].mass) - ballen[i].vx * ballen[i].vx);
	
	// TODO uit de datastructure halen
	ballen[j].mass = 1;
	ballen[j].vx = (tempVx*oldMass - ballen[i].vx*ballen[i].mass)/ballen[j].mass + 1;
	ballen[j].vy = (tempVy*oldMass - ballen[i].vy*ballen[i].mass)/ballen[j].mass;
	
	setColor(ballen[i].ball, "BLUE");	
	setColor(ballen[j].ball, "BLUE");
	
	double balliX = getX(ballen[i]);
	double balliY = getY(ballen[i]);
	
	setLocation(ballen[j].ball, balliX + 2.3*RADIUS, balliY);
	
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
BALL DeepCopyBall(BALL input, BALL blueprint)
{
	setColor(input.ball, blueprint.color);
	setFilled(input.ball, true);
	input.mass = blueprint.mass;
	input.counter = blueprint.counter;
	input.type = blueprint.type;
	input.exists = blueprint.exists;
	return input;	
}

////////////////////////////////////////////////////////////////////////////////
// Datastructure for information print to and read out
///////////////////////////////////////////////////////////////////////////////
void UpdateDataArray(int data[])
{
	for( int i = 0; i < 10; i++)
	{
		datablok* new_blok = malloc(sizeof(datablok));
		new_blok->next = DataArray[i].next;
		new_blok->amount = data[i];
		DataArray[i].next = new_blok;
	}
}
void PrintDataArray()
{
	for( int i = 0; i < 0; i++)
	{
		
	}
}

////////////////////////////////////////////////////////////////////////////////
// Datastructure and Database
////////////////////////////////////////////////////////////////////////////////

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
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			if (i == 0 && j != 0)
			{
				hashtable[i].reactions[j].react = true;
				hashtable[i].reactions[j].Eact = 1;
				hashtable[i].reactions[j].product = i + j + 'A' + 1;
				hashtable[i].reactions[j].chance = 0.35;
			}
			else if (j == 0)
			{
				hashtable[i].reactions[j].react = true;
				hashtable[i].reactions[j].Eact = 1;
				hashtable[i].reactions[j].product = i + j + 'A' + 1;
				hashtable[i].reactions[j].chance = 0.35;
			}
			else
			{
				hashtable[i].reactions[j].react = false;
			}
		}
	}
	hashtable[0].decompinfo.possible = false;
	hashtable[1].decompinfo.possible = true;
	hashtable[1].decompinfo.time = 80;
	hashtable[1].decompinfo.type1 = 'A';
	hashtable[1].decompinfo.type2 = 'A';
}
