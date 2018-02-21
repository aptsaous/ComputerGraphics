#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

/**************************************************************
 *                  Constants
 **************************************************************/

#define BLACK 1
#define BLUE 2
#define CYAN 3
#define GRAY 4
#define GREEN 5
#define LIME 6
#define MAGENTA 7
#define MAROON 8
#define NAVY 9
#define OLIVE 10
#define PURPLE 11
#define RED 12
#define SILVER 13
#define TEAL 14
#define YELLOW 15
#define WHITE 16

#define INFO 0
#define COLOR 17
#define ESC_KEY 27

#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 600

/**************************************************************
 *                  Data types
 **************************************************************/

typedef struct 
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    
} color;

typedef enum { FALSE, TRUE } boolean;

/**************************************************************
 *                  Global variables
 **************************************************************/
GLfloat width, height; // width and height of drawing window
GLint x[3]; // x-coords for the three vertices of the triangle
GLint y[3]; // y-coords for the three vertices of the triangle
int vertices = 0; // counts drawn vertices
boolean isResized = FALSE; // checks if window has been re-sized

color c[3]; // array that holds the color of the triangle's vertices

/**************************************************************
 *                  Function prototypes
 **************************************************************/

void setup();
void setPixel( GLint, GLint );
GLint getXmin();
GLint getYmin();
GLint getXmax();
GLint getYmax();
void showInfo();
void menuEvent( int );
void mouseEvent( int, int, int, int );
GLfloat f01( GLint, GLint );
GLfloat f12( GLint, GLint );
GLfloat f20( GLint, GLint );
void display();
void changeSize( int, int );
void fillTriangle();
void setVertexColor( int );
void keyEvent( unsigned char, int, int );

/**************************************************************
 *                  Start of program
 **************************************************************/


int main( int argc, char *argv[] )
{
    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;
    
    glutInit( &argc, argv );
    
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( width, height );
    glutCreateWindow( "Project 2a" );
    
    setup();
    
    glutDisplayFunc( display );
    glutReshapeFunc( changeSize );
    
    glutKeyboardFunc( keyEvent );
    glutMouseFunc( mouseEvent );
    
    glutMainLoop();
    
    return 0;
}

/**************************************************************
 *                  GLUT initialization functions
 **************************************************************/

void setup()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // set clear color to black
    
    glEnable( GL_POINT_SMOOTH );
    
    glClear( GL_COLOR_BUFFER_BIT ); // clear screen with default clear color
    
    glFlush();
    
    c[0].r = 1.0; c[0].g = 0.0; c[0].b = 0.0; // color of first vertex, red
    c[1].r = 0.0; c[1].g = 1.0; c[1].b = 0.0; // color of second vertex, green
    c[2].r = 0.0; c[2].g = 0.0; c[2].b = 1.0; // color of third vertex, blue
    
    int vertexColor = glutCreateMenu( setVertexColor );
    glutAddMenuEntry( "Color", COLOR );
    glutAddMenuEntry( "-----------------", COLOR );
    glutAddMenuEntry( "Black", BLACK );
    glutAddMenuEntry( "Blue", BLUE );
    glutAddMenuEntry( "Cyan", CYAN );
    glutAddMenuEntry( "Gray", GRAY );
    glutAddMenuEntry( "Green", GREEN );
    glutAddMenuEntry( "Lime", LIME );
    glutAddMenuEntry( "Magenta", MAGENTA );
    glutAddMenuEntry( "Maroon", MAROON );
    glutAddMenuEntry( "Navy" , NAVY );
    glutAddMenuEntry( "Olive", OLIVE );
    glutAddMenuEntry( "Purple", PURPLE );
    glutAddMenuEntry( "Red", RED );
    glutAddMenuEntry( "Silver", SILVER );
    glutAddMenuEntry( "Teal", TEAL );
    glutAddMenuEntry("Yellow", YELLOW );
    glutAddMenuEntry( "White", WHITE );
    
    glutCreateMenu( menuEvent );
    glutAddMenuEntry( "Project 2a", INFO );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddSubMenu( "Choose vertex color", vertexColor );
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    
}

void display()
{
    if ( isResized == TRUE )
    {
        glClear( GL_COLOR_BUFFER_BIT ); // clear screen with default clear color
        
        glFlush();
        
        isResized = FALSE;
        
    }
}

void changeSize( int w, int h )
{
    width = w;
    height = h;
    
    glViewport( 0, 0, w, h ); // Set the viewport to be the entire window
    
    glMatrixMode( GL_PROJECTION ); // Use the Projection Matrix
    glLoadIdentity(); // Reset Matrix
    glOrtho( -( width / 2.0 ), ( width / 2.0 ), -( height / 2.0 ), ( height / 2.0 ), -1.0, 1.0 );
    
    isResized = TRUE;
}

/**************************************************************
 *                  Event handlers
 **************************************************************/

void menuEvent( int selection )
{
    switch ( selection )
    {
        case INFO:
            showInfo();
            break;
        default:
            break;
    }
}

void mouseEvent( int button, int state, int xx, int yy )
{
    if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) )
    {
        xx -= ( width / 2 );
        
        if ( yy <= ( height / 2 ) )
            yy = ( height / 2 ) - yy;
        else
            yy = -( yy - ( height / 2 ) );
        
        x[ vertices ] = xx;
        y[ vertices ] = yy;
        
        glColor3f( c[ vertices ].r, c[ vertices ].g, c[ vertices ].b );
        setPixel( x[ vertices ], y[ vertices ] );
        
        glFlush();
        
        vertices++;
        
        if ( vertices == 3 )
        {
            vertices = 0;
            fillTriangle();
        }
    }
}

void setVertexColor( int selection )
{
    switch ( selection )
    {
        case BLACK:
            c[vertices].r = 0.0; c[vertices].g = 0.0; c[vertices].b = 0.0;
            break;
        case BLUE:
            c[vertices].r = 0.0; c[vertices].g = 0.0; c[vertices].b = 1.0;
            break;
        case CYAN:
            c[vertices].r = 0.0; c[vertices].g = 1.0; c[vertices].b = 1.0;
            break;
        case GRAY:
            c[vertices].r = 128.0 / 255; c[vertices].g = 128.0 / 255; c[vertices].b = 128.0 / 255;
            break;
        case GREEN:
            c[vertices].r = 0.0; c[vertices].g = 128.0 / 255; c[vertices].b = 0.0;
            break;
        case LIME:
            c[vertices].r = 0.0; c[vertices].g = 1.0; c[vertices].b = 0.0;
            break;
        case MAGENTA:
            c[vertices].r = 1.0; c[vertices].g = 1.0; c[vertices].b = 1.0;
            break;
        case MAROON:
            c[vertices].r = 128.0 / 255; c[vertices].g = 0.0; c[vertices].b = 0.0;
            break;
        case NAVY:
            c[vertices].r = 0.0; c[vertices].g = 0.0; c[vertices].b = 128.0 / 255;
            break;
        case OLIVE:
            c[vertices].r = 128.0 / 255; c[vertices].g = 128.0 / 255; c[vertices].b = 0.0;
            break;
        case PURPLE:
            c[vertices].r = 128.0 / 255; c[vertices].g = 0.0; c[vertices].b = 128.0 / 255;
            break;
        case RED:
            c[vertices].r = 1.0; c[vertices].g = 0.0; c[vertices].b = 0.0;
            break;
        case SILVER:
            c[vertices].r = 192.0 / 255; c[vertices].g = 192.0 / 255; c[vertices].b = 192.0 / 255;
            break;
        case TEAL:
            c[vertices].r = 0.0; c[vertices].g = 128.0 / 255; c[vertices].b = 128.0 / 255;
            break;
        case YELLOW:
            c[vertices].r = 1.0; c[vertices].g = 1.0; c[vertices].b = 0.0;
            break;
        case WHITE:
            c[vertices].r = 1.0; c[vertices].g = 1.0; c[vertices].b = 1.0;
            break;
            
        default:
            printf( "Default color for the first point is red, for the second is green and for the third is blue\n" );
            break;
    }
}

void keyEvent( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case ESC_KEY:
            exit(0);
            break;
            
        default:
            break;
    }
}


/**************************************************************
 *                  Drawing Algorithms
 **************************************************************/

void fillTriangle()
{
    GLfloat alpha, beta, gamma;
    GLint xx, yy;
    color cNew;
    
    GLint xMin = floor( getXmin() );
    GLint yMin = floor( getYmin() );
    GLint xMax = ceil( getXmax() );
    GLint yMax = ceil( getYmax() );
    
    for ( yy = yMin; yy <= yMax; yy++ )
        for ( xx = xMin; xx <= xMax; xx++ )
        {
            alpha = f12( xx, yy ) / f12( x[0], y[0] );
            beta = f20( xx, yy ) / f20( x[1], y[1] );
            gamma = f01( xx, yy ) / f01( x[2], y[2] );
            
            if ( ( alpha > 0 ) && ( beta > 0 ) && ( gamma > 0 ) )
            {
                cNew.r = alpha * c[0].r + beta * c[1].r + gamma * c[2].r;
                cNew.g = alpha * c[0].g + beta * c[1].g + gamma * c[2].g;
                cNew.b = alpha * c[0].b + beta * c[1].b + gamma * c[2].b;
                
                glColor3f( cNew.r, cNew.g, cNew.b );
                setPixel( xx, yy );
            }
        }
    
    glFlush();
    
}

GLfloat f01( GLint xx, GLint yy )
{
    return ( y[0] - y[1] ) * xx + ( x[1] - x[0] ) * yy + x[0] * y[1] - x[1] * y[0];
}

GLfloat f12( GLint xx, GLint yy )
{
    return ( y[1] - y[2] ) * xx + ( x[2] - x[1] ) * yy + x[1] * y[2] - x[2] * y[1];
}

GLfloat f20( GLint xx, GLint yy )
{
    return ( y[2] - y[0] ) * xx + ( x[0] - x[2] ) * yy + x[2] * y[0] - x[0] * y[2];
}

void setPixel( GLint x, GLint y )
{
    glBegin( GL_POINTS );
    glVertex2i( x, y );
    glEnd();
}


/**************************************************************
 *                  Utility Functions
 **************************************************************/

GLint getXmin()
{
    int i;
    GLint min = x[0];
    
    for ( i = 1; i < 3; i++ )
        if ( x[i] < min )
            min = x[i];
    
    return min;
}

GLint getYmin()
{
    int i;
    GLint min = y[0];
    
    for ( i = 1; i < 3; i++ )
        if ( y[i] < min )
            min = y[i];
    
    return min;
}

GLint getXmax()
{
    int i;
    GLint max = x[0];
    
    for ( i = 1; i < 3; i++ )
        if ( x[i] > max )
            max = x[i];
    
    return max;
}

GLint getYmax()
{
    int i;
    GLint max = y[0];
    
    for ( i = 1; i < 3; i++ )
        if ( y[i] > max )
            max = y[i];
    
    return max;
}

void showInfo()
{
    printf( "Project 2a\n" );
    printf( "Created by:\n" );
    printf( "Orestis Mpakatsis 1658\n" );
    printf( "Orsalia Stamatiou 1666\n" );
    printf( "Ioanna Strati 1676\n" );
    printf( "Apostolos Tsaousis 1714\n" );
    
}

