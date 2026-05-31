#include <iostream>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <GL/glut.h>
using namespace std;

void plot_circle_pixel(int x, int y, int xc, int yc){
    glBegin(GL_POINTS);
    glVertex2i(x + xc, y + yc);
    glEnd();
}

void mid_point_circle(int xc, int yc, int r){
    int x = 0;
    int y = r;

    float decision = 5.0f / 4.0f - (float)r;

    while (y >= x) {
        plot_circle_pixel( x,  y, xc, yc);
        plot_circle_pixel( x, -y, xc, yc);
        plot_circle_pixel(-x,  y, xc, yc);
        plot_circle_pixel(-x, -y, xc, yc);
        plot_circle_pixel( y,  x, xc, yc);
        plot_circle_pixel(-y,  x, xc, yc);
        plot_circle_pixel( y, -x, xc, yc);
        plot_circle_pixel(-y, -x, xc, yc);

        if (decision < 0) {
            x++;
            decision += 2.0f * x + 3.0f;
        } else {
            y--;
            x++;
            decision += 2.0f * (x - y) + 5.0f;
        }
    }
}

void draw_line_pixel(int x, int y){
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

void bresenham_line(int xf, int yf, int xs, int ys){
    int dx, dy, p, x, y, inc_x = 1, inc_y = 1;
    dx = xs - xf;
    dy = ys - yf;
    if(dx < 0) dx *= -1;
    if(dy < 0) dy *= -1;

    if(xs < xf) inc_x = -1;
    if(ys < yf) inc_y = -1;

    if(dx > dy){
        p = 2 * dy - dx;
        x = xf;
        y = yf;
        draw_line_pixel(x, y);
        for(int i = 0; i < dx; i++){
            if(p < 0){
                p += 2 * dy;
                x += inc_x;
            }
            else{
                p += 2 * dy - 2 * dx;
                x += inc_x;
                y += inc_y;
            }
            draw_line_pixel(x, y);
        }
    }
    else{
        p = 2 * dx - dy;
        x = xf;
        y = yf;
        draw_line_pixel(x, y);
        for(int i = 0; i < dy; i++){
            if(p < 0){
                p += 2 * dx;
                y += inc_y;
            }
            else{
                p += 2 * dx - 2 * dy;
                x += inc_x;
                y += inc_y;
            }
            draw_line_pixel(x, y);
        }
    }
}


int clockCenterX = 320;
int clockCenterY = 255;   // CHANGED: shifted up slightly (was 240) to make room for digital clock
int clockRadius  = 150;

float degToRad(float deg) {
    return (3.14159265f / 180.0f) * deg;
}


void draw_hand(int cx, int cy, float angleDeg, float length) {
    float rad = degToRad(angleDeg);

    int x0 = cx;
    int y0 = cy;

    int x1 = cx + (int)(length * sin(rad));
    int y1 = cy + (int)(length * cos(rad));
    bresenham_line(x0, y0, x1, y1);
}

// ── CHANGE 1: gradient clock face ────────────────────────────────────────────
// Instead of a single flat glColor3f + draw_filled_circle, we fill the face
// with GL_TRIANGLE_FAN using per-vertex colors.  The center vertex is set to
// the "blue" end (0.18, 0.55, 0.90) and every perimeter vertex is set to the
// "green" end (0.10, 0.80, 0.55).  OpenGL interpolates smoothly between them,
// producing a radial blue-to-green gradient across the face.
// ─────────────────────────────────────────────────────────────────────────────
void draw_gradient_face(int cx, int cy, int r) {
    // center color: deep sky-blue
    float cR = 0.18f, cG = 0.55f, cB = 0.90f;
    // edge color: teal-green
    float eR = 0.10f, eG = 0.80f, eB = 0.55f;

    glBegin(GL_TRIANGLE_FAN);
        glColor3f(cR, cG, cB);                        // center vertex = blue
        glVertex2f((float)cx, (float)cy);
        for (int a = 0; a <= 360; a++) {
            float rad = degToRad((float)a);
            glColor3f(eR, eG, eB);                    // perimeter vertices = green
            glVertex2f(cx + r * cos(rad), cy + r * sin(rad));
        }
    glEnd();
}

void draw_filled_circle(int cx, int cy, int r) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f((float)cx, (float)cy);
    for (int a = 0; a <= 360; a++) {
        float rad = degToRad((float)a);
        float x = cx + r * cos(rad);
        float y = cy + r * sin(rad);
        glVertex2f(x, y);
    }
    glEnd();
}

void draw_ticks() {
    for (int i = 0; i < 60; i++) {
        float angle = (float)i * 6.0f;
        float rad = degToRad(angle);

        int xOuter = clockCenterX + (int)((clockRadius - 5) * sin(rad));
        int yOuter = clockCenterY + (int)((clockRadius - 5) * cos(rad));

        int tickLen = (i % 5 == 0) ? 20 : 6;
        int xInner = clockCenterX + (int)((clockRadius - 5 - tickLen) * sin(rad));
        int yInner = clockCenterY + (int)((clockRadius - 5 - tickLen) * cos(rad));

        glPointSize((i % 5 == 0) ? 4.0f : 1.5f);

        bresenham_line(xInner, yInner, xOuter, yOuter);
    }
}

void get_time_angles(float &hourAngle, float &minuteAngle, float &secondAngle) {
    std::time_t t = std::time(nullptr);
    std::tm *now = std::localtime(&t);

    int hr = now->tm_hour % 12;
    int mn = now->tm_min;
    int sc = now->tm_sec;

    secondAngle = sc * 6.0f;
    minuteAngle = mn * 6.0f + (sc / 60.0f) * 6.0f;
    hourAngle   = hr * 30.0f + (mn / 60.0f) * 30.0f;
}

// ── CHANGE 2: draw_digital_clock ─────────────────────────────────────────────
// NEW function.  Reads the current wall-clock time and renders it as a
// "HH:MM:SS" string using glutBitmapCharacter with GLUT_BITMAP_HELVETICA_18.
// It is drawn centered horizontally below the analog face.
// The label "DIGITAL" is drawn above in a smaller font for polish.
// ─────────────────────────────────────────────────────────────────────────────
void draw_digital_clock() {
    std::time_t t = std::time(nullptr);
    std::tm *now  = std::localtime(&t);

    // format as HH:MM:SS  (24-hour)
    char timeStr[16];
    std::snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d",
                  now->tm_hour, now->tm_min, now->tm_sec);

    // ── measure string width so we can center it ──────────────────────────────
    int timeWidth = 0;
    for (int i = 0; timeStr[i] != '\0'; i++)
        timeWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_18, timeStr[i]);

    // ── "DIGITAL" label in smaller font, centered above the time string ───────
    const char *label = "DIGITAL";
    int labelWidth = 0;
    for (int i = 0; label[i] != '\0'; i++)
        labelWidth += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, label[i]);

    int digitalY     = clockCenterY - clockRadius - 28;  // below the clock face
    int digitalLabelY = digitalY + 18;                    // "DIGITAL" sits above time

    // draw "DIGITAL" label — muted teal-white
    glColor3f(0.60f, 0.90f, 0.80f);
    glRasterPos2i(clockCenterX - labelWidth / 2, digitalLabelY);
    for (int i = 0; label[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, label[i]);

    // draw the time string — bright white with a slight blue tint
    glColor3f(0.92f, 0.98f, 1.00f);
    glRasterPos2i(clockCenterX - timeWidth / 2, digitalY);
    for (int i = 0; timeStr[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, timeStr[i]);
}

void draw_clock() {

    // ── CHANGED: use gradient face instead of flat filled circle ─────────────
    draw_gradient_face(clockCenterX, clockCenterY, clockRadius - 1);

    // ── outer bezel ───────────────────────────────────────────────────────────
    glColor3f(0.18f, 0.20f, 0.25f);
    glPointSize(3.5f);
    mid_point_circle(clockCenterX, clockCenterY, clockRadius);

    // ── inner bezel ring for depth ────────────────────────────────────────────
    glColor3f(0.30f, 0.32f, 0.38f);
    glPointSize(2.0f);
    mid_point_circle(clockCenterX, clockCenterY, clockRadius - 3);

    // ── tick marks: bright white so they read on the gradient ─────────────────
    glColor3f(1.0f, 1.0f, 1.0f);          // CHANGED: was dark slate; white on gradient
    draw_ticks();

    // ── center pivot ring ─────────────────────────────────────────────────────
    glColor3f(0.95f, 0.95f, 0.95f);       // CHANGED: white to stand out on gradient
    glPointSize(3.0f);
    mid_point_circle(clockCenterX, clockCenterY, 7);

    float hourAngle, minuteAngle, secondAngle;
    get_time_angles(hourAngle, minuteAngle, secondAngle);

    // ── hour hand: white ──────────────────────────────────────────────────────
    glColor3f(1.0f, 1.0f, 1.0f);          // CHANGED: white on gradient
    glPointSize(5.0f);
    draw_hand(clockCenterX, clockCenterY, hourAngle, clockRadius * 0.50f);

    // ── minute hand: white ────────────────────────────────────────────────────
    glColor3f(1.0f, 1.0f, 1.0f);          // CHANGED: white on gradient
    glPointSize(3.5f);
    draw_hand(clockCenterX, clockCenterY, minuteAngle, clockRadius * 0.72f);

    // ── second hand: gold/amber ───────────────────────────────────────────────
    glColor3f(0.82f, 0.62f, 0.10f);
    glPointSize(2.0f);
    draw_hand(clockCenterX, clockCenterY, secondAngle, clockRadius * 0.88f);

    // ── center cap: gold ──────────────────────────────────────────────────────
    glColor3f(0.82f, 0.62f, 0.10f);
    draw_filled_circle(clockCenterX, clockCenterY, 5);

    // ── CHANGE 2: draw the digital clock below the analog face ────────────────
    draw_digital_clock();
}

void timerFunc(int value) {
    glutPostRedisplay();
    glutTimerFunc(33, timerFunc, 0);
}

void myInit(void){
    glClearColor(0.11f, 0.12f, 0.16f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 640.0, 0.0, 480.0);
}

void myDisplay(void){
    glClear(GL_COLOR_BUFFER_BIT);
    draw_clock();
    glFlush();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 150);
    glutCreateWindow("Analog clock assignment");

    myInit();
    glutDisplayFunc(myDisplay);
    glutTimerFunc(0, timerFunc, 0);

    glutMainLoop();
    return 0;
}
