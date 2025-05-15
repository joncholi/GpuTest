#include <SDL3/SDL.h>
#include <PxPhysicsAPI.h>
#include <iostream>
#include <vector>
#include <random>

using namespace physx;

PxDefaultAllocator gAllocator;
PxDefaultErrorCallback gErrorCallback;
PxFoundation* gFoundation = nullptr;
PxPhysics* gPhysics = nullptr;
PxScene* gScene = nullptr;
PxCudaContextManager* gCudaContextManager = nullptr;
PxMaterial* gMaterial = nullptr;
PxDefaultCpuDispatcher* gCpuDispatcher = nullptr;
PxPvd* gPvd = nullptr;
bool gravityEnabled = true;

std::default_random_engine rng;
std::uniform_real_distribution<float> dist(-5.0f, 5.0f);

struct Box {
    PxRigidDynamic* actor;
    SDL_Color color;
};

std::vector<Box> boxes;

void initPhysics()
{
    gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    gPvd = PxCreatePvd(*gFoundation);
    gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);

    PxCudaContextManagerDesc cudaContextManagerDesc;
    gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc);

    if (gCudaContextManager && gCudaContextManager->contextIsValid()) {
        std::cout << "GPU PhysX in use!\n";
    }
    else {
        std::cout << "GPU PhysX not available, fallback to CPU.\n";
        gCudaContextManager = nullptr;
    }

    PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, gravityEnabled ? -9.81f : 0.0f, 0.0f);
    gCpuDispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = gCpuDispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
    if (gCudaContextManager)
        sceneDesc.cudaContextManager = gCudaContextManager;

    gScene = gPhysics->createScene(sceneDesc);

    // Material: friction 0.5, restitution 0.8 = bouncing!
    gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.8f);

    // floor
    PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
    gScene->addActor(*groundPlane);
}

void createBox()
{
    float posX = dist(rng);
    float posZ = dist(rng);
    PxTransform transform(PxVec3(posX, 10.0f, posZ));

    // make shape
    PxBoxGeometry boxGeom(PxVec3(0.5f, 0.5f, 0.5f));
    PxShape* shape = gPhysics->createShape(boxGeom, *gMaterial);
    shape->setContactOffset(0.02f); // default is 0.02, you can increase or decrease it
    shape->setRestOffset(0.001f);   // can't be bigger than contactOffset

    PxRigidDynamic* box = gPhysics->createRigidDynamic(transform);
    box->attachShape(*shape);

    // calculate mass properties
    PxRigidBodyExt::updateMassAndInertia(*box, 1.0f);

    // release temporary shape (it was copied for actor)
    shape->release();

    gScene->addActor(*box);

    SDL_Color color = { (Uint8)(rand() % 256), (Uint8)(rand() % 256), (Uint8)(rand() % 256), 255 };
    boxes.push_back({ box, color });
}

void stepPhysics(float deltaTime)
{
    gScene->simulate(deltaTime);
    gScene->fetchResults(true);
}

void cleanupPhysics()
{
    for (auto& box : boxes) {
        box.actor->release();
    }
    boxes.clear();
    gScene->release();
    gCpuDispatcher->release();
    if (gCudaContextManager) gCudaContextManager->release();
    gPhysics->release();
    gPvd->release();
    gFoundation->release();
}

void render(SDL_Renderer* renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    for (auto& box : boxes) {
        PxTransform transform = box.actor->getGlobalPose();
        float x = transform.p.x * 20.0f + 400.0f;
        float y = 300.0f - transform.p.y * 20.0f;

        SDL_FRect rect = { x - 10, y - 10, 20, 20 };

        SDL_SetRenderDrawColor(renderer, box.color.r, box.color.g, box.color.b, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("PhysX + SDL3 Test", 800, 600, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    initPhysics();
    createBox();

    bool running = true;
    uint64_t lastTime = SDL_GetTicks();
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                if (event.key.key == SDLK_SPACE)
                    createBox();
                else if (event.key.key == SDLK_G)
                {
                    gravityEnabled = !gravityEnabled;
                    gScene->setGravity(gravityEnabled ? PxVec3(0.0f, -9.81f, 0.0f) : PxVec3(0.0f, 0.0f, 0.0f));
                    std::cout << "Gravity: " << (gravityEnabled ? "ON" : "OFF") << std::endl;
                }
            }
        }

        uint64_t currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        stepPhysics(deltaTime);
        render(renderer);

        SDL_Delay(16);
    }

    cleanupPhysics();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
