#include <Geode/Geode.hpp>
#include <Geode/modify/GameObject.hpp>

using namespace geode::prelude;

GameObject* unitObject = nullptr;
std::unordered_map<std::string, CCSpriteFrame*> quickFrameCache;

class FakeSpriteCache : public CCSpriteFrameCache {
 public:
    void moveToQuickCache() {
        quickFrameCache.clear();
        auto keys = m_pSpriteFrames->allKeys();

        for (int i = 0; i < keys->count(); ++i) {
            auto key = ((CCString*)keys->objectAtIndex(i))->getCString();

            quickFrameCache[key] = (CCSpriteFrame*)m_pSpriteFrames->objectForKey(key);
        }
    }
};

// this is absolutely wild
class $modify(MyGameObject, GameObject) {
 public:
    void deepInitialize(std::string frameName) {
        m_pActionManager->retain();
        m_pScheduler->retain();
        m_eScriptType = kScriptTypeNone;
        
        struct ComponentContainer {
            void* vtable;
            CCDictionary* a;
            CCNode* b;
        };

        auto buf = new ComponentContainer;
        
        buf->vtable = *reinterpret_cast<void**>(m_pComponentContainer);
        buf->a = nullptr;
        buf->b = this;

        m_pComponentContainer = reinterpret_cast<CCComponentContainer*>(buf);

        auto frame = quickFrameCache.at(frameName);//CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(frameName);
        auto rect = frame->getRect();
        setTexture(frame->getTexture());
        setTextureRect(rect, false, rect.size);
        setDisplayFrame(frame);

        m_textureName = std::move(frameName);
    }

    static GameObject* createWithFrame(char const* frame) {
        MyGameObject* object = static_cast<MyGameObject*>(malloc(sizeof(GameObject)));

        #if __APPLE__
        __builtin_memcpy_inline(object, unitObject, sizeof(GameObject));
        #else
        memcpy(object, unitObject, sizeof(GameObject));
        #endif

        object->deepInitialize(frame);
        object->commonSetup();
        object->autorelease();

        return object;
    }
};