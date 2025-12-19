#include "Angel.h"
#include "InitShader.cpp" // Including implementation for single-file compile convenience
#include <stack>
#include <vector>

//----------------------------------------------------------------------------
// Types and Constants
//----------------------------------------------------------------------------
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

const int NumVerticesCube = 36;
const int NumVerticesCylinder = 360; // Approximate
// We will generate a buffer large enough for all our primitives
// Let's use a simpler approach: Generate all vertices for Primitives once -> VBO
// Then use offsets to draw.

// Primitives:
// 1. Cube (Standard 1x1x1 centered at origin)
// 2. Cylinder (Radius 0.5, Height 1, centered)
// 3. Cone (Radius 0.5, Height 1)

// Geometry Data
std::vector<point4> points;
std::vector<vec3>   normals;
std::vector<color4> colors; // Optional if we use material uniforms

// Shader Uniform Locations
GLuint  ModelLoc, ViewLoc, ProjectionLoc;
GLuint  AmbientProductLoc, DiffuseProductLoc, SpecularProductLoc;
GLuint  LightPositionLoc, ShininessLoc;
GLuint  program;

// Viewing Parameters
GLfloat radius = 10.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.1, zFar = 100.0;

vec4 eye( 0.0, 10.0, 20.0, 1.0 );
vec4 at( 0.0, 0.0, 2.0, 1.0 );
vec4 up( 0.0, 1.0, 0.0, 0.0 );

// Lighting Parameters
point4 light_position( 10.0, 10.0, 10.0, 1.0 );
color4 light_ambient( 0.2, 0.2, 0.2, 1.0 );
color4 light_diffuse( 1.0, 1.0, 1.0, 1.0 );
color4 light_specular( 1.0, 1.0, 1.0, 1.0 );

color4 material_ambient( 1.0, 0.0, 1.0, 1.0 );
color4 material_diffuse( 1.0, 0.8, 0.0, 1.0 );
color4 material_specular( 1.0, 0.8, 0.0, 1.0 );
float  material_shininess = 100.0;

// Lighting Toggle
bool light_on = true;

// Control State
int selected_object = 0; // 0=None, 1-8=Planes
struct ObjectState {
    vec3 position; // x, y, z
    vec3 rotation; // pitch, yaw, roll (x, y, z)
    float propeller_angle;
    float propeller_speed;
    float aux_angle; // generic aux (e.g. wheels, door)
    bool  aux_state; // open/close
};

ObjectState planes[9]; // 1-based index (1..8)

// Global Time
float time_of_day = 12.0f; // 0-24
float rotation_global = 0.0f;

// Matrix Stack for Hierarchy
std::stack<mat4> mvstack;
mat4 model_view;
mat4 projection;
mat4 view_matrix;

// Primitive Offsets
int offset_cube = 0;
int count_cube = 0;
int offset_cyl = 0;
int count_cyl = 0;
int offset_cone = 0;
int count_cone = 0;

//----------------------------------------------------------------------------
// Geometry Generation
//----------------------------------------------------------------------------

// Quad function for Cube
void quad( int a, int b, int c, int d, const point4* vertices )
{
    vec3 u = vec3(vertices[b]-vertices[a]);
    vec3 v = vec3(vertices[c]-vertices[b]);
    vec3 normal = normalize( cross(u, v) );

    normals.push_back( normal ); points.push_back( vertices[a] ); colors.push_back(color4(1,0,0,1));
    normals.push_back( normal ); points.push_back( vertices[b] ); colors.push_back(color4(1,0,0,1));
    normals.push_back( normal ); points.push_back( vertices[c] ); colors.push_back(color4(1,0,0,1));
    normals.push_back( normal ); points.push_back( vertices[a] ); colors.push_back(color4(1,0,0,1));
    normals.push_back( normal ); points.push_back( vertices[c] ); colors.push_back(color4(1,0,0,1));
    normals.push_back( normal ); points.push_back( vertices[d] ); colors.push_back(color4(1,0,0,1));
}

void generateCube()
{
    offset_cube = points.size();
    point4 vertices[8] = {
	point4( -0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5,  0.5,  0.5, 1.0 ),
	point4(  0.5, -0.5,  0.5, 1.0 ),
	point4( -0.5, -0.5, -0.5, 1.0 ),
	point4( -0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5,  0.5, -0.5, 1.0 ),
	point4(  0.5, -0.5, -0.5, 1.0 )
    };

    quad( 1, 0, 3, 2, vertices );
    quad( 2, 3, 7, 6, vertices );
    quad( 3, 0, 4, 7, vertices );
    quad( 6, 5, 1, 2, vertices );
    quad( 4, 5, 6, 7, vertices );
    quad( 5, 4, 0, 1, vertices );

    count_cube = points.size() - offset_cube;
}

void generateCylinder()
{
    offset_cyl = points.size();
    const int segments = 32;
    float top_y = 0.5;
    float bot_y = -0.5;
    
    // Side
    for(int i=0; i<segments; ++i) {
        float theta1 = (float)i/segments * 2*M_PI;
        float theta2 = (float)(i+1)/segments * 2*M_PI;
        
        point4 p1(0.5*cos(theta1), top_y, 0.5*sin(theta1), 1.0);
        point4 p2(0.5*cos(theta1), bot_y, 0.5*sin(theta1), 1.0);
        point4 p3(0.5*cos(theta2), bot_y, 0.5*sin(theta2), 1.0);
        point4 p4(0.5*cos(theta2), top_y, 0.5*sin(theta2), 1.0);

        vec3 u = vec3(p2-p1);
        vec3 v = vec3(p3-p2);
        vec3 normal = normalize( cross(u, v) ); 
        // Actually for smooth shading cylinder normal is just (x,0,z)
        // But let's use flat normals for simplicity or compute smooth if requested.
        // User asked for "all geometry from triangles".
        // Let's use smooth normals for side.
        
        vec3 n1(cos(theta1), 0, sin(theta1));
        vec3 n2(cos(theta2), 0, sin(theta2));

        // Triangle 1
        points.push_back(p1); normals.push_back(n1); colors.push_back(color4(0,1,0,1));
        points.push_back(p2); normals.push_back(n1); colors.push_back(color4(0,1,0,1));
        points.push_back(p4); normals.push_back(n2); colors.push_back(color4(0,1,0,1));
        
        // Triangle 2
        points.push_back(p2); normals.push_back(n1); colors.push_back(color4(0,1,0,1));
        points.push_back(p3); normals.push_back(n2); colors.push_back(color4(0,1,0,1));
        points.push_back(p4); normals.push_back(n2); colors.push_back(color4(0,1,0,1));
    }
    
    // Caps logic omitted for brevity, but can be added if needed. 
    // Usually cylinder needs caps.
    // Adding Top Cap
    for(int i=0; i<segments; ++i) {
        float theta1 = (float)i/segments * 2*M_PI;
        float theta2 = (float)(i+1)/segments * 2*M_PI;
        point4 c(0, top_y, 0, 1);
        point4 p1(0.5*cos(theta1), top_y, 0.5*sin(theta1), 1.0);
        point4 p2(0.5*cos(theta2), top_y, 0.5*sin(theta2), 1.0);
        
        points.push_back(p1); normals.push_back(vec3(0,1,0)); colors.push_back(color4(0,1,0,1));
        points.push_back(c);  normals.push_back(vec3(0,1,0)); colors.push_back(color4(0,1,0,1));
        points.push_back(p2); normals.push_back(vec3(0,1,0)); colors.push_back(color4(0,1,0,1));
    }
    
    count_cyl = points.size() - offset_cyl;
}

void generateCone() {
    offset_cone = points.size();
    const int segments = 32;
    float h = 1.0f;
    float r = 0.5f;

    for (int i=0; i<segments; ++i) {
        float theta1 = (float)i/segments * 2*M_PI;
        float theta2 = (float)(i+1)/segments * 2*M_PI;

        point4 top(0, h/2, 0, 1.0); // Tip
        point4 p1(r*cos(theta1), -h/2, r*sin(theta1), 1.0);
        point4 p2(r*cos(theta2), -h/2, r*sin(theta2), 1.0);

        // Normal calculation (simplified)
        vec3 u = vec3(p1 - top);
        vec3 v = vec3(p2 - p1);
        vec3 n = normalize(cross(u, v));

        points.push_back(top); normals.push_back(n); colors.push_back(color4(0,0,1,1));
        points.push_back(p1);  normals.push_back(n); colors.push_back(color4(0,0,1,1));
        points.push_back(p2);  normals.push_back(n); colors.push_back(color4(0,0,1,1));

        // Base cap
        point4 center(0, -h/2, 0, 1.0);
        points.push_back(p1);    normals.push_back(vec3(0,-1,0)); colors.push_back(color4(0,0,1,1));
        points.push_back(center); normals.push_back(vec3(0,-1,0)); colors.push_back(color4(0,0,1,1));
        points.push_back(p2);    normals.push_back(vec3(0,-1,0)); colors.push_back(color4(0,0,1,1));
    }
    count_cone = points.size() - offset_cone;
}

//----------------------------------------------------------------------------
// Hierarchical Drawing Helper
//----------------------------------------------------------------------------

void SetMaterial(color4 amb, color4 diff, color4 spec, float shin) {
    glUniform4fv( AmbientProductLoc, 1, amb );
    glUniform4fv( DiffuseProductLoc, 1, diff );
    glUniform4fv( SpecularProductLoc, 1, spec );
    glUniform1f( ShininessLoc, shin );
}

void DrawCube(mat4 transform, color4 color) {
    SetMaterial(color*0.2, color, vec4(1,1,1,1), 50.0);
    glUniformMatrix4fv(ModelLoc, 1, GL_TRUE, transform);
    glDrawArrays(GL_TRIANGLES, offset_cube, count_cube);
}

void DrawCylinder(mat4 transform, color4 color) {
    SetMaterial(color*0.2, color, vec4(1,1,1,1), 50.0);
    glUniformMatrix4fv(ModelLoc, 1, GL_TRUE, transform);
    glDrawArrays(GL_TRIANGLES, offset_cyl, count_cyl);
}

void DrawCone(mat4 transform, color4 color) {
    SetMaterial(color*0.2, color, vec4(1,1,1,1), 50.0);
    glUniformMatrix4fv(ModelLoc, 1, GL_TRUE, transform);
    glDrawArrays(GL_TRIANGLES, offset_cone, count_cone);
}

//----------------------------------------------------------------------------
// Aircraft Hierarchical Models
//----------------------------------------------------------------------------

// 1. Toy Jet
void DrawJet(mat4 mt, int id) {
    ObjectState& s = planes[id];
    
    // Body
    mat4 m_body = mt * Scale(1.0, 1.0, 3.0);
    DrawCylinder(m_body, color4(0.8, 0.2, 0.2, 1.0));

    // Wings
    mat4 m_wing = mt * Translate(0, 0, 0) * Scale(4.0, 0.1, 1.0);
    DrawCube(m_wing, color4(0.6, 0.6, 0.6, 1.0));

    // Tail
    mat4 m_tail = mt * Translate(0, 0.5, 1.2) * RotateX(-45) * Scale(1.5, 0.1, 0.8);
    DrawCube(m_tail, color4(0.6, 0.6, 0.6, 1.0));

    // Engine Turbine (Rotatable)
    float rot = s.propeller_angle;
    mat4 m_eng_l = mt * Translate(-1.0, -0.2, 0.5) * RotateZ(rot) * Scale(0.3, 0.3, 1.0);
    DrawCylinder(m_eng_l, color4(0.2, 0.2, 0.2, 1.0));
    mat4 m_eng_r = mt * Translate(1.0, -0.2, 0.5) * RotateZ(rot) * Scale(0.3, 0.3, 1.0);
    DrawCylinder(m_eng_r, color4(0.2, 0.2, 0.2, 1.0));

    // Landing Gear (Retractable)
    if(s.aux_state) { // if open
        mat4 m_gear_f = mt * Translate(0, -0.8, -1.0) * Scale(0.1, 0.5, 0.1);
        DrawCube(m_gear_f, color4(0.1, 0.1, 0.1, 1));
        // Wheel
        mat4 m_wheel_f = mt * Translate(0, -1.0, -1.0) * RotateY(90) * Scale(0.3, 0.3, 0.1);
        DrawCylinder(m_wheel_f, color4(0,0,0,1));
    }
}

// 2. Propeller Plane
void DrawPropPlane(mat4 mt, int id) {
    ObjectState& s = planes[id];
    
    // Fuselage
    DrawCube(mt * Scale(1, 1, 2.5), color4(0.2, 0.8, 0.2, 1));

    // Propeller
    mat4 m_prop = mt * Translate(0, 0, -1.3) * RotateZ(s.propeller_angle); // Spinning
    DrawCube(m_prop * Scale(2.0, 0.1, 0.1), color4(0.1, 0.1, 0.1, 1));
    DrawCube(m_prop * Scale(0.1, 2.0, 0.1), color4(0.1, 0.1, 0.1, 1));

    // Wings
    DrawCube(mt * Translate(0, 0.2, -0.5) * Scale(3.5, 0.1, 0.8), color4(1, 1, 0, 1));
}

// 3. Helicopter
void DrawHelicopter(mat4 mt, int id) {
    ObjectState& s = planes[id];
    
    // Bubble cockpit
    DrawCylinder(mt * Scale(1.2, 1.2, 1.5), color4(0.2, 0.2, 0.8, 1));
    
    // Tail boom
    DrawCube(mt * Translate(0, 0, 1.5) * Scale(0.3, 0.3, 2.0), color4(0.5, 0.5, 0.5, 1));
    
    // Main Rotor
    mat4 m_rotor = mt * Translate(0, 0.7, 0) * RotateY(s.propeller_angle);
    DrawCube(m_rotor * Scale(4.0, 0.05, 0.2), color4(0.1, 0.1, 0.1, 1));
    DrawCube(m_rotor * RotateY(90) * Scale(4.0, 0.05, 0.2), color4(0.1, 0.1, 0.1, 1));

    // Tail Rotor
    mat4 m_tailrotor = mt * Translate(0.2, 0, 2.5) * RotateX(s.propeller_speed * 10); // Rotate fast
    DrawCube(m_tailrotor * Scale(0.05, 1.0, 0.1), color4(0.1,0.1,0.1,1));
}

// 4. Paper Plane
void DrawPaperPlane(mat4 mt, int id) {
    // Simple dart shape using scaling
    DrawCone(mt * RotateX(-90) * Scale(1.0, 2.0, 0.1), color4(1,1,1,1));
}

// 5. Drone
void DrawDrone(mat4 mt, int id) {
    ObjectState& s = planes[id];
    
    // Center Body
    DrawCube(mt * Scale(0.5, 0.2, 0.5), color4(0.1, 0.1, 0.1, 1));
    
    // Arms
    DrawCube(mt * RotateY(45) * Scale(2.0, 0.1, 0.1), color4(0.3, 0.3, 0.3, 1));
    DrawCube(mt * RotateY(-45) * Scale(2.0, 0.1, 0.1), color4(0.3, 0.3, 0.3, 1));

    // Props
    float rots[4] = {45, 135, 225, 315};
    for(int i=0; i<4; i++) {
        float r = 1.0;
        float x = r * cos(rots[i]*DegreesToRadians);
        float z = r * sin(rots[i]*DegreesToRadians);
        mat4 m_p = mt * Translate(x, 0.1, z) * RotateY(s.propeller_angle * (i%2==0?1:-1));
        DrawCylinder(m_p * Scale(0.4, 0.05, 0.4), color4(0,1,1,1)); // Propeller disc approximation
    }
}

// 6. Rocket
void DrawRocket(mat4 mt, int id) {
    DrawCylinder(mt * Scale(0.5, 2.0, 0.5), color4(0.9, 0.9, 0.9, 1)); // Body
    DrawCone(mt * Translate(0, 1.0, 0) * Scale(0.5, 0.8, 0.5), color4(1, 0, 0, 1)); // Nose
    // Fins
    DrawCube(mt * Translate(0, -0.8, 0) * Scale(1.5, 0.5, 0.1), color4(1,0,0,1));
    DrawCube(mt * Translate(0, -0.8, 0) * RotateY(90) * Scale(1.5, 0.5, 0.1), color4(1,0,0,1));
}

// 7. Balloon
void DrawBalloon(mat4 mt, int id) {
    // Balloon
    DrawCylinder(mt * Translate(0, 1.0, 0) * Scale(1.5, 1.8, 1.5), color4(1, 0.5, 0, 1)); // Use cyl as primitive sphere approximation
    // Basket
    DrawCube(mt * Translate(0, -0.5, 0) * Scale(0.5, 0.5, 0.5), color4(0.6, 0.4, 0.2, 1));
}

// 8. Fighter Jet
void DrawFighter(mat4 mt, int id) {
    ObjectState& s = planes[id];
    // Main Body
    DrawCube(mt * Scale(0.8, 0.5, 3.0), color4(0.3, 0.3, 0.4, 1));
    // Swept Wings
    DrawCube(mt * Translate(0,0,0.5) * Scale(3.0, 0.1, 1.5), color4(0.3, 0.3, 0.4, 1));
    // Missiles
    if (s.aux_state) { // Fire! (Simple translation)
       DrawCylinder(mt * Translate(1.0, -0.2, -1.0) * Scale(0.1, 0.1, 0.8), color4(1,1,1,1));
    } else {
       DrawCylinder(mt * Translate(1.0, -0.2, 0.0) * Scale(0.1, 0.1, 0.8), color4(1,1,1,1));
       DrawCylinder(mt * Translate(-1.0, -0.2, 0.0) * Scale(0.1, 0.1, 0.8), color4(1,1,1,1));
    }
}

// Environment
void DrawShop() {
    // Floor
    mat4 m_floor = Translate(0, -5, 0) * Scale(40, 0.1, 40);
    DrawCube(m_floor, color4(0.8, 0.7, 0.5, 1));

    // Shelves
    for(int i=-1; i<=1; i++) {
        mat4 m_shelf = Translate(i*8, -2, -10);
        // Base
        DrawCube(m_shelf * Scale(4, 6, 2), color4(0.4, 0.2, 0.0, 1));
        // Planks
        DrawCube(m_shelf * Translate(0, 1, 0) * Scale(4.2, 0.1, 2.1), color4(0.5, 0.25, 0.0, 1));
    }

    // Counter
    mat4 m_counter = Translate(10, -3.5, 5) * Scale(4, 3, 2);
    DrawCube(m_counter, color4(0.9, 0.9, 0.9, 1));
}

//----------------------------------------------------------------------------
// Initialization & Loop
//----------------------------------------------------------------------------

void init()
{
    generateCube();
    generateCylinder();
    generateCone();

    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    GLuint buffer;
    glGenBuffers( 1, &buffer );
    glBindBuffer( GL_ARRAY_BUFFER, buffer );
    glBufferData( GL_ARRAY_BUFFER, points.size()*sizeof(point4) + normals.size()*sizeof(vec3) + colors.size()*sizeof(color4),
		  NULL, GL_STATIC_DRAW );
    
    // Subdata
    int offset = 0;
    glBufferSubData( GL_ARRAY_BUFFER, offset, points.size()*sizeof(point4), &points[0] );
    offset += points.size()*sizeof(point4);
    glBufferSubData( GL_ARRAY_BUFFER, offset, normals.size()*sizeof(vec3), &normals[0] );
    offset += normals.size()*sizeof(vec3);
    glBufferSubData( GL_ARRAY_BUFFER, offset, colors.size()*sizeof(color4), &colors[0] );

    // Load shaders
    program = InitShader( "vshader.glsl", "fshader.glsl" );
    glUseProgram( program );

    // Set up vertex arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    GLuint vNormal = glGetAttribLocation( program, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(points.size()*sizeof(point4)) );
    
    // Uniforms
    ModelLoc = glGetUniformLocation( program, "Model" );
    ViewLoc = glGetUniformLocation( program, "View" );
    ProjectionLoc = glGetUniformLocation( program, "Projection" );
    
    AmbientProductLoc = glGetUniformLocation(program, "AmbientProduct");
    DiffuseProductLoc = glGetUniformLocation(program, "DiffuseProduct");
    SpecularProductLoc = glGetUniformLocation(program, "SpecularProduct");
    LightPositionLoc = glGetUniformLocation(program, "LightPosition");
    ShininessLoc = glGetUniformLocation(program, "Shininess");

    glEnable( GL_DEPTH_TEST );
    glClearColor( 0.5, 0.7, 1.0, 1.0 ); // Sky blue bg

    // Init Planes Position
    for(int i=1; i<=8; i++) {
        planes[i].position = vec3( (i-4.5)*3.0, 0.0, 0.0 );
        planes[i].aux_state = false;
        planes[i].propeller_speed = 5.0;
    }
}

void display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Camera
    // Spherical to Cartesian for eye
    // Actually user wants fps style or spherical. Let's stick to what we have in "eye".
    // Or logic to move "eye" based on WASD.
    // For simplicity, let's keep LookAt logic using global 'eye' and 'at'.
    
    view_matrix = LookAt( eye, at, up );
    glUniformMatrix4fv( ViewLoc, 1, GL_TRUE, view_matrix );

    projection = Perspective( fovy, aspect, zNear, zFar );
    glUniformMatrix4fv( ProjectionLoc, 1, GL_TRUE, projection );

    // Lighting
    glUniform4fv( LightPositionLoc, 1, light_position );
    
    // Draw Environment
    DrawShop();

    // Draw Planes
    for(int i=1; i<=8; i++) {
        mat4 mt = Translate(planes[i].position);
        
        // Apply Orientation
        mt *= RotateY(planes[i].rotation.y);
        mt *= RotateX(planes[i].rotation.x);
        mt *= RotateZ(planes[i].rotation.z);
        
        // Select Model
        switch(i) {
            case 1: DrawJet(mt, i); break;
            case 2: DrawPropPlane(mt, i); break;
            case 3: DrawHelicopter(mt, i); break;
            case 4: DrawPaperPlane(mt, i); break;
            case 5: DrawDrone(mt, i); break;
            case 6: DrawRocket(mt, i); break;
            case 7: DrawBalloon(mt, i); break;
            case 8: DrawFighter(mt, i); break;
        }
    }

    glutSwapBuffers();
}

void keyboard( unsigned char key, int x, int y )
{
    // Camera Move Step
    float step = 0.5;
    vec3 forward = normalize(at - eye);
    vec3 right = normalize(cross(forward, vec3(0,1,0)));
    
    // Global Camera Control (Key 0) or specific plane control
    if (selected_object == 0) {
        switch(key) {
            case 'w': eye += forward*step; at += forward*step; break;
            case 's': eye -= forward*step; at -= forward*step; break;
            case 'a': eye -= right*step; at -= right*step; break;
            case 'd': eye += right*step; at += right*step; break;
            case 'q': eye.y += step; at.y += step; break;
            case 'e': eye.y -= step; at.y -= step; break;
            case '9': light_on = !light_on; 
                      glUniform4fv( AmbientProductLoc, 1, light_on ? light_ambient : color4(0,0,0,1) );
                      break;
        }
    } else {
        // Plane Control
        // W/S: Forward/Back (Local Z)
        // A/D: Yaw
        ObjectState& b = planes[selected_object];
        float move_speed = 0.2;
        float rot_speed = 2.0;
        
        switch(key) {
            case 'w': b.position.z -= move_speed; break;
            case 's': b.position.z += move_speed; break;
            case 'a': b.rotation.y += rot_speed; break;
            case 'd': b.rotation.y -= rot_speed; break;
            case 'q': b.position.y += move_speed; break;
            case 'e': b.position.y -= move_speed; break;
            case 'r': b.rotation.x -= rot_speed; break;
            case 'f': b.rotation.x += rot_speed; break;
            case 'u': b.propeller_speed += 1.0; break;
            case 'j': b.propeller_speed -= 1.0; break;
            case 'i': b.aux_state = !b.aux_state; break;
            case ' ': if (selected_object == 6) b.position.y += 0.5; break; // Rocket launch
        }
    }

    // Select Plane
    if (key >= '0' && key <= '8') {
        selected_object = key - '0';
        std::cout << "Selected Object: " << selected_object << std::endl;
    }
    
    if (key == 033) exit(0); // Esc
    glutPostRedisplay();
}

void idle( void )
{
    rotation_global += 0.1;
    
    // Update Animations
    for(int i=1; i<=8; i++) {
        planes[i].propeller_angle += planes[i].propeller_speed;
        if(planes[i].propeller_angle > 360) planes[i].propeller_angle -= 360;
    }
    
    glutPostRedisplay();
}

void reshape( int width, int height )
{
    glViewport( 0, 0, width, height );
    aspect = GLfloat(width)/height;
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 1024, 768 );
    glutCreateWindow( "Toy Airplane Shop - OpenGL Core Profile" );

    #ifndef __APPLE__
    glewInit();
    #endif

    init();

    glutDisplayFunc( display );
    glutKeyboardFunc( keyboard );
    glutReshapeFunc( reshape );
    glutIdleFunc( idle );

    glutMainLoop();
    return 0;
}
