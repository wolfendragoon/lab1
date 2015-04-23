//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <unistd.h>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
extern "C" {
#include "fonts.h"
}
#define rnd() (float)rand() / (float)RAND_MAX
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600
#define numbox 5
#define MAX_PARTICLES 4000
#define GRAVITY 0.3
#define pi 3.1415926
//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures
bool bubbler = false;
struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape *box;
    Shape box2;
    Particle *particle;
    int n;
    float c1,c2,c3;
    int count;
    ~Game() { delete [] particle;}
    Game(){
	c1 = 130;
	c2 = 130;
	c3 = 130;
	count = 0;
	int z = 1;
	particle = new Particle[MAX_PARTICLES];
	box = new Shape[numbox];
	for(int i = 0; i < numbox; i++){
	n=0;
	z= z* -1;	
	box[i].width = 100;
	box[i].height = 10;
	box[i].center.x = 200-(((i+3)*60)) + 5*65;
	box[i].center.y = 500+(((i+3)*40)) - 5*60;
	}
	box2.width = 10;
	box2.height = 100;
	box2.radius = 80;
    
	box2.center.x =470;
	box2.center.y = 180 ;
    }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;
    game.n=0;

    //declare a box shape

    //start animation
    while(!done) {
	while(XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask |
	PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
	return;
    //std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = (rnd()-.5) *-2.0;
    p->velocity.x =  (rnd()) *(rnd()*2);
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    makeParticle(game, e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    bubbler ^= true;
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;
	makeParticle(game, e->xbutton.x,WINDOW_HEIGHT - e->xbutton.y);
	if (++n < 10)
	    return;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.

    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if (game->n <= 0)
	return;

    for(int z=0; z<game->n;z++){
	p = &game->particle[z];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;

	//check for collision with shapes...
	//Shape *s;

	//Vec *zed = p->center;

	for (int i = 0; i < numbox; i++)
	    if((p->s.center.x <game->box[i].center.x + game->box[i].width)&&
		(p->s.center.x >game->box[i].center.x - game->box[i].width)&&
		(p->s.center.y < game->box[i].center.y + game->box[i].height ) && 
		(p->s.center.y >game->box[i].center.y - game->box[i].height)){
	    if(p->s.center.y > game->box[i].center.y + ((1/2)* game->box[i].height)){

		p->s.center.y = game->box[i].center.y+1+game->box[i].height;
	    }else{

		p->s.center.y = game->box[i].center.y-1-game->box[i].height;
	    }
	    p->velocity.y = p->velocity.y * (-.4);
	}
	if((p->velocity.x < .1)&& (p->velocity.x > -.1)){
	    p->velocity.x = p->velocity.x*1;
	}

    //check for circle collision
    Shape *cir;
    cir = &game->box2;
    float xx = (cir->center.x - p->s.center.x);
    float yy = (cir->center.y - p->s.center.y);
    float x2 = (cir->center.x - p->s.center.x) * (cir->center.x - p->s.center.x);
    float y2 = (cir->center.y - p->s.center.y) * (cir->center.y - p->s.center.y);
    float dist2 = (xx*xx) + (yy*yy);

    if(dist2 <= ((cir->radius * cir->radius))){
        std::cout << "in circle " << p->s.center.x << " " << p->s.center.y << std::endl;
        float dist = sqrt(dist2);
        float xnorm = xx/dist;
        float ynorm = yy/dist;
        float vx2 = p->velocity.x * p->velocity.x;
        float vy2 = p->velocity.y * p->velocity.y;
        float v2 = vx2 + vy2;
        p->s.center.x = (-(xnorm * (cir->radius+2))) + cir->center.x;
        p->s.center.y = (-(ynorm * (cir->radius+2))) + cir->center.y;
        p->velocity.x = (sqrt(v2) * -xnorm)*.6;
        p->velocity.y = (sqrt(v2) * -ynorm)*.6;//=0;// p->velocity.y * -.2;
        //p->velocity.y = 0;//sqrt(v2) * ynorm;//=0;// p->velocity.y * -.2;
    }
    /*
	if((p->s.center.x <(350+(5*65)+10))&&(p->s.center.x > (350 + (5*65)-10))&&
		(p->s.center.y < (500 -(5*60) +100)) && (p->s.center.y > (500 - (5*60) -100))){
	    p->velocity.x = p->velocity.x * (-.7);
	}

*/
	//float dist = sqrt(((game->box2.center.x - p->s.center.x)*(game->box2.center.x - p->s.center.x)) +
	//	((game->box2.center.x - p->s.center.x) *(game->box2.center.x - p->s.center.x)));

	//std::cout << "distance " <<dist << std::endl;

	/*if( dist <= game->box2.radius)
	{
	    p->velocity.y = p->velocity.y * (-.7);
	    p->velocity.x = p->velocity.x * (-.7);

	}
	*/

	//add gravity
	p->velocity.y -= GRAVITY;

	//check for off-screen
	if (p->s.center.y < 0.0) {
	    std::cout << "off screen" << std::endl;
	    memcpy(&game->particle[z],&game->particle[game->n-1],sizeof(Particle));
	    game->n -=1;
	}
    }
}

void render(Game *game)
{

    Shape *temp = &game->box[1];
    Rect zr;
    zr.bot =  WINDOW_HEIGHT/2 ;
    zr.left = 10 ;
    zr.center = 0;
    glClear(GL_COLOR_BUFFER_BIT);

    float w, h,ww,hh;
    game->count +=1;
    if(game->count >=100){
	game->c1=rnd()*250;
	game->c2=rnd()*250;
	game->c3=rnd()*250;
	game->count = 0;
    }
    
    //Draw shapes...

    //draw box
    Shape *s;
    glColor3ub(250,100,100);
    for(int i = 0; i < numbox;i++){
    s = &game->box[i];
    glPushMatrix();
    glTranslatef(s->center.x, s->center.y, s->center.z);
    w = s->width;
    h = s->height;
    glBegin(GL_QUADS);
    glVertex2i(-w,-h);
    glVertex2i(-w, h);
    glVertex2i( w, h);
    glVertex2i( w,-h);
    glEnd();
    glPopMatrix();

    }
    //print text
    Shape*s2;
    /*
    std::string text[numbox];
    
    if(numbox > 5){
        for (int xxy = 5; xxy < numbox; xxy++){
            text[xxy] = "extra";
        }
    }
    text[0] = "Maintenance";
    text[1] = "Testing";
    text[2] = "Coding";
    text[3] = "Design";
    text[4] = "Requirements";
    */
    for (int i = 0; i < numbox;i++){
        s2 = &game->box[i];

        zr.bot = s2->center.y-10;
        zr.left = s2->center.x-50;

        if(i == 0)ggprint16(&zr,20,0,"Maintenance");
        if(i == 1)ggprint16(&zr,20,0,"Testing");
        if(i == 2)ggprint16(&zr,20,0, "Coding");
        if(i == 3)ggprint16(&zr,20,0, "Design");
        if(i == 4)ggprint16(&zr,20,0, "Requirements");
    }
    Shape *q;
    glColor3ub(game->c2,game->c3,game->c1);
    glColor3ub(250,100,100);
    q = &game->box2;
    glPushMatrix();
    ww = q->radius;
    //hh = q->height;
    float cx,cy,crot;

    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(q->center.x,q->center.y);
    for(int ndx = 0; ndx <= 30; ndx++){
        glVertex2f(
                q->center.x + (q->radius * cos(ndx * (2* pi)/30)),
                q->center.y + (q->radius * sin(ndx * (2* pi)/30))
                );
    
    }
    /*
     

    glBegin(GL_QUADS);
    glVertex2i(-ww,-hh);
    glVertex2i(-ww, hh);
    glVertex2i( ww, hh);
    glVertex2i( ww,-hh);
    */

    glEnd();
    glPopMatrix();    

	if(bubbler){
	    makeParticle(game,(51),(496));
	    makeParticle(game,(50),(495));
	    makeParticle(game,(45),(495));
        makeParticle(game,(49),(500));
	    makeParticle(game,(40),(495));
	    makeParticle(game,(50),(495));
	    makeParticle(game,(50),(495));
	    makeParticle(game,(50),(500));
	    makeParticle(game,(45),(495));
	    makeParticle(game,(50),(505));
	    makeParticle(game,(50),(495));
	}    
    //draw all particles here
    for(int zz = 0; zz< game->n;zz++){
	glPushMatrix();
	glColor3ub(10,10,250);
	Vec *c = &game->particle[zz].s.center;
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
	glPopMatrix();
    }
    glPushMatrix();
    
}



