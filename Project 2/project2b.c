#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

/**************************************************************
 *                  Constants
 **************************************************************/

#define INFO 0
#define DRAW 1
#define FILL 2
#define CLEAR 3
#define ENTER_KEY 13
#define ESC_KEY 27

#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 600

/**************************************************************
 *                  Data types
 **************************************************************/

struct edgeList
{
    char id[3]; // name of the edge
    int xStart; // begin point x-coord
    int yStart; // begin point y-coord
    int xEnd; // end point x-coord
    int yEnd; // end point y-coord
    struct edgeList *next;
    
};

typedef struct edgeList edgeT;

struct entry
{
    char id[3]; // name of the edge
    int yMax; // maximum y-value of the edge
    float xAtYmin; // x value at the point where the edge has its minimum y-value
    float invSlope; // inverse slope of the edge
    struct entry *next;
};

typedef struct entry edgeTableEntry;

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

edgeT *head = NULL; // root node of edge list
edgeTableEntry *root = NULL; // root node of active edget table
edgeTableEntry **globalEdgeTable;

int xStart, yStart, xEnd, yEnd, xInitial, yInitial;
int clickCount = 0;
int yMin, yMax;
char vertexName = 'A';
GLfloat width, height;
color lineColor;
boolean enableDraw = FALSE;

/**************************************************************
 *                  Function prototypes
 **************************************************************/

void destroyEdgeList();
void destroyAET();
void resetDefaultValues();
int getYmin();
int getYmax();
void displayGlobalEdgeTable( int );
void showInfo();
void displayList();
void menuEvent( int );
void keyEvent( unsigned char, int, int );
void mouseEvent( int, int, int, int );
void bresenhamLine( GLint, GLint, GLint, GLint );
void swap( GLint *, GLint * );
void setPixel( GLint, GLint );
void setPixelf( GLfloat, GLfloat );
void setAndColorPixel( GLint, GLint, double, color );
void fillPolygon();
void createGlobalEdgeTable();
void moveListFromGETtoAET( int );
void removeEdges( int );
void sortActiveEdgeList();
void colorPixelsBetweenPairs( int );
void updateXcoOrdinates();
void insertEdge( int, int, int, int, char * );
void closePolygon();
void setup();
void display();
void clear();

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
    glutCreateWindow( "Project 2b" );
    
    glutDisplayFunc( display );
    
    glutMouseFunc( mouseEvent );
    glutKeyboardFunc( keyEvent );
    
    setup();

    glutMainLoop();
    
    return 0;
    
}

/**************************************************************
 *                  GLUT initialization functions
 **************************************************************/

void setup()
{
    glViewport( 0, 0, width, height ); // Set the viewport to be the entire window
    
    glMatrixMode( GL_PROJECTION ); // Use the Projection Matrix
    glLoadIdentity(); // Reset Matrix
    glOrtho( -( width / 2.0 ), ( width / 2.0 ), -( height / 2.0 ), ( height / 2.0 ), -1.0, 1.0 );
    
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // set clear color to black
    
    // set default drawing color to yellow
    lineColor.r = 1.0;
    lineColor.g = 1.0;
    lineColor.b = 0.0;
    
    glutCreateMenu( menuEvent );
    glutAddMenuEntry( "Project 2", INFO );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddMenuEntry( "Draw Polygon (max. 10 vertices)", DRAW );
    glutAddMenuEntry( "Fill Polygon", FILL );
    glutAddMenuEntry( "Clear Screen", CLEAR );
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
    
    clear();
}

void display()
{
}

void clear()
{
    int i = 0;
    
    glClear( GL_COLOR_BUFFER_BIT ); // clear screen with default clear color
    
    glColor3f( 1.0, 1.0, 1.0 ); // set color to white
    
    char str[] = "Press <enter> to close to polygon";
    
    glRasterPos2i( -width/2 + 10, -height/2 + 10 );
    
    for ( i = 0; i < strlen( str ); i++ )
        glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, str[i] ); // display message on screen
    
    glFlush();
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
        case DRAW:
            enableDraw = TRUE;
            break;
        case FILL:
            enableDraw = FALSE;
            fillPolygon();
            break;
        case CLEAR:
            clear();
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
        case ENTER_KEY:
            if ( enableDraw == TRUE )
                closePolygon();
            break;
        case ESC_KEY:
            exit(0);
            break;
            
        default:
            break;
    }
}

void mouseEvent( int button, int state, int x, int y )
{
    char id[3]; // edge's name
    
    if ( ( enableDraw == TRUE ) && ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) )
    {
        if ( clickCount == 0 )
        {
            // set start point of edge
            xStart = x;
            yStart = y;
            
            // convert co-ordinates to match those set by glOrtho
            xStart -= ( width / 2 );
            
            if ( yStart <= ( height / 2 ) )
                yStart = ( height / 2 ) - yStart;
            else
                yStart = -( yStart - ( height / 2 ) );
            
            // draw starting point
            setPixel( xStart, yStart );
            glFlush();
            
            // save co-ordinates from the first point, so that the program knows where to finish the last line and close the polygon
            xInitial = xStart;
            yInitial = yStart;
        }
        else
        {
            // set end point of edge
            xEnd = x;
            yEnd = y;
            
            // convert co-ordinates to match those set by glOrtho
            xEnd -= ( width / 2 );
            
            if ( yEnd <= ( height / 2 ) )
                yEnd = ( height / 2 ) - yEnd;
            else
                yEnd = -( yEnd - ( height / 2 ) );
            
            // set an name for the edge based on the start and end vertices -- for debug purposes only
            id[0] = vertexName;
            id[1] = ++vertexName;
            id[2] = '\0';
            
            // save the edge in a list with all polygon edges
            insertEdge( xStart, yStart, xEnd, yEnd, id );
            
            // draw edge
            bresenhamLine( xStart, yStart, xEnd, yEnd );
            
            // set end point of the current edge, as the start point of the next edge
            xStart = xEnd;
            yStart = yEnd;
            
        }
        
        clickCount++;
        
        // if the limit of the 10 vertices has been reached
        if ( clickCount == 10 )
            closePolygon();
        
    }
}

/**************************************************************
 *                  Drawing Algorithms
 **************************************************************/

void bresenhamLine( GLint xStart, GLint yStart, GLint xEnd, GLint yEnd )
{
    
    int dx, dy, incrS, incrD, d, x, y, xStep, yStep, yDir, i;
    float D=0, sin_a, cos_a, smc_a, Denom;
    
    dy = abs( yEnd - yStart );
    dx = abs( xEnd - xStart );
    
    x = xStart;
    y = yStart;
    
    if ( dy > dx )
    {
        swap( &dx, &dy );
        yDir = 1;
        
    }
    else
        yDir = 0;
    
    d = 2*dy - dx;
    
    xStep = ( ( xEnd - xStart ) == 0 ) ? 0 : ( ( ( xEnd - xStart ) > 0 ) ? 1 : -1 );
    yStep = ( ( yEnd - yStart ) == 0 ) ? 0 : ( ( ( yEnd - yStart ) > 0 ) ? 1 : -1 );
    
    incrS = 2 * dy;
    incrD = 2 * ( dy - dx );
    
    Denom = sqrt( dx*dx + dy*dy );
    sin_a = dy / Denom; cos_a = dx / Denom;
    smc_a = sin_a - cos_a;
    
    i = 0;
    
    while ( i < dx )
    {
        if ( yDir )
        {
            if ( xStep == 0 )
            {
                setAndColorPixel( x, y, D, lineColor ); // current pixel
            }
            else
            {
                setAndColorPixel( x, y, D, lineColor ); // current pixel
                setAndColorPixel( x+xStep, y, D-cos_a, lineColor ); // West or East
                setAndColorPixel( x-xStep, y, D+cos_a, lineColor ); // East or West
            }
            
            
            y += yStep;
        }
        else
        {
            if ( yStep == 0 )
            {
                setAndColorPixel( x, y, D, lineColor ); // current pixel
            }
            else
            {
                setAndColorPixel( x, y, D, lineColor ); // current pixel
                setAndColorPixel( x, y+yStep, D-cos_a, lineColor ); // North or South
                setAndColorPixel( x, y-yStep, D+cos_a, lineColor ); // South or North
            }
            
            x += xStep;
        }
        
        if (d<=0)
        {
            D += sin_a;
            d += incrS;
        }
        else
        {
            D += smc_a;
            d += incrD;
            
            if ( yDir )
                x += xStep;
            else
                y += yStep;
            
        }
        
        i++;
    }
    
    glFlush();
}

void setPixel( GLint x, GLint y )
{
    glBegin( GL_POINTS );
    glVertex2i( x, y );
    glEnd();
}

void setPixelf( GLfloat x, GLfloat y )
{
    glBegin( GL_POINTS );
    glVertex2f( x, y );
    glEnd();
}

void setAndColorPixel( GLint x, GLint y, double D, color pixColor )
{
    float d;
    color newPixColor;
    
    if ( D < 0 )
        d = -D;
    else
        d = D;
    
    newPixColor.r = pixColor.r * ( 1-d / 1.5 );
    newPixColor.g = pixColor.g * ( 1-d / 1.5 );
    newPixColor.b = pixColor.b * ( 1-d / 1.5 );
    
    glColor3f( newPixColor.r, newPixColor.g, newPixColor.b );
    setPixel(x,y);
}

void fillPolygon()
{
    int y;
    
    glColor3f( 1.0, 1.0, 0.0 );
    
    createGlobalEdgeTable();
    
    for ( y = yMin; y < yMax; y++ )
    {
        // move an edge list from the GET to the active edge list
        moveListFromGETtoAET( y );
        
        // search for entries that have yMax = y and remove them
        removeEdges( y );
        
        // sort entries by increasing x
        sortActiveEdgeList();
        
        // color pixels between pairs
        colorPixelsBetweenPairs( y );
        
        // for each edge in AET, calculate x + invSlope and update value
        updateXcoOrdinates();
        
    }
    
    glFlush();
    
    
}

void createGlobalEdgeTable()
{
    int size, i, index;
    edgeT *curr;
    edgeTableEntry *entry;
    
    yMin = getYmin();
    yMax = getYmax();
    
    size = abs( yMin ) + abs( yMax );
    
    globalEdgeTable = ( edgeTableEntry ** ) malloc( sizeof( edgeTableEntry * ) * ( size + 1 ) ); // creates for simplicity a large array.. each index corresponds to a scanline
    
    for ( i = 0; i < size + 1; i++ )
        globalEdgeTable[i] = NULL;
    
    for ( curr = head; curr != NULL; curr = curr->next )
    {
        if ( curr->yStart == curr->yEnd ) // don't include horizontal edges
            continue;
        else if ( curr->yStart < curr->yEnd )
            index = curr->yStart + abs( yMin );
        else
            index = curr->yEnd + abs( yMin );
        
        entry = ( edgeTableEntry * ) malloc( sizeof( edgeTableEntry ) );
        
        entry->yMax = ( curr->yStart > curr->yEnd ) ? curr->yStart : curr->yEnd;
        entry->xAtYmin = ( curr->yStart < curr->yEnd ) ? curr->xStart : curr->xEnd;
        entry->invSlope = 1.0 * ( curr->xEnd - curr->xStart ) / ( curr->yEnd - curr->yStart );
        entry->next = globalEdgeTable[ index ];
        
        globalEdgeTable[ index ] = entry;
        strcpy( globalEdgeTable[ index ]->id, curr->id );
        
    }
    
    displayGlobalEdgeTable( size );
}

void moveListFromGETtoAET( int y )
{
    edgeTableEntry *entry;
    
    if ( globalEdgeTable[ y + abs( yMin ) ] != NULL ) // if the GET bucket is empty, then skip
    {
        if ( root == NULL )
            root = globalEdgeTable[ y + abs( yMin ) ]; // if the AET is empty, then transfer the global edge entries to the active edge list
        else
        {
            // if AET isn't empty, then append the global edge entries to the active edge list
            for ( entry = root; entry->next != NULL; entry = entry->next );
            
            entry->next = globalEdgeTable[ y + abs( yMin ) ];
            
        }
    }
    
}

void removeEdges( int y )
{
    edgeTableEntry *curr, *prev, *temp;
    
    for ( prev = NULL, curr = root; curr != NULL; prev = curr, curr = curr->next )
    {
        if ( ( curr != NULL ) && ( curr->yMax == y ) )
        {
            if ( prev == NULL )
            {
                temp = curr->next;
                root = temp;
                free( curr );
                curr = root;
            }
            else
            {
                temp = curr->next;
                prev->next = temp;
                free( curr );
                curr = prev;
            }
            
            
        }
    }
}

void sortActiveEdgeList()
{
    edgeTableEntry *curr, *prev, *temp;
    int count = 0;
    int i;
    
    for ( curr = root; curr != NULL; curr = curr->next )
        count++;
    
    for ( i = 0; i < count; i++ )
    {
        for ( prev = NULL, curr = root; curr != NULL; prev = curr, curr = curr->next )
        {
            if ( ( curr->next != NULL ) && ( curr->xAtYmin > curr->next->xAtYmin ) )
            {
                if ( prev == NULL )
                {
                    temp = curr->next;
                    curr->next = curr->next->next;
                    temp->next = root;
                    root = temp;
                }
                else
                {
                    temp = curr->next;
                    curr->next = curr->next->next;
                    temp->next = curr;
                    prev->next = temp;
                }
                
            }
        }
    }
}

void colorPixelsBetweenPairs( int y )
{
    edgeTableEntry *entry;
    int x0, x1;
    
    for ( entry = root; entry != NULL; )
    {
        if ( entry->next == NULL )
            break;
        
        x0 = entry->xAtYmin;
        x1 = entry->next->xAtYmin;
        
        while ( x0 < x1 )
        {
            setPixelf( x0, y );
            x0++;
        }
        
        if ( entry->next->next != NULL )
            entry = entry->next->next;
        else
            break;
    }
}

void updateXcoOrdinates()
{
    edgeTableEntry *entry;
    
    for ( entry = root; entry != NULL; entry = entry->next )
        entry->xAtYmin += entry->invSlope;
}

void closePolygon()
{
    char id[3];
    
    // draw final edge
    bresenhamLine( xStart, yStart, xInitial, yInitial );
    
    id[0] = vertexName;
    id[1] = 'A';
    id[2] = '\0';
    
    // add the edge to the list of all polygon edges
    insertEdge( xStart, yStart, xInitial, yInitial, id );
    
    displayList();
    
    // disable drawing
    enableDraw = FALSE;
}


/**************************************************************
 *                  Utility Functions
 **************************************************************/

void destroyEdgeList()
{
    edgeT *curr, *temp;
    
    for ( curr = head; curr != NULL; )
    {
        temp = curr->next;
        free( curr );
        curr = temp;
    }
    
    head = NULL;
}

void destroyAET()
{
    edgeTableEntry *curr, *temp;
    
    for ( curr = root; curr != NULL; )
    {
        temp = curr->next;
        free( curr );
        curr = temp;
    }
    
    root = NULL;
}

void resetDefaultValues()
{
    destroyAET();
    free( globalEdgeTable );
    destroyEdgeList();
    
    xStart = 0; yStart = 0;
    xEnd = 0; yEnd = 0;
    xInitial = 0; yInitial = 0;
    
    vertexName = 'A';
    clickCount = 0;
    enableDraw = FALSE;
}

int getYmin()
{
    edgeT *curr;
    
    int min = yInitial; // sets an initial min value
    
    for ( curr = head ; curr != NULL; curr = curr->next )
    {
        if ( curr->yStart < min )
            min = curr->yStart;
    }
    
    return min;
}

int getYmax()
{
    edgeT *curr;
    
    int max = yInitial; // sets an initial max value
    
    for ( curr = head; curr != NULL; curr = curr->next )
    {
        if ( curr->yStart > max )
            max = curr->yStart;
    }
    
    return max;
}

void swap( GLint *x, GLint *y )
{
    GLint temp;
    
    temp = *x;
    *x = *y;
    *y = temp;
}

void insertEdge( int x0, int y0, int x1, int y1, char *id )
{
    edgeT *edge = ( edgeT * ) malloc( sizeof( edgeT ) );
    
    edge->xStart = x0;
    edge->yStart = y0;
    edge->xEnd = x1;
    edge->yEnd = y1;
    strcpy( edge->id, id );
    
    edge->next = head;
    head = edge;
    
}


void displayGlobalEdgeTable( int size )
{
    int i;
    edgeTableEntry *curr;
    
    printf( "\n========== Global Edge Table ==========\n" );
    
    for ( i = size - 1; i >= 0; i-- )
    {
        if ( globalEdgeTable[i] != NULL )
        {
            printf( "Scanline[%d]: ", i + yMin );
            
            for ( curr = globalEdgeTable[i]; curr != NULL; curr = curr->next )
                printf("%s ", curr->id );
            
            printf("\n");
        }
    }
    
    printf( "=======================================\n" );
    
}

void showInfo()
{
    printf( "Project 2b\n" );
    printf( "Created by:\n" );
    printf( "Orestis Mpakatsis 1658\n" );
    printf( "Orsalia Stamatiou 1666\n" );
    printf( "Ioanna Strati 1676\n" );
    printf( "Apostolos Tsaousis 1714\n" );
    
}

void displayList()
{
    edgeT *curr;
    
    printf( "\n========== List of edges ==========\n" );
    
    for ( curr = head; curr != NULL; curr = curr->next )
        printf("%s: X0: %d Y0: %d XN: %d YN: %d\n", curr->id, curr->xStart, curr->yStart, curr->xEnd, curr->yEnd );
    
    printf( "===================================\n" );
    
    printf( "\n========== Min/Max y values ==========\n" );
    printf( "Minimum y: %d\n", getYmin() );
    printf( "Maximum y: %d\n", getYmax() );
    printf( "======================================\n" );
    
}


