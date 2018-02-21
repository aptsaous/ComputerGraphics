#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

/**************************************************************
 *                  Constants
 **************************************************************/

#define INFO 0
#define CLEAR 1
#define INPUT 2
#define COLOR 3
#define KEYBOARD_L 5
#define KEYBOARD_E 6

#define ENTER_KEY 13
#define ESC_KEY 27

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
color startColor, endColor, ellipseColor; // color for line and ellipse
boolean drawLine; // if TRUE draws line, else draws ellipse
boolean byMouse; // if TRUE input method is mouse, else is keyboard
boolean isResized = FALSE; // checks if window has been re-sized
char data[ 10 ]; // array that stores input data from keyboard

/**************************************************************
 *                  Function prototypes
 **************************************************************/

void changeSize( int, int );
void setup();
void drawCoordinateAxes();
void display();
void keyboardInfoLine();
void keyboardInfoEllipse();
void showInfo();
void menuEvent( int );
void methodMenu( int );
void setStartColor( int );
void setEndColor( int );
void setEllipseColor( int );
void setPixel( GLint, GLint );
void setAndColorPixel( GLint, GLint, double, color );
void swap( GLint *, GLint * );
color *getGradientColor( color, color, int );
void bresenhamLine( GLint, GLint, GLint, GLint );
void bresenhamEllipse( GLint, GLint, GLint, GLint );
void keyEvent( unsigned char, int, int );
void mouseEvent( int, int, int, int );

/**************************************************************
 *                  Start of program
 **************************************************************/

int main( int argc, char *argv[] )
{
    // Set defaults values for width and height
    width = DEFAULT_WIDTH;
    height = DEFAULT_HEIGHT;
    
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_SINGLE | GLUT_RGB );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( width, height );
    glutCreateWindow( "Project 1" );
    
    glutDisplayFunc( display );
    glutReshapeFunc( changeSize );
    glutMouseFunc( mouseEvent );
    glutKeyboardFunc( keyEvent );
    
    setup();
    
    glutMainLoop();
    
    return 0;
}

/**************************************************************
 *                  Event handlers
 **************************************************************/

void keyEvent( unsigned char key, int xx, int yy )
{
    char digit[2]; // string that stores a digit entered by the user
    static int i = 0;
    static GLint x[2], y[2]; // x-y coords for start and end points of line
    static GLint Rx, Ry; // x-y radius for ellipse
    
    switch ( key )
    {
        case ESC_KEY:
            exit(0);
            break;
            
        default:
            break;
    }
    
    if ( drawLine && !byMouse )
    {
        if ( key == ENTER_KEY )
        {
            if ( i == 0 ) x[i] = atoi( data );
            else if ( i == 1 ) y[i-1] = atoi( data );
            else if ( i == 2 ) x[i-1] = atoi( data );
            else if ( i == 3 ) y[i-2] = atoi( data );
            else if ( i == 4 ) startColor.r = atof( data );
            else if ( i == 5 ) startColor.g = atof( data );
            else if ( i == 6 ) startColor.b = atof( data );
            else if ( i == 7 ) endColor.r = atof( data );
            else if ( i == 8 ) endColor.g = atof( data );
            else if ( i == 9 ) endColor.b = atof( data );
            
            printf( "xStart: %d yStart: %d xEnd: %d yEnd: %d RGB start: %.2f %.2f %.2f RGB end: %.2f %.2f %.2f\n",
                    x[0], y[0], x[1], y[1], startColor.r, startColor.g, startColor.b, endColor.r, endColor.g, endColor.b  );
            
            i++;
            data[0] = '\0';
            
            if ( i == 10 )
            {
                bresenhamLine( x[0], y[0], x[1], y[1] );
                i = 0;
            }
        }
        
        digit[0] = key;
        digit[1] = '\0';
        strcat( data, digit );
    }
    else if ( !drawLine && !byMouse )
    {
        if ( key == ENTER_KEY )
        {
            if ( i == 0 ) x[i] = atoi( data );
            else if ( i == 1 ) y[i-1] = atoi( data );
            else if ( i == 2 ) Rx = atoi( data );
            else if ( i == 3 ) Ry = atoi( data );
            else if ( i == 4 ) ellipseColor.r = atof( data );
            else if ( i == 5 ) ellipseColor.g = atof( data );
            else if ( i == 6 ) ellipseColor.b = atof( data );
            
            printf( "xCenter: %d yCenter: %d xRadius: %d yRadius: %d RGB: %.2f %.2f %.2f\n",
                   x[0], y[0], Rx, Ry, ellipseColor.r, ellipseColor.g, ellipseColor.b );
            
            i++;
            data[0] = '\0';
            
            if ( i == 7 )
            {
                bresenhamEllipse( x[0], y[0], Rx, Ry );
                i = 0;
            }
        }
        
        digit[0] = key;
        digit[1] = '\0';
        strcat( data, digit );
    }
    
}

void mouseEvent( int button, int state, int x, int y )
{
    static int lineClickCount = 0;
    static int ellipseClickCount = 0;
    static GLint xStart, yStart, xEnd, yEnd; // co-ordinates for the line
    static GLint xCenter, yCenter, Rx, Ry; // co-ordinates for the ellipse
    
    if ( ( button == GLUT_LEFT_BUTTON ) && ( state == GLUT_DOWN ) )
    {
        if ( drawLine && byMouse )
        {
            ellipseClickCount = 0;
            
            if ( ( lineClickCount % 2 ) == 0 )
            {
                xStart = x;
                yStart = y;
                
                xStart -= ( width / 2 );
                
                if ( yStart <= ( height / 2 ) )
                    yStart = ( height / 2 ) - yStart;
                else
                    yStart = -( yStart - ( height / 2 ) );
                
                glColor3f( startColor.r, startColor.g, startColor.b );
                setPixel( xStart, yStart );
                glFlush();
            }
            else
            {
                xEnd = x;
                yEnd = y;
                
                xEnd -= ( width / 2 );
                
                if ( yEnd <= ( height / 2 ) )
                    yEnd = ( height / 2 ) - yEnd;
                else
                    yEnd = -( yEnd - ( height / 2 ) );
                
                bresenhamLine( xStart, yStart, xEnd, yEnd );
            }
            
            lineClickCount++;
        }
        else if ( !drawLine && byMouse )
        {
            lineClickCount = 0;
            
            if ( ( ellipseClickCount % 3 ) == 0 )
            {
                xCenter = x;
                yCenter = y;
                
                xCenter -= ( width / 2 );
                
                if ( yCenter <= ( height / 2 ) )
                    yCenter = ( height / 2 ) - yCenter;
                else
                    yCenter = -( yCenter - ( height / 2 ) );
                
                setPixel( xCenter, yCenter );
                glFlush();
                
            }
            else if ( ( ellipseClickCount % 3 ) == 1 )
            {
                Rx = x;
                Rx -= ( width / 2 );
            }
            else
            {
                Ry = y;
                
                if ( Ry <= ( height / 2 ) )
                    Ry = ( height / 2 ) - Ry;
                else
                    Ry = -( Ry - ( height / 2 ) );
                
                bresenhamEllipse( xCenter, yCenter, Rx, Ry );
            }
            
            ellipseClickCount++;
        }
    }
}

void menuEvent( int selection )
{
    switch ( selection )
    {
        case INFO:
            showInfo();
            break;
        case CLEAR:
            glClear( GL_COLOR_BUFFER_BIT );
            drawCoordinateAxes();
            break;
            
        default:
            break;
    }
}

void methodMenu( int selection )
{
    
    switch ( selection )
    {
        case KEYBOARD_L:
            byMouse = FALSE;
            drawLine = TRUE;
            keyboardInfoLine();
            break;
        case KEYBOARD_E:
            byMouse = FALSE;
            drawLine = FALSE;
            keyboardInfoEllipse();
            break;
            
        default:
            break;
    }
}

/**************************************************************
 *                  Drawing Algorithms
 **************************************************************/

void bresenhamLine( GLint xStart, GLint yStart, GLint xEnd, GLint yEnd )
{
    
    int dx, dy;
    int incrS, incrD, p; // bresenham decision variables
    int x, y, xStep, yStep, i;
    boolean yDir; // true if line goes more to the y direction (dy larger than dx)
    float D = 0; // distance from the center of the pixel to the line
    float cosA, sinA, smcA, Denom; // variables to calculate the distance of pixels with the line
    color *colors;
    
    dy = abs( yEnd - yStart );
    dx = abs( xEnd - xStart );
    
    x = xStart;
    y = yStart;
    
    if ( dy > dx )
    {
        swap( &dx, &dy );
        yDir = TRUE;
        
    }
    else
        yDir = FALSE;
    
    colors = getGradientColor( startColor, endColor, dx ); // sets up gradient colors
    
    p = 2*dy - dx;
    
    xStep = ( ( xEnd - xStart ) == 0 ) ? 0 : ( ( ( xEnd - xStart ) > 0 ) ? 1 : -1 );
    yStep = ( ( yEnd - yStart ) == 0 ) ? 0 : ( ( ( yEnd - yStart ) > 0 ) ? 1 : -1 );
    
    incrS = 2 * dy;
    incrD = 2 * ( dy - dx );
    
    // calculates distance and angles for applying anti-aliasing
    Denom = sqrt( dx*dx + dy*dy );
    sinA = dy / Denom; 
    cosA = dx / Denom;
    smcA = sinA - cosA;
    
    i = 0;
    
    while ( i < dx )
    {
        if ( yDir )
        {
            if ( xStep == 0 )
            {
                setAndColorPixel( x, y, D, colors[i] ); // current pixel
            }
            else
            {
                setAndColorPixel( x, y, D, colors[i] ); // current pixel
                setAndColorPixel( x + xStep, y, D - cosA, colors[i] ); // West or East
                setAndColorPixel( x - xStep, y, D + cosA, colors[i] ); // East or West
            }
            
            
            y += yStep;
        }
        else
        {
            if ( yStep == 0 )
            {
                setAndColorPixel( x, y, D, colors[i] ); // current pixel
            }
            else
            {
                setAndColorPixel( x, y, D, colors[i] ); // current pixel
                setAndColorPixel( x, y + yStep, D - cosA, colors[i] ); // North or South
                setAndColorPixel( x, y - yStep, D + cosA, colors[i] ); // South or North
            }
            
            
            x += xStep;
        }
        
        if ( p <= 0 )
        {
            D += sinA;
            p += incrS;
        }
        else
        {
            D += smcA;
            p += incrD;
            
            if ( yDir )
                x += xStep;
            else
                y += yStep;
            
        }
        
        i++;
    }
    
    glFlush();
}

void bresenhamEllipse( GLint xCenter, GLint yCenter, GLint xRadius, GLint yRadius )
{
    long xRadius2, yRadius2; // x^2, y^2
    
    xRadius2 = xRadius * xRadius;
    yRadius2 = yRadius * yRadius;
    
    int dx = 0;
    int dy = abs( yRadius );
    
    long p = yRadius2 - ( 2*dy - 1 ) * xRadius2;
    long p2;
    
    glColor3f( ellipseColor.r, ellipseColor.g, ellipseColor.b );
    
    do
    {
        setPixel( xCenter + dx, yCenter + dy );
        setPixel( xCenter - dx, yCenter + dy );
        setPixel( xCenter - dx, yCenter - dy );
        setPixel( xCenter + dx, yCenter - dy );
        
        p2 = 2*p;
        
        if ( p2 < ( 2*dx + 1 ) * yRadius2 )
        {
            dx++;
            p += ( 2*dx + 1 ) * yRadius2;
        }
        
        if ( p2 > -( 2*dy - 1 ) * xRadius2 )
        {
            dy--;
            p -= ( 2*dy - 1 ) * xRadius2;
        }
        
    } while ( dy >= 0 );
    
    while ( dx++ < abs( xRadius ) )
    {
        setPixel( xCenter + dx, yCenter );
        setPixel( xCenter - dx, yCenter );
    }
    
    glFlush();
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
    setPixel( x, y );
}

void drawCoordinateAxes()
{
    glColor3f( 1.0, 1.0, 1.0 ); // set axis color to white
    
    glBegin( GL_LINES );
    glVertex3f( -( width / 2.0 ), 0.0, 0.0 );
    glVertex3f( (width / 2.0 ), 0.0, 0.0 );
    glVertex3f( 0.0, -( height / 2.0 ), 0.0 );
    glVertex3f( 0.0, ( height / 2.0 ), 0.0 );
    glEnd();
    
    glBegin( GL_LINE_STRIP );
    glVertex3f( -( width / 2.0 ) * 0.95, -( height * 0.02 ), 0.0 );
    glVertex3f( -( width / 2.0 ), 0.0, 0.0 );
    glVertex3f( -( width / 2.0 ) * 0.95, ( height * 0.02 ), 0.0 );
    glEnd();
    
    glBegin( GL_LINE_STRIP );
    glVertex3f( ( width / 2.0 ) * 0.95, -( height * 0.02 ), 0.0 );
    glVertex3f( ( width / 2.0 ), 0.0, 0.0 );
    glVertex3f( ( width / 2.0 ) * 0.95, ( height * 0.02 ), 0.0 );
    glEnd();
    
    glBegin( GL_LINE_STRIP );
    glVertex3f( -( width * 0.02 ), -( height / 2.0 ) * 0.95, 0.0 );
    glVertex3f( 0.0, -( height / 2.0 ), 0.0 );
    glVertex3f( ( width * 0.02 ), -( height / 2.0 ) * 0.95, 0.0 );
    glEnd();
    
    glBegin( GL_LINE_STRIP );
    glVertex3f( -( width * 0.02 ), ( height / 2.0 ) * 0.95, 0.0 );
    glVertex3f( 0.0, ( height / 2.0 ), 0.0 );
    glVertex3f( ( width * 0.02 ), ( height / 2.0 ) * 0.95, 0.0 );
    glEnd();
    
    glFlush();
}


/**************************************************************
 *                  GLUT initialization functions
 **************************************************************/

void setup()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // set clear color to black
    
    setStartColor( LIME ); // set default color of the line's starting point to lime
    setEndColor( PURPLE ); // set default color of the line's ending point to purple
    
    
    int startColor = glutCreateMenu( setStartColor );
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
    
    int endColor = glutCreateMenu( setEndColor );
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
    
    int mouseLine = glutCreateMenu( NULL );
    glutAddMenuEntry( "Color", COLOR );
    glutAddMenuEntry( "-----------------", COLOR );
    glutAddSubMenu( "Start Point", startColor );
    glutAddSubMenu( "End Point", endColor );
    
    int mouseEllipse = glutCreateMenu( setEllipseColor );
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
    
    int line = glutCreateMenu( methodMenu );
    glutAddMenuEntry( "Input Method", INPUT );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddSubMenu( "Mouse", mouseLine );
    glutAddMenuEntry( "Keyboard", KEYBOARD_L );
    
    int ellipse = glutCreateMenu( methodMenu );
    glutAddMenuEntry( "Input Method", INPUT );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddSubMenu( "Mouse", mouseEllipse );
    glutAddMenuEntry( "Keyboard", KEYBOARD_E );
    
    glutCreateMenu( menuEvent );
    glutAddMenuEntry( "Project 1", INFO );
    glutAddMenuEntry( "-----------------", INFO );
    glutAddSubMenu( "Bresenham Line", line );
    glutAddSubMenu( "Bresenham Ellipse", ellipse );
    glutAddMenuEntry( "Clear Screen", CLEAR );
    
    glutAttachMenu( GLUT_RIGHT_BUTTON );
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

void display()
{
    if ( isResized == TRUE )
    {
        glClear( GL_COLOR_BUFFER_BIT ); // clear screen with default clear color
        
        drawCoordinateAxes();
        
        isResized = FALSE;
        
    }
}

/**************************************************************
 *                  Utility Functions
 **************************************************************/

void setStartColor( int selection )
{
    drawLine = TRUE; // disable ellipse drawing and enable line drawing
    byMouse = TRUE;
    
    switch ( selection )
    {
        case BLACK:
            startColor.r = 0.0; startColor.g = 0.0; startColor.b = 0.0;
            break;
        case BLUE:
            startColor.r = 0.0; startColor.g = 0.0; startColor.b = 255.0;
            break;
        case CYAN:
            startColor.r = 0.0; startColor.g = 255.0; startColor.b = 255.0;
            break;
        case GRAY:
            startColor.r = 128.0; startColor.g = 128.0; startColor.b = 128.0;
            break;
        case GREEN:
            startColor.r = 0.0; startColor.g = 128.0; startColor.b = 0.0;
            break;
        case LIME:
            startColor.r = 0.0; startColor.g = 255.0; startColor.b = 0.0;
            break;
        case MAGENTA:
            startColor.r = 255.0; startColor.g = 255.0; startColor.b = 255.0;
            break;
        case MAROON:
            startColor.r = 128.0; startColor.g = 0.0; startColor.b = 0.0;
            break;
        case NAVY:
            startColor.r = 0.0; startColor.g = 0.0; startColor.b = 128.0;
            break;
        case OLIVE:
            startColor.r = 128.0; startColor.g = 128.0; startColor.b = 0.0;
            break;
        case PURPLE:
            startColor.r = 128.0; startColor.g = 0.0; startColor.b = 128.0;
            break;
        case RED:
            startColor.r = 255.0; startColor.g = 0.0; startColor.b = 0.0;
            break;
        case SILVER:
            startColor.r = 192.0; startColor.g = 192.0; startColor.b = 192.0;
            break;
        case TEAL:
            startColor.r = 0.0; startColor.g = 128.0; startColor.b = 128.0;
            break;
        case YELLOW:
            startColor.r = 255.0; startColor.g = 255.0; startColor.b = 0.0;
            break;
        case WHITE:
            startColor.r = 255.0; startColor.g = 255.0; startColor.b = 255.0;
            break;
            
        default:
            break;
    }
}

void setEndColor( int selection )
{
    drawLine = TRUE; // disable ellipse drawing and enable line drawing
    byMouse = TRUE;
    
    switch ( selection )
    {
        case BLACK:
            endColor.r = 0.0; endColor.g = 0.0; endColor.b = 0.0;
            break;
        case BLUE:
            endColor.r = 0.0; endColor.g = 0.0; endColor.b = 255.0;
            break;
        case CYAN:
            endColor.r = 0.0; endColor.g = 255.0; endColor.b = 255.0;
            break;
        case GRAY:
            endColor.r = 128.0; endColor.g = 128.0; endColor.b = 128.0;
            break;
        case GREEN:
            endColor.r = 0.0; endColor.g = 128.0; endColor.b = 0.0;
            break;
        case LIME:
            endColor.r = 0.0; endColor.g = 255.0; endColor.b = 0.0;
            break;
        case MAGENTA:
            endColor.r = 255.0; endColor.g = 255.0; endColor.b = 255.0;
            break;
        case MAROON:
            endColor.r = 128.0; endColor.g = 0.0; endColor.b = 0.0;
            break;
        case NAVY:
            endColor.r = 0.0; endColor.g = 0.0; endColor.b = 128.0;
            break;
        case OLIVE:
            endColor.r = 128.0; endColor.g = 128.0; endColor.b = 0.0;
            break;
        case PURPLE:
            endColor.r = 128.0; endColor.g = 0.0; endColor.b = 128.0;
            break;
        case RED:
            endColor.r = 255.0; endColor.g = 0.0; endColor.b = 0.0;
            break;
        case SILVER:
            endColor.r = 192.0; endColor.g = 192.0; endColor.b = 192.0;
            break;
        case TEAL:
            endColor.r = 0.0; endColor.g = 128.0; endColor.b = 128.0;
            break;
        case YELLOW:
            endColor.r = 255.0; endColor.g = 255.0; endColor.b = 0.0;
            break;
        case WHITE:
            endColor.r = 255.0; endColor.g = 255.0; endColor.b = 255.0;
            break;
            
        default:
            break;
    }
}

void setEllipseColor( int selection )
{
    drawLine = FALSE; // disable line drawing and enable ellipse drawing
    byMouse = TRUE;
    
    switch ( selection )
    {
        case BLACK:
            ellipseColor.r = 0.0; ellipseColor.g = 0.0; ellipseColor.b = 0.0;
            break;
        case BLUE:
            ellipseColor.r = 0.0; ellipseColor.g = 0.0; ellipseColor.b = 255.0;
            break;
        case CYAN:
            ellipseColor.r = 0.0; ellipseColor.g = 255.0; ellipseColor.b = 255.0;
            break;
        case GRAY:
            ellipseColor.r = 128.0; ellipseColor.g = 128.0; ellipseColor.b = 128.0;
            break;
        case GREEN:
            ellipseColor.r = 0.0; ellipseColor.g = 128.0; ellipseColor.b = 0.0;
            break;
        case LIME:
            ellipseColor.r = 0.0; ellipseColor.g = 255.0; ellipseColor.b = 0.0;
            break;
        case MAGENTA:
            ellipseColor.r = 255.0; ellipseColor.g = 255.0; ellipseColor.b = 255.0;
            break;
        case MAROON:
            ellipseColor.r = 128.0; ellipseColor.g = 0.0; ellipseColor.b = 0.0;
            break;
        case NAVY:
            ellipseColor.r = 0.0; ellipseColor.g = 0.0; ellipseColor.b = 128.0;
            break;
        case OLIVE:
            ellipseColor.r = 128.0; ellipseColor.g = 128.0; ellipseColor.b = 0.0;
            break;
        case PURPLE:
            ellipseColor.r = 128.0; ellipseColor.g = 0.0; ellipseColor.b = 128.0;
            break;
        case RED:
            ellipseColor.r = 255.0; ellipseColor.g = 0.0; ellipseColor.b = 0.0;
            break;
        case SILVER:
            ellipseColor.r = 192.0; ellipseColor.g = 192.0; ellipseColor.b = 192.0;
            break;
        case TEAL:
            ellipseColor.r = 0.0; ellipseColor.g = 128.0; ellipseColor.b = 128.0;
            break;
        case YELLOW:
            ellipseColor.r = 255.0; ellipseColor.g = 255.0; ellipseColor.b = 0.0;
            break;
        case WHITE:
            ellipseColor.r = 255.0; ellipseColor.g = 255.0; ellipseColor.b = 255.0;
            break;
            
        default:
            break;
    }
}

void setPixel( GLint x, GLint y )
{
    glBegin( GL_POINTS );
    glVertex2i( x, y );
    glEnd();
}

color *getGradientColor( color startColor, color endColor, int steps )
{
    int i;
    
    float rStep = ( endColor.r - startColor.r ) / steps;
    float gStep = ( endColor.g - startColor.g ) / steps;
    float bStep = ( endColor.b - startColor.b ) / steps;
    
    color *colorChart = ( color * ) malloc( sizeof( color ) * ( steps + 1 ) );
    
    colorChart[0].r = startColor.r;
    colorChart[0].g = startColor.g;
    colorChart[0].b = startColor.b;
    
    for ( i = 1; i < steps + 1; i++ )
    {
        
        colorChart[i].r = colorChart[i-1].r + rStep;
        colorChart[i].g = colorChart[i-1].g + gStep;
        colorChart[i].b = colorChart[i-1].b + bStep;
    }
    
    for ( i = 0; i < steps + 1; i++ )
    {
        colorChart[i].r = ( roundf( colorChart[i].r / 255 * 100 ) ) / 100;
        colorChart[i].g = ( roundf( colorChart[i].g / 255 * 100 ) ) / 100;
        colorChart[i].b = ( roundf( colorChart[i].b / 255 * 100 ) ) / 100;
    }
    
    return colorChart;
}

void keyboardInfoLine()
{
    printf( "Keyboard mode for drawing lines is enabled\n" );
    printf( "Usage is: x0 y0 x1 y1 c0.r c0.g c0.b c1.r c1.g c1.b\n" );
    printf( "x0: x-coordinate of start point (Range: -300:300)\n" );
    printf( "y0: y-coordinate of start point (Range: -300:300)\n" );
    printf( "x1: x-coordinate of end point (Range: -300:300)\n" );
    printf( "y1: y-coordinate of end point (Range: -300:300)\n" );
    printf( "c0.r: red color portion of start point (Range: 0-255)\n" );
    printf( "c0.g: green color portion of start point (Range: 0-255)\n" );
    printf( "c0.b: blue color portion of start point (Range: 0-255)\n" );
    printf( "c1.r: red color portion of end point (Range: 0-255)\n" );
    printf( "c1.g: green color portion of end point (Range: 0-255)\n" );
    printf( "c1.b: blue color portion of end point (Range: 0-255)\n" );
    
    
}

void keyboardInfoEllipse()
{
    printf( "Keyboard mode for drawing ellipses is enabled\n" );
    printf( "Usage is: xC yC Rx Ry c.r c.g c.b\n" );
    printf( "xC: x-coordinate of the center point (Range: -300:300)\n" );
    printf( "yC: y-coordinate of the center point (Range: -300:300)\n" );
    printf( "Rx: radius of x-axis (Range 0:300)\n" );
    printf( "Ry: radius of y-axis (Range 0:300)\n" );
    printf( "c.r: red color portion of the ellipse (Range: 0-255)\n" );
    printf( "c.g: green color portion of the ellipse (Range: 0-255)\n" );
    printf( "c.b: blue color portion of the ellipse (Range: 0-255)\n" );
}

void showInfo()
{
    printf( "Project 1\n" );
    printf( "Created by:\n" );
    printf( "Orestis Mpakatsis 1658\n" );
    printf( "Orsalia Stamatiou 1666\n" );
    printf( "Ioanna Strati 1676\n" );
    printf( "Apostolos Tsaousis 1714\n" );
    
}

void swap( GLint *x, GLint *y )
{
    GLint temp;
    
    temp = *x;
    *x = *y;
    *y = temp;
}

