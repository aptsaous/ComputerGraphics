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

#define MAX_VERTICES 30
#define CLIP_EDGES 4

#define INFO 0
#define CLEAR 1

#define ENTER_KEY 13
#define ESC_KEY 27
#define SPACE_BAR_KEY 32

#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 600

/**************************************************************
 *                  Data types
 **************************************************************/

typedef struct
{
    GLfloat x; // x co-ordinate
    GLfloat y; // y co-ordinate
    
} vertexT;

typedef struct
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
    
} color;

typedef enum { FALSE, TRUE } boolean;
typedef enum { LEFT, RIGHT, TOP, BOTTOM } positionT;

typedef struct
{
    vertexT v1; // start point of edge
    vertexT v2; // end point of edge
    positionT position; // position of edge
    
} clipEdgeT;

/**************************************************************
 *                  Global variables
 **************************************************************/

vertexT vertices[ MAX_VERTICES ]; // array that stores the vertices of the initial polygon
vertexT clippedVertices[ MAX_VERTICES ]; // array that stores the vertices of the polygon after clipping
vertexT clipRect[ 2 ]; // array that stores the lower-left and upper-right vertices of the clipping rectangle
clipEdgeT clipEdges[ CLIP_EDGES ]; // array that stores the edges of the clipping rectangle
color polygonColor, clipRectColor, clippedPolyColor;

GLfloat width, height; // width and height of the window

boolean drawPolygon = TRUE; // If TRUE draw polygon, else draw clipping rectangle
boolean hideClipRect = FALSE; // If TRUE hide the clipping rectangle, else show the clipping rectangle
boolean isLeftPressed = FALSE; // If TRUE left mouse button is pressed, else not
boolean drawRectangle = FALSE;

int vertex = 0; // counts the vertices of the initial polygon

/**************************************************************
 *                  Function prototypes
 **************************************************************/

void setup();
void clear();
void display();
void mouseEvent( int, int, int, int );
void motionEvent( int, int );
void menuEvent( int );
void keyEvent( unsigned char, int, int );
void specialKeyEvent( int, int, int );
void sutherlandHodgman();
void setClipEdges();
boolean isInside( vertexT, clipEdgeT );
vertexT intersect( vertexT, vertexT, clipEdgeT );
void reDrawPolygon();
void drawClipRect();
void showInfo();
void userManual();
void resetDefaultValues();

/**************************************************************
 *                  Start of program
 **************************************************************/

int main( int argc, char *argv[] )
{
    userManual();
    
    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;
    
    glutInit( &argc, argv );
    
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGBA );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( width, height );
    glutCreateWindow( "Project 3" );
    
    setup();
    
    glutMotionFunc( motionEvent );
    glutMouseFunc( mouseEvent );
    glutKeyboardFunc( keyEvent );
    glutSpecialFunc( specialKeyEvent );
    glutDisplayFunc( display );
    
    glutMainLoop();
    
    return 0;
}

/**************************************************************
 *                  GLUT initialization functions
 **************************************************************/

void setup()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // set clear color to black
    
    glViewport( 0, 0, width, height ); // Set the viewport to be the entire window

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity(); // Reset Matrix
    glOrtho( -( width / 2.0 ), ( width / 2.0 ), -( height / 2.0 ), ( height / 2.0 ), -1.0, 1.0 );
    
    glEnable( GL_LINE_SMOOTH ); // enable anti-aliasing
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    polygonColor.r = 1.0;
    polygonColor.g = 1.0;
    polygonColor.b = 0.0;
    
    clipRectColor.r = 0.0;
    clipRectColor.g = 1.0;
    clipRectColor.b = 0.0;
    
    clippedPolyColor.r = 1.0;
    clippedPolyColor.g = 0.0;
    clippedPolyColor.b = 1.0;
    
    glutCreateMenu( menuEvent );
    glutAddMenuEntry( "Project 3", INFO );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddMenuEntry( "Clear Screen", CLEAR );
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    
    clear();
}

// clears screen and sets initial drawing color
void clear()
{
    glClear( GL_COLOR_BUFFER_BIT ); // clear screen with the default clearing color
    glFlush();
    glColor3f( polygonColor.r, polygonColor.g, polygonColor.b );
}

void display()
{
    
}

/**************************************************************
 *                  Event handlers
 **************************************************************/

void mouseEvent( int button, int state, int x, int y )
{
    if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) && ( drawPolygon == TRUE ) )
    {
        x -= ( width / 2 );
        
        if ( y <= ( height / 2 ) )
            y = ( height / 2 ) - y;
        else
            y = -( y - ( height / 2 ) );
        
        vertices[ vertex ].x = x;
        vertices[ vertex ].y = y;
        
        if ( vertex == 0 )
        {
            // draw starting point
            glBegin( GL_POINTS );
            glVertex2i( vertices[ vertex ].x, vertices[ vertex ].y );
            glEnd();
            glFlush();
        }
        else
        {
            glBegin( GL_LINES );
            glVertex2i( vertices[ vertex-1 ].x, vertices[ vertex-1 ].y );
            glVertex2i( vertices[ vertex ].x, vertices[ vertex ].y );
            glEnd();
            glFlush();
        }
        
        vertex++;
    }
    else if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) && ( drawPolygon == FALSE ) && ( drawRectangle == TRUE ) )
    {
        isLeftPressed = TRUE;
        
        x -= ( width / 2 );
        
        if ( y <= ( height / 2 ) )
            y = ( height / 2 ) - y;
        else
            y = -( y - ( height / 2 ) );
        
        clipRect[0].x = x;
        clipRect[0].y = y;
    }
    else
        isLeftPressed = FALSE;
}

// handles the mouse press and hold movement event, to determine the shape of the clipping rectangle
void motionEvent( int x, int y )
{
    if ( isLeftPressed == TRUE )
    {
        x -= ( width / 2 );
        
        if ( y <= ( height / 2 ) )
            y = ( height / 2 ) - y;
        else
            y = -( y - ( height / 2 ) );
        
        clipRect[1].x = x;
        clipRect[1].y = y;
        
        // clears the screen, redraws the initial polygon and then draws the clipping rectangle
        glClear( GL_COLOR_BUFFER_BIT );
        reDrawPolygon();
        drawClipRect();
        
        hideClipRect = FALSE;
        
    }
}

// handles a right-click menu selection event
void menuEvent( int selection )
{
    switch ( selection )
    {
        case INFO:
            showInfo();
            break;
        case CLEAR:
            resetDefaultValues();
            break;
            
        default:
            break;
    }
}

void keyEvent( unsigned char key, int x, int y )
{
    switch ( key )
    {
        case 'c': case 'C':
            if ( drawRectangle == TRUE )
            {
                setClipEdges();
                sutherlandHodgman();
            }
            break;
        case ENTER_KEY:
            glBegin( GL_LINES );
            glVertex2i( vertices[ vertex-1 ].x, vertices[ vertex-1 ].y );
            glVertex2i( vertices[0].x, vertices[0].y );
            glEnd();
            glFlush();
            drawPolygon = FALSE;
            break;
        case SPACE_BAR_KEY:
            if ( drawPolygon == FALSE )
            {
                glClear( GL_COLOR_BUFFER_BIT );
                reDrawPolygon();
                hideClipRect = !hideClipRect;
                if ( !hideClipRect )
                    drawClipRect();
            }
            break;
        case ESC_KEY:
            exit(0);
            break;
            
        default:
            break;
    }
}

void specialKeyEvent( int key, int x, int y )
{
    switch ( key )
    {
        case GLUT_KEY_F1:
            drawRectangle = !drawRectangle;
            //drawPolygon = !drawPolygon;
            break;
            
        default:
            break;
    }
}

/**************************************************************
 *                  Drawing Algorithms
 **************************************************************/

// applies the Sutherland-Hodgman algorithm to clip a polygon based on a clipping rectangle
void sutherlandHodgman()
{
    int i, j;
    boolean p1, p2; // booleans that shows whether a vertex is inside or outside the clipping area defined by an edge
    
    int out = 0;
    int in = vertex;
    
    vertexT inputArray[ MAX_VERTICES ]; // temp array that stores the vertices of the initial polygon and in each repetition stores the new vertices after clipping by one edge
    
    // saves the vertices of the initial polygon to a local array, to keep the initial polygon intact
    for ( j = 0; j < vertex; j++ )
        inputArray[j] = vertices[j];
    
    for ( j = 0; j < CLIP_EDGES; j++ )
    {
        for ( i = 0; i < in; i++ )
        {
            p1 = isInside( inputArray[i], clipEdges[j] );
            if ( i == in - 1 )
                p2 = isInside( inputArray[0], clipEdges[j] );
            else
                p2 = isInside( inputArray[i+1], clipEdges[j] );
            
            
            if ( ( p1 == FALSE ) && ( p2 == FALSE ) ) // if both vertices of the edge are outside the clipping area, save nothing
                continue;
            else if ( ( p1 == TRUE ) && ( p2 == TRUE ) ) // if both vertices of the edge are inside the clipping area, save the end point of the edge
            {
                clippedVertices[ out ] = ( i == in - 1 ) ? inputArray[0] : inputArray[i+1];
                out++;
            }
            else if ( ( p1 == TRUE ) && ( p2 == FALSE ) ) // if start point is inside and end point is outside the clipping area, save the intersection with the clipping edge
            {
                clippedVertices[ out ] = ( i == in - 1 ) ? intersect( inputArray[i], inputArray[0], clipEdges[j] ) : intersect( inputArray[i], inputArray[i+1], clipEdges[j] );
                out++;
            }
            else // if start point is outside and end point is inside the clipping area, save the intersection with the clipping edge and save the end point of the edge
            {
                clippedVertices[ out ] = ( i == in - 1 ) ? intersect( inputArray[i], inputArray[0], clipEdges[j] ) : intersect( inputArray[i], inputArray[i+1], clipEdges[j] );
                out++;
                clippedVertices[ out ] = ( i == in - 1 ) ? inputArray[0] : inputArray[i+1];
                out++;
            }
            
            
        }
        
        in = out; // updates size of the input array
        out = 0; // resets the size of the output array
        
        // updates the values of the input array
        for ( i = 0; i < in; i++ )
            inputArray[i] = clippedVertices[i];
    }
    
    
    glColor3f( clippedPolyColor.r, clippedPolyColor.g, clippedPolyColor.b );
    
    glBegin( GL_LINE_STRIP );
    
    for ( i = 0; i < in; i++ )
        glVertex2i( clippedVertices[i].x, clippedVertices[i].y );
    
    glVertex2i( clippedVertices[0].x, clippedVertices[0].y );
    glEnd();
    
    glFlush();
}

// sets the co-ordinates of the clipping edges and their position, based on the lower-left and upper-right vertex of the rectangle
void setClipEdges()
{
    clipEdges[0].v1 = clipRect[0];
    clipEdges[0].v2.x = clipRect[0].x;
    clipEdges[0].v2.y = clipRect[1].y;
    
    clipEdges[1].v1.x = clipRect[0].x;
    clipEdges[1].v1.y = clipRect[1].y;
    clipEdges[1].v2 = clipRect[1];
    
    clipEdges[2].v1 = clipRect[1];
    clipEdges[2].v2.x = clipRect[1].x;
    clipEdges[2].v2.y = clipRect[0].y;
    
    clipEdges[3].v1.x = clipRect[1].x;
    clipEdges[3].v1.y = clipRect[0].y;
    clipEdges[3].v2 = clipRect[0];
    
    if ( clipRect[0].x < clipRect[1].x )
    {
        clipEdges[0].position = LEFT;
        clipEdges[2].position = RIGHT;
    }
    else
    {
        clipEdges[0].position = RIGHT;
        clipEdges[2].position = LEFT;
    }
    
    if ( clipRect[0].y < clipRect[1].y )
    {
        clipEdges[1].position = TOP;
        clipEdges[3].position = BOTTOM;
    }
    else
    {
        clipEdges[1].position = BOTTOM;
        clipEdges[3].position = TOP;
    }
    
}

// checks whether a vertex lies inside the clipping part of an edge
boolean isInside( vertexT v, clipEdgeT clipEdge )
{
    switch ( clipEdge.position )
    {
        case LEFT:
            return v.x > clipEdge.v1.x;
        case RIGHT:
            return v.x < clipEdge.v1.x;
        case TOP:
            return v.y < clipEdge.v1.y;
        case BOTTOM:
            return v.y > clipEdge.v1.y;
    }
    
    return FALSE;
}

// calculates the intersection point between an edge of the polygon and an edge of the clipping rectangle
vertexT intersect( vertexT v1, vertexT v2, clipEdgeT clipEdge )
{
    vertexT v;
    
    GLfloat d = ( clipEdge.v2.x - clipEdge.v1.x ) * ( v2.y - v1.y ) - ( clipEdge.v2.y - clipEdge.v1.y ) * ( v2.x - v1.x );
    
    if (d == 0)
    {
        printf( "Error!! Trying to divide by 0\n" );
        exit(0);
    }
    
    v.x = ( ( v2.x - v1.x ) * ( clipEdge.v2.x * clipEdge.v1.y - clipEdge.v2.y * clipEdge.v1.x ) - ( clipEdge.v2.x - clipEdge.v1.x ) * ( v2.x * v1.y - v2.y * v1.x ) ) / d;
    v.y = ( ( v2.y - v1.y ) * ( clipEdge.v2.x * clipEdge.v1.y - clipEdge.v2.y * clipEdge.v1.x ) - ( clipEdge.v2.y - clipEdge.v1.y ) * ( v2.x * v1.y - v2.y * v1.x ) ) / d;
    
    return v;
    
}

// re-draws the initial polygon
void reDrawPolygon()
{
    int i;
    
    glColor3f( polygonColor.r, polygonColor.g, polygonColor.b );
    
    glBegin( GL_LINE_STRIP );
    
    for ( i = 0; i < vertex; i++ )
        glVertex2i( vertices[i].x, vertices[i].y );
    
    glVertex2i( vertices[0].x, vertices[0].y );
    glEnd();
    
    glFlush();
}

// Draws the clipping rectangle
void drawClipRect()
{
    glColor4f( clipRectColor.r, clipRectColor.g, clipRectColor.b, 0.2 );
    
    glBegin( GL_QUADS );
    glVertex3f( clipRect[0].x, clipRect[0].y, 0.0 );
    glVertex3f( clipRect[0].x, clipRect[1].y, 0.0 );
    glVertex3f( clipRect[1].x, clipRect[1].y, 0.0 );
    glVertex3f( clipRect[1].x, clipRect[0].y, 0.0 );
    glEnd();
    
    glFlush();
}

/**************************************************************
 *                  Utility Functions
 **************************************************************/

void userManual()
{
    printf( "First draw polygon by selecting the vertices of the polygon\n" );
    printf( "Press enter when you want to connect the first with the last vertex\n" );
    printf( "Then press F1 to switch from drawing mode to clipping mode\n ");
    printf( "Draw the clipping rectangle by holding the left button of the mouse and dragging\n" );
    printf( "By pressing the space-bar key you can hide/show the clipping rectangle\n" );
    printf( "Clip the polygon by pressing 'C' or 'c'\n" );
    printf( "Clear screen by right clicking the mouse and selecting Clear Screen from the pop-up menu\n" );
    printf( "Press Esc to quit applicaton\n" );
}

// shows who created the project
void showInfo()
{
    printf( "Project 3\n" );
    printf( "Created by:\n" );
    printf( "Orestis Mpakatsis 1658\n" );
    printf( "Orsalia Stamatiou 1666\n" );
    printf( "Ioanna Strati 1676\n" );
    printf( "Apostolos Tsaousis 1714\n" );
    
}

// resets global variables to initial values
void resetDefaultValues()
{
    drawPolygon = TRUE; // enables polygon drawing
    hideClipRect = FALSE; // hides clipping rectangle
    drawRectangle = FALSE; // disables rectangle drawing
    isLeftPressed = FALSE;
    vertex = 0; // resets vertices count to 0
    
    clear(); // clears screen and sets initial drawing color
}


