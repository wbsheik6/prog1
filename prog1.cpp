//============================================================================
// Name        : prog1.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <utility>
#include <limits>
#include <cstring>
#include <algorithm>
//#include "neighbor.h"

using namespace std;

#define INF std::numeric_limits<int>::max()




Display *display_ptr;
Screen *screen_ptr;
int screen_num;
char *display_name = NULL;
unsigned int display_width, display_height;

Window win;
int border_width;
unsigned int win_width, win_height;
int win_x, win_y;

XWMHints *wm_hints;
XClassHint *class_hints;
XSizeHints *size_hints;
XTextProperty win_name, icon_name;
char *win_name_string = "Example Window";
char *icon_name_string = "Icon for Example Window";

XEvent report;

GC gc, gc_yellow, gc_red, gc_grey;
unsigned long valuemask = 0;
XGCValues gc_values, gc_yellow_values, gc_red_values, gc_grey_values;
Colormap color_map;
XColor tmp_color1, tmp_color2;

struct pt{
        int x,y;
    };

class objects {
    public:
    std::vector <pt> A, B, C;

    void setObj (pt X, pt Y, pt Z){

        this->A.push_back(X);
        this->B.push_back(Y);
        this->C.push_back(Z);

    }

    void eraseObj (objects &O, int index){
        this->A.erase(A.begin()+index);
        this->B.erase(B.begin()+index);
        this->C.erase(C.begin()+index);


    }
    int showObj (objects T, int index){

        return (this->A[index].x, this->A[index].y, this->B[index].x, this->B[index].y, this->C[index].x, this->C[index].y);

    }
};

class vertex {

public:

    pt orig;
    std::vector <pt> neigh;         //vector of adjacent points


    vertex (pt o){
        this->orig=o;
    }

};

void readline (int &, int [], int []);
void scale (int [], int [],int ,unsigned int, int);
void makeNeighbors (std::vector <vertex>&, objects, int);
void intersectTest (std::vector <vertex>&, objects, int, int);
int orientation (pt, pt, pt);
bool onSegment (pt, pt, pt);
double distance (int, int,int,int);
bool comparePT(pt, pt);

int main(int argc, char **argv) {

    /* opening display: basic connection to X Server */
      if( (display_ptr = XOpenDisplay(display_name)) == NULL )
        { printf("Could not open display. \n"); exit(-1);}
      printf("Connected to X server  %s\n", XDisplayName(display_name) );
      screen_num = DefaultScreen( display_ptr );
      screen_ptr = DefaultScreenOfDisplay( display_ptr );
      color_map  = XDefaultColormap( display_ptr, screen_num );
      display_width  = DisplayWidth( display_ptr, screen_num );
      display_height = DisplayHeight( display_ptr, screen_num );

      printf("Width %d, Height %d, Screen Number %d\n",
               display_width, display_height, screen_num);
      /* creating the window */
      border_width = 10;
      win_x = 0; win_y = 0;
      win_width = display_width/2;
      win_height = (int) (win_width / 1.7); /*rectangular window*/

      win= XCreateSimpleWindow( display_ptr, RootWindow( display_ptr, screen_num),
                                win_x, win_y, win_width, win_height, border_width,
                                BlackPixel(display_ptr, screen_num),
                                WhitePixel(display_ptr, screen_num) );
      /* now try to put it on screen, this needs cooperation of window manager */
      size_hints = XAllocSizeHints();
      wm_hints = XAllocWMHints();
      class_hints = XAllocClassHint();
      if( size_hints == NULL || wm_hints == NULL || class_hints == NULL )
        { printf("Error allocating memory for hints. \n"); exit(-1);}

      size_hints -> flags = PPosition | PSize | PMinSize  ;
      size_hints -> min_width = 60;
      size_hints -> min_height = 60;

      XStringListToTextProperty( &win_name_string,1,&win_name);
      XStringListToTextProperty( &icon_name_string,1,&icon_name);

      wm_hints -> flags = StateHint | InputHint ;
      wm_hints -> initial_state = NormalState;
      wm_hints -> input = False;

      class_hints -> res_name = "x_use_example";
      class_hints -> res_class = "examples";

      XSetWMProperties( display_ptr, win, &win_name, &icon_name, argv, argc,
                        size_hints, wm_hints, class_hints );

      /* what events do we want to receive */
      XSelectInput( display_ptr, win,
                ExposureMask | StructureNotifyMask | ButtonPressMask );

      /* finally: put window on screen */
      XMapWindow( display_ptr, win );

      XFlush(display_ptr);

      /* create graphics context, so that we may draw in this window */
      gc = XCreateGC( display_ptr, win, valuemask, &gc_values);
      XSetForeground( display_ptr, gc, BlackPixel( display_ptr, screen_num ) );
      XSetLineAttributes( display_ptr, gc, 4, LineSolid, CapRound, JoinRound);
      /* and three other graphics contexts, to draw in yellow and red and grey*/
      gc_yellow = XCreateGC( display_ptr, win, valuemask, &gc_yellow_values);
      XSetLineAttributes(display_ptr, gc_yellow, 6, LineSolid,CapRound, JoinRound);
      if( XAllocNamedColor( display_ptr, color_map, "yellow",
                &tmp_color1, &tmp_color2 ) == 0 )
        {printf("failed to get color yellow\n"); exit(-1);}
      else
        XSetForeground( display_ptr, gc_yellow, tmp_color1.pixel );
      gc_red = XCreateGC( display_ptr, win, valuemask, &gc_red_values);
      XSetLineAttributes( display_ptr, gc_red, 6, LineSolid, CapRound, JoinRound);
      if( XAllocNamedColor( display_ptr, color_map, "red",
                &tmp_color1, &tmp_color2 ) == 0 )
        {printf("failed to get color red\n"); exit(-1);}
      else
        XSetForeground( display_ptr, gc_red, tmp_color1.pixel );
      gc_grey = XCreateGC( display_ptr, win, valuemask, &gc_grey_values);
      if( XAllocNamedColor( display_ptr, color_map, "light grey",
                &tmp_color1, &tmp_color2 ) == 0 )
        {printf("failed to get color grey\n"); exit(-1);}
      else
        XSetForeground( display_ptr, gc_grey, tmp_color1.pixel );

    int obstaclesX[3000]={0};
    int obstaclesY[3000]={0};
    int scaledX[3000]= {0};
    int scaledY[3000]= {0};
    int counter, i,j,k=0;
    printf ("Adding values...\n");


    //create points track number of obstacles
    readline(counter, obstaclesX, obstaclesY);

    //find max in both x and y

    int maxX=0;
    int maxY=0;

    for (int x=0; x<counter; x++){
        if (maxX < obstaclesX[x])
            maxX = obstaclesX[x];

        if (maxY < obstaclesY[x])
            maxY = obstaclesY[x];
    }

    double border = 1.1;


    pt o;
    std::vector <vertex> V;
    objects O;


    j=1; //set for internal counter to make obstacles
    for (i=0; i<counter; i++){
        o.x=obstaclesX[i];
        o.y=obstaclesY[i];
        vertex V2(o);
        V.push_back(V2);
        printf("added (%d,%d) to V...\n", V[i].orig.x, V[i].orig.y);
        if (j == 3){
            O.setObj(V[i-2].orig, V[i-1].orig, V[i].orig);
            printf("Object # %d has points (%d,%d) , (%d,%d) , (%d,%d) \n", (i+1)/3, V[i-2].orig, V[i-1].orig,V[i].orig);
            j=0;
        }
        j++;
    }

    makeNeighbors(V, O, counter);

    /* and now it starts: the event loop */
      while(1)
        { XNextEvent( display_ptr, &report );
          switch( report.type )
        {
        case Expose:


        //reset points after each re draw

        // for loop to connect every three points as given by obstaclesX, obstaclesY
        if (maxY>win_height && maxX>win_width){
            scale (scaledX, obstaclesX, maxX, win_height, counter);
            scale (scaledY, obstaclesY, maxY, win_height, counter);
            for (int i=0; i<counter; i+=3){
                XDrawLine(display_ptr, win, gc, scaledX[i], scaledY[i], scaledX[i+1], scaledY[i+1]);
                XDrawLine(display_ptr, win, gc, scaledX[i+1], scaledY[i+1], scaledX[i+2], scaledY[i+2]);
                XDrawLine(display_ptr, win, gc, scaledX[i+2], scaledY[i+2], scaledX[i], scaledY[i]);
            }
        }
        else if (maxX>win_width){
            scale (scaledX, obstaclesX, maxX, win_width, counter);
            for (int i=0; i<counter; i+=3){
                XDrawLine(display_ptr, win, gc, scaledX[i], obstaclesY[i], scaledX[i+1], obstaclesY[i+1]);
                XDrawLine(display_ptr, win, gc, scaledX[i+1], obstaclesY[i+1], scaledX[i+2], obstaclesY[i+2]);
                XDrawLine(display_ptr, win, gc, scaledX[i+2], obstaclesY[i+2], scaledX[i], obstaclesY[i]);
            }
        }
        else if (maxY>win_height){
            scale (scaledY, obstaclesY, maxY, win_height, counter);
            for (int i=0; i<counter; i+=3){
                XDrawLine(display_ptr, win, gc, obstaclesX[i], scaledY[i], obstaclesX[i+1], scaledY[i+1]);
                XDrawLine(display_ptr, win, gc, obstaclesX[i+1], scaledY[i+1], obstaclesX[i+2], scaledY[i+2]);
                XDrawLine(display_ptr, win, gc, obstaclesX[i+2], scaledY[i+2], obstaclesX[i], scaledY[i]);
            }
        }
        else {
            for (int i=0; i<counter; i+=3){
                XDrawLine(display_ptr, win, gc, obstaclesX[i], obstaclesY[i], obstaclesX[i+1], obstaclesY[i+1]);
                XDrawLine(display_ptr, win, gc, obstaclesX[i+1], obstaclesY[i+1], obstaclesX[i+2], obstaclesY[i+2]);
                XDrawLine(display_ptr, win, gc, obstaclesX[i+2], obstaclesY[i+2], obstaclesX[i], obstaclesY[i]);
            }
        }

        //draw box border
        printf("Drawing rectangle...\n");
        XDrawRectangle(display_ptr, win, gc, 0, 0, (maxY*border), (maxX*border));
        XDrawRectangle(display_ptr, win, gc, 0, 0, win_width, win_height);

        break;
        case ConfigureNotify:
        /* This event happens when the user changes the size of the window*/
        win_width = report.xconfigure.width;
        win_height = report.xconfigure.height;
        break;

        default:
                    /* this is a catch-all for other events; it does not do anything.
                           One could look at the report type to see what the event was */
        break;


       }

        }
    exit(0);
}

void readline (int &counter, int obstaclesX [], int obstaclesY []){
    char file [50];  //max file name length is 50
    FILE *f;
    char i[1000]; //tmp throw away variable
    counter=0;


    printf("*************************************************\n Please enter file name: ");
    scanf("%s", file);

    f = fopen(file, "r");

    if (f == NULL)
    {
        printf("Unable to open file.\n");
    }

    else
    {
        while(fscanf(f, "%s", i) == 1){
            fscanf (f, "%*c ( %d , %d ) ( %d , %d ) ( %d , %d )",&obstaclesX[counter],&obstaclesY[counter],&obstaclesX[counter+1],&obstaclesY[counter+1], &obstaclesX[counter+2],&obstaclesY[counter+2]);
            counter+=3;
        }
        fclose(f);
    }



    //test for read in
        printf("counter is %d..\n", counter);
        for (int z=0; z<counter; z++){
            printf("( %d , %d ) ,",obstaclesX[z], obstaclesY[z]);
        }
            printf("\n");

};

void scale (int Sobstacles [], int obstacles [], int maxValue, unsigned int dimension, int counter){

    printf("calling scale \n");

    //printf("dimension is %d , max is %d ...", dimension, maxValue);

    double factor = dimension;

    factor /= maxValue;

    //printf("double factor is %lf ...", factor);



    memcpy (Sobstacles, obstacles, sizeof(obstacles));

    for (int z=0; z<counter; z++)
        Sobstacles[z]=obstacles[z]*factor;

};

void makeNeighbors (std::vector <vertex>& V, objects O, int counter){

    printf("Making neighbors ... \n");
    int i;
    for (i=0; i<counter; i++)
        intersectTest(V, O, i, counter);
};

void intersectTest (std::vector <vertex>& V, objects O, int index, int counter){
    int i,j,k, t1, t2,t3, t4, test=0;
    for (i=0; i<counter; i++) { //cycle through all other origins to create potential neighbor
        for (j=0; j<O.A.size(); j++){ // j loop runs through the objects...
            //need to test object pts A to B, B to C, and C to A do not intersect desired line
            if (!comparePT(V[index].orig,V[i].orig)){
            //test each line segment , do any objects contain segments which intersect desired neighbor??
                t1 = orientation(V[index].orig,  V[i].orig, O.A[j]);
                t2 = orientation(V[index].orig,  V[i].orig, O.B[j]);
                t3 = orientation(V[i].orig,  O.B[j], V[index].orig);
                t4 = orientation(V[i].orig,  O.B[j], V[index].orig);
                if (t1 != t2 && t3 != t4)
                    printf ("Segments from point (%d, %d) to point (%d, %d) intersect segment index %d to index %d", V[index].orig.x, V[index].orig.y, O.A[i], V[i].orig.y, j, k );
                else{
                    test++;
                }
                t1 = orientation(V[index].orig,  V[i].orig, O.B[j]);
                t2 = orientation(V[index].orig,  V[i].orig, O.C[j]);
                t3 = orientation(V[i].orig, O.C[j], V[index].orig);
                t4 = orientation(V[i].orig,  O.C[j], V[index].orig);
                if (t1 != t2 && t3 != t4)
                    printf ("Segments from point (%d, %d) to point (%d, %d) intersect segment index %d to index %d", V[index].orig.x, V[index].orig.y, V[i].orig.x, V[i].orig.y, j, k );
                else{
                    test++;
                }
                t1 = orientation(V[index].orig,  V[i].orig, O.C[j]);
                t2 = orientation(V[index].orig,  V[i].orig, O.A[j]);
                t3 = orientation(V[i].orig,  O.A[j], V[index].orig);
                t4 = orientation(V[i].orig,  O.A[j], V[index].orig);
                if (t1 != t2 && t3 != t4)
                    printf ("Segments from point (%d, %d) to point (%d, %d) intersect segment index %d to index %d", V[index].orig.x, V[index].orig.y, V[i].orig.x, V[i].orig.y, j, k );
                else{
                    test++;

                }
                if (test ==3){
                    V[index].neigh.push_back(V[i].orig);
                    printf ("added neighbor from point (%d, %d) to point (%d, %d)\n", V[index].orig.x, V[index].orig.y, V[i].orig.x, V[i].orig.y);
                }
            }
        }
    }
};

int orientation (pt p, pt q, pt r){

    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;

    return (val >0) ? 1: 2;

};

bool onSegment (pt p, pt q, pt r){

        if (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) && q.y <= std::max (p.y, r.y) && q.y >= std::min (p.y, r.y))
                return true;
        else
            return false;
};

double distance (int X, int Y ,int x,int y){
        double dist=INF;
        if (X>=0 && Y>=0 && x>=0 && y>=0)
            dist=sqrt(pow(abs(X-x), 2)+pow(abs(Y-y), 2));
        return (dist);
};

bool comparePT(pt A, pt B){

    if (A.x == B.x && B.y == A.y)
            return true;

    return false;

};





