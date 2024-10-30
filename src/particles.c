#include "particles.h"

#include "rlgl.h"

// GetRandomFloat returns a random float between 0.0 and 1.0.
float GetRandomFloat(float min, float max) {
    float range = max - min;
    float n = (float)GetRandomValue(0, RAND_MAX) / (float)RAND_MAX;
    return n * range + min;
}

// LinearFade fades from Color c1 to Color c2. Fraction is a value between 0 and 1.
// The interpolation is linear.
Color LinearFade(Color c1, Color c2, float fraction) {
    unsigned char newr = (unsigned char)((float)((int)c2.r - (int)c1.r) * fraction + (float)c1.r);
    unsigned char newg = (unsigned char)((float)((int)c2.g - (int)c1.g) * fraction + (float)c1.g);
    unsigned char newb = (unsigned char)((float)((int)c2.b - (int)c1.b) * fraction + (float)c1.b);
    unsigned char newa = (unsigned char)((float)((int)c2.a - (int)c1.a) * fraction + (float)c1.a);

    Color c = {
        .r = newr,
        .g = newg,
        .b = newb,
        .a = newa
    };

    return c;
}

// Particle_DeactivatorAge is the default deactivator function that
// disables particles only if their age exceeds their time to live.
bool Particle_DeactivatorAge(Particle* p) {
    return p->age > p->ttl;
}

float Particle_Decellerator_Default(Particle* p) {
    (void)p;
    return 0.0f;
}
float Particle_Decelerator_Linear(Particle* p) {
    return p->age / p->haltTime;
}
float Particle_Decelerator_Cubic(Particle* p) {
    return (p->age * p->age) / p->haltTime;
}
float Particle_Decelerator_Sudden(Particle* p) {
    return p->age < p->haltTime ? 0.0f : 1.0f;
}

// Emitter_New creates a new Emitter object.
Emitter* Emitter_New(EmitterConfig cfg) {
    Emitter* e = (Emitter*)calloc(1, sizeof(Emitter));
    if (e == NULL) {
        return NULL;
    }
    e->config = cfg;
    e->offset.x = e->config.texture.width / 2.0f;
    e->offset.y = e->config.texture.height / 2.0f;
    //e->particles = (Particle*)calloc(e->config.capacity, sizeof(Particle));

    e->particlePositions = (Vector2*)calloc(e->config.capacity, sizeof(Vector2));
    e->particleVelocities = (Vector2*)calloc(e->config.capacity, sizeof(Vector2));
    e->particlesAccellerationExt = (Vector2*)calloc(e->config.capacity, sizeof(Vector2));
    e->particleSizes = (float*)calloc(e->config.capacity, sizeof(float));
    e->particleAges = (float*)calloc(e->config.capacity, sizeof(float));
    e->particleTTL = (float*)calloc(e->config.capacity, sizeof(float));
    e->particleHaltTimes = (float*)calloc(e->config.capacity, sizeof(float));

    e->activeParticles = 0;

    for (size_t i = 0; i < e->config.capacity; i++) {
        e->particlePositions[i] = (Vector2){ 0.0f, 0.0f };
        e->particleVelocities[i] = (Vector2){ 0.0f, 0.0f };
        e->particlesAccellerationExt[i] = (Vector2){ 0.0f, 0.0f };
        e->particleSizes[i] = 1.0f;
        e->particleAges[i] = 0.0f;
        e->particleTTL[i] = 0.0f;
        e->particleHaltTimes[i] = 0.0f;
    }

    //if(e->particles == NULL) {
    //    free(e);
    //    return NULL;
    //}
    e->mustEmit = 0;
    // Normalize direction for future uses.
    e->config.direction = Vector2Normalize(e->config.direction);

    for (size_t i = 0; i < e->config.capacity; i++) {
        //e->particles[i] = {
        //    .particle_Deactivator = e->config.particle_Deactivator == NULL ? Particle_DeactivatorAge : e->config.particle_Deactivator,
        //    .particle_Decellerator = e->config.particle_Decellerator == NULL ? Particle_Decellerator_Default : e->config.particle_Decellerator,
        //};
    }

    return e;
}

void Emitter_Tweak_Direction(Emitter* e, Vector2 dirNew) {
    e->config.direction = dirNew;
}
void Emitter_Tweak_velocity(Emitter* e, float velMin, float velMax) {
    e->config.velocity = (FloatRange){ velMin, velMax };
}
void Emitter_Tweak_External_Acceleration(Emitter* e, Vector2 accNew) {
    e->config.externalAcceleration = accNew;
}
void Emitter_Tweak_Burst(Emitter* e, int burstMin, int burstMax) {
    e->config.burst = (IntRange){ burstMin, burstMax };
}


void Emitter_Set_Origin(Emitter* e, Vector2 origin) {
    e->config.origin = origin;
}

// Emitter_Start activates Particle emission.
void Emitter_Start(Emitter* e) {
    e->isEmitting = true;
}

// Emitter_Start deactivates Particle emission.
void Emitter_Stop(Emitter* e) {
    e->isEmitting = false;
}

// Emitter_Free frees all allocated resources.
void Emitter_Free(Emitter* e) {
    free(e->particlePositions);
    free(e->particleVelocities);
    free(e->particlesAccellerationExt);
    free(e->particleSizes);
    free(e->particleAges);
    free(e->particleTTL);
    free(e->particleHaltTimes);
    free(e);
}

// Emitter_Burst emits a specified amount of particles at once,
// ignoring the state of e->isEmitting. Use this for singular events
// instead of continuous output.
void Emitter_Burst(Emitter* e) {
    int amount = GetRandomValue(e->config.burst.min, e->config.burst.max);

    // TODO Config is too big for this
    for (int i = e->activeParticles; i < e->activeParticles + amount; i++) {
        // Get a random angle to find an random velocity.
        float randa = GetRandomFloat(e->config.directionAngle.min, e->config.directionAngle.max);

        // Rotate base direction with the given angle.
        Vector2 res = Vector2Rotate(e->config.direction, randa * DEG2RAD);

        // Get a random value for velocity range (direction is normalized).
        float randv = GetRandomFloat(e->config.velocity.min, e->config.velocity.max);

        // Get a random angle to rotate the velocity vector.
        randa = GetRandomFloat(e->config.velocityAngle.min, e->config.velocityAngle.max);

        float rando = GetRandomFloat(e->config.offset.min, e->config.offset.max);

        e->particlePositions[i] = (Vector2){ e->config.origin.x + rando, e->config.origin.y + rando };
        e->particleVelocities[i] = Vector2Rotate((Vector2){ res.x * randv, res.y * randv }, randa * DEG2RAD);
        e->particlesAccellerationExt[i] = (Vector2){ e->config.externalAcceleration.x, e->config.externalAcceleration.y };
        e->particleSizes[i] = GetRandomFloat(e->config.size.min, e->config.size.max);
        e->particleAges[i] = 0.0f;
        e->particleTTL[i] = GetRandomFloat(e->config.age.min, e->config.age.max);
        e->particleHaltTimes[i] = GetRandomFloat(e->config.haltTime.min, e->config.haltTime.max);
    }

    e->activeParticles += amount;
}

void Emitter_Update(Emitter* e, float dt) {
    size_t emitNow = 0;

    if (e->isEmitting) {
        e->mustEmit += dt * (float)e->config.emissionRate;
        emitNow = (size_t)e->mustEmit; // floor
    }

    if (emitNow > 0) {
        Emitter_Tweak_Burst(e, emitNow, emitNow);
        Emitter_Burst(e);

        e->mustEmit -= emitNow;
    }

    for (int i = 0; i < e->activeParticles; i++) {
        e->particleAges[i] += dt;
    }

    for (int i = 0; i < e->activeParticles; i++) {
        if (e->particleAges[i] > e->particleTTL[i]) {
            e->particlePositions[i] = e->particlePositions[e->activeParticles - 1];
            e->particleVelocities[i] = e->particleVelocities[e->activeParticles - 1];
            e->particlesAccellerationExt[i] = e->particlesAccellerationExt[e->activeParticles - 1];
            e->particleSizes[i] = e->particleSizes[e->activeParticles - 1];
            e->particleAges[i] = e->particleAges[e->activeParticles - 1];
            e->particleTTL[i] = e->particleTTL[e->activeParticles - 1];
            e->particleHaltTimes[i] = e->particleHaltTimes[e->activeParticles - 1];

            e->activeParticles--;
        }
    }

    // TODO, ideally, this moves particles between two arrays (moving and non-moving), or to a different section in the same array
    for (int i = 0; i < e->activeParticles; i++) {
        if (e->particleAges[i] > e->particleHaltTimes[i]) {
            e->particleVelocities[i] = (Vector2){ 0.0f, 0.0f };
            e->particlesAccellerationExt[i] = (Vector2){ 0.0f, 0.0f };
        }
    }

    for (int i = 0; i < e->activeParticles; i++) {
        e->particleVelocities[i] = Vector2Add(e->particleVelocities[i], Vector2Multiply(e->particlesAccellerationExt[i], (Vector2) { dt, dt }));
    }

    for (int i = 0; i < e->activeParticles; i++) {
        e->particlePositions[i] = Vector2Add(e->particlePositions[i], Vector2Multiply(e->particleVelocities[i], (Vector2) { dt, dt }));
    }
}

// Emitter_Draw draws all active particles.
void Emitter_Draw(Emitter* e) {
    BeginBlendMode(e->config.blendMode);

    const Color emitterColor = e->config.Color;

    rlSetTexture(1);

    for (int i = 0; i < e->activeParticles; i++) {
        DrawRectangleV((Vector2){ e->particlePositions[i].x, e->particlePositions[i].y }, (Vector2){ e->particleSizes[i], e->particleSizes[i] }, emitterColor);
    }

    rlSetTexture(0);

   EndBlendMode();
}


// Particlesystem_New creates a new particle system
// with the given amount of emitters.
ParticleSystem* ParticleSystem_New(void) {
    ParticleSystem* ps = (ParticleSystem*)calloc(1, sizeof(ParticleSystem));
    if (ps == NULL) {
        return NULL;
    }
    ps->active = false;
    ps->length = 0;
    ps->capacity = 2;
    ps->origin = (Vector2){ .x = 0, .y = 0 };
    ps->emitters = (Emitter**)calloc(ps->capacity, sizeof(Emitter*));
    if (ps->emitters == NULL) {
        free(ps);
        return NULL;
    }
    return ps;
}

// ParticleSystem_Register registers an emitter to the system.
// The emitter will be controlled by all particle system functions.
// Returns true on success and false otherwise.
bool ParticleSystem_Register(ParticleSystem* ps, Emitter* emitter) {
    // If there is no space for another emitter we have to realloc.
    if (ps->length >= ps->capacity) {
        // Double capacity.
        Emitter** newEmitters = (Emitter**)realloc(ps->emitters, 2 * ps->capacity * sizeof(Emitter*));
        if (newEmitters == NULL) {
            return false;
        }
        ps->emitters = newEmitters;
        ps->capacity *= 2;
    }

    // Now the new Emitter can be registered.
    ps->emitters[ps->length] = emitter;
    ps->length++;

    return true;
}

// ParticleSystem_Deregister deregisters an Emitter by its pointer.
// Returns true on success and false otherwise.
bool ParticleSystem_Deregister(ParticleSystem* ps, Emitter* emitter) {
    for (size_t i = 0; i < ps->length; i++) {
        if (ps->emitters[i] == emitter) {
            // Remove this emitter by replacing its pointer with the
            // last pointer, if it is not the only Emitter.
            if (i != ps->length - 1) {
                ps->emitters[i] = ps->emitters[ps->length - 1];
            }
            // Then NULL the last emitter. It is either a duplicate or
            // the removed one.
            ps->length--;
            ps->emitters[ps->length] = NULL;

            return true;
        }
    }
    // Emitter not found.
    return false;
}

// ParticleSystem_SetOrigin sets the origin for all registered Emitters.
void ParticleSystem_SetOrigin(ParticleSystem* ps, Vector2 origin) {
    ps->origin = origin;
    for (size_t i = 0; i < ps->length; i++) {
        ps->emitters[i]->config.origin = origin;
    }
}

// ParticleSystem_Start runs Emitter_Start on all registered Emitters.
void ParticleSystem_Start(ParticleSystem* ps) {
    for (size_t i = 0; i < ps->length; i++) {
        Emitter_Start(ps->emitters[i]);
    }
}

// ParticleSystem_Stop runs Emitter_Stop on all registered Emitters.
void ParticleSystem_Stop(ParticleSystem* ps) {
    for (size_t i = 0; i < ps->length; i++) {
        Emitter_Stop(ps->emitters[i]);
    }
}

// ParticleSystem_Burst runs Emitter_Burst on all registered Emitters.
void ParticleSystem_Burst(ParticleSystem* ps) {
    for (size_t i = 0; i < ps->length; i++) {
        Emitter_Burst(ps->emitters[i]);
    }
}

// ParticleSystem_Draw runs Emitter_Draw on all registered Emitters.
void ParticleSystem_Draw(ParticleSystem* ps) {
    for (size_t i = 0; i < ps->length; i++) {
        Emitter_Draw(ps->emitters[i]);
    }
}

// ParticleSystem_Update runs Emitter_Update on all registered Emitters.
void ParticleSystem_Update(ParticleSystem* ps, float dt) {
    for (size_t i = 0; i < ps->length; i++) {
        Emitter_Update(ps->emitters[i], dt);
    }
}

// ParticleSystem_Free only frees its own resources.
// The emitters referenced here must be freed on their own.
void ParticleSystem_Free(ParticleSystem* p) {
    free(p->emitters);
    free(p);
}

void ParticleSystem_CleanAndFree(ParticleSystem* p) {
    for (size_t i = 0; i < p->length; i++) {
        Emitter_Free(p->emitters[i]);
        p->emitters[i] = NULL;
    }
    p->length = 0;

    ParticleSystem_Free(p);
}

int ParticleSystem_GetAllActiveParticleCount(ParticleSystem* ps) {
    int count = 0;
    for (size_t i = 0; i < ps->length; i++) {
        count += ps->emitters[i]->activeParticles;
    }

    return count;
}