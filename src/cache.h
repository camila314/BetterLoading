#include <Geode/Geode.hpp>
#include <Geode/modify/GameObject.hpp>

using namespace geode::prelude;

static GameObject* unitObject = nullptr;
static LabelGameObject* unitLabelObject = nullptr;
static std::unordered_map<std::string, CCSpriteFrame*> quickFrameCache;

// god weeps
static class AllocPool {
    GameObject* m_poolStart = nullptr;
    GameObject* m_pool = nullptr;
    GameObject* m_poolEnd = nullptr;
 public:
    void drain() {
        if (!m_poolStart)
            return;

        Loader::get()->queueInMainThread([=]() {
            bool anyLeft = false;
            for (auto it = m_poolStart; it != m_pool; ++it) {
                if (it->retainCount() > 1) {
                    anyLeft = true;
                }
            }

            if (anyLeft) {
                drain();
                return;
            } else {
                for (auto it = m_poolStart; it != m_pool; ++it) {
                    it->~GameObject();
                }

                free(m_poolStart);
                m_poolStart = m_pool = m_poolEnd = nullptr;
            }
        });
    }

    void alloc(size_t objects) {
        m_poolStart = m_pool = (GameObject*)calloc(objects, sizeof(GameObject));
        if (m_pool == 0)
            m_poolEnd = 0;
        else
            m_poolEnd = m_pool + objects;
    }

    GameObject* get() {
        if (m_pool == m_poolEnd)
            return nullptr;

        return m_pool++;
    }

} allocPool;

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
class MyGameObject : public GameObject {
    public:
       void deepInitialize(std::string frameName) {
           m_pActionManager->retain();
           m_pScheduler->retain();
           m_eScriptType = kScriptTypeNone;
           m_pobTexture = nullptr;
           m_pShaderProgram = nullptr;
           m_uReference = 1;

           
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

           auto frame = quickFrameCache.at(frameName);
           setDisplayFrame(frame);

           m_textureName = std::move(frameName);
       }
};

class $modify(GameObject) {
    static GameObject* createWithFrame(char const* frame) {
        if (!PlayLayer::get())
            return GameObject::createWithFrame(frame);

        auto pooledObj = reinterpret_cast<MyGameObject*>(allocPool.get());
        auto object = pooledObj;
        if (!pooledObj)
            object = static_cast<MyGameObject*>(malloc(sizeof(GameObject)));

        #if __APPLE__
        __builtin_memcpy_inline(object, unitObject, sizeof(GameObject));
        #else
        memcpy(object, unitObject, sizeof(GameObject));
        #endif

        object->deepInitialize(frame);
        object->commonSetup();

        if (!pooledObj)
            object->autorelease();

        return object;
    }
};
