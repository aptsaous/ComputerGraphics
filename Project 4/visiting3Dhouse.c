#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "getbmp.h"

/**************************************************************
 *                  Constants
 **************************************************************/

#define ESC 27

#define CTRL_PTS_X 15
#define CTRL_PTS_Y 10

// if movevent is either too fast or slow, change the values of the following three constants
#define MOVE_FACTOR 0.1
#define MOVE_STEP 0.5
#define TURN_STEP 0.01

#define TEXTURES 6

#define FLOOR_WIDTH 11.0
#define FLOOR_DEPTH 9.0
#define GARDEN_WIDTH 25.0
#define GARDEN_DEPTH 25.0
#define DOOR_WIDTH 1.2
#define OUTER_WALL 0.25
#define INNER_WALL 0.1
#define WALL_HEIGHT 3.0
#define WINDOW_SIZE 1.0

/**************************************************************
 *                  Global variables
 **************************************************************/

unsigned int texture[6]; // Array of texture indices.
int doorAngle = 0; // calculates the rotation angle of the door, when the door is opening

float angle = 0.0f; // rotation angle along the z-axis for the camera direction
float theta = 0.0f; // rotation angle along the y-axis for the camera direction

float lx = 0.0f, lz = -1.0f, ly = 0.0f; // actual vector representing the camera's direction
float x = 0.0f, z = 20.0f; // XZ position of the camera

float deltaAngle = 0.0f;
float deltaTheta = 0.0f;
float deltaMove = 0.0f;

float controlPointsLeft[CTRL_PTS_X][CTRL_PTS_Y][3]; // control points for the left part of the hills
float controlPointsBack[CTRL_PTS_X][CTRL_PTS_Y][3]; // control points for the back part of the hills
float controlPointsRight[CTRL_PTS_X][CTRL_PTS_Y][3]; // control points for the right part of the hills

GLUnurbsObj *nurbs; // Pointer to NURBS object.

float uknots[19] = { 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 12.0, 12.0, 12.0 };
float vknots[14] = { 0.0, 0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 7.0, 7.0, 7.0 };

float uTextureknots[4] = { 0.0, 0.0, 12.0, 12.0 }; // knot vector along the u-parameter for the spline surface in texture space.
float vTextureknots[4] = { 0.0, 0.0, 7.0, 7.0 }; // knot vector along the v-parameter for the spline surface in texture space.


// Control points for applying texture
static float texturePoints[2][2][2] =
{
    { { 0.0, 0.0 }, { 0.0, 100.0 } },
    { { 100.0, 0.0 }, { 100.0, 100.0 } }
};

/**************************************************************
 *                  Function prototypes
 **************************************************************/

void setup();
void keyEvent( unsigned char, int, int );
void changeSize( int, int );
void renderScene();
void loadExternalTextures();
void releaseKey( int, int, int );
void pressKey( int, int, int ); 
void computePos( float );
void computeDir( float );
void initControlPoints();
void createGround();
void createHillyTerrain();
void createFence();
void createSky();
void createStonePath();
void createOuterHouse();
void createRoof();
void createInnerHouse();
void createWindows();

/**************************************************************
 *                  Start of program
 **************************************************************/

int main( int argc, char *argv[] )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA );
    
    glutInitWindowPosition( 100, 100 );
    glutInitWindowSize( 800, 600 );
    glutCreateWindow( "3D House" );
    
    glutDisplayFunc( renderScene );
    glutReshapeFunc( changeSize );
    
    glutKeyboardFunc( keyEvent );
    glutSpecialFunc( pressKey );
    
    glutIgnoreKeyRepeat(1);
    glutSpecialUpFunc( releaseKey );
    
    setup();
    
    glutMainLoop();
    
    return 0;
}

/**************************************************************
 *                  Event handlers
 **************************************************************/

void pressKey( int key, int x, int y )
{
    
    switch ( key )
    {
        case GLUT_KEY_LEFT: deltaAngle = -TURN_STEP; break;
        case GLUT_KEY_RIGHT: deltaAngle = TURN_STEP; break;
        case GLUT_KEY_UP: deltaMove = MOVE_STEP; break;
        case GLUT_KEY_DOWN: deltaMove = -MOVE_STEP; break;
        case GLUT_KEY_PAGE_UP: deltaTheta = TURN_STEP; break;
        case GLUT_KEY_PAGE_DOWN: deltaTheta = -TURN_STEP; break;
    }
    
    glutPostRedisplay();
    
}

void releaseKey( int key, int x, int y )
{
    
    switch ( key )
    {
        case GLUT_KEY_LEFT:
        case GLUT_KEY_RIGHT: deltaAngle = 0.0f; break;
        case GLUT_KEY_UP:
        case GLUT_KEY_DOWN: deltaMove = 0.0f; break;
        case GLUT_KEY_PAGE_UP:
        case GLUT_KEY_PAGE_DOWN: deltaTheta = 0.0f; break;
    }
}

void keyEvent( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case ESC:
            glDeleteTextures( TEXTURES, texture );
            gluDeleteNurbsRenderer( nurbs );
            exit(0);
            break;
            
        default:
            break;
    }
}

/**************************************************************
 *                  GLUT initialization functions
 **************************************************************/

void setup()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 );
    
    glEnable( GL_DEPTH_TEST );
    
    glGenTextures( TEXTURES, texture ); // Create texture ids.
    
    loadExternalTextures(); // Load external textures.
    
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE ); // Specify how texture values combine with current surface color values.
    
    glEnable( GL_TEXTURE_2D ); // Turn on OpenGL texturing.
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );
    
    nurbs = gluNewNurbsRenderer(); // Create NURBS object
    gluNurbsProperty( nurbs, GLU_SAMPLING_METHOD, GLU_PATH_LENGTH );
    gluNurbsProperty( nurbs, GLU_SAMPLING_TOLERANCE, 25.0 );
    gluNurbsProperty( nurbs, GLU_DISPLAY_MODE, GLU_FILL );
    gluNurbsProperty( nurbs, GLU_CULLING, GL_TRUE );
    
    initControlPoints();
    
    
}

void changeSize( int w, int h )
{
    GLfloat aspectRatio =  w * 1.0 / h;
    
    glViewport( 0, 0, w, h ); // Set the viewport to be the entire window
    
    glMatrixMode( GL_PROJECTION ); // Use the Projection Matrix
    glLoadIdentity(); // Reset Matrix
    gluPerspective( 45.0f, aspectRatio, 0.1f, 100.0f ); // Set the correct perspective.
    
    glMatrixMode( GL_MODELVIEW ); // Get Back to the Modelview
}

/**************************************************************
 *                  Utility Functions
 **************************************************************/

// Initialise control points.
void initControlPoints()
{
    int i, j;
    
    for (i = 0; i < CTRL_PTS_X; i++)
        for (j = 0; j < CTRL_PTS_Y; j++)
        {
            controlPointsLeft[i][j][0] = -50 + j * ( 37.5 / ( CTRL_PTS_Y - 1 ) );
            controlPointsBack[i][j][0] = -50 + j * ( 100.0 / ( CTRL_PTS_Y - 1 ) );
            controlPointsRight[i][j][0] = 12.5 + j * ( 37.5 / ( CTRL_PTS_Y - 1 ) );
            
            // set perimeter's y-coords to zero
            if ( ( j == 0 ) || ( i == 0 ) || ( j == CTRL_PTS_Y - 1 ) || ( i ==  CTRL_PTS_X - 1 ) )
            {
                controlPointsLeft[i][j][1] = 0;
                controlPointsBack[i][j][1] = 0;
                controlPointsRight[i][j][1] = 0;
            }
            else // set random height for realistic landscape
            {
                controlPointsLeft[i][j][1] = rand() % 5;
                controlPointsBack[i][j][1] = rand() % 5;
                controlPointsRight[i][j][1] = rand() % 5;
            }
            
            controlPointsLeft[i][j][2] = 12.5 - i * ( 25.0 / ( CTRL_PTS_X - 1 ) );
            controlPointsBack[i][j][2] = -12.5 - i * ( 37.5 / ( CTRL_PTS_X - 1 ) );
            controlPointsRight[i][j][2] = 12.5 - i * ( 25.0 / ( CTRL_PTS_X - 1 ) );

        }
}

void computePos( float deltaMove )
{

	x += deltaMove * lx * MOVE_FACTOR;
	z += deltaMove * lz * MOVE_FACTOR;
}

void computeDir( float deltaAngle ) 
{

	angle += deltaAngle;
    theta += deltaTheta;
    
    if ( theta > 1 )
        theta = 1;
    
    if ( theta < -1 )
        theta = -1;

	lx = sin( angle );
	lz = -cos( angle );
    ly = sin( theta );
    
}


void loadExternalTextures()
{
    BitMapFile *image[TEXTURES]; // Local storage for bmp image data.
    int i;
    
    // Load the images.
    image[0] = getbmp( "Textures/sky.bmp" );
    image[1] = getbmp( "Textures/grass.bmp" );
    image[2] = getbmp( "Textures/floor_wood.bmp" );
    image[3] = getbmp( "Textures/wall.bmp" );
    image[4] = getbmp( "Textures/fence.bmp" );
    image[5] = getbmp( "Textures/path.bmp" );
    
    // make white hue transparent
    for( i = 0; i < ( 4 * image[4]->sizeY * image[4]->sizeX ); i += 4 )
    {
        if ( ( image[4]->data[i] >= 128 ) && ( image[4]->data[i+1] >= 128 ) && ( image[4]->data[i+2] >= 128 ) )
        image[4]->data[i+3] = 0x00;
    }
    
    // Bind sky image to texture object texture[1]
    glBindTexture( GL_TEXTURE_2D, texture[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    
    for ( i = 1; i < TEXTURES; i++ )
    {
        glBindTexture( GL_TEXTURE_2D, texture[i] );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, image[i]->sizeX, image[i]->sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, image[i]->data );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
    }
    

}


void createGround()
{
    glBindTexture( GL_TEXTURE_2D, texture[1] ); // Map the grass texture

    // Garden inside the fence
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 32.0, 0.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 32.0, 32.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 32.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glEnd();
    
    createHillyTerrain();
    
    // flat terrain infront of the house
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -50.0, 0.0, 25.0 );
    glTexCoord2f( 32.0, 0.0 ); glVertex3f( 50.0, 0.0, 25.0 );
    glTexCoord2f( 32.0, 32.0 ); glVertex3f( 50.0, 0.0, 12.5 );
    glTexCoord2f( 0.0, 32.0 ); glVertex3f( -50.0, 0.0, 12.5 );
    glEnd();
}

void createHillyTerrain()
{
    // hills on the left of the house
    gluBeginSurface( nurbs );
    gluNurbsSurface( nurbs, 19, uknots, 14, vknots,30, 3, controlPointsLeft[0][0], 4, 4, GL_MAP2_VERTEX_3 );
    gluNurbsSurface( nurbs, 4, uTextureknots, 4, vTextureknots,4, 2, texturePoints[0][0], 2, 2, GL_MAP2_TEXTURE_COORD_2 );
    gluEndSurface( nurbs );
    
    // hills on the back of the house
    gluBeginSurface( nurbs );
    gluNurbsSurface( nurbs, 19, uknots, 14, vknots,30, 3, controlPointsBack[0][0], 4, 4, GL_MAP2_VERTEX_3 );
    gluNurbsSurface( nurbs, 4, uTextureknots, 4, vTextureknots,4, 2, texturePoints[0][0], 2, 2, GL_MAP2_TEXTURE_COORD_2 );
    gluEndSurface( nurbs );
    
    // hills on the right of the house
    gluBeginSurface( nurbs );
    gluNurbsSurface( nurbs, 19, uknots, 14, vknots,30, 3, controlPointsRight[0][0], 4, 4, GL_MAP2_VERTEX_3 );
    gluNurbsSurface( nurbs, 4, uTextureknots, 4, vTextureknots,4, 2, texturePoints[0][0], 2, 2, GL_MAP2_TEXTURE_COORD_2 );
    gluEndSurface( nurbs );
}

void createFence()
{
    glBindTexture( GL_TEXTURE_2D, texture[4] ); // Map the fence texture
    
    // back fence
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 0.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 4.0 ); glVertex3f( GARDEN_WIDTH / 2, 1.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 4.0 ); glVertex3f( -GARDEN_WIDTH / 2, 1.0, -GARDEN_DEPTH / 2 );
    glEnd();
    
    // left fence
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 0.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 4.0 ); glVertex3f( -GARDEN_WIDTH / 2, 1.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 4.0 ); glVertex3f( -GARDEN_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glEnd();
    
    // right fence
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, -GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 0.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 4.0 ); glVertex3f( GARDEN_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 4.0 ); glVertex3f( GARDEN_WIDTH / 2, 1.0, -GARDEN_DEPTH / 2 );
    glEnd();
    
    // front fence - left part
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 0.0 ); glVertex3f( -DOOR_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 4.0 ); glVertex3f( - DOOR_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 4.0 ); glVertex3f( -GARDEN_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glEnd();
    
    // front fence - right part
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( DOOR_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 0.0 ); glVertex3f( GARDEN_WIDTH / 2, 0.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 4.0, 4.0 ); glVertex3f( GARDEN_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glTexCoord2f( 0.0, 4.0 ); glVertex3f( DOOR_WIDTH / 2, 1.0, GARDEN_DEPTH / 2 );
    glEnd();
}

void createSky()
{
    glBindTexture( GL_TEXTURE_2D, texture[0] ); // Map the sky texture
    
    // back sky
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -50.0, 0.0, -50.0 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( 50.0, 0.0, -50.0 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( 50.0, 120.0, -50.0 );
    glTexCoord2f( 0.0, 1.0 ); glVertex3f( -50.0, 120.0, -50.0 );
    glEnd();
    
    // front sky
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -50.0, 0.0, 25.0 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( 50.0, 0.0, 25.0 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( 50.0, 120.0, 25.0 );
    glTexCoord2f( 0.0, 1.0 ); glVertex3f( -50.0, 120.0, 25.0 );
    glEnd();
    
    // top sky
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -50.0, 60.0, 50.0 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( 50.0, 60.0, 50.0 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( 50.0, 60.0, -50.0 );
    glTexCoord2f( 0.0, 1.0 ); glVertex3f( -50.0, 60.0, -50.0 );
    glEnd();
    
    // right sky
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( 50.0, 0.0, -50.0 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( 50.0, 0.0, 50.0 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( 50.0, 120.0, 50.0 );
    glTexCoord2f( 0.0, 1.0 ); glVertex3f( 50.0, 120.0, -50.0 );
    glEnd();
    
    // left sky
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -50.0, 0.0, -50.0 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( -50.0, 0.0, 50.0 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( -50.0, 120.0, 50.0 );
    glTexCoord2f( 0.0, 1.0 ); glVertex3f( -50.0, 120.0, -50.0 );
    glEnd();
}

void createStonePath()
{
    glBindTexture( GL_TEXTURE_2D, texture[5] ); // map stone path texture
    
    // stone path
    glBegin( GL_QUADS );
    glTexCoord2f( -1.0, 0.0 ); glVertex3f( -DOOR_WIDTH / 2, 0.001, FLOOR_DEPTH / 2 );
    glTexCoord2f( 1.0, 0.0 ); glVertex3f( DOOR_WIDTH / 2, 0.001, FLOOR_DEPTH / 2 );
    glTexCoord2f( 1.0, 1.0 ); glVertex3f( DOOR_WIDTH / 2, 0.001, GARDEN_DEPTH / 2 );
    glTexCoord2f( -1.0, 1.0 ); glVertex3f( -DOOR_WIDTH / 2, 0.001, GARDEN_DEPTH / 2 );
    glEnd();
}

void createOuterHouse()
{
    glColor3f( 238 / 255.0, 232 / 255.0, 170 / 255.0 );
    
    // left wall
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 - OUTER_WALL / 2, WALL_HEIGHT / 2, 0.0f );
    glScalef( OUTER_WALL, WALL_HEIGHT, FLOOR_DEPTH );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // right wall
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH/2 + OUTER_WALL / 2, WALL_HEIGHT / 2, 0.0f );
    glScalef( OUTER_WALL, WALL_HEIGHT, FLOOR_DEPTH );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // back wall
    glPushMatrix();
    glTranslatef( 0.0, WALL_HEIGHT / 2, -FLOOR_DEPTH / 2 - OUTER_WALL / 2 );
    glScalef( FLOOR_WIDTH + 2 * OUTER_WALL, WALL_HEIGHT, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Left part until the window
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + ( 2.4 - WINDOW_SIZE / 2 ) / 2 - OUTER_WALL / 2, WALL_HEIGHT / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( OUTER_WALL + 2.4 - WINDOW_SIZE / 2, WALL_HEIGHT, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Left part under the window
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 2.4, 1.0 / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, 1.0, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Left part over the window
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 2.4, 1.0 / 2 + 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, 1.0, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Left part until the door
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 4.8 - WINDOW_SIZE + INNER_WALL, WALL_HEIGHT / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( 2.4 - WINDOW_SIZE / 2 + INNER_WALL, WALL_HEIGHT, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Right part until window
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - ( 2.4 - WINDOW_SIZE / 2 ) / 2 + OUTER_WALL / 2, WALL_HEIGHT / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( OUTER_WALL + 2.4 - WINDOW_SIZE / 2, WALL_HEIGHT, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Right part under the window
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4, 1.0 / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, 1.0, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Right part over the window
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4, 1.0/2 + 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, 1, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - Right part until the door
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 4.8 + WINDOW_SIZE - INNER_WALL, WALL_HEIGHT / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( 2.4 - WINDOW_SIZE / 2 + INNER_WALL, WALL_HEIGHT, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // front wall - over the door
    glPushMatrix();
    glTranslatef( 0.0, 1.0 / 2 + 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( DOOR_WIDTH, 1.0, OUTER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glColor3f( 101 / 255.0, 67 / 255.0, 33 / 255.0 );
    
    // Main door
    glPushMatrix();
    
    if ( doorAngle != 0 )
    {
        glTranslatef( DOOR_WIDTH / 2, 2.0 / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 + INNER_WALL ); //return to original coordinate.
        glRotated( -doorAngle, 0, 1, 0 );
        glTranslatef( -DOOR_WIDTH / 2, 0, 0 );
    }
    else
        glTranslatef( 0, 2.0 / 2, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    
    glScalef( DOOR_WIDTH, 2.0, INNER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
}

void createRoof()
{
    glColor3f( 178 / 255.0, 34 / 255.0, 34 / 255.0 );
    
    // back trapezoid
    glBegin( GL_QUADS );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.5, 2.9f, -FLOOR_DEPTH / 2 - 0.5 );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.5, 5.0f, 0.0f );
    glVertex3f( FLOOR_WIDTH / 2 + 0.5, 5.0f, 0.0f );
    glVertex3f( FLOOR_WIDTH / 2 + 0.5f, 2.9f, -FLOOR_DEPTH / 2 - 0.5 );
    glEnd();
    
    // front trapezoid
    glBegin( GL_QUADS );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.5, 5.0f, 0.0f );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.5, 2.9f, FLOOR_DEPTH / 2 + 0.5 );
    glVertex3f( FLOOR_WIDTH / 2 + 0.5, 2.9f, FLOOR_DEPTH / 2 + 0.5 );
    glVertex3f( FLOOR_WIDTH / 2 + 0.5f, 5.0f, 0.0f );
    glEnd();
    
    // left triangle
    glBegin( GL_TRIANGLES );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.25, 3.0f, -FLOOR_DEPTH / 2 - 0.25 );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.25, 5.0f, 0.0f );
    glVertex3f( -FLOOR_WIDTH / 2 - 0.25, 3.0f, FLOOR_DEPTH / 2 + 0.25 );
    glEnd();
    
    // right triangle
    glBegin( GL_TRIANGLES );
    glVertex3f( FLOOR_WIDTH / 2 + 0.25, 3.0f, -FLOOR_DEPTH / 2 - 0.25 );
    glVertex3f( FLOOR_WIDTH / 2 + 0.25, 5.0f, 0.0f );
    glVertex3f( FLOOR_WIDTH / 2 + 0.25, 3.0f, FLOOR_DEPTH / 2 + 0.25 );
    glEnd();
}

void createInnerHouse()
{
    glColor3f( 250 / 255.0, 128 / 255.0, 114 / 255.0 );

    // 1h aristerh mesotoixia
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, WALL_HEIGHT / 2, FLOOR_DEPTH / 2 - 1.75 / 2 );
    glScalef( INNER_WALL, WALL_HEIGHT, 1.75 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // dokari portas kouzinas
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, 1 / 2.0 + 2, FLOOR_DEPTH / 2 - 1.75 / 2 - 1.4 );
    glScalef( INNER_WALL, 1.0, 1.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // 2h aristerh mesotoixia
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, WALL_HEIGHT / 2, 2.25/2 - 0.25 - 0.1/2 );
    glScalef( INNER_WALL, WALL_HEIGHT, 1.75 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // wall between kitchen and bedroom
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 4.8 / 2, WALL_HEIGHT / 2, 0.0 );
    glScalef( 4.8, WALL_HEIGHT, INNER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // teleutaia aristerh mesotoixia
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, WALL_HEIGHT / 2, -FLOOR_DEPTH / 2 + 1.7 / 2 );
    glScalef( INNER_WALL, WALL_HEIGHT, 2.2 - 0.5 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // dokari portas upnodwmatiou
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, 1 / 2.0 + 2, -FLOOR_DEPTH / 2 + 1.75 / 2 + 1.4 );
    glScalef( INNER_WALL, 1.0, 1.2 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // 3h aristerh mesotoixia
    glPushMatrix();
    glTranslatef( -DOOR_WIDTH / 2 - INNER_WALL / 2, WALL_HEIGHT / 2, -2.2 / 2 + 0.25 + 0.1 / 2 );
    glScalef( INNER_WALL, WALL_HEIGHT, 4.4 / 2 - 0.5 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    
    // 1h deksia mesotoixia
    glPushMatrix();
    glTranslatef( DOOR_WIDTH / 2 + INNER_WALL, WALL_HEIGHT / 2, FLOOR_DEPTH / 4 + 0.5 );
    glScalef( INNER_WALL, WALL_HEIGHT, FLOOR_DEPTH / 2 - 1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // 2h deksia mesotoixia
    glPushMatrix();
    glTranslatef( DOOR_WIDTH / 2 + INNER_WALL, WALL_HEIGHT / 2, -FLOOR_DEPTH / 4 - 0.5 );
    glScalef( INNER_WALL, WALL_HEIGHT, FLOOR_DEPTH / 2 - 1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // dokari salonotrapezarias
    glPushMatrix();
    glTranslatef( DOOR_WIDTH / 2 + INNER_WALL, 1.0 /2 + 2, 0.0 );
    glScalef( INNER_WALL, 1, 2 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // ceiling
    glBegin( GL_POLYGON );
    glVertex3f( -FLOOR_WIDTH / 2 , WALL_HEIGHT, FLOOR_DEPTH / 2 );
    glVertex3f( FLOOR_WIDTH / 2, WALL_HEIGHT, FLOOR_DEPTH / 2 );
    glVertex3f( FLOOR_WIDTH / 2, WALL_HEIGHT, -FLOOR_DEPTH / 2 );
    glVertex3f( -FLOOR_WIDTH / 2, WALL_HEIGHT, -FLOOR_DEPTH / 2 );
    glEnd();
}

void createWindows()
{
    glColor4f( 0.5, 0.5, 0.5, 0.3 );
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 2.4, WINDOW_SIZE / 2 + 1, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, WINDOW_SIZE, INNER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4, WINDOW_SIZE / 2 + 1, FLOOR_DEPTH / 2 + OUTER_WALL / 2 );
    glScalef( WINDOW_SIZE, WINDOW_SIZE, INNER_WALL );
    glutSolidCube( 1.0 );
    glPopMatrix();
}

void createFurnitures()
{
    glColor3f( 165 / 255.0, 42 / 255.0, 42 / 255.0 );
    
    // krevati
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 2.0 / 2 + 0.1, 0.4 / 2, -FLOOR_DEPTH / 2 + 2.0 / 2 + 0.5 );
    glScalef( 2.0, 0.4, 0.9 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // proskefalo krevatiou
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 0.1 / 2, 1.0 / 2, -FLOOR_DEPTH / 2 + 2.0 / 2 + 0.5 );
    glScalef( 0.1, 1.0, 0.9 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glColor3f( 1.0, 1.0, 1.0 );
    
    // strwma
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 2.0 / 2 + 0.1, 0.2 / 2 + 0.4, -FLOOR_DEPTH / 2 + 2.0 / 2 + 0.5 );
    glScalef( 2.0, 0.2, 0.9 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // sentoni
    glColor3f( 1.0, 0.0, 0.0 );
    glPushMatrix();
    glTranslatef( -FLOOR_WIDTH / 2 + 1.5 / 2 + 0.6, 0.01 / 2 + 0.6, -FLOOR_DEPTH / 2 + 2.0 / 2 + 0.5 );
    glScalef( 1.5, 0.01, 0.9 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // 1.40 χ 0.90 χ 0.78
    glColor3f( 1.0, 0.5, 0.0 );
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.08 / 2 + 0.7, -FLOOR_DEPTH / 2 + 7.0 );
    glScalef( 0.9, 0.08, 1.4 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 - 0.8/2 , 0.7 / 2, -FLOOR_DEPTH / 2 + 7.0 - 0.7/2 );
    glScalef( 0.1, 0.7, 0.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 - 0.8/2 , 0.7 / 2, -FLOOR_DEPTH / 2 + 7.0 + 0.7/2 );
    glScalef( 0.1, 0.7, 0.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();

    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 + 0.8/2 , 0.7 / 2, -FLOOR_DEPTH / 2 + 7.0 - 0.7/2 );
    glScalef( 0.1, 0.7, 0.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 + 0.8/2 , 0.7 / 2, -FLOOR_DEPTH / 2 + 7.0 + 0.7/2 );
    glScalef( 0.1, 0.7, 0.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glColor3f( 0.0, 1.0, 0.0 );
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4, 0.78 + 0.15/2, -FLOOR_DEPTH / 2 + 7.0 + 0.7/2 );
    glutSolidTeapot( 0.15 );
    glPopMatrix();
    
    // plath kareklas
    glColor3f( 0.0, 0.0, 1.0 );
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 1.0 / 2, -FLOOR_DEPTH / 2 + 7.0 - 1.4 / 2 - 0.45 );
    glScalef( 0.8, 1.0, 0.05 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // ka8isma kareklas
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.4 + 0.05 / 2, -FLOOR_DEPTH / 2 + 7.0 - 1.4 / 2 - 0.45/2 );
    glScalef( 0.8, 0.05, 0.45 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    // mprosta meros kareklas
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.4 / 2, -FLOOR_DEPTH / 2 + 7.0 - 1.4 / 2 - 0.05/2 );
    glScalef( 0.8, 0.4, 0.05 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.45 / 2, -0.8 );
    glScalef( 1.8, 0.45, 0.8 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.75 / 2, -0.8/2 + 0.1/2 );
    glScalef( 1.8, 0.75, 0.1 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.45 / 2, -FLOOR_DEPTH / 2 + 0.45 / 2);
    glScalef( 0.9, 0.45, 0.45 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
    glColor3f( 1.0, 1.0, 0.0 );
    glPushMatrix();
    glTranslatef( FLOOR_WIDTH / 2 - 2.4 , 0.45 + 0.5 / 2, -FLOOR_DEPTH / 2 + 0.45 / 2);
    glScalef( 0.7, 0.5, 0.05 );
    glutSolidCube( 1.0 );
    glPopMatrix();
    
}

void renderScene()
{
    if ( deltaMove )
    {
		computePos( deltaMove );
        glutPostRedisplay();
    }
    
	if ( deltaAngle || deltaTheta )
    {
		computeDir( deltaAngle );
        glutPostRedisplay();
    }
    
    if ( ( z >= -10.0 ) && ( z <= 10.0 ) && ( x >= -2.0 ) && ( x <= 2.0 ) )
    {
        if ( doorAngle != 90 )
            doorAngle++;
        
        glutPostRedisplay();
    }
    else
        doorAngle = 0;
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glLoadIdentity();
    
    gluLookAt( x, 1.5f, z, x + lx, 1.5f + ly,  z + lz, 0.0f, 1.0f,  0.0f );
    
    createGround();
    createFence();
    createSky();
    createStonePath();
    
    glBindTexture( GL_TEXTURE_2D, texture[2] ); // Map floor texture
    
    // floor
    glBegin( GL_POLYGON );
    glTexCoord2f( 0.0, 0.0 ); glVertex3f( -FLOOR_WIDTH/2.0, 0.001, FLOOR_DEPTH/2.0 );
    glTexCoord2f( 8.0, 0.0 ); glVertex3f( FLOOR_WIDTH/2.0, 0.001, FLOOR_DEPTH/2.0 );
    glTexCoord2f( 8.0, 8.0 ); glVertex3f( FLOOR_WIDTH/2.0, 0.001, -FLOOR_DEPTH/2.0 );
    glTexCoord2f( 0.0, 8.0 ); glVertex3f( -FLOOR_WIDTH/2.0, 0.001, -FLOOR_DEPTH/2.0 );
    glEnd();
    
    glDisable( GL_TEXTURE_2D );
    
    createOuterHouse();
    createRoof();
    createInnerHouse();
    createFurnitures();
    createWindows();

    glEnable( GL_TEXTURE_2D );
    
    glutSwapBuffers();
}



