//
//  AnotherScene.cpp
//  cocos2d_tests
//
//  Created by minggo on 9/23/16.
//
//

#include "AnotherScene.h"
#include "2d/CCLabel.h"
#include "base/CCDirector.h"
#include "2d/CCMenuItem.h"
#include "json/document.h"
#include "audio/include/AudioEngine.h"

#include "HelloWorldScene.h"

using namespace cocos2d;

std::vector<std::string> AnotherScene::__keys = {"key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9", "key10"};

cocos2d::Scene* AnotherScene::create()
{
    auto scene = Scene::create();
    auto layer = new AnotherScene();
    layer->autorelease();
    scene->addChild(layer);
    
    return scene;
}

typedef rapidjson::GenericDocument<rapidjson::UTF8<>, rapidjson::CrtAllocator> RapidJsonDocument;
typedef rapidjson::GenericValue<rapidjson::UTF8<>, rapidjson::CrtAllocator> RapidJsonValue;

AnotherScene::AnotherScene()
: _currentLevel(0)
, _index(0)
{
    auto origin = Director::getInstance()->getVisibleOrigin();
    
    auto label = Label::createWithSystemFont("return to test scene", "", 15);
    auto menuItem = MenuItemLabel::create(label, CC_CALLBACK_1(AnotherScene::returnSceneCallback, this));
    menuItem->setPosition(Vec2(100, 100));
    auto menu = Menu::create(menuItem, nullptr);
    menu->setPosition(Vec2(origin.x, origin.y));
    this->addChild(menu);
    
    // preload resources
    Director::getInstance()->getTextureCache()->addImage("sprite0.png");
    Director::getInstance()->getTextureCache()->addImage("sprite1.png");
    experimental::AudioEngine::preload("effec0.mp3");
    experimental::AudioEngine::preload("effec1.mp3");
    experimental::AudioEngine::preload("effec2.mp3");
    experimental::AudioEngine::preload("effec3.mp3");
    experimental::AudioEngine::preload("effec4.mp3");
    experimental::AudioEngine::preload("effec5.mp3");
    experimental::AudioEngine::preload("effec6.mp3");
    experimental::AudioEngine::preload("effec7.mp3");
    experimental::AudioEngine::preload("effec8.mp3");
    experimental::AudioEngine::preload("effec9.mp3");
    
    _parentNode = Node::create();
    this->addChild(_parentNode);
    
    _resourceLevelInfos = {
        // sprite, drawCall, action, particle, audio
        {120, 120,  0,   0,   0}, // CPU=0,GPU=0
        {300, 300,  0, 50, 1}, // CPU=1,GPU=1
        {350, 350,  0, 200, 1}, // CPU=1,GPU=2
        {500, 500,  0, 300, 2}, // CPU=2,GPU=3
        {600, 550,  0, 300, 2}, // CPU=2,GPU=4
        {1200, 650,  0, 500, 3}, // CPU=3,GPU=5
        
        {1200, 700,  0, 500, 3}, // CPU=3,GPU=6
        {1500, 700,  0,   400, 4}, // CPU=4,GPU=7
        {2000, 900,  0,   400, 4}, // CPU=4,GPU=8
        {5000, 2000,  0, 1000, 5}, // CPU=5,GPU=9
    };
    
    parseJson();
    int order = AnotherScene::__runningOrder[0] - 1;
    std::string key = __keys[order];
    int duration = __durations[_index];
    scheduleOnce(CC_CALLBACK_1(AnotherScene::scheduleCallback, this), duration, key);
    
    _emitter = ParticleSun::create();
    _emitter->setTexture(Director::getInstance()->getTextureCache()->addImage("fire.png"));
    _emitter->setTotalParticles(0);
    _emitter->setPosition(Vec2(100, 100));
    _emitter->pause();
    this->addChild(_emitter);
    
    myutils::addResource(_parentNode, _emitter, _resourceLevelInfos[order], _audioIDVecs);
}

void AnotherScene::returnSceneCallback(cocos2d::Ref* sender)
{
    auto scene = HelloWorld::scene();
    experimental::AudioEngine::stopAll();
    Director::getInstance()->replaceScene(scene);
}

void AnotherScene::scheduleCallback(float dt)
{
    int nextLevel = _currentLevel + 1;
    if (nextLevel >= 10)
    {
        _parentNode->removeAllChildren();
        _emitter->removeFromParent();
        experimental::AudioEngine::stopAll();
        
        experimental::AudioEngine::end();
        Director::getInstance()->getTextureCache()->removeUnusedTextures();
        
        return;
    }
    
    int nextOrder = __runningOrder[nextLevel] - 1;
    int currentOrder = __runningOrder[_currentLevel] - 1;
    std::string key = __keys[nextOrder];
    int duration = __durations[++_index];
    scheduleOnce(CC_CALLBACK_1(AnotherScene::scheduleCallback, this), duration, key);
    
    auto currentResourceInfo = _resourceLevelInfos[currentOrder];
    auto nextResourceInfo = _resourceLevelInfos[nextOrder];
    myutils::ResourceInfo subResourceInfo;
    subResourceInfo.spriteNumber = nextResourceInfo.spriteNumber - currentResourceInfo.spriteNumber;
    subResourceInfo.actionNumber = nextResourceInfo.actionNumber - currentResourceInfo.actionNumber;
    subResourceInfo.drawcallNumber = nextResourceInfo.drawcallNumber - currentResourceInfo.drawcallNumber;
    subResourceInfo.particleNumber = nextResourceInfo.particleNumber;
    subResourceInfo.audioNumber = nextResourceInfo.audioNumber - currentResourceInfo.audioNumber;
    myutils::addResource(_parentNode, _emitter, subResourceInfo, _audioIDVecs);
    
    _currentLevel = nextLevel;
}

void AnotherScene::parseJson()
{

    auto fileUtils = FileUtils::getInstance();
    fileUtils->addSearchPath("/sdcard", true);
    fileUtils->addSearchPath(fileUtils->getWritablePath(), true);
    
    auto fileContent = fileUtils->getStringFromFile("gameScene.json");
    
    RapidJsonDocument document;
    document.Parse(fileContent.c_str());
    
    // get duration
    const RapidJsonValue& duration = document["duration"];
    for (auto iter = duration.Begin(); iter != duration.End(); ++iter)
    {
        __durations.push_back(iter->GetInt());
    }
    
    if (document.HasMember("running_order"))
    {
        const RapidJsonValue& runningOrder = document["running_order"];
        for (auto iter = runningOrder.Begin(); iter != runningOrder.End(); ++ iter)
        {
            __runningOrder.push_back(iter->GetInt());
        }
    }
    else
    {
        __runningOrder = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    }
}
