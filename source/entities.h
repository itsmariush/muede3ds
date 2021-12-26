#ifndef ENTITIES_H
#define ENTITIES_H
#include <string>
#include <memory>

#include <3ds.h>
#include <box2d/box2d.h>
#include <citro2d.h>

#define PIXEL_TO_METER 1.7f

enum _moveState {
    MS_STOP,
    MS_LEFT,
    MS_RIGHT,
};

enum _jumpState {
    JS_JUMP,
    JS_GROUND,
};
class Entity{
    public:
        std::string tag;
        virtual void startContact();
        virtual void endContact();
        ~Entity(){}
};

class Sprite{
public:
    C2D_Sprite spr;
    float width;
    float height;
    ~Sprite(){}
};

class Treasure : public Entity{
    public:
        Sprite spr;
        Sprite openspr;
        b2Body* body;
        Sprite content;
        bool isopen = false; 
        void startContact();
        void endContact();
        void render();
        ~Treasure(){}
};

class Platform : public Entity{
public:
    float width;
    float height;
    b2Body* body;
    int spritenum;
    // TODO: dynamic way to initialize
    Sprite sprites[32];
    Platform();
    Platform(float w, float h, Sprite s);
    void render();
    ~Platform();
};

class Player : public Entity{
public:
    _jumpState jumpstate;
    _moveState lastMove;
    float width;
    float height;
    b2Body* body;
    C2D_Sprite sprite;
    int maxJumps = 2;
    int currentJumps =0;
    void startContact();
    void endContact();
};

class Level : public Entity{
    public:
        Level *next;
        void (*createNext)(std::unique_ptr<b2World>&);
        Sprite background;
        float nextLevel[2];
        int numplatforms = 6;
        // TODO: add dynamic way
        Platform platforms[6];
        Treasure treasure;
};
#endif
