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

// build a datastructure to keep itteration count and reaction in 
typedef struct datablok{
	long moment;
	struct datablok* next;
}datablok;

////////////////////////////////////////////////////////////////////////////////
// Methods declarations and instance variables
////////////////////////////////////////////////////////////////////////////////

// methods for running program
void initBall(GWindow window, BALL ballen[]);
void collision(BALL* ball1, BALL* ball2, double rand);
bool Eact(BALL* ball1, BALL* ball2, int index, int index2); 
void react(BALL* ball1, BALL* ball2, int index, int index2);
bool decompose_time(int counter, int index, double random);
void decompose(BALL ball[], int i, double rand);

// setting up database for easy read data
void initDataStructure();
BALL DeepCopyBall(BALL input, BALL blueprint);
void changeDecomp(int index, bool pos, int time, char type1, char type2);
void changeReaction(bool react, double Eact, int index1, int index2, char type, double chance);

// setting up data to read from program
void updateDataArray(int i);
void printDataArray();
void saveData();

int hashfunction(char type)
{
	return type - 'A';
}
datablok* DataArray[2];
GWindow window;
int T;
node hashtable[10];
long iterationCounter;
char test[40];
////////////////////////////////////////////////////////////////////////////////
// Functions
///////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	sprintf(test, "/home/jharvard/Dropbox/Testdata Thema 3/%s.csv", argv[1]);
	// keep track of number of particles
	int particles[PARTICLES];
	// initialize values to 0
	for(int i = 0; i<PARTICLES; i++){
		particles[i] = 0;
	}
	
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
    initBall(window, ballen);
	
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

    			// count iterations
				BALL* ball1 = &ballen[i];
				if (hashtable[index].decompinfo.possible)
				{
					ballen[i].counter++;
				}
				if (decompose_time(ballen[i].counter, index, drand48()) && hashtable[index].decompinfo.possible)
				{
					decompose(ballen, i, drand48());
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
    	pause(1);
    	iterationCounter++;
    	
    	GEvent MouseClick = getNextEvent(MOUSE_CLICKED);
        if (MouseClick != NULL)
        {
        	if (getEventType(MouseClick) == MOUSE_CLICKED)
        	{
        		saveData();
        		printDataArray();
        	}	
        }
        
    	if (iterationCounter%1000 == 0)
    	{
    		printDataArray();
    		saveData();
    		if (iterationCounter >= 20000){
    			break;
    		}
    	}
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
	
	//setVisible(ball2->ball, false);
	//ball2->exists = false;
	*ball2 = DeepCopyBall(*ball2, *hashtable[0].blueprint);
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
	ball2->vx = drand48() * T + 1;
	if(drand48() <= 0.5)
	{
		ball2->vx = -ball2->vx;
	}
	ball2->vy = drand48() * T + 1;
	if(drand48() <= 0.5)
	{
		ball2->vy = -ball2->vy;
	}
	addAt(window, ball2->ball, ballx, bally);
	if (ball1->type == 'E')
	{
		updateDataArray(0); 
	}
}
// check if the ball should decompose
bool decompose_time(int counter, int index, double rand)
{
	int time = hashtable[index].decompinfo.time;
	
	// get chance on decomposition 
	double chance = T/2.0 * counter * rand;
	 
	// decompose depend on type and chance
	if (hashtable[index].decompinfo.possible && chance > time) return true;
	
	// or not
	return false;
}
// decompose the ball back to lower molecule form
void decompose(BALL ballen[], int i, double rand)
{
	if (ballen[i].type == 'J') updateDataArray(1);
	// get index for look up types products
	int index = hashfunction(ballen[i].type);
	
	srand48(time(NULL));
	
	// get free spot to make ball and types of product balls
	int j = rand * BALLS;
	int ProductType1 = hashfunction(hashtable[index].decompinfo.type1); 
	int ProductType2 = hashfunction(hashtable[index].decompinfo.type2);

	double oldVx = ballen[i].vx;
	double oldVy = ballen[i].vy;
	int oldMass = ballen[i].mass;
	double Etot = 0.5 * oldMass * (oldVx*oldVx + oldVy*oldVy);
	
	ballen[i] = DeepCopyBall(ballen[i], *hashtable[ProductType1].blueprint);
	ballen[j] = DeepCopyBall(ballen[j], *hashtable[ProductType2].blueprint);
	
	ballen[i].vx = drand48() * oldVx;
	ballen[i].vy = sqrt((Etot/ballen[i].mass) - ballen[i].vx * ballen[i].vx);
	
	ballen[j].vx = (oldVx*oldMass - ballen[i].vx*ballen[i].mass)/ballen[j].mass;
	ballen[j].vy = (oldVy*oldMass - ballen[i].vy*ballen[i].mass)/ballen[j].mass;
	
	double balliX = getX(ballen[i]);
	double balliY = getY(ballen[i]);
	
	setLocation(ballen[j].ball, balliX + 2.3*RADIUS, balliY);
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
////////////////////////////////////////////////////////////////////////////////

void updateDataArray(int i)
{
	datablok* new_block = malloc(sizeof(datablok));
	new_block->next = DataArray[i];
	new_block->moment = iterationCounter;
	DataArray[i] = new_block;
	
}
void printDataArray()
{
	for(int i = 0; i < 2; i++)
	{
		datablok* temp = DataArray[i];
		while(temp != NULL)
		{
			printf(" %ld ", temp->moment);
			temp = temp->next;
		}
		printf("\n");
	}
}
void saveData()
{
	FILE* outfile = fopen(test, "w");
	for (int i = 0; i < 2; i++)
	{
		datablok* temp = DataArray[i];
		while(temp != NULL)
		{
			fprintf(outfile, "%ld,", temp->moment);
			temp = temp->next;
		}
		fprintf(outfile, "\n");
	}
	fclose(outfile);
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
	int masses[PARTICLES] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	char* colors[PARTICLES] = {"BLUE", "RED", "CYAN", "ORANGE", "BLACK", "LIGHT_GRAY", "LIGHT_GRAY", "LIGHT_GRAY", "LIGHT_GRAY", "PINK"};
	
	for(int i = 0; i < PARTICLES; i++)
	{
		BALL* new_blueprint = malloc(sizeof(BALL));
		new_blueprint->color = colors[i];
		new_blueprint->mass = masses[i];
		new_blueprint->type = 'A' + i;
		new_blueprint->exists = true;
		hashtable[i].blueprint = new_blueprint;
	}
	
	for(int i = 0; i < PARTICLES; i++)
	{
		for(int j = 0; j < PARTICLES; j++)
		{
			hashtable[i].reactions[j].react = false;
		}
	}
	
	////////////////
	// Reactions
		
/*changeReaction(bool react, double Eact, int index1, int index2, char type, double chance)*/

	changeReaction(true, 1.5, 0, 0, 'B', 0.40);
	changeReaction(true, 1.6, 0, 1, 'C', 0.40);	
	changeReaction(true, 1.7, 0, 2, 'D', 0.40);		
	changeReaction(true, 1.8, 0, 3, 'E', 0.40);
	//template ^	
	changeReaction(true, 1, 0, 4, 'F', 0.70);	
	changeReaction(true, 0.9, 0, 5, 'G', 0.65);	
	changeReaction(true, 0.8, 0, 6, 'H', 0.60);	
	changeReaction(true, 0.7, 0, 7, 'I', 0.55);
	changeReaction(true, 0.6, 0, 8, 'J', 0.50);		
	
	/////////////////
	// Decompositions
	
/*changeDecomp(int index, bool pos, int time, char type1, char type2)*/
	hashtable[0].decompinfo.possible = false;
	
	changeDecomp(1, true, 500, 'A', 'A');
	changeDecomp(2, true, 600, 'A', 'B');
	changeDecomp(3, true, 700, 'A', 'C');
	changeDecomp(4, true, 1000, 'A', 'D');
	//template ^
	changeDecomp(5, true, 100, 'A', 'E');
	changeDecomp(6, true, 200, 'A', 'F');
	changeDecomp(7, true, 300, 'A', 'G');
	changeDecomp(8, true, 400, 'A', 'H'); 
	changeDecomp(9, true, 20, 'E', 'E'); 	
}

void changeReaction(bool react, double Eact, int index1, int index2, char type, double chance){
	hashtable[index1].reactions[index2].react = react;
	hashtable[index1].reactions[index2].Eact = Eact;
	hashtable[index1].reactions[index2].product = type;
	hashtable[index1].reactions[index2].chance = chance;	
}

void changeDecomp(int index, bool pos, int time, char type1, char type2){
	hashtable[index].decompinfo.possible = pos;
	hashtable[index].decompinfo.time = time;
	hashtable[index].decompinfo.type1 = type1;
	hashtable[index].decompinfo.type2 = type2; 	
}
