/*
 * This code uses the termcolor library by Ihor Kalnytskyi.
 * Copyright (c) 2013, Ihor Kalnytskyi. All rights reserved.
 *
 * Redistribution and use in source and binary forms of the software as well
 * as documentation, with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following
 *   disclaimer in the documentation and/or other materials provided
 *   with the distribution.
 *
 * - The names of the contributors may not be used to endorse or
 *   promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE AND DOCUMENTATION IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE AND DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/* 
Name: Ruze 
Date: 11/7/22
Description: This program will take user inputs that will specify the dimensions of a 2D array, the drop zone, and
the number of particles and particle life. This will be used to generate a raw particle map which then be normalized to 255
and then polished into an island with color.
Usage: <exe> [-s seed]
*/   

#include <iostream>
#include <fstream>
#include "termcolor.hpp"
#include <stdlib.h>
#include <stdio.h>
#include <iomanip>
#include <time.h>
#include <math.h>
#include <string.h>
using std::cout;
using std::endl;
using std::cin;
using std::setw;
using std::ofstream;
using namespace termcolor; 

float frand();
int** makeParticleMap(int** map, int width, int height, int windowX, int windowY, int radius, int numParticles, int maxLife, ofstream& outFile);
bool moveExists(int** map, int width, int height, int x, int y, int newX, int newY);
int findMax(int** map, int width, int height);
int** normalizeMap(int** norMap, int width, int height, ofstream& outFile);
void generateIsland(int** map, int width, int height, int waterLine, ofstream& outFile);
void printGrid(int** map, int width, int height, ofstream& outFile);

int main(int argc, char** argv)
{
    //Command line argument checks and seeding srand
    if(argc == 1)
        srand(time(0)); //srand is seeded with time(0) if [-s integer] is not selected
    else if (argc == 3 && strcmp(argv[1], "-s") == 0) 
        srand(atoi(argv[2])); //Seeds a number if [-s integer] is selected
    else
    {
        printf("Error -- Usage: <exe> [-s seed]\n"); //Anything that doesn't follow the format of the Usage will error
        return 0;
    }

    int width, height, xCor, yCor, zoneRadius, particleNum, particleLife, waterLine;

    printf("Welcome to Mathm Alkaabi's CSE240 Island Generator!\n");
    
    //Collects the width and checks that it's above 0 and is a number
    printf("Enter grid width: ");
    cin >> width;
    while(cin.fail() || width <= 0)
    {
        printf("Error -- Please input a positive integer number.\n");
        printf("Enter grid width: ");
        cin.clear();
        cin.ignore(256,'\n');
        cin >> width; 
    }

    //Collects the height and checks that it's above 0 and is a number
    printf("Enter grid height: ");
    cin >> height;
    while(cin.fail() || height <= 0)
    {
        printf("Error -- Please input a positive integer number.\n");
        printf("Enter grid height: ");
        cin.clear();
        cin.ignore(256,'\n');
        cin >> height;
    }

    //Collects the x-coordinate for drop zone and checks that its between 0 and the width and is a number
    printf("Enter drop-zone x-coordinate (0 - %d): ", width);
    cin >> xCor;
    while(cin.fail() || xCor < 0 || xCor > width)
    {
         if (cin.fail() || xCor < 0 || xCor > width)
            printf("Error -- Entered a value less than 0 or greater than the grid width, please re-input.\n");
        printf("Enter drop-zone x-coordinate (0 - %d): ", width);
        cin.clear();
        cin.ignore(256,'\n');
        cin >> xCor;
    } 

    //Collects the y-coordinate for drop zone and checks that its between 0 and the height and is a number
    printf("Enter drop-zone y-coordinate (0 - %d): ", height);
    cin >> yCor;
    while(cin.fail() || yCor < 0 || yCor > height)
    {
         if (cin.fail() || yCor < 0 || yCor > height)
            printf("Error -- Entered a value less than 0 or greater than the grid height, please re-input.\n");
        printf("Enter drop-zone y-coordinate (0 - %d): ", height);
        cin.clear();
        cin.ignore(256,'\n');
        cin >> yCor;
    } 

    //Collect the radius for the drop zone and checks that it's above 2 and is a number
    //The inputted radius can only be as big as the width or height of the 2D grid (whichever is smaller)
    //This is to limit the amount of times the initial particle drops outside of bounds
    //There is also a special case that if width or height is 1 the only valid radius is 2 so it's set automaically
    if(width < 2 || height < 2)
    {
        zoneRadius = 2;
        printf("Since the width or height of the 2D grid is less than 2, the radius is automatically set at 2.\n");
    }
    else
    {
        printf("Enter drop-zone radius (minimum 2): ");
        cin >> zoneRadius;
        while(cin.fail() || zoneRadius < 2 || (zoneRadius > width) || (zoneRadius > height))
        {
            if(zoneRadius < 2)
                printf("Error -- Entered a value less than 2 for radius, please re-input.\n");
            else if(zoneRadius > width || zoneRadius > height)
                printf("Error -- Radius entered is out of bounds of the 2D grid (must not be greater than the width or height of the 2D grid), please re-input.\n");
            printf("Enter drop-zone radius (minimum 2): ");
            cin.clear();
            cin.ignore(256,'\n');
            cin >> zoneRadius;
        }
    }
    
    //Collects number of particle and checks if the input is a number
    printf("Enter number of particles to drop: ");
    cin >> particleNum;
    while(cin.fail() || particleNum < 0)
    {
        if(cin.fail() || particleNum < 0)
            printf("Error -- Number of particles inputted is invalid, please re-input.\n");
        printf("Enter number of particles to drop: ");
        cin.clear();
        cin.ignore(256,'\n');
        cin >> particleNum;
    }

    //Collects the max life of particles and checks if the input is a number
    printf("Enter max life of particles: ");
    cin >> particleLife;
    while(cin.fail() || particleLife < 0)
    {
        if(cin.fail() || particleLife < 0)
            printf("Error -- Max life inputted is invalid, please re-input.\n");
        printf("Enter max life of particles: ");
        cin.clear();
        cin.ignore(256,'\n');
        cin >> particleLife;
    }

    //Collects the waterline and checks that the input is between 40 and 200 and is a number
    printf("Enter value for waterline (40-200): ");
    cin >> waterLine;
    while (cin.fail() || waterLine < 40 || waterLine > 200)
    {
        if(cin.fail() || waterLine < 40 || waterLine > 200)
           printf("Error -- Entered a value less than 40 or greater than 200 for waterline, please re-input.\n");
        printf("Enter value for waterline (40-200): ");
        cin.clear();
        cin.ignore(256,'\n');
        cin >> waterLine;
    } 

    //Create the initial 2D int array and fill it with 0s
    int** map;
    map = new int*[height];
    for(int row = 0; row < height; row++)
    {
        map[row] = new int[width];
    }
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            map[row][col] = 0;
        }
    }

    //Open a file called island.txt to output the maps to and create the Raw Grid, the Normalized Grid and generate the Polished Island
    ofstream outFile("island.txt");
    int** particleMap = makeParticleMap(map, width, height, xCor, yCor, zoneRadius, particleNum, particleLife, outFile);
    int** normalizedMap = normalizeMap(particleMap, width, height, outFile);
    generateIsland(normalizedMap, width, height, waterLine, outFile);
    
    //Close the output file and delete the 2D array
    outFile.close();
    for(int row = 0; row < height; row++)
    {
      delete[] map[row];
    }
    delete[] map;

    return 0;
}

//Method makeParticleMap will preform the particle roll algorithm and create a raw grid containing the raw numbers
int** makeParticleMap(int** map, int width, int height, int windowX, int windowY, int radius, int numParticles, int maxLife, ofstream& outFile)
{
   
    double r, theta;
    int x, y;
    const double PI = 3.1415926535897;

    //Will loop until all particles have been dropped
    while(numParticles != 0)
    { 
        //Will loop over and over again if x or y is out of bounds of the 2D array until the coordinate is inside the bounds
        do
        {
            r = radius * sqrt(frand());
            theta = frand() * 2 * PI;
            x = (int) (windowX + r * cos(theta));
            y = (int) (windowY + r * sin(theta));
        } while (x >= width || x < 0 || y >= height || y < 0);

        map[y][x]++; //Increment the initial particle dropped

        //Loop through a particle's life until it dies
        for(int i = maxLife; i > 0; i--)
        {
            //boolean flags for checking valid directions
            bool valid = false;
            bool invalidN = false;
            bool invalidNE = false;
            bool invalidE = false;
            bool invalidSE = false;
            bool invalidS = false;
            bool invalidSW = false;
            bool invalidW = false;
            bool invalidNW = false;
            bool zeroValid = false;
            float randomNum;

            //Loop that will randomly look into the Moore's neighborhood of a coordinate until it finds a direction
            //that is valid or kills the particle if there are no valid moves
            //The loop will only break when a valid direction is found and bool valid is set true or 
            //all directions are invalid with their respective bool flags being set true
            while(!valid && !zeroValid)
            {
                //Creates a random number between 1-8 with each number being associated to a direction
                randomNum = (rand() % 8) + 1; 
                if(randomNum == 1)
                {
                    if(moveExists(map, width, height, x, y, x , y - 1)) //Moves north
                    {
                        map[y-1][x]++;
                        y--;
                        valid = true; 
                    }
                    else    
                        invalidN = true; 
                }   
                else if(randomNum == 2)
                {
                    if(moveExists(map, width, height, x, y, x + 1 , y - 1)) //Moves north east
                    {
                        map[y-1][x+1]++;
                        y--;
                        x++;
                        valid = true;
                    }
                    else
                        invalidNE = true;
                }
                else if(randomNum == 3)
                {
                    if(moveExists(map, width, height, x, y, x + 1 , y)) //Moves east
                    {
                        map[y][x+1]++;
                        x++;
                        valid = true;
                    }
                    else
                        invalidE = true;
                }
                else if(randomNum == 4)
                {
                    if(moveExists(map, width, height, x, y, x + 1, y + 1)) //Moves south east
                    {
                        map[y+1][x+1]++;
                        y++;
                        x++;
                        valid = true;
                    }
                    else
                        invalidSE = true;
                }
                else if(randomNum == 5)
                {
                    if(moveExists(map, width, height, x, y, x, y + 1)) //Moves south
                    {  
                        map[y+1][x]++;
                        y++;
                        valid = true;
                    }
                    else
                        invalidS = true;
                }
                else if(randomNum == 6)
                {
                    if(moveExists(map, width, height, x, y, x - 1, y + 1)) //Moves south west
                    {
                        map[y+1][x-1]++;
                        y++;
                        x--;
                        valid = true;
                    }
                    else
                        invalidSW = true;
                }
                else if(randomNum == 7)
                {
                    if(moveExists(map, width, height, x, y, x - 1, y)) //Moves west
                    {
                        map[y][x-1]++;
                        x--;
                        valid = true;
                    }
                    else
                        invalidW = true;
                }
                else if(randomNum == 8)
                {
                    if(moveExists(map, width, height, x, y, x - 1, y - 1)) //Moves north west
                    {
                        map[y-1][x-1]++;
                        y--;
                        x--;
                        valid = true;
                    }
                    else
                        invalidNW = true;
                }
                zeroValid = invalidN && invalidNE && invalidE && invalidSE && invalidS && invalidSW && invalidW && invalidNW;
                if(zeroValid) //If there are no valid directions to move to i will be set to 0 which kills the particle
                    i = 0;
            } //end of valid loop
        } // end of maxLife loop
        numParticles--;
    } //end of numParticles loop

    //Print the raw grid to the console and outFile
    printf("\nRaw Grid:\n");
    outFile << "Raw Grid:" << endl;    
    printGrid(map, width, height, outFile);
    return map;
} //End of makeParticleMap method

//Method normalizeMap will use the largest number and normalize all elements in the 2D int array to 255
int** normalizeMap(int** norMap, int width, int height, ofstream& outFile)
{
    int maxVal = findMax(norMap, width, height);
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            norMap[row][col] = ((double) norMap[row][col] / maxVal) * 255; //This will normalize a coordinate to 255
        }
    }

    //Print the normalized grid to the console and outFile
    printf("Normalized Grid:\n");
    outFile << "Normalized Grid:" << endl;
    printGrid(norMap, width, height, outFile);
    return norMap;
} //End of normalizeMap method

//Method findMax will search and find the largest number in a 2D int array
int findMax(int** map, int width, int height)
{
    int largest = map[0][0];
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            if(map[row][col] > largest)
                largest = map[row][col];
        }
    }
    return largest;
} //End of findMax method

//Method moveExists will check if a valid move exists on a particular coordinate
bool moveExists(int** map, int width, int height, int x, int y, int newX, int newY)
{
    if((x >= 0 && x < width) && (y >= 0 && y < height) && (newX >= 0 && newX < width) && (newY >= 0 && newY < height) ) //Checks bounds
        if(map[newY][newX] <= map[y][x]) //Checks if the direction is smaller or equal to the current point
            return true;

    return false;
} //End of moveExists method

void generateIsland(int** map, int width, int height, int waterLine, ofstream& outFile)
{
    int landZone = 255 - waterLine;

    //Dynmatically create a 2D char array for the island generation
    char** island;
    island = new char*[height];
    for(int row = 0; row < height; row++)
    {
        island[row] = new char[width];
    }

    //2D Char array with it's elements assigned accordingly
    for(int row = 0; row < height; row++) 
    {
        for(int col = 0; col < width; col++)
        {
            if(map[row][col] < (0.5 * waterLine))
                island[row][col] = '#';
            else if(map[row][col] >= (0.5 * waterLine) && map[row][col] <= waterLine)
                island[row][col] = '~';
            else if(map[row][col] > waterLine && map[row][col] < (waterLine + (0.15 * landZone)))
                island[row][col] = '.';
            else if(map[row][col] > waterLine && map[row][col] >= (waterLine + (0.15 * landZone)) && map[row][col] < (waterLine + (0.4 * landZone)))
                island[row][col] = '-';
            else if(map[row][col] > waterLine && map[row][col] >= (waterLine + (0.4 * landZone)) && map[row][col] < (waterLine + (0.8 * landZone)))
                island[row][col] = '*';
            else
                island[row][col] = '^';
        }
    }

    //Print and color the 2D char array to console and outFile
    printf("Polished Island:\n");
    outFile << "Polished Island:" << endl;
    for(int row = 0; row < height; row++) 
    {
        for(int col = 0; col < width; col++)
        {
            if(island[row][col] == '#')
            {
                cout  << on_blue << blue << '#' << reset; //Deep Water
                outFile  << '#' << reset; 
            }
            else if(island[row][col] == '~')
            {
                cout << on_cyan << cyan << '~' << reset; //Shallow Water
                outFile << '~' << reset; 
            }
            else if(island[row][col] == '.')
            {
                cout << on_yellow << white << '.' << reset; //Coast/Beach
                outFile << '.' << reset; 
            }
            else if(island[row][col] == '-')
            {
                cout << on_bright_green << green << '-' << reset; //Plains/Grass
                outFile << '-' << reset;
            }    
            else if(island[row][col] == '*')
            {
                cout <<  on_green << green << '*' << reset; //Forests
                outFile << '*' << reset; 
            }    
            else if(island[row][col] == '^')
            {
                cout << on_bright_grey << white << '^' << reset; //Mountains
                outFile << '^' << reset;
            }
        }
        cout << endl;
        outFile << endl;
    }

    //Delete 2D char array
    for(int row = 0; row < height; row++) 
    {
      delete[] island[row];
    }
    delete[] island;

} //End of generateIsland method

//Method printGrid will print out any 2D int arrays (Used for raw grid and normalized grid)
void printGrid(int** map, int width, int height, ofstream& outFile)
{
    for(int row = 0; row < height; row++)
    {
        for(int col = 0; col < width; col++)
        {
            cout << setw(3) << map[row][col] << " ";
            outFile << setw(3) << map[row][col] << " ";
        }
        cout << endl;
        outFile << endl;
    }
    cout << endl;
    outFile << endl;
} //End of printGrid method

//Generate a random number between 0-1
float frand()
{
    return (float) (rand() % RAND_MAX) / (float) RAND_MAX;
} //End of frand method
