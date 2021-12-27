#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>

#include <box2d/box2d.h>
#include <citro2d.h>

#include <memory>
#include <string>
#include <math.h>
#include <cstdint>

#include "entities.h"

#define SCREEN_WIDTH 400 //320
#define SCREEN_HEIGHT 240
#define PIXEL_TO_METER 1.7f

class ContactListener : public b2ContactListener {
    void BeginContact(b2Contact* contact) {
        uintptr_t bodyUserData = contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        if ( bodyUserData ) {
            static_cast<Entity*>( reinterpret_cast<void*>(bodyUserData) )->startContact();
        }
        bodyUserData = contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        if ( bodyUserData ) {
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

static Player player;

static C2D_SpriteSheet spritesheet;
static Sprite bigpinkspr;
static Sprite smallpinkspr;
static Sprite pinkplatformspr;
static Sprite mountainbgspr;
static Sprite treasure_closed;
static Sprite treasure_open;
static Sprite heartspr;
static Sprite arrowspr;
static Sprite waterblockspr;
static Sprite bigsandblockspr;
static Sprite smallsandblockspr;
static Sprite springbgspr;
static Sprite start_screen;
static Sprite press_start;
static Sprite bedspr;
static Sprite winspr;
static void initSprites() 
{
    Player* p = &player;
    C2D_SpriteFromSheet(&p->sprite, spritesheet, 4);
    //C2D_SpriteSetCenter(&p->sprite, -0.3f,0.3f);
    C2D_SpriteSetCenter(&p->sprite, 0,0.5f);

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

    C2D_SpriteFromSheet(&heartspr.spr, spritesheet, 8);
    C2D_SpriteFromSheet(&arrowspr.spr, spritesheet, 9);
    C2D_SpriteSetCenter(&arrowspr.spr, 0.2f,0.8f);
    
    C2D_SpriteFromSheet(&springbgspr.spr, spritesheet , 10);
    C2D_SpriteFromSheet(&bigsandblockspr.spr, spritesheet, 11);
    C2D_SpriteSetCenter(&bigsandblockspr.spr, 0.0f,1.0f);
    bigsandblockspr.width=48;
    bigsandblockspr.height=16;
    C2D_SpriteFromSheet(&smallsandblockspr.spr, spritesheet, 12);
    smallsandblockspr.width=16;
    smallsandblockspr.height=16;
    C2D_SpriteFromSheet(&waterblockspr.spr, spritesheet, 13);
    C2D_SpriteSetCenter(&waterblockspr.spr, 0.0f,1.0f);
    waterblockspr.width=48;
    waterblockspr.height=16;
    
    C2D_SpriteFromSheet(&start_screen.spr, spritesheet, 14);
    C2D_SpriteFromSheet(&press_start.spr, spritesheet, 15);
    
    C2D_SpriteFromSheet(&bedspr.spr, spritesheet, 16);
    C2D_SpriteSetCenter(&bedspr.spr, 0.0f,0.5f);
    bedspr.width = 96;
    bedspr.height = 36;

    C2D_SpriteFromSheet(&winspr.spr, spritesheet, 17);
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
    tres.content = heartspr; 
    tres.tag = "treasure";
    return tres;
}
std::string LEVEL;
static Level level0;
static Level level1;
static Level level2;
static Level level3;
static Level levelEnd;
void createLevelEnd(std::unique_ptr<b2World> &world)
{
    LEVEL = "LevelEnd";
    levelEnd.background = start_screen;
    levelEnd.numplatforms = 0;
    levelEnd.treasure = createTreasure(world, -20,-20);
    float next[2] = {-10, -10};
    levelEnd.nextLevel[0] = next[0];
    levelEnd.nextLevel[1] = next[1];

    C2D_SpriteSetPos(&winspr.spr, SCREEN_WIDTH/2-100, SCREEN_HEIGHT/2);
    C2D_SpriteSetPos(&arrowspr.spr, next[0],next[1]);
}
void createLevel3(std::unique_ptr<b2World> &world) 
{
    LEVEL = "Level3";
    level3.next= &levelEnd;
    level3.createNext = &createLevelEnd;
    level3.background = mountainbgspr;
    level3.numplatforms = 2;
    level3.platforms[0] = createPlatform(world, 0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, 20, bigpinkspr);
    level3.platforms[1] = createPlatform(world, SCREEN_WIDTH/2+100, SCREEN_HEIGHT-32, 48, 6, bedspr);
    level3.treasure = createTreasure(world, 160, SCREEN_HEIGHT-20);
    
    float next[2] = {SCREEN_WIDTH/2+150, SCREEN_HEIGHT+100};
    level3.nextLevel[0] = next[0];
    level3.nextLevel[1] = next[1];
    C2D_SpriteSetPos(&arrowspr.spr, next[0],SCREEN_HEIGHT-48);
}
void createLevel2(std::unique_ptr<b2World> &world)
{
    LEVEL = "Level2";
    level2.next = &level3;
    level2.createNext= &createLevel3;
    level2.background = springbgspr;
    level2.numplatforms = 5;
    level2.platforms[0] = createPlatform(world, 0.0f, SCREEN_HEIGHT, SCREEN_WIDTH/2, 16, bigsandblockspr);
    level2.platforms[1] = createPlatform(world, SCREEN_WIDTH-32, SCREEN_HEIGHT, SCREEN_WIDTH/2, 16, waterblockspr);
    level2.platforms[2] = createPlatform(world, 16, SCREEN_HEIGHT-80, 32, 8, smallsandblockspr);
    level2.platforms[3] = createPlatform(world, 120, SCREEN_HEIGHT-160, 16, 8, smallsandblockspr);
    level2.platforms[4] = createPlatform(world, 300, SCREEN_HEIGHT-180, 48, 8, smallsandblockspr);
    level2.treasure = createTreasure(world, 120, SCREEN_HEIGHT-170);
    float next[2] = {SCREEN_WIDTH/2+150, 45};
    level2.nextLevel[0] = next[0];
    level2.nextLevel[1] = next[1];

    C2D_SpriteSetPos(&arrowspr.spr, next[0],next[1]);
}
void createLevel1(std::unique_ptr<b2World> &world)
{
    LEVEL="Level1";
    level1.next = &level2;
    level1.createNext= &createLevel2;
    level1.background = mountainbgspr;
    level1.numplatforms = 5;
    level1.platforms[0] = createPlatform(world, 0.0f, SCREEN_HEIGHT, SCREEN_WIDTH, 20, bigpinkspr);
    level1.platforms[1] = createPlatform(world, SCREEN_WIDTH/2-80, SCREEN_HEIGHT/2+25,16, 10, smallpinkspr);
    level1.platforms[2] = createPlatform(world, SCREEN_WIDTH/2+80, SCREEN_HEIGHT/2+25,32, 10, smallpinkspr);
    level1.platforms[3] = createPlatform(world, SCREEN_WIDTH/2, SCREEN_HEIGHT/2-25,32, 10, smallpinkspr);
    level1.platforms[4] = createPlatform(world, SCREEN_WIDTH/2+130, SCREEN_HEIGHT/2-60,48, 10, smallpinkspr);
    level1.treasure = createTreasure(world, SCREEN_WIDTH/2+130, SCREEN_HEIGHT/2-70);
    float next[2] = {SCREEN_WIDTH/2+150, 45};
    level1.nextLevel[0] = next[0];
    level1.nextLevel[1] = next[1];

    C2D_SpriteSetPos(&arrowspr.spr, next[0],next[1]);
}
void createLevel0(std::unique_ptr<b2World> &world)
{
    LEVEL = "Level0";
    level0.background = start_screen;
    level0.next = &level1;
    level0.createNext = &createLevel1;
    level0.numplatforms = 0;
    level0.treasure = createTreasure(world, 0,0);
    float next[2] = {-10, -10};
    level0.nextLevel[0] = next[0];
    level0.nextLevel[1] = next[1];

    C2D_SpriteSetPos(&press_start.spr, SCREEN_WIDTH/2-100, SCREEN_HEIGHT/2);
    C2D_SpriteSetPos(&arrowspr.spr, next[0],next[1]);
}
int resetLevel(std::unique_ptr<b2World> &world, Level l){
    player.body->SetTransform(b2Vec2(10,SCREEN_HEIGHT-60),player.body->GetAngle());
    for(int i = 0; i < l.numplatforms; i++){
        world->DestroyBody(l.platforms[i].body);
    }
    world->DestroyBody(l.treasure.body);
    return 0;
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
    C3D_RenderTarget* top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	spritesheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	if (!spritesheet) svcBreak(USERBREAK_PANIC);

    cfguInit();
    initSprites();

    b2Vec2 gravity(0.0f, 100.0f);
    std::unique_ptr<b2World> world = std::make_unique<b2World>(gravity);
    ContactListener myContactListenerInstance;
    world->SetContactListener(&myContactListenerInstance);
    
    createLevel0(world);
    Level level = level0;

    // dynamic body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(10, 0.0f);
    b2Body* body = world->CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(4.0f, 16.0f);
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
    int framenum = 0;
    while(aptMainLoop())
    {
        consoleClear();
        
        printf(LEVEL.c_str());
        b2Vec2 playerpos = body->GetPosition();
        printf("\nLevel End: %f %f",level.nextLevel[0], level.nextLevel[1]);
        printf("\nPlayer X: %f Y: %f", playerpos.x, playerpos.y);
        if(playerpos.x >= level.nextLevel[0] && playerpos.y <= level.nextLevel[1]) {
            printf("\nEnd level");
            resetLevel(world,level);
            level.createNext(world);
            level = *level.next;
            printf("\nWorld reset");
        }

        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        u32 kUp = hidKeysUp();

        if(kDown & KEY_START) break;
        
        if(kDown & KEY_CPAD_LEFT || kDown & KEY_DLEFT) {
            player.lastMove = moveState;
            C2D_SpriteSetScale(&player.sprite, 1,1);
            moveState = MS_LEFT;
        }
        if(kDown & KEY_CPAD_RIGHT || kDown & KEY_DRIGHT){
            player.lastMove = moveState;
            C2D_SpriteSetScale(&player.sprite, -1,1);
            moveState = MS_RIGHT;
        }
        if(kUp & KEY_CPAD_LEFT || kUp & KEY_CPAD_RIGHT || kUp & KEY_DLEFT || kUp & KEY_DRIGHT){
            moveState = MS_STOP;
        }

        float jumpimpulse = 0;
        float jumpvel = 300;
        if(kDown & KEY_A && player.currentJumps < player.maxJumps) {
            jumpimpulse = body->GetMass() * jumpvel; // f= m*v
            player.jumpstate = JS_JUMP;
            player.currentJumps++;

            if(LEVEL.compare("Level0") == 0){
                resetLevel(world,level);
                level.createNext(world);
                level = *level.next;
            }
        }

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
            case MS_RIGHT:  desiredvel= b2Min( bodyvel.x + ACCEL, VEL);
                           break;
            default: break;
        }
        float velChange = desiredvel - bodyvel.x;
        float impulse = body->GetMass() * velChange;
        body->ApplyLinearImpulse( b2Vec2(impulse,-jumpimpulse), body->GetWorldCenter(), true);
        
        for (b2ContactEdge* edge = level.treasure.body->GetContactList(); edge; edge = edge->next) {
            level.treasure.startContact();
        }
        // Render scene clear to white
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C2D_TargetClear(top, C2D_Color32f(1,1,1,1));
        C2D_SceneBegin(top);
        // draw BG
        C2D_DrawSprite(&level.background.spr);
        C2D_DrawSprite(&arrowspr.spr);
        for(int i =0; i<level.numplatforms; i++){
            Platform p = level.platforms[i];
            p.render();
        }
        if(LEVEL.compare("Level0") == 0 || LEVEL.compare("LevelEnd") == 0) {
            framenum++;
            if(!(framenum % 40 < 10)) {
                if(level.createNext == &createLevel1)
                    C2D_DrawSprite(&press_start.spr);
                else 
                    C2D_DrawSprite(&winspr.spr);
            }
        } else { 
            level.treasure.render();
            movesprites();
            C2D_DrawSprite(&player.sprite);
        }
        //b2Vec2 bodypos = body->GetPosition();
        //C2D_DrawRectSolid(bodypos.x-8, bodypos.y-16, 0.5, 16, 32, C2D_Color32f(1,0,0,1));
        
        // Draw Bottom screen
        C2D_TargetClear(bottom, C2D_Color32f(0.5,0,0,1));
        C2D_SceneBegin(bottom);
        
        C3D_FrameEnd(0);

        world->Step(timeStep, velocityIter, positionIter);

    }
    // clean up
    cfguExit();
    C2D_Fini();
    C2D_Fini();
    romfsExit();
    gfxExit();
    return 0;
}
