#include "entities.h"

void Entity::startContact() {
}
void Entity::endContact(){
}
void Player::startContact() {
    jumpstate = JS_GROUND;
    currentJumps = 0;
}
void Player::endContact(){
    jumpstate = JS_JUMP;
}


void Treasure::render(){
    b2Vec2 pos = body->GetPosition();
    C2D_SpriteSetPos(&spr.spr, pos.x, pos.y);
    C2D_DrawSprite(&spr.spr);
    if(isopen){
        C2D_DrawSprite(&content.spr);
        C2D_SpriteMove(&content.spr, 0,1);
    }
}
void Treasure::startContact() {
    if(isopen)
        return;
    b2Vec2 pos = body->GetPosition();
    C2D_SpriteSetPos(&content.spr, pos.x, pos.y);
    spr = openspr;
    isopen= true;
}
void Treasure::endContact(){
}

Platform::Platform(){}
Platform::Platform(float w, float h, Sprite s){ 
    width = w;
    height=h;
    spritenum = ceil(width*PIXEL_TO_METER/s.width);
    for(int i=0; i<spritenum;i++) {
        memcpy(&sprites[i].spr, &s.spr, sizeof(C2D_Sprite));
        sprites[i].width = s.width;
        sprites[i].height = s.height;
    }
}
Platform::~Platform()
{
    for(int i = 0; i < spritenum; i++) {
    } 
}
void Platform::render(){
    b2Vec2 pos = body->GetPosition();
    for(int i =0;i<spritenum;i++) {
        C2D_SpriteSetPos(&sprites[i].spr, pos.x-width*PIXEL_TO_METER/2+sprites[i].width*i, pos.y);
        C2D_DrawSprite(&sprites[i].spr);
    }
    //C2D_DrawRectSolid(pos.x, pos.y, 0.5, width*PIXEL_TO_METER, height, C2D_Color32f(1,0,0,1));
}

