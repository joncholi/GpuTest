#include <PxPhysicsAPI.h>
#include <cudamanager/PxCudaContextManager.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>
#include <iostream>

using namespace physx;

class MyErrorCallback : public PxErrorCallback {
public:
    void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) override {
        std::cerr << "PhysX Error [" << code << "] at " << file << ":" << line << " - " << message << std::endl;
    }
};

class MyAllocator : public PxAllocatorCallback {
public:
    void* allocate(size_t size, const char*, const char*, int) override {
        return malloc(size);
    }

    void deallocate(void* ptr) override {
        free(ptr);
    }
};

int not_main() {
    MyErrorCallback errorCallback;
    MyAllocator allocator;

    PxFoundation* foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
    if (!foundation) {
        std::cerr << "Failed to create PxFoundation" << std::endl;
        return 1;
    }

    PxCudaContextManagerDesc cudaDesc;
    //cudaDesc.interopMode = PxCudaInteropMode::NO_INTEROP;

    PxCudaContextManager* cudaManager = PxCreateCudaContextManager(*foundation, cudaDesc);
    if (!cudaManager) {
        std::cout << "Failed to create CUDA context manager (nullptr returned)" << std::endl;
    }
    else if (!cudaManager->contextIsValid()) {
        std::cout << "CUDA context manager created, but context is invalid (no GPU support?)" << std::endl;
    }
    else {
        std::cout << "CUDA context initialized successfully! GPU is available for PhysX." << std::endl;
    }

    if (cudaManager) {
        cudaManager->release();
    }
    foundation->release();
    return 0;
}
