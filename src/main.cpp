#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

double rad_todeg = 180 / M_PI;
double deg_torad = M_PI / 180;

int WIDTH = 1280;
int HEIGHT = 720;
sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "render");

float abs_f(float i){
    return (i < 0) ? -i : i;
}

class vec3f;
class vec2f;
class line3f;
class rect3f;
vec2f transpose_point(vec3f point);
vec3f rotate_point(vec3f point, vec3f ref, vec3f rotation);
void draw_conline(sf::RenderWindow& rw, std::vector<vec3f> points);
void apply_physics();

enum axis{
    x,
    y,
    z
};

class vec3f{
public:
    float x, y, z;
    vec3f(){
        this->x = 0;
        this->y = 0;
        this->z = 0;
    }

    vec3f(float _x, float _y, float _z){
        this->x = _x;
        this->y = _y;
        this->z = _z;
    }

    vec3f operator+ (const vec3f& lhs){
        return vec3f(this->x + lhs.x, this->y + lhs.y, this->z + lhs.z);
    }

    vec3f operator- (const vec3f& lhs){
        return vec3f(this->x - lhs.x, this->y - lhs.y, this->z - lhs.z);
    }
};

class vec2f{
public:
    float x, y;
    vec2f(){
        this->x = 0;
        this->y = 0;
    }

    vec2f(float _x, float _y){
        this->x = _x;
        this->y = _y;
    }

    vec2f operator+ (const vec2f& lhs){
        return vec2f(this->x + lhs.x, this->y + lhs.y);
    }

    vec2f operator- (const vec2f& lhs){
        return vec2f(lhs.x - this->x, lhs.y - this->y);
    }
};

class line3f{
public:
    vec3f start, end;
    line3f(vec3f _st, vec3f _ed){
        this->start = _st;
        this->end = _ed;
    }
};

enum rect_o{
    flat = 0,
    horizontal = 1,
    vertical = 2
};

class rect3f{
public:
    vec3f pos;
    vec2f size;
    sf::Color col;
    rect_o ot;

    rect3f(vec3f _pos, vec2f _size, rect_o _ot){
        this->pos = _pos;
        this->size = _size;
        this->ot = _ot;
    }

    void set_color(sf::Color _col){
        this->col = _col;
    }

    void draw(sf::RenderWindow& rw){
        vec3f p1, p2, p3, p4;
        if(ot == rect_o::flat){
            p1 = pos;
            p2 = pos + vec3f(size.x, 0, 0);
            p3 = pos + vec3f(size.x, size.y, 0);
            p4 = pos + vec3f(0, size.y, 0);
        }
        else if(ot == rect_o::horizontal){
            p1 = pos;
            p2 = pos + vec3f(size.x, 0, 0);
            p3 = pos + vec3f(size.x, 0, size.y);
            p4 = pos + vec3f(0, 0, size.y);
        }
        else if(ot == rect_o::vertical){
            p1 = pos;
            p2 = pos + vec3f(0, size.x, 0);
            p3 = pos + vec3f(0, size.x, size.y);
            p4 = pos + vec3f(0, 0, size.y);
        }

        draw_conline(rw, {p1, p2, p3, p4});
    }

    rect3f operator+ (const vec3f& lhs){
        return rect3f(this->pos + lhs, this->size, this->ot);
    }
};

class cube3f{
public:
    vec3f pos;
    float size;
    vec3f rotation;

    cube3f(){}

    cube3f(vec3f _pos, float _size){
        this->pos = _pos;
        this->size = _size;
    }

    /* 
     * 4    3
     *
     * 1    2
    */
    std::vector<vec3f> get_edges(){
        std::vector<vec3f> edges;
        edges.push_back(pos);
        edges.push_back(pos + vec3f(size, 0, 0));
        edges.push_back(pos + vec3f(size, 0, size));
        edges.push_back(pos + vec3f(0, 0, size));

        edges.push_back(pos + vec3f(0, size, 0));
        edges.push_back(pos + vec3f(0, size, 0) + vec3f(size, 0, 0));
        edges.push_back(pos + vec3f(0, size, 0) + vec3f(size, 0, size));
        edges.push_back(pos + vec3f(0, size, 0) + vec3f(0, 0, size));

        vec3f ctr = center();
        for(int i = 0; i < edges.size(); i++){
            edges[i] = rotate_point(edges[i], ctr, rotation);
        }

        return edges;
    }

    void draw(sf::RenderWindow& rw){
        std::vector<vec3f> sv = get_edges();
        std::vector<std::vector<vec3f>> vvec = {
            {sv[0], sv[1], sv[2], sv[3]},
            {sv[4], sv[5], sv[6], sv[7]},
            {sv[3], sv[2], sv[6], sv[7]},
            {sv[0], sv[1], sv[5], sv[4]},
            {sv[0], sv[3], sv[7], sv[4]},
            {sv[1], sv[2], sv[6], sv[5]}
        };

        for(std::vector<vec3f> face : vvec){
            draw_conline(rw, face);
        }
    }

    vec3f center(){
        return pos + vec3f(size / 2, size / 2, size / 2);
    }

    void rotate(vec3f _rotation){
        this->rotation = this->rotation + _rotation;
    }
};

class polygon3f{
public:
    vec3f pos;
    vec3f rotation;
    int edge_num;
    float radius;

    polygon3f(vec3f _pos, float _radius, int _edge_num){
        this->pos = _pos;
        this->radius = _radius;
        this->edge_num = _edge_num;
    }

    std::vector<vec3f> get_edges(){
        std::vector<vec3f> edges;
        float step_ang = (float)360/(float)edge_num;
        for(int i = 1; i <= edge_num; i++){
            float ang = step_ang*i;
            float x = (float)1/(float)sqrt(1 + pow(tan(ang * deg_torad), 2));
            if(ang >= 90 && ang <= 270){
                x = -x;
            }
            float y = sqrt(1 - pow(x, 2));
            if(ang >= 180){
                y = -y;
            }

            edges.push_back(rotate_point(pos + vec3f(y * radius, 0, -x * radius), pos, rotation));
        }

        return edges;
    }

    void draw(sf::RenderWindow& rw){
        draw_conline(rw, get_edges());
    }
};

class sphere3f{
public:
    vec3f pos;
    vec3f rotation;
    float radius;

    sphere3f(vec3f _pos, float _radius){
        this->pos = _pos;
        this->radius = _radius;
    }

    std::vector<std::vector<vec3f>> get_layers(int edge_num = 30){
        std::vector<vec3f> base_r1;
        float step_ang = (float)360/(float)edge_num;
        for(int i = 1; i <= edge_num; i++){
            float ang = step_ang*i;
            float x = (float)1/(float)sqrt(1 + pow(tan(ang * deg_torad), 2));
            if(ang >= 90 && ang <= 270){
                x = -x;
            }
            float y = sqrt(1 - pow(x, 2));
            if(ang >= 180){
                y = -y;
            }

            base_r1.push_back(vec3f(x * radius, y * radius, 0));
        }

        std::vector<std::vector<vec3f>> layers;

        float h_step = 6;
        int mx_h = floor((float)180 / h_step);
        for(int i = 0; i <= mx_h; i++){
            float ang = h_step*i;
            float x = (float)1/(float)sqrt(1 + pow(tan(ang * deg_torad), 2));
            float y = sqrt(1 - pow(x, 2));
            if(ang >= 90 && ang <= 270){
                x = -x;
            }
            std::vector<vec3f> c_lay;
            for(vec3f v : base_r1){
                vec3f mp = pos + vec3f(v.x * y, v.y * y, x * radius);
                c_lay.push_back(rotate_point(mp, pos, rotation));
            }

            layers.push_back(c_lay);
        }

        return layers;
    }

    void draw(sf::RenderWindow& rw){
        std::vector<std::vector<vec3f>> layers = get_layers();
        for(int i = 0; i < layers.size(); i++){
            draw_conline(rw, layers[i]);
        }
    }

    void rotate(vec3f _rotation){
        this->rotation = this->rotation + _rotation;
    }
};

class block{
public:
    cube3f cube;
    std::string name;
    bool p_collision;
    bool grav;
    vec3f pos;

    block(vec3f p, std::string nm, bool pcol, bool grv)
    {
        pos = p;
        name = nm;
        p_collision = pcol;
        grav = grv;

        cube = cube3f(vec3f(pos.x/2, pos.y/2, pos.z/2), 0.5f);
    }
};

void draw_conline(sf::RenderWindow& rw, std::vector<vec3f> points){
    int vsize = points.size();
    bool insd = false;

    sf::Vertex f[vsize + 1];
    for(int i = 0; i < vsize; i++){
        vec2f s_p = transpose_point(points[i]);
        f[i] = sf::Vector2f(s_p.x, s_p.y);

        if((s_p.x > 0 && s_p.x < WIDTH) && (s_p.y > 0 && s_p.y < HEIGHT)){
            insd = true;
        }
    }
    f[vsize] = f[0];

    if(insd){
        rw.draw(f, vsize + 1, sf::LineStrip);
    }
}

vec3f rotate_point(vec3f point, vec3f ref, vec3f rotation){
    vec3f r_p = point - ref;
    
    // tg: x -> sin, y -> cos
    if(rotation.x != 0){
        vec2f tg_x = vec2f(sin(rotation.x * deg_torad), cos(rotation.x * deg_torad));
        r_p = vec3f(r_p.x, r_p.y*tg_x.y - r_p.z*tg_x.x, r_p.y*tg_x.x + r_p.z*tg_x.y);
    }
    if(rotation.y != 0){
        vec2f tg_y = vec2f(sin(rotation.y * deg_torad), cos(rotation.y * deg_torad));
        r_p = vec3f(r_p.x*tg_y.y - r_p.z*tg_y.x, r_p.y, r_p.z*tg_y.y + r_p.x*tg_y.x);
    }
    if(rotation.z != 0){
        vec2f tg_z = vec2f(sin(rotation.z * deg_torad), cos(rotation.z * deg_torad));
        r_p = vec3f(r_p.x*tg_z.y - r_p.y*tg_z.x, r_p.x*tg_z.x + r_p.y*tg_z.y, r_p.z);
    }

    return r_p + ref;
}

vec3f cam_pos(0, 0, 1);
vec3f cam_angle(0, 0, 0);
vec3f disp(0, 1, 0);
float fov = 150;

vec3f mod_angle(vec3f r){
    vec3f rot = r;
    if(rot.x > 360){
        rot.x -= 360;
    }
    if(rot.x < -360){
        rot.x += 360;
    }
    if(rot.y > 360){
        rot.y -= 360;
    }
    if(rot.y < -360){
        rot.y += 360;
    }
    if(rot.z > 360){
        rot.z -= 360;
    }
    if(rot.z < -360){
        rot.z += 360;
    }
    return rot;
}

bool is_inview(vec3f p){
    vec3f diff = p - cam_pos;
    diff = rotate_point(diff, vec3f(0, 0, 0), vec3f(0, 0, cam_angle.z) + rotate_point(vec3f(cam_angle.x, 0, 0), vec3f(0, 0, 0), vec3f(0, 0, cam_angle.z)));
    return diff.y < 0;
}

vec2f transpose_point(vec3f point){
    cam_angle = mod_angle(cam_angle);

    vec3f diff = point - cam_pos;
    diff = rotate_point(diff, vec3f(0, 0, 0), vec3f(0, 0, cam_angle.z) + rotate_point(vec3f(cam_angle.x, 0, 0), vec3f(0, 0, 0), vec3f(0, 0, cam_angle.z)));

    float scr_xd = (diff.x*disp.y)/diff.y;
    float scr_yd = (diff.z*disp.y)/diff.y;

    if(diff.y < 0){
        return vec2f(0,0);
    }

    float scr_w = tan(fov/2 * deg_torad) * disp.y;
    float o1 = (float)WIDTH / scr_w;

    return vec2f(WIDTH/2 + scr_xd*o1, HEIGHT/2 - scr_yd*o1);
}

//vec3f ang_app(0, 0, 0);
bool noclip = false;
vec3f mov_app(0, 0, 0);
float movspeed = 0.1;
float rotspeed = 2;
std::vector<sf::Keyboard::Key> pressed;
void handle_movement(sf::Keyboard::Key key, bool press_state){
    bool fnd = false;
    int index = 0;
    for(int i = 0; i < pressed.size(); i++){
        sf::Keyboard::Key k = pressed[i];
        if(k == key){
            fnd = true;
            index = i;

            break;
        }
    }

    if(press_state){
        if(fnd){
            return;
        }
        else{
            pressed.push_back(key);
        }
    }
    else{
        if(fnd){
            pressed.erase(pressed.begin() + index);
        }
        else{
            return;
        }
    }

    if(key == sf::Keyboard::Space){
        vec3f v = vec3f(0, 0, movspeed);
        if(press_state){
            mov_app = mov_app + v;
        }
        else{
            mov_app = vec3f(0, 0, 0);
        }
    }

    if(key == sf::Keyboard::LShift){
        vec3f v = vec3f(0, 0, movspeed);
        if(press_state){
            mov_app = mov_app - v;
        }
        else{
            mov_app = vec3f(0, 0, 0);
        }
    }

    if(key == sf::Keyboard::N){
        noclip = !noclip;
    }
}

void apply_movement(){
    for(sf::Keyboard::Key key : pressed){
        if(key == sf::Keyboard::W || key == sf::Keyboard::S){
            vec3f v = vec3f(movspeed*sin(cam_angle.z * deg_torad), movspeed*cos(cam_angle.z * deg_torad), 0);
            if(key == sf::Keyboard::W){
                cam_pos = cam_pos + v;
            }
            else{
                cam_pos = cam_pos - v;
            }
        }

        if(key == sf::Keyboard::A || key == sf::Keyboard::D){
            vec3f v = vec3f(movspeed*sin(cam_angle.z * deg_torad), movspeed*cos(cam_angle.z * deg_torad), 0);
            v = vec3f(-v.y, v.x, 0); // rotate 90 z

            if(key == sf::Keyboard::A){
                cam_pos = cam_pos + v;
            }
            else{
                cam_pos = cam_pos - v;
            }
        }
    }

    cam_pos = cam_pos + mov_app;
    //cam_angle = cam_angle + ang_app;
}

void apply_physics(){
    if(!noclip){

    }
}

vec2f crosshair_size(15, 1);
void draw_crosshair(){
    sf::RectangleShape pl1, pl2;
    pl1.setSize(sf::Vector2f(crosshair_size.x, crosshair_size.y));
    pl2.setSize(sf::Vector2f(crosshair_size.y, crosshair_size.x));

    pl1.setPosition(WIDTH/2 - crosshair_size.x/2, HEIGHT/2 - crosshair_size.y/2);
    pl2.setPosition(WIDTH/2 - crosshair_size.y/2, HEIGHT/2 - crosshair_size.x/2);

    pl1.setFillColor(sf::Color::White);
    pl2.setFillColor(sf::Color::White);

    window.draw(pl1);
    window.draw(pl2);
}

bool ch_init = false;
std::vector<std::vector<block>> grid;
void draw_chunk(){
    if(ch_init){
        for(auto ch : grid){
            for(block b : ch){
                if(!is_inview(b.cube.center())){
                    b.cube.draw(window);
                }
            }
        }
    }
    else{
        std::vector<block> ch1;
        int gridsize = 10;
        float tilesize = 0.5;
        for(int i = 0; i < gridsize; i++){
            for(int j = 0; j < gridsize; j++){
                cube3f c(vec3f(-(gridsize/2)*tilesize + i*tilesize, -(gridsize/2)*tilesize + j*tilesize, -tilesize), tilesize);
                //ch1.push_back(block());
            }
        }

        grid.push_back(ch1);

        ch_init = true;
    }
}

//sphere3f sph(vec3f(0, 4, 2), 0.5);
//polygon3f pol3(vec3f(0, 4, 0), 1, 7);
cube3f c(vec3f(-0.5, 7.5, 0.5), 1);
sf::Clock mov_cl;
int frame_count = 0;
void render_loop(){
    if(mov_cl.getElapsedTime().asMilliseconds() >= 25){
        apply_movement();
        // apply_physics();
        mov_cl.restart();
    }

    window.clear(sf::Color::Black);

    draw_crosshair();
    draw_chunk();

    window.display();
    frame_count++;
}

bool limit_fps = false;
bool paused = false;
int fps = 60;
float mouse_sens = 0.1;
int main(int argc, char** argv){
    sf::Clock sec_cl, fpslim_cl;
    int req_millis = 1000 / fps;
    bool mvis = false;
    window.setMouseCursorVisible(mvis);
    
    while (window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Resized){
                sf::FloatRect n_view(0, 0, event.size.width, event.size.height);
                window.setView(sf::View(n_view));

                WIDTH = event.size.width;
                HEIGHT = event.size.height;
            }

            if(event.type == sf::Event::MouseMoved){
                if(!paused){
                    float xdf = WIDTH/2 - event.mouseMove.x;
                    float ydf = HEIGHT/2 - event.mouseMove.y;
                    if(xdf != 0 && ydf != 0){
                        // std::cout << xdf << " " << ydf << std::endl;
                        vec3f app_ang = vec3f(-ydf*mouse_sens, 0, -xdf*mouse_sens);
                        if(cam_angle.x <= 90 && cam_angle.x >= -90){
                            cam_angle = cam_angle + app_ang;
                        }
                        else{
                            if(cam_angle.x < 0){
                                cam_angle.x = -90;
                            }
                            else{
                                cam_angle.x = 90;
                            }
                        }
                        
                        sf::Mouse::setPosition(sf::Vector2i(WIDTH/2, HEIGHT/2), window);
                    }
                }
            }

            if(event.type == sf::Event::KeyPressed){
                handle_movement(event.key.code, true);

                if(event.key.code == sf::Keyboard::Escape){
                    paused = !paused;
                    mvis = !mvis;

                    window.setMouseCursorVisible(mvis);
                }
            }

            if(event.type == sf::Event::KeyReleased){
                handle_movement(event.key.code, false);
            }

            if (event.type == sf::Event::Closed){
                window.close();
            }
        }

        if(sec_cl.getElapsedTime().asMilliseconds() >= 1000){
            window.setTitle("render - fps: " + std::to_string(frame_count));
            frame_count = 0;
            sec_cl.restart();
        }

        if(limit_fps){
            if(fpslim_cl.getElapsedTime().asMilliseconds() >= req_millis){
                if(!paused){
                    render_loop();
                }
                fpslim_cl.restart();
            }
        }
        else{
            if(!paused){
                render_loop();
            }
        }
    }

    return 0;
}