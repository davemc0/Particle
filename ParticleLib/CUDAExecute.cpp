// This file contains the wrapper code to call generated action list kernels.
// It also contains the device-side memory management.
// ALL CUDA runtime calls must be in this file so that it's easy to disable CUDA support in the API.

#include "CUDAExecute.h"
#include "PInternalState.h"
#include "ParticleGroup.h"

namespace PAPI
{

// Update the host's particle list from the CUDA array if needed.
void ParticleGroup::UpdateHostFromDevice()
{
    PASSERT(DeviceParticles != NULL || !update_host_from_device, "Can't need to update host from device if DeviceParticles is NULL.");

    if(DeviceParticles == NULL || !update_host_from_device) {
        update_host_from_device = false;
        return;
    }

    // Copy the particles from the CUDA array straight into the vector<>.
    // XXX

    update_host_from_device = false;
}

void ParticleGroup::UpdateDeviceFromHost()
{
    if(!update_device_from_host)
        return;

    if(DeviceParticles == NULL) {
        // Allocate the CUDA array
        // XXX
    }

    // Copy the particles from the vector<> to the CUDA array
    // XXX

    update_device_from_host = false;
}

void ParticleGroup::FreeDeviceMemory()
{
    if(DeviceParticles) {
        // XXX CUDA Free
    }

    DeviceParticles = NULL;
    update_device_from_host = false;
    update_host_from_device = false;
}

};
