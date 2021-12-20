#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

#include <box2d/box2d.h>
#include <citro2d.h>

#include <memory>
#include <string>
#include <math.h>
#include <cstdint>

#define SCREEN_WIDTH 400 //320
#define SCREEN_HEIGHT 240

#define SCREEN_TO_WORLD_WIDTH 2.3f
#define SCREEN_TO_WORLD_HEIGHT 18.0f
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
        virtual void startContact(){}
        virtual void endContact(){}
};

class Sprite{
public:
    C2D_Sprite spr;
    float width;
    float height;
};

class Treasure : public Entity{
    public:
        Sprite spr;
        Sprite openspr;
        b2Body* body;
        Sprite content;
        int hasCollided = 0;
        void startContact(){
            spr = openspr;
            hasCollided = 1;
        }
        void endContact(){
            hasCollided = 1;
        }
        void render(){
            b2Vec2 pos = body->GetPosition();
            C2D_SpriteSetPos(&spr.spr, pos.x, pos.y);
            C2D_DrawSprite(&spr.spr);
        }
};

class Platform : public Entity{
public:
    float width;
    float height;
    b2Body* body;
    int spritenum;
    // TODO: dynamic way to initialize
    Sprite sprites[32];
    Platform(){}
    Platform(float w, float h, Sprite s){ 
        width = w;
        height=h;
        spritenum = ceil(width*PIXEL_TO_METER/s.width)+1;
        for(int i=0; i<spritenum;i++) {
            memcpy(&sprites[i].spr, &s.spr, sizeof(C2D_Sprite));
            sprites[i].width = s.width;
            sprites[i].height = s.height;
        }
    }
    void render(){
        b2Vec2 pos = body->GetPosition();
        for(int i =0;i<spritenum;i++) {
            C2D_SpriteSetPos(&sprites[i].spr, pos.x-width*PIXEL_TO_METER/2+sprites[i].width*i, pos.y);
            C2D_DrawSprite(&sprites[i].spr);
        }
        //C2D_DrawRectSolid(pos.x, pos.y, 0.5, width*PIXEL_TO_METER, height, C2D_Color32f(1,0,0,1));
    }
};

class Player : public Entity{
public:
    _jumpState jumpstate;
    float width;
    float height;
    b2Body* body;
    C2D_Sprite sprite;
    int maxJumps = 2;
    int currentJumps =0;
    void startContact() {
        jumpstate = JS_GROUND;
        currentJumps = 0;
    }
    void endContact(){
        jumpstate = JS_JUMP;
    }
};
class ContactListener : public b2ContactListener {
    void BeginContact(b2Contact* contact) {
        uintptr_t bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        if ( bodyUserData ) {
            /*std::string tag = static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->tag;
            if(tag.compare("player")==0)
                static_cast<Player*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
            if(tag.compare("treasure")==0)
                static_cast<Treasure*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
            */
            static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
        }
        bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        if ( bodyUserData ) {
            /*std::string tag = static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->tag;
            if(tag.compare("player")==0)
                static_cast<Player*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
            if(tag.compare("treasure")==0)
                static_cast<Treasure*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
            */
            static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
        }
    }
    void EndContact(b2Contact* contact) {
        uintptr_t bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        if ( bodyUserData )
            static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->endContact();
                
        //check if fixture B was a ball
        bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        if ( bodyUserData )
            static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->endContact();
    }
};

const int NUM_PLATFORMS = 5;
static Platform platforms[NUM_PLATFORMS];
static Player player;
static Treasure treasure;

static C2D_SpriteSheet spritesheet;
static Sprite bigpinkspr;
static Sprite smallpinkspr;
static Sprite pinkplatformspr;
static Sprite mountainbgspr;
static Sprite treasure_closed;
static Sprite treasure_open;
static void initSprites() 
{
    Player* p = &player;
    C2D_SpriteFromSheet(&p->sprite, spritesheet, 4);
    C2D_SpriteSetCenter(&p->sprite, -0.3f,0.3f);

    C2D_SpriteFromSheet(&bigpinkspr.spr, spritesheet, 1);
    C2D_SpriteSetCenter(&bigpinkspr.spr, -1.0f,0.35f);
    bigpinkspr.width=48;
    bigpinkspr.height=48;
    C2D_SpriteFromSheet(&smallpinkspr.spr, spritesheet, 2);
    //C2D_SpriteSetCenter(&smallpinkspr.spr, 0.5f,0.5f);
    smallpinkspr.width = 16;
    smallpinkspr.height = 16;
    C2D_SpriteFromSheet(&pinkplatformspr.spr, spritesheet, 3);
    //C2D_SpriteSetCenter(&pinkplatformspr.spr, 0.5f,0.5f);
    C2D_SpriteFromSheet(&mountainbgspr.spr, spritesheet, 5);
    C2D_SpriteSetScale(&mountainbgspr.spr, 2,2);
    
    C2D_SpriteFromSheet(&treasure_closed.spr, spritesheet, 6);
    C2D_SpriteFromSheet(&treasure_open.spr, spritesheet, 7);
}

static void movesprites()
{
    Player* p = &player;
    b2Vec2 pos = p->body->GetPosition();
    C2D_SpriteSetPos(&p->sprite, pos.x, pos.y);
}

Platform createPlatform(std::unique_ptr<b2World> &world, float x, float y, float width, float height, Sprite sprite)
{
    Platform plat(width,height, sprite);
    // ground body
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(x, y);
    b2Body *groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(width, height);
    groundBody->CreateFixture(&groundBox, 0.0f);
    groundBody->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(&plat);

    plat.body = groundBody;
    plat.tag = "platform";
    return plat;
}
Treasure createTreasure(std::unique_ptr<b2World> &world, float x, float y)
{
    Treasure tres;
    tres.spr=treasure_closed;
    tres.openspr = treasure_open;
    // ground body
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(x, y);
    b2Body *groundBody = world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(1,1);
    groundBody->CreateFixture(&groundBox, 0.0f);
    groundBody->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(&tres);

    tres.body = groundBody;
    tres.tag = "treasure";
    return tres;
}

int main(int argc, char** argv)
{
    _moveState moveState;
    moveState = MS_STOP;

    romfsInit();
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);

	spritesheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spritesheet) svcBreak(USERBREAK_PANIC);

    cfguInit();
    initSprites();

    b2Vec2 gravity(0.0f, 100.0f);
    std::unique_ptr<b2World> world = std::make_unique<b2World>(gravity);
    ContactListener myContactListenerInstance;
    world->SetContactListener(&myContactListenerInstance);
    
    platforms[0] = createPlatform(world, 0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, 16, bigpinkspr);
    platforms[1] = createPlatform(world, SCREEN_WIDTH/2-80, SCREEN_HEIGHT/2+25,18, 1, smallpinkspr);
    platforms[2] = createPlatform(world, SCREEN_WIDTH/2+80, SCREEN_HEIGHT/2+25,34, 1, smallpinkspr);
    platforms[3] = createPlatform(world, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-25,26, 1, smallpinkspr);
    platforms[4] = createPlatform(world, SCREEN_WIDTH/2+130, SCREEN_HEIGHT/2-60,48, 1, smallpinkspr);
    treasure = createTreasure(world, SCREEN_WIDTH/2+130, SCREEN_HEIGHT/2-70);

    // dynamic body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(10, 0.0f);
    b2Body* body = world->CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(16.0f, 16.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.8f;
    body->CreateFixture(&fixtureDef);
    player.body = body;
    player.tag = "player";
    body->GetUserData().pointer = reinterpret_cast<std::uintptr_t>(&player);
    player.jumpstate = JS_GROUND;

    float timeStep = 1.0f / 45.0f;
    int32_t velocityIter = 6;
    int32_t positionIter = 2;
    consoleInit(GFX_BOTTOM, NULL);
    while(aptMainLoop())
    {
        consoleClear();
        
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        u32 kUp = hidKeysUp();

        if(kDown & KEY_START) break;
        
        if(kDown & KEY_CPAD_LEFT)
            moveState = MS_LEFT;
        if(kDown & KEY_CPAD_RIGHT)
            moveState = MS_RIGHT;
        if(kUp & KEY_CPAD_LEFT || kUp & KEY_CPAD_RIGHT)
            moveState = MS_STOP;

        float jumpimpulse = 0;
        float jumpvel = 300;
        if(kDown & KEY_A && player.currentJumps < player.maxJumps) {
            jumpimpulse = body->GetMass() * jumpvel; // f= m*v
            player.jumpstate = JS_JUMP;
            player.currentJumps++;
        }
        printf("Jump State: %d", player.jumpstate);
        printf("Treasure State: %d", treasure.hasCollided);
        b2Vec2 bodyvel = body->GetLinearVelocity();
        float desiredvel = 0.0f;
        float VEL = 40;
        float ACCEL = 8;
        switch(moveState)
        {
            case MS_LEFT: desiredvel = b2Max( bodyvel.x - ACCEL, -VEL ); break;
            case MS_STOP:   
                            if(player.jumpstate == JS_GROUND)
                                desiredvel = bodyvel.x * 0.98f; 
                        break;
            case MS_RIGHT: desiredvel= b2Min( bodyvel.x + ACCEL, VEL); break;
            default: break;
        }
        float velChange = desiredvel - bodyvel.x;
        float impulse = body->GetMass() * velChange;
        body->ApplyLinearImpulse( b2Vec2(impulse,-jumpimpulse), body->GetWorldCenter(), true);
        
        for (b2ContactEdge* edge = treasure.body->GetContactList(); edge; edge = edge->next) {
            treasure.startContact();
        }
        // Render scene clear to white
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(bottom, C2D_Color32f(1,1,1,1));
        C2D_SceneBegin(bottom);
        // draw BG
        C2D_DrawSprite(&mountainbgspr.spr);

        for(int i =0; i<NUM_PLATFORMS; i++){
            Platform p = platforms[i];
            p.render();
        }
        treasure.render();
        movesprites();
        C2D_DrawSprite(&player.sprite);
        //b2Vec2 bodypos = body->GetPosition();
        //C2D_DrawRectSolid(bodypos.x, bodypos.y, 0.5, 16, 18, C2D_Color32f(1,0,0,1));
        C3D_FrameEnd(0);
        world->Step(timeStep, velocityIter, positionIter);
    }

    // clean up
    cfguExit();
    C2D_Fini();
    C2D_Fini();

    gfxExit();


    return 0;
}
